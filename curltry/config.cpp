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
std::string config::clientid;        // REST connected app
std::string config::clientsecret;    // REST connected app secret
std::string config::domain;
std::string config::username;
std::string config::password;

const std::vector<config::tokenDesc> config::tokenDescriptions = {
    {config::token::DOMAIN, "_domain_:"},
    {config::token::CLIENTID, "_clientid_:"},
    {config::token::CLIENTSECRET, "_clientsecret_:"},
    {config::token::USERNAME, "_username_:"},
    {config::token::PASSWORD, "_password_:"},
    {config::token::SOBJECT, "_object_:"},
};

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
