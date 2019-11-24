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
std::string config::securitytoken {};
std::string config::apiversion;

bool config::isASandbox {false};
bool config::useFileForAttributeList {false};

std::map<std::string,std::vector<std::string>> config::excludedAttributesByObj;

const std::vector<config::tokenDesc> config::tokenDescriptions = {
    {config::token::DOMAIN, "_domain_:"},
    {config::token::CLIENTID, "_clientid_:"},
    {config::token::CLIENTSECRET, "_clientsecret_:"},
    {config::token::USERNAME, "_username_:"},
    {config::token::PASSWORD, "_password_:"},
    {config::token::SECURITYTOKEN, "_token_:"},
    {config::token::SOBJECT, "_object_:"},
    {config::token::ISPROD, "_isprod_:"},
    {config::token::USEFILEFORATTRLIST, "_usefileforattributelist_:"},
    {config::token::APIVERSION, "_apiversion_:"}
};
//
//  for test purpose
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
void config::getIsSandbox(const std::string& line) {
    size_t firstColon = line.find_first_of(':');
    if (firstColon != std::string::npos) {
        std::string value = line.substr(firstColon+1);
        if (value.compare("false") == 0)
            isASandbox = true;
        else if (value.compare("true") == 0)
            isASandbox = false;
    }
}
//
void config::getUseFileForAttributeList(const std::string&line) {
    size_t firstColon = line.find_first_of(':');
    if (firstColon != std::string::npos) {
        std::string value = line.substr(firstColon+1);
        if (value.compare("true") == 0)
            useFileForAttributeList = true;
        else if (value.compare("false") == 0)
            useFileForAttributeList = false;
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
        case token::APIVERSION:
            apiversion = value;
            break;
        case token::SECURITYTOKEN:
            securitytoken = value;
            break;
        case token::SOBJECT:
            updateExcludedAttributes(line);
            break;
        case token::ISPROD:
            getIsSandbox(line);
            break;
        case token::USEFILEFORATTRLIST:
            getUseFileForAttributeList(line);
            break;

        default:
            break;
    }
}
//
bool config::getConfig(const std::string filename) {
    std::ifstream configFile {filename};
    std::string currentLine;
    
    int nbline {0};
    
    while (getline(configFile,currentLine)) {
        nbline++;
        processLine(currentLine);
    }
    
    return (nbline > 0);
}
//
//
bool config::getExcludedAttributes(const std::string object, std::vector<std::string>& target) {
    auto it = excludedAttributesByObj.find(object);
    if (it == excludedAttributesByObj.end())
        return false;
    else {
        target = it->second;
        return true;
    }
}
//
//
void config::getAttributeList(const std::string dirname, const std::string object, std::vector<std::string>& target) {
    std::string filename = dirname + object;
    std::ifstream configFile {filename};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        target.push_back(currentLine);
    }

}
//
//
bool config::checkConfig() {

    bool status {true};
    
    if (clientid == "") {
        std::cerr << "config file: clientid parameter is mandatory" << std::endl;
        status = false;
     }
    if (clientsecret == "") {
        std::cerr << "config file: clientsecret parameter is mandatory" << std::endl;
        status = false;
     }
    if (domain == "") {
        if (isSandbox()) {
            std::cout << "domain not specified, assuming 'test.salesforce.com'" << std::endl;
            domain = "test.salesforce.com";
        }
        else {
            std::cout << "domain not specified, assuming 'login.salesforce.com'" << std::endl;
            domain = "login.salesforce.com";
        }
    }
    if (username == "") {
        std::cerr << "config file: username parameter is mandatory" << std::endl;
        status = false;
     }
    if (password == "") {
        std::cerr << "config file: password parameter is mandatory" << std::endl;
        status = false;
     }
    if (apiversion == "") {
        std::cerr << "config file: apiversion parameter is mandatory" << std::endl;
        status = false;
     }

    return status;
}
