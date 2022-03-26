//
// Created by cuiyo on 2022-03-14.
//
#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <unordered_set>
#include <sys/timeb.h>
#include <climits>
#include <unistd.h>
#include <ctime>

using namespace std;

// 字符串两边的\r\n
string trim(string str) {
    if (str.empty()) {
        return "";
    }
    str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    return str;
}

void cout_details(int node_idx, int user_idx, int time_idx, int now_demand) {
    cout << endl << "node: " << node_idx << " ,user: " << user_idx << " ,time: " << time_idx
         << " total demand:" << now_demand << endl;
}

void InsertionSort(vector<int> &a, int len) {
    for (int j = 1; j < len; j++) {
        int key = a[j];
        int i = j - 1;
        while (i >= 0 && a[i] > key) {
            a[i + 1] = a[i];
            i--;
        }
        a[i + 1] = key;
    }
}

void InsertionSort_form_big_to_small(vector<int> &a, int len) {
    for (int j = 1; j < len; j++) {
        int key = a[j];
        int i = j - 1;
        while (i >= 0 && a[i] < key) {
            a[i + 1] = a[i];
            i--;
        }
        a[i + 1] = key;
    }
}

// index, value
void InsertionSort(vector<pair<int, int> > &a, int len) {
    for (int j = 1; j < len; j++) {
        auto key = a[j];
        int i = j - 1;
        while (i >= 0 && a[i].second > key.second) {
            a[i + 1] = a[i];
            i--;
        }
        a[i + 1] = key;
    }
}

// index, value
void InsertionSort_form_big_to_small(vector<pair<int, int> > &a, int len) {
    for (int j = 1; j < len; j++) {
        auto key = a[j];
        int i = j - 1;
        while (i >= 0 && a[i].second < key.second) {
            a[i + 1] = a[i];
            i--;
        }
        a[i + 1] = key;
    }
}


void change_idx(vector<pair<int, int> > &a, int len, int where) {
    int temp = where + 1;
    for (int l = where; l < a.size(); ++l) {
        if (a[l].second < a[temp].second) {
            swap(a[l], a[temp]);
            temp++;
        } else {
            break;
        }
    }
}

// 从小到大
bool Less(const pair<int, int> &s1, const pair<int, int> &s2) {
    return s1.second < s2.second;
}

// 从大到小
bool Great(const pair<int, int> &s1, const pair<int, int> &s2) {
    return s1.second > s2.second;
}

/*
arrat：数组 ， n:数组的大小;  target:查找的数据； 返回target所在数组的下标
*/
int binarySearch2(vector<pair<int, int>> array, int n, int target) {
    int low = 0, high = n, middle = 0;

    while (low < high) {

        middle = (low + high) / 2;
        if (target == array[middle].first) {
            return middle;
        } else if (target < array[middle].first) {
            high = middle;
        } else if (target > array[middle].first) {
            low = middle + 1;
        }
    }
    return -1;
}

class cmp {
public:
    bool operator()(const pair<int, int> &i, const pair<int, int> &j) {
        return i.first > j.first;
    }
};


class Node {
public:
    string name = "";
    int index = 0;
    int bandwidth = 0;
    int remain = 0;
    int remain_after = 0;
    int now_used = 0;
    int percent_95 = 0;
    pair<int, int> pair_percent_95;
    int idx_95 = 0;
    int total_used = 0;
    int use95 = 0;
    vector<int> available;
    vector<int> history;
    vector<int> all_remain;
    vector<vector<int> > time_not_available;
    vector<pair<int, int> > pair_history;
    vector<pair<int, int> > pair_history_unsorted;

    Node() {

    }

    void sort_available() {
        sort(available.begin(), available.end());
    }

    Node(string name, int index) {
        this->name = trim(name);
        this->index = index;
    }

    Node(const Node &node) {
        this->name = node.name;
        this->index = node.index;
        this->remain = node.remain;
        this->remain_after = node.remain_after;
        this->now_used = node.now_used;
        this->bandwidth = node.bandwidth;
        this->available = node.available;
        this->percent_95 = node.percent_95;
        this->idx_95 = node.idx_95;
        this->history = node.history;
        this->total_used = node.total_used;
        this->pair_history_unsorted = node.pair_history_unsorted;
        this->all_remain = node.all_remain;
        this->pair_history = node.pair_history;
        this->pair_percent_95 = node.pair_percent_95;
        this->time_not_available = node.time_not_available;
        this->use95 = node.use95;
    }

    Node &operator=(const Node &node) {
        this->name = node.name;
        this->index = node.index;
        this->remain = node.remain;
        this->remain_after = node.remain_after;
        this->now_used = node.now_used;
        this->bandwidth = node.bandwidth;
        this->available = node.available;
        this->percent_95 = node.percent_95;
        this->idx_95 = node.idx_95;
        this->history = node.history;
        this->total_used = node.total_used;
        this->time_not_available = node.time_not_available;
        this->pair_history_unsorted = node.pair_history_unsorted;
        this->pair_history = node.pair_history;
        this->pair_percent_95 = node.pair_percent_95;
        this->all_remain = node.all_remain;
        this->use95 = node.use95;
        return *this;
    }


    int get_95() {
        InsertionSort(history, history.size());
        idx_95 = ceil(history.size() * 0.95) - 1;
        percent_95 = history[idx_95];
        return percent_95;
    }

    pair<int, int> get_95_pair() {
//        InsertionSort(pair_history, pair_history.size());
        sort(pair_history.begin(), pair_history.end(), Less);
        idx_95 = ceil(pair_history.size() * 0.95) - 1;
        pair_percent_95 = pair_history[idx_95];
        return pair_percent_95;
    }

};


class User {
public:
    string name;
    int index;
    vector<int> available;
    vector<vector<int> > time_not_available;

    User(string name, int index) {
        this->name = trim(name);
        this->index = index;
    }
};

class Common {
public:
    Common() {}

    Common(vector<int> common_users) {
        this->common_users = common_users;
    }

    vector<int> common_users;

    Common(const Common &common) {
        this->common_users = common.common_users;
    }

    const Common &operator=(const Common &common) {
        this->common_users = common.common_users;
    }
};

class Time {
public:
    string name;
    int index;

    Time(string name, int index) {
        this->name = trim(name);
        this->index = index;
    }
};

void logger_mat2d(const vector<vector<int> > mat2d) {
    for (const auto &vec : mat2d) {
        for (const auto &i : vec) {
            cout << i << " ";
        }
        cout << endl;
    }
}

void logger_node(Node node, int used) {
    cout << "<" << node.name << "," << used << ">";
}

void logger_node(Node node) {
    cout << "<" << node.name << "," << node.now_used << ">";
}

void logger_nodes(vector<Node> nodes) {
    if (nodes.size() == 0) {
        return;
    }
    int n = nodes.size();
    for (int i = 0; i < n - 1; ++i) {
        logger_node(nodes[i]);
        cout << ",";
    }
    logger_node(nodes[n - 1]);
}

void logger_line(User user, vector<Node> nodes) {
    cout << "\n";
    cout << user.name << ":";
    logger_nodes(nodes);
}

void logger_line1(User user, vector<Node> nodes) {
    cout << user.name << ":";
    logger_nodes(nodes);
}

void logger_standard(User user, vector<Node> nodes) {
    cout << user.name << ":";
    if (nodes.size() == 0) {
        cout << endl;
        return;
    }
    int n = nodes.size();
    for (int i = 0; i < n - 1; ++i) {
        cout << "<" << nodes[i].name << "," << nodes[i].now_used << ">";
        cout << ",";
    }
    cout << "<" << nodes[n - 1].name << "," << nodes[n - 1].now_used << ">" << endl;
}

void logger_nodes(map<int, Node> nodes) {
    if (nodes.size() == 0) {
        return;
    }
    int n = nodes.size();
    vector<int> idx;
    for (auto kv: nodes) {
        idx.emplace_back(kv.first);
    }

    for (int i = 0; i < n - 1; ++i) {
        logger_node(nodes[idx[i]]);
        cout << ",";
    }

    logger_node(nodes[idx[idx.size() - 1]]);
}

void logger_line(User user, map<int, Node> nodes) {
    cout << "\n";
    cout << user.name << ":";
    logger_nodes(nodes);
}

void logger_line1(User user, map<int, Node> nodes) {
    cout << user.name << ":";
    logger_nodes(nodes);
}


// 随机数排列 num: 总长度
vector<int> randperm(int num) {
    timeb timeSeed;
    ftime(&timeSeed);
    srand(timeSeed.time * 1000 + timeSeed.millitm);
    vector<int> temp;
    for (int i = 0; i < num; ++i) {
        temp.push_back(i);
    }

    random_shuffle(temp.begin(), temp.end());

    return temp;
}

long long compute_cost(vector<vector<vector<int> > > output) {

    int num_nodes = output[0][1].size();
    int num_users = output[0].size();
    vector<vector<int> > history(num_nodes, vector<int>(output.size(), 0));
    for (int l = 0; l < output.size(); ++l) {
        for (int i = 0; i < num_nodes; ++i) {
            int sum = 0;
            for (int j = 0; j < num_users; ++j) {
                sum += output[l][j][i];
            }
            history[i][l] = sum;
        }
    }
    int idx_95 = ceil(output.size() * 0.95) - 1;
    long long cost = 0;
    for (int i = 0; i < num_nodes; ++i) {
        sort(history[i].begin(), history[i].end());
        cost += history[i][idx_95];
    }

    return cost;
}

void delay(int time) {
    clock_t now = clock();
    while (clock() - now < time);
}

