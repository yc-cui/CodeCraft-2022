//
// Created by cuiyo on 2022-03-14.
//
#pragma once
#include <iostream>
#include <vector>

using namespace std;

class Node {
public:
    string name;
    int index;
    int bandwidth;
    int remain;
    int now_used;
    Node(string name, int index) {
        this->name = name;
        this->index = index;
    }
};

class User {
public:
    string name;
    int index;
    vector<int> available;
    User(string name, int index) {
        this->name = name;
        this->index = index;
    }
};

class Time {
public:
    string name;
    int index;
    Time(string name, int index) {
        this->name = name;
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
    int n = nodes.size();
    for (int i = 0; i < n - 1; ++i) {
        logger_node(nodes[i]);
        cout << ",";
    }
    logger_node(nodes[n - 1]);
}

void logger_line(User user, vector<Node> nodes) {
    cout << user.name << ":";
    logger_nodes(nodes);
    cout << endl;
}