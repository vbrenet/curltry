//
//  packages.cpp
//  curltry
//
//  Created by Vincent Brenet on 02/12/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <fstream>

#include "packages.hpp"

std::map <std::string, std::string> packages::packageMap;
//
//
void packages::processLine (const std::string &inputline) {
    
    size_t semicolon = inputline.find_first_of(':');
    
    if (semicolon != std::string::npos) {
        std::string key = inputline.substr(0,semicolon);
        std::string value = inputline.substr(semicolon+1);
        packageMap[key] = value;
    }
}
//
//
void packages::initPackagesFromFile (const std::string &inputfile) {
    std::ifstream packagesFile {inputfile};
    
    std::string currentLine;
    
    while (getline(packagesFile,currentLine)) {
        // skip comments
        if (currentLine[0] == '#')
            continue;
        processLine(currentLine);
    }

    packagesFile.close();
}
//
std::string packages::getPackageName(const std::string inputvalue) {
    auto it = packageMap.find(inputvalue);
    if (it == packageMap.end())
        return inputvalue;
    else
        return it->second;
}
