//
// Created by cuiyo on 2022-03-14.
//
#pragma once
#include <iostream>
#include <vector>

using namespace std;

void logger_mat2d(const vector<vector<int> > mat2d) {
    for (const auto& vec : mat2d) {
        for (const auto& i : vec) {
            cout << i << " ";
        }
        cout << endl;
    }
}