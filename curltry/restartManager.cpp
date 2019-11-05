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
    if (!restartMode) {
        // if not restart mode, clear batchids file
        std::ofstream batchesFile {workingDirectory + "/batchids", std::ofstream::out};
        batchesFile.close();
        return;
    }
    
    // if restart mode then initialize batchids vector from batchids file
    std::ifstream batchesFile {workingDirectory + "/batchids"};
    std::string currentLine;
    
    while (getline(batchesFile,currentLine)) {
        batchids.push_back(currentLine);
    }
    
    batchesFile.close();
    
    std::cout << "RESTART MODE" << std::endl;
    std::cout << "batchs results already read:" << std::endl;
    for (auto it=batchids.begin(); it!=batchids.end(); ++it)
        std::cout << *it << std::endl;
}
//
//
void restartManager::saveBatchId(const std::string batchid) {
    std::ofstream ofs {workingDirectory + "/batchids", std::ofstream::app};
    ofs << batchid << std::endl;
    ofs.close();
}
//
//
bool restartManager::isAlreadyRead(const std::string theid)  {
    auto it = std::find(batchids.begin(), batchids.end(), theid);
    std::cout << "isAlreadyRead ? " << theid << std::endl;
    std::cout << (it != batchids.end() ? "Yes" : "No") << std::endl;
    return (it != batchids.end());
}
