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
    enum class token  {DOMAIN, CLIENTID, CLIENTSECRET, USERNAME, PASSWORD, SOBJECT, UNKNOWN};
    struct tokenDesc {
        token theToken;
        std::string literal;
    };
    static const std::vector<tokenDesc> tokenDescriptions;
//
    static std::map<std::string,std::vector<std::string>> excludedAttributesByObj;
    
    static void updateExcludedAttributes(const std::string&);
    static token getTokenValue(const std::string&, std::string&);
    static void processLine(const std::string&);
public:
    static std::string clientid;        // REST connected app
    static std::string clientsecret;    // REST connected app secret
    static std::string domain;
    static std::string username;
    static std::string password;
    
    static void getConfig(const std::string filename);
    //
    //  config accessors
    //
    static bool getExcludedAttributes(const std::string object, std::vector<std::string>&);
    static std::string& getClientId() {return clientid;};
    static std::string& getClientSecret() {return clientsecret;};
    static std::string& getDomain() {return domain;};
    static std::string& getUsername() {return username;};
    static std::string& getPassword() {return password;};
    //
    static void printMap();
};
#endif /* config_hpp */
