#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include "utils.h"
#include <cmath>
#include <map>
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
    while (getline(data, tmp_line)) {
        stringstream ss(tmp_line);
        string str;
        vector<int> lineArray;
        int is_first = true;
        while (getline(ss, str, ',')) {
            if (is_first) {
                g_times.emplace_back(Time(str, g_times.size()));
                is_first = false;
            } else {
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
            } else {
                int idx = cnt++;
                g_nodes[idx].bandwidth = atoi(str.c_str());
                g_nodes[idx].remain = g_nodes[idx].bandwidth;
                g_nodes[idx].all_remain = vector<int>(g_demand.size(), g_nodes[idx].bandwidth);
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
        ratio.emplace_back((double) g_nodes[all[j]].remain / sum * (double) demand);
    }
    return ratio;
}

// 给定 nodes 的 remain 的比例
map<int, int> ratio_nodes(map<int, Node> all, int demand) {
    double sum = 0;
    for (auto &kv: all) {
        sum += kv.second.remain;
    }
    map<int, int> ratio;
    for (auto &kv: all) {
        ratio[kv.first] = (double) kv.second.remain / sum * (double) demand;
    }
    return ratio;
}

// szz's baseline
vector<vector<vector<int> > > baseline_1(vector<vector<vector<int> > > &output) {
    srand((int) time(0));
    // 对于每个时间点
    for (int i = 0; i < 1; ++i) {
        // 对于每个用户
        for (int i1 = 0; i1 < g_nodes.size(); ++i1) {
            g_nodes[i1].history.emplace_back(0);
        }
        for (int j = 0; j < g_demand[i].size(); ++j) {
            // 均分用户流量到所有服务器
            int user_demand = g_demand[i][j];
            int a_little_left = user_demand;
            if (user_demand == 0) {
                vector<Node> nodes;
                if (i == 0 && j == 0) {
                    logger_line1(g_users[j], nodes);
                } else {
                    logger_line(g_users[j], nodes);
                }
                continue;
            }
            // 随机限制节点，对于这些节点，虽然 qos 满足，但仍不能请求流量
            int non_restrict = 1;
            int num_useful = g_users[j].available.size() * non_restrict;
            vector<int> useful_idx = randperm(g_users[j].available.size());
            vector<int> temp;
            temp.reserve(num_useful);
            for (int m = 0; m < num_useful; ++m) {
                temp.emplace_back(g_users[j].available[useful_idx[m]]);
            }
            if (i == 0) {
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
                        Node &now_node = g_nodes[temp[k]];
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

                    // 如果流量还没被分完，释放被限制的节点
                    if (satisfied_idx.empty()) {
                        for (int m = num_useful; m < g_users[j].available.size(); ++m) {
                            temp.emplace_back(g_users[j].available[useful_idx[m]]);
                        }
                        continue;
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
                            } else {
                                used_nodes[l].now_used += g_nodes[used_nodes[l].index].remain;
                                g_nodes[used_nodes[l].index].remain = 0;
                            }
                        }
                        break;
                    }
                    temp = satisfied_idx;
                }
                for (int n = 0; n < used_nodes.size(); ++n) {
                    g_nodes[used_nodes[n].index].history[i] += used_nodes[n].now_used;
                }
                // 输出
                if (i == 0 && j == 0) {
                    logger_line1(g_users[j], used_nodes);
                } else {
                    logger_line(g_users[j], used_nodes);
                }
            }
        }
        // 重置每个服务器的带宽
        reset_bandwidth();
    }
    //时间
    for (int i = 1; i < g_demand.size(); ++i) {
        //边缘节点
        for (int j = 0; j < g_nodes.size(); j++) {
            Node &now_node = g_nodes[j];
            int now_95 = now_node.get_95();
            now_node.history.emplace_back(0);
            //客户
            for (int k = 0; k < now_node.available.size(); ++k) {
                if (now_95 > 0) {
                    int now_demand = g_demand[i][now_node.available[k]];
                    if (now_demand <= now_95) {
                        now_node.remain -= now_demand;
                        now_95 -= now_demand;
                        now_node.history[i] += now_demand;
                        g_demand[i][now_node.available[k]] = 0;
                        output[i][now_node.available[k]][j] += now_demand;
                    } else {
                        g_demand[i][now_node.available[k]] -= now_95;
                        now_node.remain -= now_95;
                        now_node.history[i] += now_95;
                        output[i][now_node.available[k]][j] += now_95;
                        now_95 = 0;
                    }
                } else {
                    break;
                }
            }
        }
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
                        int now_remain = g_nodes[g_users[l].available[j]].remain;
                        if (now_remain > 0) {
                            if (single > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].remain = 0;
                                g_nodes[g_users[l].available[j]].history[i] += now_remain;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].remain -= single;
                                g_nodes[g_users[l].available[j]].history[i] += single;
                                output[i][l][g_users[l].available[j]] += single;
                                all_demand -= single;
                                g_demand[i][l] -= single;
                            }
                        }
                    }
                    for (int j = 0; j < g_users[l].available.size(); ++j) {
                        int now_remain = g_nodes[g_users[l].available[j]].remain;
                        if (now_remain > 0) {
                            if (g_demand[i][l] > now_remain) {
                                g_demand[i][l] -= now_remain;
                                g_nodes[g_users[l].available[j]].remain = 0;
                                g_nodes[g_users[l].available[j]].history[i] += now_remain;
                                output[i][l][g_users[l].available[j]] += now_remain;
                            } else {
                                g_nodes[g_users[l].available[j]].remain -= g_demand[i][l];
                                g_nodes[g_users[l].available[j]].history[i] += g_demand[i][l];
                                output[i][l][g_users[l].available[j]] += g_demand[i][l];
                                all_demand -= g_demand[i][l];
                                g_demand[i][l] -= g_demand[i][l];
                                break;
                            }
                        }
                    }
                } else {
                    continue;
                }
            }
        }
        // 重置每个服务器的带宽
        reset_bandwidth();
    }


    for (int i1 = 1; i1 < output.size(); ++i1) {
        for (int i = 0; i < output[i1].size(); ++i) {
            vector<Node> v;
            for (int j = 0; j < output[i1][i].size(); ++j) {
                if (g_qos[j][i] < QOS_MAX && output[i1][i][j] > 0) {
                    Node n = Node(g_nodes[j].name, j);
                    n.now_used = output[i1][i][j];
                    v.emplace_back(n);
                }
            }
            logger_line(User(g_users[i].name, i), v);
        }
    }

    return output;

}

// cyc's baseline
vector<vector<vector<int> > > baseline(vector<vector<vector<int> > > &output) {
    srand((int) time(0));
    // 对于每个时间点
    for (int i = 0; i < g_demand.size(); ++i) {
        // 对于每个用户
        for (int j = 0; j < g_demand[i].size(); ++j) {
            // 均分用户流量到所有服务器
            int user_demand = g_demand[i][j];
            int a_little_left = user_demand;
            if (user_demand == 0) {
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
                    Node &now_node = g_nodes[temp[k]];
                    // 满足
                    if (now_node.all_remain[i] >= ratio[k]) {
                        satisfied_idx.emplace_back(now_node.index);
                    }
                        // 不满足
                    else {
                        // 将该服务器所有剩余带宽分配出去
                        user_demand -= now_node.all_remain[i];
                        now_node.now_used = now_node.all_remain[i];
                        a_little_left -= now_node.all_remain[i];
                        now_node.all_remain[i] = 0;
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
                        g_nodes[satisfied_idx[k]].all_remain[i] -= ratio[k];
                        a_little_left -= ratio[k];
                        used_nodes.emplace_back(g_nodes[satisfied_idx[k]]);
                    }
                    // 分剩余
                    for (int l = 0; l < used_nodes.size(); ++l) {
                        if (used_nodes[l].all_remain[i] >= a_little_left) {
                            g_nodes[used_nodes[l].index].all_remain[i] -= a_little_left;
                            used_nodes[l].now_used += a_little_left;
                            break;
                        } else {
                            used_nodes[l].now_used += g_nodes[used_nodes[l].index].all_remain[i];
                            g_nodes[used_nodes[l].index].all_remain[i] = 0;
                        }
                    }
                    break;
                }
                temp = satisfied_idx;
            }
            // 输出
            for (auto &kv : used_nodes) {
                output[i][j][kv.index] += kv.now_used;
            }
        }
        // 重置每个服务器的带宽
//        reset_bandwidth();
    }
    return output;
}

const int G_PART = 0;
const int G_MAX_LOOP = 1;

// cxy's baseline
void baseline_2(vector<vector<vector<int> > > &output) {
    // 每个时间节点
    for (int i = 0; i < g_demand.size(); ++i) {
        // 已分配节点列表
        vector<Node> used_nodes(g_nodes.size(), Node());
        // 每个用户
        for (int j = 0; j < g_users.size(); ++j) {
            // 总用户需求
            int user_demand = g_demand[i][j];
            int a_little_left = user_demand;
            if (user_demand == 0) {
                continue;
            }
            // 统计所有可放置节点
            map<int, Node> temp_nodes;
            for (int k = 0; k < g_users[j].available.size(); ++k) {
                int now_node_idx = g_users[j].available[k];
                // 当前节点没有在已分配节点列表
                if (used_nodes[now_node_idx].name == "") {
                    // 添加到已分配节点列表
                    used_nodes[now_node_idx] = Node(g_nodes[now_node_idx]);
                    // now_used 仅供输出使用
                }
                temp_nodes[now_node_idx] = used_nodes[now_node_idx];
                temp_nodes[now_node_idx].now_used = 0;
            }
            map<int, Node> temp_nodes_copy(temp_nodes);
            // 计算已分配节点列表的总 demand
            int total_demand = 0;
            for (auto &kv : temp_nodes) {
                total_demand += kv.second.total_used;
            }
            // 需要重新平均分配的总需求量
            int now_total = user_demand + total_demand;
            // 分配到每个节点上的平均值
            int avg = now_total / temp_nodes.size();
            // 放不下的节点下标
            vector<int> cannot_put;
            vector<Node> erased_nodes;
            // 待分配
            int left = 0;
            // 限制最大循环次数
            int cnt = 0;
            while (true) {
                if (cnt++ >= G_MAX_LOOP) {
                    left = user_demand;
                    break;
                }
                // 如果每台节点都放得下 avg，则分配完毕，break
                bool can_put = true;
                for (auto &kv: temp_nodes) {
                    int now_node_idx = kv.first;
                    int now_bandwidth = g_nodes[now_node_idx].all_remain[i];
                    if ((now_bandwidth < avg) || (avg < kv.second.total_used)) {
                        can_put = false;
                        cannot_put.emplace_back(kv.first);
                    }
                }
                if (can_put) {
                    for (auto &kv: temp_nodes) {
                        int additional = avg - kv.second.total_used;
                        kv.second.total_used += additional;
                        kv.second.now_used = additional;
                        a_little_left -= additional;
                    }

                    while (a_little_left > 0) {
                        for (auto &kv: temp_nodes) {
                            int now_node_idx = kv.first;
                            int now_bandwidth = g_nodes[now_node_idx].all_remain[i];
                            if (now_bandwidth - kv.second.total_used >= a_little_left) {
                                kv.second.total_used += a_little_left;
                                kv.second.now_used += a_little_left;
                                a_little_left = 0;
                                break;
                            }
                        }
                    }

                    left = 0;
                    break;
                }

                // 至少有一台节点放不下 avg
                // 从可用节点中去除
                for (int k = 0; k < cannot_put.size(); ++k) {
                    auto key = temp_nodes.find(cannot_put[k]);
                    if (key != temp_nodes.end()) {
                        erased_nodes.emplace_back(Node(temp_nodes[key->first]));
                        now_total -= temp_nodes[key->first].total_used;
                        temp_nodes.erase(key);
                    }
                }
                // 极端情况
                if (temp_nodes.empty()) {
                    temp_nodes = temp_nodes_copy;
                    // 减少 new_total 数量，暂存至 left
                    if (left + G_PART >= user_demand) {
                        left = user_demand;
                        break;
                    }
                    left += G_PART;
                    now_total -= G_PART;
                    a_little_left -= G_PART;
                }

                // 更新 avg
                avg = now_total / temp_nodes.size();
            }

            // 目前还剩 left 未分配，将其按剩余比例分配
            if (left != 0) {
                // 取回不能分配的节点
                for (int m = 0; m < erased_nodes.size(); ++m) {
                    int idx = erased_nodes[m].index;
                    temp_nodes[idx] = erased_nodes[m];
                }
                temp_nodes_copy = temp_nodes;

                a_little_left = left;
                while (true) {
                    // 计算满足 qos 的服务器的 remain 的比例
                    map<int, int> ratio = ratio_nodes(temp_nodes_copy, left);
                    // remain 够的下标
                    map<int, Node> satisfied_idx;
                    // 是否能进行最后的分配
                    bool can_put = true;
                    for (auto &kv: ratio) {
                        int idx = kv.first;
                        // 满足
                        if (temp_nodes_copy[idx].all_remain[i] - temp_nodes_copy[idx].total_used >= ratio[idx]) {
                            satisfied_idx[idx] = temp_nodes_copy[idx];
                        }
                            // 不满足
                        else {
                            can_put = false;
                        }
                    }

                    // 如果都满足比例分配则分配，跳出循环
                    if (can_put) {
                        // 按比例分
                        for (auto &kv: temp_nodes_copy) {
                            int idx = kv.first;
                            temp_nodes[idx].now_used = ratio[idx];
                            temp_nodes[idx].total_used += ratio[idx];
                            a_little_left -= ratio[idx];
                        }
                        // 分剩余
                        for (auto &kv: temp_nodes_copy) {
                            int idx = kv.first;
                            int remain = temp_nodes[idx].all_remain[i] - temp_nodes[idx].total_used;
                            if (remain >= a_little_left) {
                                temp_nodes[idx].now_used += a_little_left;
                                temp_nodes[idx].total_used += a_little_left;
                                break;
                            } else {
                                temp_nodes[idx].now_used += remain;
                                temp_nodes[idx].total_used = temp_nodes[idx].all_remain[i];
                            }
                        }
                        break;
                    }
                    temp_nodes_copy = satisfied_idx;
                }
            }

            // 更新 used_nodes
            for (auto &kv: temp_nodes) {
                used_nodes[kv.first] = kv.second;
            }

            for (auto &kv : temp_nodes) {
                output[i][j][kv.first] = kv.second.now_used;
            }


        }
    }
}

// 替换比例
const float G_SUB_RATIO_LEFT = 0.01;
const float G_SUB_RATIO_RIGHT = 0.1;
const float MAX_OPTIMIZE_LOOP = 25;

vector<vector<Common> > get_common_users_mat() {
    // 客户交集矩阵
    vector<vector<Common> > common_users_mat(g_nodes.size(), vector<Common>(g_nodes.size(), Common()));
    for (int m = 0; m < g_nodes.size(); ++m) {
        for (int i = 0; i < g_nodes.size(); ++i) {
            if (m >= i) {
                vector<int> common_users;
                set_intersection(g_nodes[m].available.begin(), g_nodes[m].available.end(),
                                 g_nodes[i].available.begin(), g_nodes[i].available.end(),
                                 back_inserter(common_users));
                common_users_mat[m][i] = Common(common_users);
            } else {
                common_users_mat[m][i] = common_users_mat[i][m];
            }
        }
    }
    g_common_users_mat = common_users_mat;
    return common_users_mat;
}

void optimize(vector<vector<vector<int> > > &output) {

    srand((int) time(0));
    // 服务器的 available 排序，方便取交集
    for (int i = 0; i < g_nodes.size(); ++i) {
        g_nodes[i].sort_available();
    }
    // 服务器的 history
    for (int l = 0; l < output.size(); ++l) {
        for (int i = 0; i < g_nodes.size(); ++i) {
            int sum = 0;
            for (int j = 0; j < g_users.size(); ++j) {
                sum += output[l][j][i];
            }
            g_nodes[i].pair_history.emplace_back(make_pair(l, sum));
        }
    }
    for (int i = 0; i < g_nodes.size(); ++i) {
        g_nodes[i].pair_history_unsorted = g_nodes[i].pair_history;
        g_nodes[i].get_95_pair();
    }
    // 客户交集矩阵
    vector<vector<Common> > common_users_mat(g_nodes.size(), vector<Common>(g_nodes.size(), Common()));
    for (int m = 0; m < g_nodes.size(); ++m) {
        for (int i = 0; i < g_nodes.size(); ++i) {
            if (m >= i) {
                vector<int> common_users;
                set_intersection(g_nodes[m].available.begin(), g_nodes[m].available.end(),
                                 g_nodes[i].available.begin(), g_nodes[i].available.end(),
                                 back_inserter(common_users));
                common_users_mat[m][i] = Common(common_users);
            } else {
                common_users_mat[m][i] = common_users_mat[i][m];
            }
        }
    }


    for (int lp = 0; lp < MAX_OPTIMIZE_LOOP; ++lp) {
        // 优化开始
        for (int i = 0; i < g_nodes.size(); ++i) {
            // 节点 1
            Node &now_node1 = g_nodes[i];
            int now_node1_idx = i;
            // 拿到 now_node 的 95 分位时间点，未排序的
            int now_node1_95_idx = now_node1.pair_percent_95.first;

            int now_node2_idx = 0;
            // solution1: 找到在该时间点上最远离 95 分位的服务器
//        int dis = -1;
//        for (int j = 0; j < g_nodes.size(); ++j) {
//            int num_95th = g_nodes[j].history[g_nodes[j].idx_95];
//            if (j != now_node1_idx) {
//                int num_min = g_nodes[j].history_unsorted[now_node1_95_idx];
//                if (dis < (num_95th - num_min) && (num_95th - num_min) > 0) {
//                    now_node2_idx = j;
//                    dis = num_95th - num_min;
//                }
//            }
//        }
            // solution2: 找到和 node1 共同用户最多的服务器
            now_node2_idx = 0;
            int num_common_user = 0;
            for (int j = 0; j < g_nodes.size(); ++j) {
                if (j != now_node1_idx) {
                    // 取两服务器的客户交集
                    if (num_common_user < common_users_mat[now_node1_idx][j].common_users.size()) {
                        now_node2_idx = j;
                        num_common_user = common_users_mat[now_node1_idx][j].common_users.size();
                    }
                }
            }
            if (now_node2_idx == now_node1_idx) continue;
            Node &now_node2 = g_nodes[now_node2_idx];

            // 拿到二维表 [ n_客户, n_服务器 ]
            vector<vector<int> > &now_time = output[now_node1_95_idx];

            // 取两服务器的客户交集
            vector<int> common_users = common_users_mat[now_node1_idx][now_node2_idx].common_users;

            // 减小 now_node1 的 95 分位成本
            for (int k = 0; k < common_users.size(); ++k) {

                int now_user_idx = common_users[k];
                int now_user_node1 = now_time[now_user_idx][now_node1_idx];

                int least = G_SUB_RATIO_LEFT * now_user_node1;
                int most = G_SUB_RATIO_RIGHT * now_user_node1;

                int delta = random(least, most);

                // delta 需要小于 node2 在当前时间节点下的剩余量
                int sum = now_node2.pair_history_unsorted[now_node1_95_idx].second;
                int left_node2 = now_node2.bandwidth - sum;

                if (delta > left_node2) {
                    continue;
                }

                // 交换 delta
                now_time[now_user_idx][now_node2_idx] += delta;
                now_time[now_user_idx][now_node1_idx] -= delta;
                now_node1.pair_history_unsorted[now_node1_95_idx].second -= delta;
                now_node2.pair_history_unsorted[now_node1_95_idx].second += delta;

                // 更新这两个服务器的 95 分位
                now_node1.pair_history = now_node1.pair_history_unsorted;
                now_node2.pair_history = now_node2.pair_history_unsorted;
                now_node1.get_95_pair();
                now_node2.get_95_pair();


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

vector<vector<vector<int> > > maximize_95plus() {
    // 输出
    vector<vector<vector<int> > > output(g_demand.size(), vector<vector<int> >(g_users.size(),
                                                                               vector<int>(g_nodes.size(), 0)));
    // 前 5% 的时间节点个数

    int num_95plus = g_demand.size() - g_demand.size() * 0.95;

    // 每个边缘节点 计算每个时刻 所有用户的总需求流量 [nodes size, timeline size] pair: time_index, asked
    vector<vector<pair<int, int>>> node_time_asked(g_nodes.size(),
                                                   vector<pair<int, int>>(g_demand.size(), make_pair(0, 0)));
    for (int i = 0; i < g_nodes.size(); ++i) {
        for (int j = 0; j < g_demand.size(); ++j) {
            int now_time_idx = j;
            int sum = 0;
            for (int k = 0; k < g_nodes[i].available.size(); ++k) {
                int now_user_idx = g_nodes[i].available[k];
                sum += g_demand[now_time_idx][now_user_idx];
            }
            node_time_asked[i][now_time_idx] = make_pair(now_time_idx, sum);
        }
    }

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
            // 每个节点的总需求按时间线从大到小排序
            vector<vector<pair<int, int> > > sorted_node_time_asked = node_time_asked;
//            sort(sorted_node_time_asked[i].begin(), sorted_node_time_asked[i].end(), Great);
            InsertionSort_form_big_to_small(sorted_node_time_asked[i], sorted_node_time_asked[i].size());
            // 当前边缘节点
            Node &now_node = g_nodes[i];
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
                    for (int k = 0; k < now_node.available.size(); ++k) {
                        int now_user_idx = g_users[now_node.available[k]].index;
                        float unit_flow =
                                g_demand[real_time_idx][now_user_idx] / g_users[now_user_idx].available.size();
                        unit_user_flow.emplace_back(make_pair(now_user_idx, unit_flow));
                    }
                    // 对其由大到小排序
                    sort(unit_user_flow.begin(), unit_user_flow.end(), Great);
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
                            node_time_asked[now_node.index][real_time_idx].second -= now_user_demand;
                            now_node.all_remain[real_time_idx] -= now_user_demand;
                            sorted_node_time_asked[now_node.index][j].second -= now_user_demand;
                            // 更新输出
                            output[real_time_idx][now_user_idx][now_node.index] += now_user_demand;
                            can_break = false;
                        }
                            // 放不下
                        else {
                            break;
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
    // 接之前的思路
    // TODO

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

int main() {

//    clock_t t1 = clock();

    freopen(SOLUTION_PATH.c_str(), "w", stdout);

    read_conf();

    read_qos();
    read_demand();
    read_bandwidth();

//    g_output = baseline_2();
//    g_output = baseline();
    g_output = maximize_95plus();
//    clock_t t3=clock();
//    double totaltime = (t3 - t1)*1.0 / CLOCKS_PER_SEC;
//    cout <<"执行时间:" <<totaltime<<"秒" << endl;
    // baseline(g_output);
//    optimize(g_output);
    prt(g_output);

    fclose(stdout);

//    clock_t t2 = clock();
//    double totaltime1 = (t2 - t1)*1.0 / CLOCKS_PER_SEC;
//    cout <<"执行时间:" <<totaltime1<<"秒" << endl;


    return 0;
}