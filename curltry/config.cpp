//
//  config.cpp
//  curltry
//
//  Created by Vincent Brenet on 16/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "config.hpp"
#include <fstream>
//
//
void config::processLine(const std::string& line) {
    
}

//
void config::getConfig(const std::string filename) {
    std::ifstream configFile {filename};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        processLine(currentLine);
    }

}
