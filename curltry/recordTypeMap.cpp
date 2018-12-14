//
//  recordTypeMap.cpp
//  curltry
//
//  Created by Vincent Brenet on 14/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <fstream>
#include "recordTypeMap.hpp"
//
//
//
recordTypeMap::recordTypeMap(const std::string filename) {
    /*
    ID                  NAME
    ──────────────────  ──────────────
    01258000000QfQqAAK  france catalog
    01258000000QfQvAAK  spain catalog
     */
    std::ifstream thefile {filename};
    std::string currentLine;
    
    while (getline(thefile,currentLine)) {
        if (currentLine.find("012") != std::string::npos) {
            std::string id = currentLine.substr(0,18);
            size_t begin = currentLine.find_first_not_of(' ',18);
            if (begin != std::string::npos) {
                std::string name = currentLine.substr(begin);
                recordTypes.insert(std::pair<std::string,std::string>({id},{name}));
            }
        }
    }

}
//
//
//
std::string recordTypeMap::getnamebyid(const std::string id) {
    std::string result;
    auto it = recordTypes.find(id);
    if (it == recordTypes.end())
        result = "unknown";
    else
        result = it->second;
    return result;
}
