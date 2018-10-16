//
//  config.hpp
//  curltry
//
//  Created by Vincent Brenet on 16/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef config_hpp
#define config_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <vector>

class config {
private:
    static std::map<std::string,std::vector<std::string>> excludedAttributesByObj;
    
    static void processLine(const std::string&);
public:
    static std::string clientid;        // REST connected app
    static std::string clientsecret;    // REST connected app secret
    static std::string domain;
    static std::string username;
    static std::string password;
    
    static void getConfig(const std::string filename);
    static bool getExcludedAttributes(const std::string object, std::vector<std::string>&);
};
#endif /* config_hpp */
