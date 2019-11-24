//
//  sessionCredentials.hpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef sessionCredentials_hpp
#define sessionCredentials_hpp

#include <string>

struct sessionCredentials {
    bool isSandbox;
    std::string domain;
    std::string username;
    std::string password;
    std::string clientId;
    std::string clientSecret;
    std::string userSecurityToken;
    
    sessionCredentials(bool sandbox, const std::string d, const std::string theClientId, const std::string theClientSecret, const std::string u, const std::string p, const std::string st) : isSandbox{sandbox}, domain {d}, username{u}, password{p}, userSecurityToken{st}, clientId{theClientId}, clientSecret{theClientSecret} {}
};

#endif /* sessionCredentials_hpp */
