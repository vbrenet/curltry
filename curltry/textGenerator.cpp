//
//  textGenerator.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <fstream>
#include "textGenerator.hpp"

std::vector<std::string> textGenerator::words;
std::string textGenerator::text;
//
//
void textGenerator::init() {
    std::srand((unsigned int)std::time((std::time_t *)nullptr));
    
    std::ifstream configFile {"/usr/share/dict/web2"};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        words.push_back(currentLine);
    }
    
}
//
//
const std::string& textGenerator::gettext() {
    text = words[rand()%words.size()] + ' ' + words[rand()%words.size()] + ' ' + words[rand()%words.size()] + ' ' + words[rand()%words.size()] + ' ' + words[rand()%words.size()];
    return text;
}
