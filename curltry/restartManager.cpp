//
//  restartManager.cpp
//  curltry
//
//  Created by Vincent Brenet on 04/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <fstream>
#include "restartManager.hpp"

bool restartManager::restartMode = false;
std::vector<std::string> restartManager::batchids;

extern std::string workingDirectory;
//
//
void restartManager::init() {
    if (!restartMode)
        return;
    
    // if restart mode then initialize batchids vector from batchids file
    std::ifstream batchesFile {workingDirectory + "/batchids"};
    std::string currentLine;
    
    while (getline(batchesFile,currentLine)) {
        batchids.push_back(currentLine);
    }
    
    batchesFile.close();
}
//
//
void restartManager::saveBatchId(const std::string batchid) {
    std::ofstream ofs {workingDirectory + "/batchids", std::ofstream::app};
    ofs << batchid << std::endl;
    ofs.close();
}
