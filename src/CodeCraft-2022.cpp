#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "utils.h"

// [a,b]的随机整数
#define random(a,b) (rand() % ( b - a + 1) + a)

using namespace std;


class Node;
class User;
class Time;

string CONFIG_PATH = "./data/config.ini";
string DATA_PATH = "./data/";
string SOLUTION_PATH = "./output/solution.txt";


int QOS_MAX;
vector<Node> g_nodes;
vector<User> g_users;
vector<Time> g_times;
vector<vector<int>> g_qos;
vector<vector<int>> g_demand;

// 初始化 g_qos g_users g_nodes
void read_qos() {
    std::ifstream data;
    data.open(DATA_PATH + "qos.csv");
    std::string tmp_line;
    // g_users
    getline(data, tmp_line);
    stringstream ss(tmp_line);
    string str;
    int is_first = true;
    while (getline(ss, str, ',')) {
        if (is_first) {
            is_first = false;
        }
        else {
            int pos = g_users.size();
            g_users.emplace_back(User(str, pos));
        }
    }
    //    for (int i = 0; i < g_users.size(); ++i) {
    //        cout << g_users[i].name << " " << g_users[i].index << endl;
    //    }
    // g_qos
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray;
        int cnt = -1;
        while (getline(ss, str, ',')) {
            if (cnt == -1) {
                g_nodes.emplace_back(Node(str, g_nodes.size()));
            }
            else {
                int current_qos = atoi(str.c_str());
                lineArray.push_back(current_qos);
                if (current_qos < QOS_MAX) {
                    g_users[cnt].available.emplace_back(g_nodes.size() - 1);
                }
            }
            cnt++;
        }
        g_qos.emplace_back(lineArray);
    }
    data.close();
    data.clear();
}

// 初始化 g_times
void read_demand() {
    std::ifstream data;
    data.open(DATA_PATH + "demand.csv");
    std::string tmp_line;
    getline(data, tmp_line);
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray;
        int is_first = true;
        while (getline(ss, str, ',')) {
            if (is_first) {
                g_times.emplace_back(Time(str, g_times.size()));
                is_first = false;
            }
            else {
                lineArray.push_back(atoi(str.c_str()));
            }
        }
        g_demand.emplace_back(lineArray);
    }
    data.close();
    data.clear();
//        for (int i = 0; i < g_times.size(); ++i) {
//            cout << g_times[i].name << " " << g_times[i].index << endl;
//        }

}

// 初始化 g_nodes
void read_bandwidth() {
    std::ifstream data;
    data.open(DATA_PATH + "site_bandwidth.csv");
    std::string tmp_line;
    getline(data, tmp_line);
    int cnt = 0;
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        while (getline(ss, str, ',')) {
            if (is_first) {
                is_first = false;
            }
            else {
                int idx = cnt++;
                g_nodes[idx].bandwidth = atoi(str.c_str());
                g_nodes[idx].remain = g_nodes[idx].bandwidth;
            }
        }
    }
    data.close();
    data.clear();
//    for (int i = 0; i < g_nodes.size(); ++i) {
//        cout << g_nodes[i].name << " " << g_nodes[i].index << " " << g_nodes[i].bandwidth <<endl;
//    }
}

// 读取 qos max
void read_conf(){
    std::ifstream config;
    config.open(CONFIG_PATH);
    std::string tmp_line;
    getline(config,tmp_line);
    while(getline(config,tmp_line)){
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        while (getline(ss, str, '=')) {
            if (is_first) {
                is_first = false;
            }
            else {
                QOS_MAX = atoi(str.c_str());
                break;
            }
        }
    }
}

// 重置每个服务器的带宽
void reset_bandwidth() {
    for (auto &i : g_nodes) {
        i.remain = i.bandwidth;
    }
}

// 给定 nodes 的 remain 的比例
vector<int> ratio_nodes(vector<int> all, int demand) {
    double sum = 0;
    for (int i = 0; i < all.size(); ++i) {
        sum += g_nodes[all[i]].remain;
    }
    vector<int> ratio;
    ratio.reserve(all.size());
    for (int j = 0; j < all.size(); ++j) {
        ratio.emplace_back((double)g_nodes[all[j]].remain / sum * (double)demand);
    }
    return ratio;
}

// baseline
void baseline() {
    srand((int)time(0));
    // 对于每个时间点
    for (int i = 0; i < g_demand.size(); ++i) {
        // 对于每个用户
        for (int j = 0; j < g_demand[i].size(); ++j) {
            // 均分用户流量到所有服务器
            int user_demand = g_demand[i][j];
            int a_little_left = user_demand;
            if (user_demand == 0) {
                vector<Node> nodes;
                if (i == 0 && j == 0) {
                    logger_line1(g_users[j], nodes);
                }
                else {
                    logger_line(g_users[j], nodes);
                }
                continue;
            }
            vector<int> temp(g_users[j].available);
            // 已分配，将会被输出的 node
            vector<Node> used_nodes;
            while (true) {
                // 计算满足 qos 的服务器的 remain 的比例
                vector<int> ratio = ratio_nodes(temp, user_demand);
                // remain 不够的下标
                vector<int> unsatisfied_idx;
                // remain 够的下标
                vector<int> satisfied_idx;
                for (int k = 0; k < ratio.size(); ++k) {
                    Node& now_node = g_nodes[temp[k]];
                    // 满足
                    if (now_node.remain >= ratio[k]) {
                        satisfied_idx.emplace_back(now_node.index);
                    }
                    // 不满足
                    else {
                        // 将该服务器所有剩余带宽分配出去
                        user_demand -= now_node.remain;
                        now_node.now_used = now_node.remain;
                        a_little_left -= now_node.remain;
                        now_node.remain = 0;
                        unsatisfied_idx.emplace_back(now_node.index);
                        used_nodes.emplace_back(Node(now_node));
                    }
                }
                // 如果都满足比例分配则跳出循环
                if (unsatisfied_idx.empty()) {
                    int num_available = ratio.size();
                    // 按比例分
                    for (int k = 0; k < num_available; ++k) {
                        g_nodes[satisfied_idx[k]].now_used = ratio[k];
                        g_nodes[satisfied_idx[k]].remain -= ratio[k];
                        a_little_left -= ratio[k];
                        used_nodes.emplace_back(g_nodes[satisfied_idx[k]]);
                    }
                    // 分剩余
                    for (int l = 0; l < used_nodes.size(); ++l) {
                        if (used_nodes[l].remain >= a_little_left) {
                            g_nodes[used_nodes[l].index].remain -= a_little_left;
                            used_nodes[l].now_used += a_little_left;
                            break;
                        }
                        else {
                            used_nodes[l].now_used += g_nodes[used_nodes[l].index].remain;
                            g_nodes[used_nodes[l].index].remain = 0;
                        }
                    }
                    break;
                }
                temp = satisfied_idx;
            }
            // 输出
            if (i == 0 && j == 0) {
                logger_line1(g_users[j], used_nodes);
            }
            else {
                logger_line(g_users[j], used_nodes);
            }
        }
        // 重置每个服务器的带宽
        reset_bandwidth();
    }
}

//vector<Node> g_nodes;
//vector<User> g_users;
//vector<Time> g_times;
//vector<vector<int>> g_qos;
//vector<vector<int>> g_demand;


int main() {

    freopen(SOLUTION_PATH.c_str(), "w", stdout);

    read_conf();
//    cout << QOS_MAX << endl;

    read_qos();
    read_demand();
    read_bandwidth();

//    cout << "read g_demand.csv" << endl;
//    logger_mat2d(g_demand);

//    cout << "read g_qos.csv" << endl;
//    logger_mat2d(g_qos);

//    cout << "g_nodes" << endl;
//    for (int i = 0; i < g_nodes.size(); ++i) {
//        cout << g_nodes[i].name << " " << g_nodes[i].index << " " << g_nodes[i].bandwidth << endl;
//    }
//
//    cout << "g_users" << endl;
//    for (int i = 0; i < g_users.size(); ++i) {
//        cout << g_users[i].name << " " << g_users[i].index << endl;
//    }
//
//    cout << "g_times" << endl;
//    for (int i = 0; i < g_times.size(); ++i) {
//        cout << g_times[i].name << " " << g_times[i].index << endl;
//    }
//
    baseline();
    fclose(stdout);
    return 0;
}