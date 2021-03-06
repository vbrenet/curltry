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
    enum class token  {TOKENDOM, CLIENTID, CLIENTSECRET, USERNAME, PASSWORD, SOBJECT, ISPROD, USEFILEFORATTRLIST, APIVERSION, SECURITYTOKEN, CUSTOMER, DATEOUTPUT, UNKNOWN};
    struct tokenDesc {
        token theToken;
        std::string literal;
    };
    static const std::vector<tokenDesc> tokenDescriptions;
    static std::string clientid;        // REST connected app
    static std::string clientsecret;    // REST connected app secret
    static std::string domain;
    static std::string username;
    static std::string password;
    static std::string securitytoken;
    static std::string apiversion;
    static std::string customer;
    static std::string dateoutput;
    static bool isASandbox;
    static bool useFileForAttributeList;

    static std::map<std::string,std::vector<std::string>> excludedAttributesByObj;
//
    static void getIsSandbox(const std::string&);
    static void getUseFileForAttributeList(const std::string&);
    static void updateExcludedAttributes(const std::string&);
    static token getTokenValue(const std::string&, std::string&);
    static void processLine(const std::string&);
public:
    
    static bool getConfig(const std::string filename);
    static void getAttributeList(const std::string dirname, const std::string object, std::vector<std::string>&);
    //
    //  config accessors
    //
    static bool getExcludedAttributes(const std::string object, std::vector<std::string>&);
    static std::string& getClientId() {return clientid;};
    static std::string& getClientSecret() {return clientsecret;};
    static std::string& getDomain() {return domain;};
    static std::string& getUsername() {return username;};
    static std::string& getPassword() {return password;};
    static std::string& getSecurityToken() {return securitytoken;};
    static std::string& getApiVersion() {return apiversion;};
    static std::string& getCustomer() {return customer;};
    static std::string& getDateOutput() {return dateoutput;};

    static bool isSandbox() {return isASandbox;};
    static bool useFileForAttrList() {return useFileForAttributeList;}

    //
    static void printMap();
    static bool checkConfig();
};
#endif /* config_hpp */
