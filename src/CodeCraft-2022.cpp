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

// 判断给定节点哪些能放下所需流量
vector<int> final_nodes(vector<int> all, int t) {
    vector<int> final;
    for (int i = 0; i < all.size(); ++i) {
        if (g_nodes[all[i]].remain >= (t + 1)) {
            final.emplace_back(all[i]);
        }
    }
    return final;
}

// 均分
void baseline() {
    srand((int)time(0));
    // 对于每个时间点
    for (int i = 0; i < g_demand.size(); ++i) {
        // 对于每个用户
        for (int j = 0; j < g_demand[i].size(); ++j) {
            // 均分用户流量到所有服务器
            int user_demand = g_demand[i][j];
            vector<int> temp(g_users[j].available);
            int num_available = g_users[j].available.size();
            int singe = user_demand / num_available;
            vector<int> final_available;
            while (true) {
                // 每个 node 的流量
                singe = user_demand / num_available;
                final_available = final_nodes(temp, singe);
                num_available = final_available.size();
                if (temp.size() == final_available.size()) {
                    break;
                }
                temp = final_available;
            }
            
            // 小数部分量随机分配
            int random_index = random(0, num_available - 1);

            // 更新 node 的剩余量
            vector<Node> used_nodes;
            for (int k = 0; k < num_available; ++k) {
                int node_index = final_available[k];
                if (k != random_index) {
                    g_nodes[node_index].remain -= singe;
                    g_nodes[node_index].now_used = singe;
                }
                else {
                    int left = user_demand - singe * (num_available - 1);
                    g_nodes[node_index].remain -= left;
                    g_nodes[node_index].now_used = left;
                }
                used_nodes.emplace_back(g_nodes[node_index]);
            }
            // 输出
            logger_line(g_users[j], used_nodes);
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

    return 0;
}