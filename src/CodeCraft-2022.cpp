#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "utils.h"
#include <cmath>
#include <set>
#include <sys/timeb.h>
#include <map>
#include <thread>
//#include <thread>
// [a,b]的随机整数
#define random(a, b) (rand() % ( b - a + 1) + a)

using namespace std;


class Node;

class User;

class Common;

class Time;

string CONFIG_PATH = "/data/config.ini";
string DATA_PATH = "/data/";
string SOLUTION_PATH = "/output/solution.txt";


int QOS_MAX;
vector<Node> g_nodes;
vector<User> g_users;
vector<Time> g_times;
vector<vector<int>> g_qos;
vector<vector<int>> g_demand;
//vector<Node> temp_g_nodes;
//vector<User> temp_g_users;
//vector<vector<int> > temp_g_demand;
map<string, int> name2idx_user;
map<string, int> name2idx_node;
vector<vector<vector<int> > > g_output;
vector<vector<Common> > g_common_users_mat;

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
        } else {
            int pos = g_users.size();
            g_users.emplace_back(User(str, pos));
            name2idx_user[str] = pos;
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
                name2idx_node[str] = g_nodes.size() - 1;
            } else {
                int current_qos = atoi(str.c_str());
                lineArray.push_back(current_qos);
                if (current_qos < QOS_MAX) {
                    g_users[cnt].available.emplace_back(g_nodes.size() - 1);
                    g_nodes[g_nodes.size() - 1].available.emplace_back(cnt);
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
    stringstream ss(tmp_line);
    vector<string> names_non_order;
    string str;
    int is_first = true;
    while (getline(ss, str, ',')) {
        if (is_first) {
            is_first = false;
        } else {
            names_non_order.emplace_back(str);
        }
    }
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray(g_users.size(), 0);
        int is_first = true;
        int cnt = 0;
        while (getline(ss, str, ',')) {
            if (is_first) {
                g_times.emplace_back(Time(str, g_times.size()));
                is_first = false;
            } else {
                lineArray[name2idx_user[names_non_order[cnt++]]] = atoi(str.c_str());
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
//    stringstream ss(tmp_line);
//    vector<string> names_non_order;
//    string str;
//    int is_first = true;
//    while (getline(ss, str, ',')) {
//        if (is_first) {
//            is_first = false;
//        } else {
//            names_non_order.emplace_back(str);
//        }
//    }
    int cnt = 0;
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        int real_node_idx = 0;
        while (getline(ss, str, ',')) {
            if (is_first) {
                is_first = false;
                real_node_idx = name2idx_node[str];
            } else {
                int idx = cnt++;
                g_nodes[real_node_idx].bandwidth = atoi(str.c_str());
                g_nodes[real_node_idx].remain = g_nodes[real_node_idx].bandwidth;
                g_nodes[real_node_idx].all_remain = vector<int>(g_demand.size(), g_nodes[real_node_idx].bandwidth);
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
void read_conf() {
    std::ifstream config;
    config.open(CONFIG_PATH);
    std::string tmp_line;
    getline(config, tmp_line);
    while (getline(config, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        int is_first = true;
        while (getline(ss, str, '=')) {
            if (is_first) {
                is_first = false;
            } else {
                QOS_MAX = atoi(str.c_str());
                break;
            }
        }
    }
}


void prt(const vector<vector<vector<int> > > &output) {
    for (int i1 = 0; i1 < output.size(); ++i1) {
        for (int i = 0; i < output[i1].size(); ++i) {
            vector<Node> v;
            for (int j = 0; j < output[i1][i].size(); ++j) {
                if (g_qos[j][i] < QOS_MAX && output[i1][i][j] > 0) {
                    Node n = Node(g_nodes[j].name, j);
                    n.now_used = output[i1][i][j];
                    v.emplace_back(n);
                }
            }
            logger_standard(User(g_users[i].name, i), v);
        }
    }
}

vector<vector<vector<int> > >
maximize_95plus(vector<int> order, vector<Node> g_nodes, vector<User> g_user, vector<vector<int> > g_demand) {
    // 输出
    vector<vector<vector<int> > > output(g_demand.size(), vector<vector<int> >(g_users.size(),
                                                                               vector<int>(g_nodes.size(), 0)));
    // 前 5% 的时间节点个数

    int num_95plus = g_demand.size() - g_demand.size() * 0.95;

    // 每个边缘节点 计算每个时刻 所有用户的总需求流量 [nodes size, timeline size] pair: time_index, asked
    vector<vector<pair<int, int>>> node_time_asked(g_nodes.size(),
                                                   vector<pair<int, int>>(g_demand.size(), make_pair(0, 0)));
    // 节点选取顺序
//    vector<pair<int, int> > nodes_series_1(g_nodes.size(), make_pair(0, 0));
//    vector<pair<int, int> > nodes_series_2(g_nodes.size(), make_pair(0, 0));
    for (int i = 0; i < g_nodes.size(); ++i) {
        int sum_all_time = 0;
        for (int j = 0; j < g_demand.size(); ++j) {
            int now_time_idx = j;
            int sum = 0;
            for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                int now_user_idx = g_nodes[i].available[k];
                sum += g_demand[now_time_idx][now_user_idx];
            }
            sum_all_time += sum;
            node_time_asked[i][now_time_idx] = make_pair(now_time_idx, sum);
        }
//        nodes_series_1[i].first = i;
//        nodes_series_1[i].second = sum_all_time;
    }
    // 节点序列选取 step 1 所有时间点的流量和 由大到小
//    InsertionSort_form_big_to_small(nodes_series_1, nodes_series_1.size());
//    // 节点序列选取 step 2 每个节点的 available 由小到大排序
//    for (int i = 0; i < g_nodes.size(); ++i) {
//        nodes_series_2[i].first = nodes_series_1[i].first;
////        cout << nodes_series_1[i].first << " " << nodes_series_1[i].second << " " << g_nodes[nodes_series_1[i].first].available.size() << endl;
//        nodes_series_2[i].second = g_nodes[nodes_series_1[i].first].available.size();
//    }
//    InsertionSort(nodes_series_2, nodes_series_2.size());
//    for (int i = 0; i < g_nodes.size(); ++i) {
//        cout << nodes_series_2[i].first << " " << nodes_series_2[i].second << endl;
//    }


    // 每个边缘节点 每个时刻最大安排流量 [nodes size, timeline size]
    vector<vector<int> > node_time_remain(g_nodes.size(), vector<int>(g_demand.size(), 0));
    for (int i = 0; i < g_nodes.size(); ++i) {
        for (int j = 0; j < g_demand.size(); ++j) {
            node_time_remain[i][j] = g_nodes[i].bandwidth;
        }
    }

    // 循环直到需求不能再被分配
    while (true) {

        // 是否跳出循环
        bool can_break = true;

        // 对于每个边缘节点 前5%（取下整），对应时刻
        for (int i = 0; i < g_nodes.size(); ++i) {
//            int now_node_idx = nodes_series_2[i].first;
            int now_node_idx = order[i];
            // 当前边缘节点
            Node &now_node = g_nodes[now_node_idx];
            if (now_node.available.size() == 0) {
                continue;
            }
            for (int j = 0; j < g_demand.size(); ++j) {
                int now_time_idx = j;
                int sum = 0;
                for (int k = 0; k < g_nodes[now_node_idx].available.size(); ++k) {
                    int now_user_idx = g_nodes[now_node_idx].available[k];
                    sum += g_demand[now_time_idx][now_user_idx];
                }
                node_time_asked[now_node_idx][now_time_idx] = make_pair(now_time_idx, sum);
            }
//            int now_node_idx = i;
            // 每个节点的总需求按时间线从大到小排序
            vector<vector<pair<int, int> > > sorted_node_time_asked = node_time_asked;
            InsertionSort_form_big_to_small(sorted_node_time_asked[now_node_idx],
                                            sorted_node_time_asked[now_node_idx].size());

            // 取到当前节点 前 5% 的所有时间点
            vector<pair<int, int> > now_time_line_sorted(sorted_node_time_asked[now_node.index].begin(),
                                                         sorted_node_time_asked[now_node.index].begin() + num_95plus);
            // 对于这些时间点 如果总需求流量 ≤ 当前时刻最大安排流量，直接进行安排；
            for (int j = 0; j < now_time_line_sorted.size(); ++j) {
                // 当前时间线的下标
                int real_time_idx = now_time_line_sorted[j].first;
                // 当前时间线该服务器的可供应量
                int can_supply = node_time_remain[now_node.index][real_time_idx];
                // 当前时间线的总需求量
                int now_demand = now_time_line_sorted[j].second;
                if (now_demand == 0) {
                    break;
                }
                // 总需求 <= 可供应：直接放置
                if (can_supply >= now_demand) {
                    // 该服务器在该时间点的可供应量减少
                    node_time_remain[now_node.index][real_time_idx] -= now_demand;
                    now_node.all_remain[real_time_idx] -= now_demand;
                    // 需求量减少
                    node_time_asked[now_node.index][real_time_idx].second = 0;
                    // 更新该时间线上该用户的其他所有 avaliable 节点

                    sorted_node_time_asked[now_node.index][j].second = 0;
                    // 已分配，更新输出和剩余需求
                    for (int k = 0; k < now_node.available.size(); ++k) {
                        int now_usr_idx = now_node.available[k];
                        // 更新输出
                        output[real_time_idx][now_usr_idx][now_node.index] += g_demand[real_time_idx][now_usr_idx];
                        // 更新 demand
                        g_demand[real_time_idx][now_usr_idx] = 0;
                    }
                    can_break = false;
                }
                    // 总需求 > 可供应：“单位客户流量”进行有大到小排序，依次选择客户直到达到节点在当前时刻的上限
                else {
                    // 计算 单位客户流量
                    vector<pair<int, int> > unit_user_flow;
//                    for (int k = 0; k < now_node.available.size(); ++k) {
//                        int now_user_idx = g_users[now_node.available[k]].index;
////                        float unit_flow =
////                                g_demand[real_time_idx][now_user_idx] / g_users[now_user_idx].available.size();
//                        float unit_flow =
//                                g_demand[real_time_idx][now_user_idx];
//                        unit_user_flow.emplace_back(make_pair(now_user_idx, unit_flow));
//                    }
//                    // 对其由大到小排序
//                    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);
                    // 计算 客户在当前时刻候选边缘节点的个数由小到大排序
                    for (int k = 0; k < now_node.available.size(); ++k) {
                        int now_user_idx = g_users[now_node.available[k]].index;
                        int ava = g_users[now_user_idx].available.size();
                        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
                    }
                    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);
                    // 依次选择客户
//                    int n = now_node.available.size();
//                    vector<int> order_user = randperm(n);
                    for (int k = 0; k < unit_user_flow.size(); ++k) {
                        // 当前用户下标
                        int now_user_idx = unit_user_flow[k].first;
                        // 当前用户需求
                        int now_user_demand = g_demand[real_time_idx][now_user_idx];
                        // 当前服务器容量
                        int now_can_supply = node_time_remain[now_node.index][real_time_idx];
                        // 能放下
                        if (now_user_demand <= now_can_supply) {
                            // 更新容量和需求
                            g_demand[real_time_idx][now_user_idx] = 0;
                            node_time_remain[now_node.index][real_time_idx] -= now_user_demand;
                            node_time_asked[now_node.index][real_time_idx].second -= now_user_demand;
//                            // 更新该时间线上该用户的其他所有 avaliable 节点
//                            for (int l = 0; l < g_users[now_user_idx].available.size(); ++l) {
//                                int another_node_idx = g_users[now_user_idx].available[l];
//                                node_time_asked[another_node_idx][real_time_idx].second -= now_user_demand;
//                            }
                            now_node.all_remain[real_time_idx] -= now_user_demand;
                            sorted_node_time_asked[now_node.index][j].second -= now_user_demand;
                            // 更新输出
                            output[real_time_idx][now_user_idx][now_node.index] += now_user_demand;
                            can_break = false;
                        }
                            // 放不下
                        else {
                            g_demand[real_time_idx][now_user_idx] -= now_can_supply;
                            node_time_remain[now_node.index][real_time_idx] -= now_can_supply;
                            now_node.all_remain[real_time_idx] = 0;
                            output[real_time_idx][now_user_idx][now_node.index] += now_can_supply;
                        }
                    }

                }

            }
        }

        // 需求不变，break
        can_break = true;
        if (can_break) {
            break;
        }

    }
    //  prt(output);
    // 接之前的思路
    // TODO
    vector<pair<int, int> > unit_user_flow;
//    for (int k = 0; k < g_users.size(); ++k) {
//        int now_user_idx = g_users[k].index;
//        int ava = g_users[now_user_idx].available.size();
//        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
//    }
//    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);
//    vector<int> order_user = randperm(g_users.size());

    for (int k = 0; k < g_users.size(); ++k) {
        int now_user_idx = k;
        int ava = g_users[now_user_idx].available.size();
        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
    }
    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);

    for (int i = 0; i < g_demand.size(); ++i) {
        int now_time_idx = i;
        // 所有用户
        for (int j = 0; j < g_users.size(); ++j) {
            int now_user_idx = unit_user_flow[j].first;
            // 用户当前需求
            int now_user_demand = g_demand[now_time_idx][now_user_idx];
            // 需求等于0，下一用户
            if (now_user_demand == 0) {
                continue;
            }
            // 该用户能连通的所有节点下标
            vector<int> user_av = g_users[now_user_idx].available;
            int single = 0;
            // 均分后的需求 < 该节点 remain or 均分后的需求 > 该节点带宽，不行
            while (true) {
                if (user_av.empty()) {
                    break;
                }

                // 所有可达节点总需求
                int sum = 0;
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    sum += g_nodes[now_node_idx].bandwidth - g_nodes[now_node_idx].all_remain[now_time_idx];
                }
                // 均分全部需求
                single = (now_user_demand + sum) / user_av.size();

                set<int> cannot_put;
                // 对于每个节点
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    Node &now_node = g_nodes[now_node_idx];
                    if (single > now_node.bandwidth ||
                        single <= now_node.bandwidth - now_node.all_remain[now_time_idx]) {
                        cannot_put.insert(now_node_idx);
                        break;
                    }
                }

                if (cannot_put.empty()) {
                    break;
                }

                vector<int> temp;
                for (int l = 0; l < user_av.size(); ++l) {
                    int now_node_idx = user_av[l];
                    if (cannot_put.find(now_node_idx) == cannot_put.end()) {
                        temp.emplace_back(now_node_idx);
                    }
                }
                user_av = temp;

            }
            // 更新 user_av
            for (int m = 0; m < user_av.size(); ++m) {
                int now_node_idx = user_av[m];
                Node &now_node = g_nodes[now_node_idx];
                int addition = single - (now_node.bandwidth - now_node.all_remain[now_time_idx]);
                now_node.all_remain[now_time_idx] -= addition;
                g_demand[now_time_idx][now_user_idx] -= addition;
                output[now_time_idx][now_user_idx][now_node_idx] += addition;
            }

        }

    }

    for (int i = 0; i < g_demand.size(); i++) {
        int all_demand = 0;
        for (int l = 0; l < g_demand[i].size(); ++l) {
            all_demand += g_demand[i][l];
        }
        if (all_demand == 0) {
            continue;
        } else {
            for (int l = 0; l < g_demand[i].size(); ++l) {
                if (all_demand == 0)break;
                if (g_demand[i][l] > 0) {
                    int single = g_demand[i][l] / g_users[l].available.size();
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (single > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= single;
                                output[i][l][g_users[l].available[j]] += single;
                                all_demand -= single;
                                g_demand[i][l] -= single;
                            }
                        }
                    }
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (g_demand[i][l] > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= g_demand[i][l];
                                output[i][l][g_users[l].available[j]] += g_demand[i][l];
                                all_demand -= g_demand[i][l];
                                g_demand[i][l] -= g_demand[i][l];
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return output;

}

// 找到 vector_map_node_time_asked 的最大值及其索引
vector<int> find_mat2d_max(vector<vector<pair<int, int> > > &vector_node_time_asked, set<int> used_nodes,
                           vector<long long> row_sum) {
    int val = -1;
    int node_idx = 0;
    int time_idx = 0;
    int r_sum = -1;
    for (int i = 0; i < vector_node_time_asked.size(); ++i) {
        if (used_nodes.find(i) == used_nodes.end()) {
//          InsertionSort_form_big_to_small(vector_node_time_asked[i], vector_node_time_asked[i].size());
//            sort(vector_node_time_asked[i].begin(), vector_node_time_asked[i].end(), Great);
            if (r_sum < row_sum[i]) {
                r_sum = row_sum[i];
                val = vector_node_time_asked[i][0].second;
                node_idx = i;
                time_idx = vector_node_time_asked[i][0].first;
            }
//            if (vector_node_time_asked[i][0].second > val) {
//                val = vector_node_time_asked[i][0].second;
//                node_idx = i;
//                time_idx = vector_node_time_asked[i][0].first;
//            }
        }
    }


    vector<int> m{val, node_idx, time_idx};
    return m;
}

// 找到 vector_map_node_time_asked 的最大值及其索引
vector<int> find_mat2d_max(vector<vector<pair<int, int> > > &vector_node_time_asked, set<int> used_nodes) {
    int val = -1;
    int node_idx = 0;
    int time_idx = 0;
    for (int i = 0; i < vector_node_time_asked.size(); ++i) {
        if (used_nodes.find(i) == used_nodes.end()) {
//          InsertionSort_form_big_to_small(vector_node_time_asked[i], vector_node_time_asked[i].size());
//            sort(vector_node_time_asked[i].begin(), vector_node_time_asked[i].end(), Great);

            if (vector_node_time_asked[i][0].second > val) {
                val = vector_node_time_asked[i][0].second;
                node_idx = i;
                time_idx = vector_node_time_asked[i][0].first;
            }
        }
    }


    vector<int> m{val, node_idx, time_idx};
    return m;
}

vector<vector<vector<int> > > maximize_95plus_v4() {
    // 输出
    vector<vector<vector<int> > > output(g_demand.size(), vector<vector<int> >(g_users.size(),
                                                                               vector<int>(g_nodes.size(), 0)));
    // 前 5% 的时间节点个数
    int num_95plus = g_demand.size() - g_demand.size() * 0.95;

    // int: 需求量  pair<int, int>: 节点下标 时间下标
    vector<vector<pair<int, int> > > vector_node_time_asked;
    for (int i = 0; i < g_nodes.size(); ++i) {
        vector<pair<int, int> > a_node_times;
        for (int j = 0; j < g_demand.size(); ++j) {
            int now_time_idx = j;
            int sum = 0;
            for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                int now_user_idx = g_nodes[i].available[k];
                int temp = g_demand[now_time_idx][now_user_idx] / g_users[now_user_idx].available.size();
                //最大
//                if(temp>sum){
//                    sum=temp;
//                }
                //求和
//                int now_user_idx = g_nodes[i].available[k];
                sum += temp;
            }
            a_node_times.emplace_back(make_pair(now_time_idx, sum));
        }
        InsertionSort_form_big_to_small(a_node_times, a_node_times.size());
        vector_node_time_asked.emplace_back(a_node_times);
    }

    // 每个边缘节点 每个时刻最大安排流量 [nodes size, timeline size]
//    vector<vector<int> > node_time_remain(g_nodes.size(), vector<int>(g_demand.size(), 0));
//    for (int i = 0; i < g_nodes.size(); ++i) {
//        for (int j = 0; j < g_demand.size(); ++j) {
//            node_time_remain[i][j] = g_nodes[i].bandwidth;
//        }
//    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int cnt = 0;
    set<int> used_nodes;
    while (cnt < g_nodes.size()) {
        vector<int> max_val_pos = find_mat2d_max(vector_node_time_asked, used_nodes);
        // 当前时间线的总需求量
        int now_demand = max_val_pos[0];
        // 当前节点下标
        int now_node_idx = max_val_pos[1];
        // 当前时间线的下标
        int real_time_idx = max_val_pos[2];

        //求和
        int sum = 0;
        for (int k = 0; k < g_nodes[now_node_idx].available.size(); ++k) {
            int now_user_idx = g_nodes[now_node_idx].available[k];
            sum += g_demand[real_time_idx][now_user_idx];
        }
        now_demand = sum;

        // 当前边缘节点
        Node &now_node = g_nodes[now_node_idx];
        // 当前时间线该服务器的可供应量
        //   int can_supply = node_time_remain[now_node.index][real_time_idx];
        int can_supply = now_node.all_remain[real_time_idx];
        // 总需求 <= 可供应：直接放置

        if (can_supply >= now_demand) {
            // 该服务器在该时间点的可供应量减少
            //     node_time_remain[now_node.index][real_time_idx] -= now_demand;
            now_node.all_remain[real_time_idx] -= now_demand;
            // 已分配，更新输出和剩余需求
            for (int k = 0; k < now_node.available.size(); ++k) {
                int now_usr_idx = now_node.available[k];
                // 更新输出
                output[real_time_idx][now_usr_idx][now_node.index] += g_demand[real_time_idx][now_usr_idx];
                // 更新 demand
                g_demand[real_time_idx][now_usr_idx] = 0;
            }
        }
            // 总需求 > 可供应：“单位客户流量”进行有大到小排序，依次选择客户直到达到节点在当前时刻的上限
        else {
            // 计算 单位客户流量
            vector<pair<int, int> > unit_user_flow;

            for (int k = 0; k < now_node.available.size(); ++k) {
                int now_user_idx = g_users[now_node.available[k]].index;
                float unit_flow = g_demand[real_time_idx][now_user_idx] / g_users[now_user_idx].available.size();
                unit_user_flow.emplace_back(make_pair(now_user_idx, unit_flow));
            }
            // 对其由大到小排序
            sort(unit_user_flow.begin(), unit_user_flow.end(), Great);
            // 计算 客户在当前时刻候选边缘节点的个数由小到大排序
//                    for (int k = 0; k < now_node.available.size(); ++k) {
//                        int now_user_idx = g_users[now_node.available[k]].index;
//                        int ava = g_users[now_user_idx].available.size();
//                        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
//                    }
//                    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);
            // 依次选择客户
            for (int k = 0; k < unit_user_flow.size(); ++k) {
                // 当前用户下标
                int now_user_idx = unit_user_flow[k].first;
                // 当前用户需求
                int now_user_demand = g_demand[real_time_idx][now_user_idx];
                // 当前服务器容量
                // int now_can_supply = node_time_remain[now_node.index][real_time_idx];
                int now_can_supply = now_node.all_remain[real_time_idx];
                // 能放下
                if (now_user_demand <= now_can_supply) {
                    // 更新容量和需求
                    g_demand[real_time_idx][now_user_idx] = 0;
                    //      node_time_remain[now_node.index][real_time_idx] -= now_user_demand;
//                            // 更新该时间线上该用户的其他所有 avaliable 节点
//                            for (int l = 0; l < g_users[now_user_idx].available.size(); ++l) {
//                                int another_node_idx = g_users[now_user_idx].available[l];
//                                node_time_asked[another_node_idx][real_time_idx].second -= now_user_demand;
//                            }
                    now_node.all_remain[real_time_idx] -= now_user_demand;
                    // 更新输出
                    output[real_time_idx][now_user_idx][now_node.index] += now_user_demand;
                }
                    // 放不下
                else {

                    g_demand[real_time_idx][now_user_idx] -= now_can_supply;
                    //    node_time_remain[now_node.index][real_time_idx] = 0;
                    now_node.all_remain[real_time_idx] = 0;
                    output[real_time_idx][now_user_idx][now_node.index] += now_can_supply;
                }
            }

        }

        now_node.use95++;
        if (now_node.use95 == num_95plus) {
            cnt++;
            used_nodes.insert(now_node_idx);
        }
        // 更新 vector_node_time_asked
        for (int i = 0; i < g_nodes.size(); ++i) {
            if (used_nodes.find(i) == used_nodes.end()) {
                int sum = 0;
                for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                    int now_user_idx = g_nodes[i].available[k];
                    //最大
                    int temp = g_demand[real_time_idx][now_user_idx] / g_users[now_user_idx].available.size();
                    //最大
//                    if(temp>sum){
//                        sum=temp;
//                    }
                    //求和
                    sum += temp;
                }
                int where = 0;
                for (int j = 0; j < vector_node_time_asked[i].size(); ++j) {
                    if (real_time_idx == vector_node_time_asked[i][j].first) {
                        where = j;
                        break;
                    }
                }
                vector_node_time_asked[i][where].second = sum;
                change_idx(vector_node_time_asked[i], vector_node_time_asked.size(), where);
            }
        }

    }
//    prt(output);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 接之前的思路
    // TODO
    vector<pair<int, int> > unit_user_flow;
    for (int k = 0; k < g_users.size(); ++k) {
        int now_user_idx = g_users[k].index;
        int ava = (g_users[now_user_idx].available.size());
        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
    }
    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);

    for (int i = 0; i < g_demand.size(); ++i) {
        int now_time_idx = i;
        // 所有用户
        for (int j = 0; j < g_users.size(); ++j) {
            int now_user_idx = unit_user_flow[j].first;
            // 用户当前需求
            int now_user_demand = g_demand[now_time_idx][now_user_idx];
            // 需求等于0，下一用户
            if (now_user_demand == 0) {
                continue;
            }
            // 该用户能连通的所有节点下标
            vector<int> user_av = g_users[now_user_idx].available;
            int single = 0;
            // 均分后的需求 < 该节点 remain or 均分后的需求 > 该节点带宽，不行
            while (true) {
                if (user_av.empty()) {
                    break;
                }

                // 所有可达节点总需求
                int sum = 0;
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    sum += g_nodes[now_node_idx].bandwidth - g_nodes[now_node_idx].all_remain[now_time_idx];
                }
                // 均分全部需求
                single = (now_user_demand + sum) / user_av.size();

                set<int> cannot_put;
                // 对于每个节点
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    Node &now_node = g_nodes[now_node_idx];
                    if (single > now_node.bandwidth ||
                        single <= now_node.bandwidth - now_node.all_remain[now_time_idx]) {
                        cannot_put.insert(now_node_idx);
                        break;
                    }
                }

                if (cannot_put.empty()) {
                    break;
                }

                vector<int> temp;
                for (int l = 0; l < user_av.size(); ++l) {
                    int now_node_idx = user_av[l];
                    if (cannot_put.find(now_node_idx) == cannot_put.end()) {
                        temp.emplace_back(now_node_idx);
                    }
                }
                user_av = temp;

            }
            // 更新 user_av
            for (int m = 0; m < user_av.size(); ++m) {
                int now_node_idx = user_av[m];
                Node &now_node = g_nodes[now_node_idx];
                int addition = single - (now_node.bandwidth - now_node.all_remain[now_time_idx]);
                now_node.all_remain[now_time_idx] -= addition;
                g_demand[now_time_idx][now_user_idx] -= addition;
                output[now_time_idx][now_user_idx][now_node_idx] += addition;
            }

        }

    }

    for (int i = 0; i < g_demand.size(); i++) {
        int all_demand = 0;
        for (int l = 0; l < g_demand[i].size(); ++l) {
            all_demand += g_demand[i][l];
        }
        if (all_demand == 0) {
            continue;
        } else {
            for (int l = 0; l < g_demand[i].size(); ++l) {
                if (all_demand == 0)break;
                if (g_demand[i][l] > 0) {
                    int single = g_demand[i][l] / g_users[l].available.size();
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (single > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                                all_demand -= now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= single;
                                output[i][l][g_users[l].available[j]] += single;
                                all_demand -= single;
                                g_demand[i][l] -= single;
                            }
                        }
                    }
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (g_demand[i][l] > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= g_demand[i][l];
                                output[i][l][g_users[l].available[j]] += g_demand[i][l];
                                all_demand -= g_demand[i][l];
                                g_demand[i][l] -= g_demand[i][l];
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return output;

}

vector<vector<vector<int> > > maximize_95plus_v5() {
    // 输出
    vector<vector<vector<int> > > output(g_demand.size(), vector<vector<int> >(g_users.size(),
                                                                               vector<int>(g_nodes.size(), 0)));
    vector<long long> row_sum(g_nodes.size(), 0);

    // 前 5% 的时间节点个数
    int num_95plus = g_demand.size() - g_demand.size() * 0.95;

    // int: 需求量  pair<int, int>: 节点下标 时间下标
    vector<vector<pair<int, int> > > vector_node_time_asked;
    for (int i = 0; i < g_nodes.size(); ++i) {
        vector<pair<int, int> > a_node_times;
        long long sum_all_time = 0;
        for (int j = 0; j < g_demand.size(); ++j) {
            int now_time_idx = j;
            int sum = 0;
            for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                int now_user_idx = g_nodes[i].available[k];
                int temp = g_demand[now_time_idx][now_user_idx];
                //最大
//                if(temp>sum){
//                    sum=temp;
//                }
                //求和
//                int now_user_idx = g_nodes[i].available[k];
                sum += temp;
            }
            sum_all_time += sum;
            a_node_times.emplace_back(make_pair(now_time_idx, sum));
        }
        row_sum[i] = sum_all_time;
        InsertionSort_form_big_to_small(a_node_times, a_node_times.size());
        vector_node_time_asked.emplace_back(a_node_times);
    }
    // 初始化节点和客户的不满足的下标
    for (int i = 0; i < g_nodes.size(); ++i) {
        g_nodes[i].time_not_available = vector<vector<int> >(g_demand.size(), vector<int>(36, 1));
    }
    for (int i = 0; i < g_users.size(); ++i) {
        g_users[i].time_not_available = vector<vector<int> >(g_demand.size(), vector<int>(136, 1));
    }

    // 每个边缘节点 每个时刻最大安排流量 [nodes size, timeline size]
    vector<vector<int> > node_time_remain(g_nodes.size(), vector<int>(g_demand.size(), 0));
    for (int i = 0; i < g_nodes.size(); ++i) {
        for (int j = 0; j < g_demand.size(); ++j) {
            node_time_remain[i][j] = g_nodes[i].bandwidth;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    int cnt = 0;
    set<int> used_nodes;
    while (cnt < g_nodes.size()) {
        vector<int> max_val_pos = find_mat2d_max(vector_node_time_asked, used_nodes, row_sum);
        // 当前时间线的总需求量
        int now_demand = max_val_pos[0];
        // 当前节点下标
        int now_node_idx = max_val_pos[1];
        // 当前时间线的下标
        int real_time_idx = max_val_pos[2];
//        cout_details(now_node_idx, 0, real_time_idx, now_demand);
        // 当前边缘节点
        Node &now_node = g_nodes[now_node_idx];
        // 当前时间线该服务器的可供应量
        int can_supply = node_time_remain[now_node.index][real_time_idx];

        //求和
        int sum = 0;
        for (int k = 0; k < now_node.available.size(); ++k) {
            int now_user_idx = now_node.available[k];
            if (now_node.time_not_available[real_time_idx][now_user_idx] == 1) {
                sum += g_demand[real_time_idx][now_user_idx];
            }
        }
        now_demand = sum;
        // 总需求 <= 可供应：直接放置
        if (can_supply >= now_demand) {
            // 该服务器在该时间点的可供应量减少
            node_time_remain[now_node.index][real_time_idx] -= now_demand;
            now_node.all_remain[real_time_idx] -= now_demand;
            // 已分配，更新输出和剩余需求
            for (int k = 0; k < now_node.available.size(); ++k) {
                int now_usr_idx = now_node.available[k];
                if (now_node.time_not_available[real_time_idx][now_usr_idx] == 0) {
                    continue;
                }
                // 更新输出
                output[real_time_idx][now_usr_idx][now_node.index] += g_demand[real_time_idx][now_usr_idx];
                // 更新 demand
                g_demand[real_time_idx][now_usr_idx] = 0;
            }
        }
            // 总需求 > 可供应：“单位客户流量”进行有大到小排序，依次选择客户直到达到节点在当前时刻的上限
        else {
            // 计算 单位客户流量
            vector<pair<int, int> > unit_user_flow;
            for (int k = 0; k < now_node.available.size(); ++k) {
                int now_user_idx = g_users[now_node.available[k]].index;
                if (now_node.time_not_available[real_time_idx][now_user_idx] == 0) {
                    continue;
                }
                float unit_flow = g_demand[real_time_idx][now_user_idx] / (g_users[now_user_idx].available.size() -
                                                                           g_users[now_user_idx].time_not_available[real_time_idx][135] +
                                                                           1);
                unit_user_flow.emplace_back(make_pair(now_user_idx, unit_flow));
            }
            // 对其由大到小排序
            sort(unit_user_flow.begin(), unit_user_flow.end(), Great);
            // 计算 客户在当前时刻候选边缘节点的个数由小到大排序
//                    for (int k = 0; k < now_node.available.size(); ++k) {
//                        int now_user_idx = g_users[now_node.available[k]].index;
//                        int ava = g_users[now_user_idx].available.size();
//                        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
//                    }
//                    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);
            // 依次选择客户
            for (int k = 0; k < unit_user_flow.size(); ++k) {
                // 当前用户下标
                int now_user_idx = unit_user_flow[k].first;
                // 当前用户需求
                int now_user_demand = g_demand[real_time_idx][now_user_idx];
                // 当前服务器容量
                int now_can_supply = node_time_remain[now_node.index][real_time_idx];
                // 能放下
                if (now_user_demand <= now_can_supply) {
                    // 更新容量和需求
                    g_demand[real_time_idx][now_user_idx] = 0;
                    node_time_remain[now_node.index][real_time_idx] -= now_user_demand;
//                            // 更新该时间线上该用户的其他所有 avaliable 节点
//                            for (int l = 0; l < g_users[now_user_idx].available.size(); ++l) {
//                                int another_node_idx = g_users[now_user_idx].available[l];
//                                node_time_asked[another_node_idx][real_time_idx].second -= now_user_demand;
//                            }
                    now_node.all_remain[real_time_idx] -= now_user_demand;
                    // 更新输出
                    output[real_time_idx][now_user_idx][now_node.index] += now_user_demand;
                }
                    // 放不下 更新各自的 available
                else {
//                    cout << "------------------" << endl;
                    g_demand[real_time_idx][now_user_idx] -= now_can_supply;
                    node_time_remain[now_node.index][real_time_idx] -= now_can_supply;
                    now_node.all_remain[real_time_idx] = 0;
                    output[real_time_idx][now_user_idx][now_node.index] += now_can_supply;

                    now_node.time_not_available[real_time_idx][now_user_idx] = 0;
                    g_users[now_user_idx].time_not_available[real_time_idx][now_node_idx] = 0;
                    now_node.time_not_available[real_time_idx][35]++;
                    g_users[now_user_idx].time_not_available[real_time_idx][135]++;

                }
            }

        }

        now_node.use95++;
        if (now_node.use95 == num_95plus) {
            cnt++;
            used_nodes.insert(now_node_idx);
        }
        // 更新 vector_node_time_asked
        for (int i = 0; i < g_nodes.size(); ++i) {
            if (used_nodes.find(i) == used_nodes.end()) {
                int sum = 0;
                for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                    int now_user_idx = g_nodes[i].available[k];
                    if (now_node.time_not_available[real_time_idx][now_user_idx] == 0) {
                        continue;
                    }
                    //最大
                    //                 int temp = g_demand[real_time_idx][now_user_idx] / (g_users[now_user_idx].available.size() - g_users[now_user_idx].time_not_available[real_time_idx][135] + 1);
                    int temp = g_demand[real_time_idx][now_user_idx];
                    //最大
//                    if(temp>sum){
//                        sum=temp;
//                    }
                    //求和
                    sum += temp;
                }
                int where = 0;
                for (int j = 0; j < vector_node_time_asked[i].size(); ++j) {
                    if (real_time_idx == vector_node_time_asked[i][j].first) {
                        where = j;
                        break;
                    }
                }
                row_sum[i] -= (vector_node_time_asked[i][where].second - sum);
                vector_node_time_asked[i][where].second = sum;
                change_idx(vector_node_time_asked[i], vector_node_time_asked.size(), where);
            }
        }

    }
//    prt(output);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 接之前的思路
    // TODO
    vector<pair<int, int> > unit_user_flow;
    for (int k = 0; k < g_users.size(); ++k) {
        int now_user_idx = g_users[k].index;
        int ava = (g_users[now_user_idx].available.size());
        unit_user_flow.emplace_back(make_pair(now_user_idx, ava));
    }
    sort(unit_user_flow.begin(), unit_user_flow.end(), Less);

    for (int i = 0; i < g_demand.size(); ++i) {
        int now_time_idx = i;
        // 所有用户
        for (int j = 0; j < g_users.size(); ++j) {
            int now_user_idx = unit_user_flow[j].first;
            // 用户当前需求
            int now_user_demand = g_demand[now_time_idx][now_user_idx];
            // 需求等于0，下一用户
            if (now_user_demand == 0) {
                continue;
            }
            // 该用户能连通的所有节点下标
            vector<int> user_av = g_users[now_user_idx].available;
            int single = 0;
            // 均分后的需求 < 该节点 remain or 均分后的需求 > 该节点带宽，不行
            while (true) {
                if (user_av.empty()) {
                    break;
                }

                // 所有可达节点总需求
                int sum = 0;
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    sum += g_nodes[now_node_idx].bandwidth - g_nodes[now_node_idx].all_remain[now_time_idx];
                }
                // 均分全部需求
                single = (now_user_demand + sum) / user_av.size();

                set<int> cannot_put;
                // 对于每个节点
                for (int k = 0; k < user_av.size(); ++k) {
                    int now_node_idx = user_av[k];
                    Node &now_node = g_nodes[now_node_idx];
                    if (single > now_node.bandwidth ||
                        single <= now_node.bandwidth - now_node.all_remain[now_time_idx]) {
                        cannot_put.insert(now_node_idx);
                        break;
                    }
                }

                if (cannot_put.empty()) {
                    break;
                }

                vector<int> temp;
                for (int l = 0; l < user_av.size(); ++l) {
                    int now_node_idx = user_av[l];
                    if (cannot_put.find(now_node_idx) == cannot_put.end()) {
                        temp.emplace_back(now_node_idx);
                    }
                }
                user_av = temp;

            }
            // 更新 user_av
            for (int m = 0; m < user_av.size(); ++m) {
                int now_node_idx = user_av[m];
                Node &now_node = g_nodes[now_node_idx];
                int addition = single - (now_node.bandwidth - now_node.all_remain[now_time_idx]);
                now_node.all_remain[now_time_idx] -= addition;
                g_demand[now_time_idx][now_user_idx] -= addition;
                output[now_time_idx][now_user_idx][now_node_idx] += addition;
            }

        }

    }

    for (int i = 0; i < g_demand.size(); i++) {
        int all_demand = 0;
        for (int l = 0; l < g_demand[i].size(); ++l) {
            all_demand += g_demand[i][l];
        }
        if (all_demand == 0) {
            continue;
        } else {
            for (int l = 0; l < g_demand[i].size(); ++l) {
                if (all_demand == 0)break;
                if (g_demand[i][l] > 0) {
                    int single = g_demand[i][l] / g_users[l].available.size();
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (single > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                                all_demand -= now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= single;
                                output[i][l][g_users[l].available[j]] += single;
                                all_demand -= single;
                                g_demand[i][l] -= single;
                            }
                        }
                    }
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].all_remain[i];
                        if (now_remain > 0) {
                            if (g_demand[i][l] > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].all_remain[i] = 0;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].all_remain[i] -= g_demand[i][l];
                                output[i][l][g_users[l].available[j]] += g_demand[i][l];
                                all_demand -= g_demand[i][l];
                                g_demand[i][l] -= g_demand[i][l];
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return output;

}

void optimize_v2(vector<vector<vector<int> > > &output) {

}

//void run(bool show_time = false) {
//    if (show_time) {
//        clock_t be = clock();
//        srand((int) time(0));
//        read_conf();
//        read_qos();
//        read_demand();
//        read_bandwidth();
//        vector<int> order = randperm(g_nodes.size());
//        g_output = maximize_95plus(order);
//        // optimize(g_output);
//        clock_t en = clock();
//        double totaltime = (en - be) * 1.0 / CLOCKS_PER_SEC;
//        cout << "执行时间:" << totaltime << "秒" << endl;
//
//    }
//    else {
//        srand((int) time(0));
//        freopen(SOLUTION_PATH.c_str(), "w", stdout);
//        read_conf();
//        read_qos();
//        read_demand();
//        read_bandwidth();
//        vector<int> order = randperm(g_nodes.size());
//        g_output = maximize_95plus(order);
//        //  optimize(g_output);
//
//        prt(g_output);
//        fclose(stdout);
//    }
//}

//vector<vector<vector<int> > > run_thread() {
//    struct timeb timeSeed;
//    ftime(&timeSeed);
//    srand(timeSeed.time * 1000 + timeSeed.millitm);
//    vector<int> order = randperm(g_nodes.size());
//    return maximize_95plus(order,g_nodes,g_user,g_demand);
//}

void
GetSumT(vector<Node> g_nodes, vector<User> g_user, vector<vector<int> > g_demand, vector<vector<vector<int> > > &output,
        long long &cost) {
    struct timeb timeSeed;
    ftime(&timeSeed);
    srand(timeSeed.time * 1000 + timeSeed.millitm);
    vector<int> order_nodes = randperm(g_nodes.size());
    output = maximize_95plus(order_nodes, g_nodes, g_user, g_demand);
    cost = compute_cost(output);

}


int main() {
    struct timeb timeSeed;
    ftime(&timeSeed);
    srand(timeSeed.time * 1000 + timeSeed.millitm);

//    run(false);


    freopen(SOLUTION_PATH.c_str(), "w", stdout);
    read_conf();
    read_qos();
    read_demand();
    read_bandwidth();
    vector<Node> temp_g_nodes = g_nodes;
    vector<User> temp_g_users = g_users;
    vector<vector<int> > temp_g_demand = g_demand;
    //  g_output = run_thread();
    long long min_cost = LONG_LONG_MAX;
//    cout << min_cost << endl;
    for (int i = 0; i < 45; ++i) {
        g_nodes = temp_g_nodes;
        g_users = temp_g_users;
        g_demand = temp_g_demand;
//        vector<vector<vector<int> > > output = run_thread();
//        long long cost = compute_cost(output);
        vector<vector<vector<int> > > result1, result2, result3, result4;
        long long cost1, cost2, cost3, cost4;
        thread first(GetSumT, g_nodes, g_users, g_demand, std::ref(result1), std::ref(cost1)); //子线程1
        thread second(GetSumT, g_nodes, g_users, g_demand, std::ref(result2), std::ref(cost2)); //子线程1
        thread third(GetSumT, g_nodes, g_users, g_demand, std::ref(result3), std::ref(cost3)); //子线程1
        thread fouth(GetSumT, g_nodes, g_users, g_demand, std::ref(result4), std::ref(cost4)); //子线程1

        first.join(); //主线程要等待子线程执行完毕
        second.join();
        third.join();
        fouth.join();
//        cout << cost << endl;
        if (cost1 < min_cost) {
            min_cost = cost1;
            g_output = result1;
        }
        if (cost2 < min_cost) {
            min_cost = cost2;
            g_output = result2;
        }
        if (cost3 < min_cost) {
            min_cost = cost3;
            g_output = result3;
        }
        if (cost4 < min_cost) {
            min_cost = cost4;
            g_output = result4;
        }
//        cout << endl << i << " " << min_cost << " " << cost1  << " " << cost2  << " " << cost3  << " " << cost4  << endl;
    }


    prt(g_output);
    fclose(stdout);

    return 0;
}
