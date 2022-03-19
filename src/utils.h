//
// Created by cuiyo on 2022-03-14.
//
#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include "math.h"
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
void InsertionSort(vector<int>& a, int len)
{
    for (int j=1; j<len; j++)
    {
        int key = a[j];
        int i = j-1;
        while (i>=0 && a[i]>key)
        {
            a[i+1] = a[i];
            i--;
        }
        a[i+1] = key;
    }
}
class Node {
public:
    string name;
    int index;
    int bandwidth;
    int remain;
    int now_used;
    int percent_95;
    vector<int> available;
    vector<int> history;
    Node(string name, int index) {
        this->name = trim(name);
        this->index = index;
    }
    int get_95(){
        InsertionSort(history,history.size());
        
        int index=ceil(history.size()*0.95)-1;
        percent_95=history[index];
        return percent_95;
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
    for (const auto& vec : mat2d) {
        for (const auto& i : vec) {
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

// 随机数排列 num: 总长度
vector<int> randperm(int num) {

    vector<int> temp;
    for (int i = 0; i < num; ++i) {
        temp.push_back(i);
    }

    random_shuffle(temp.begin(), temp.end());

    return temp;
}