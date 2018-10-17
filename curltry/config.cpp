//
//  config.cpp
//  curltry
//
//  Created by Vincent Brenet on 16/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "config.hpp"
#include <iostream>
#include <fstream>
//
//
std::string config::clientid;        // REST connected app
std::string config::clientsecret;    // REST connected app secret
std::string config::domain;
std::string config::username;
std::string config::password;
std::map<std::string,std::vector<std::string>> config::excludedAttributesByObj;

const std::vector<config::tokenDesc> config::tokenDescriptions = {
    {config::token::DOMAIN, "_domain_:"},
    {config::token::CLIENTID, "_clientid_:"},
    {config::token::CLIENTSECRET, "_clientsecret_:"},
    {config::token::USERNAME, "_username_:"},
    {config::token::PASSWORD, "_password_:"},
    {config::token::SOBJECT, "_object_:"},
};
//
void config::printMap() {
    std::cout << "excludedAttributesByObj :" << std::endl;
    for (auto it = excludedAttributesByObj.begin(); it != excludedAttributesByObj.end(); it++) {
        std::cout << "object : " << it->first << std::endl;
        for (auto itv = it->second.begin(); itv != it->second.end(); itv++)
            std::cout << *itv << " " ;
        std::cout << std::endl;
    }
}
//
void config::updateExcludedAttributes(const std::string& line) {
    size_t firstColon = line.find_first_of(':');
    if (firstColon != std::string::npos) {
        size_t secondColon = line.find_first_of(':',firstColon+1);
        if (secondColon != std::string::npos) {
            std::string sObject = line.substr(firstColon+1,secondColon-firstColon-1);
            std::string attribute = line.substr(secondColon+1);
            auto it =excludedAttributesByObj.find(sObject);
            if (it != excludedAttributesByObj.end()) {
                it->second.push_back(attribute);
            } else {
                std::vector<std::string> v {attribute};
                excludedAttributesByObj.insert(std::pair<std::string,std::vector<std::string>>({sObject},{v}));
            }
        }
    }
}
//
//
config::token config::getTokenValue(const std::string& line, std::string& value) {
    for (auto it = tokenDescriptions.begin(); it != tokenDescriptions.end(); it++) {
        if (line.find(it->literal) != std::string::npos) {
            value = line.substr(it->literal.size());
            return it->theToken;
        }
    }
    return token::UNKNOWN;
}

//
void config::processLine(const std::string& line) {
    if (line[0] == '#') return; // skip comments
    std::string value;
    token theToken = getTokenValue(line, value);
    switch (theToken) {
        case token::DOMAIN:
            domain = value;
            break;
        case token::CLIENTID:
            clientid = value;
            break;
        case token::CLIENTSECRET:
            clientsecret = value;
            break;
        case token::USERNAME:
            username = value;
            break;
        case token::PASSWORD:
            password = value;
            break;
        case token::SOBJECT:
            updateExcludedAttributes(line);
            break;
        default:
            break;
    }
}
//
void config::getConfig(const std::string filename) {
    std::ifstream configFile {filename};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        processLine(currentLine);
    }
}
