//
//  corpNameGenerator.cpp
//  curltry
//
//  Created by Vincent Brenet on 16/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <fstream>
#include "corpNameGenerator.hpp"

std::vector<std::string> corpNameGenerator::names;
std::string corpNameGenerator::corpname;
//
//
void corpNameGenerator::init() {
     std::srand((unsigned int)std::time((std::time_t *)nullptr));
    
    std::ifstream configFile {"/usr/share/dict/propernames"};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        names.push_back(currentLine);
    }

}
//
//
const std::string& corpNameGenerator::getName() {
    int i = rand() % names.size();
    int j = rand() % names.size();
    int k = rand() % names.size();

    corpname = names[i] + " " + names[j] + " " + names[k] + " Inc.";
    
    return corpname;
}
