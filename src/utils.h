//
// Created by cuiyo on 2022-03-14.
//
#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

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
// 从小到大
bool Less(const pair<int, int>& s1, const pair<int, int>& s2) {
    return s1.second < s2.second;
}
// 从大到小
bool Great(const pair<int, int>& s1, const pair<int, int>& s2) {
    return s1.second > s2.second;
}

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
    vector<int> available;
    vector<int> history;
    vector<int> all_remain;
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
    }

    const Node& operator=(const Node& node) {
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
        this->pair_history = node.pair_history;
        this->pair_percent_95 = node.pair_percent_95;
        this->all_remain = node.all_remain;

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

    const Common& operator=(const Common& common) {
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

    vector<int> temp;
    for (int i = 0; i < num; ++i) {
        temp.push_back(i);
    }

    random_shuffle(temp.begin(), temp.end());

    return temp;
}