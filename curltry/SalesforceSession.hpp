//
//  SalesforceSession.hpp
//  curltry
//
//  Created by Vincent Brenet on 03/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef SalesforceSession_hpp
#define SalesforceSession_hpp

#include <stdio.h>
#include <string>
#include "sessionCredentials.hpp"

#endif /* SalesforceSession_hpp */

class SalesforceSession {
private:
    static std::string connectedAppToken;
    static std::string domain;
    
    static std::string extractToken(const std::string buffer);
    static std::string extractDomain(const std::string buffer);

public:
    static bool openSession(const std::string , const std::string , const std::string , const std::string , const std::string, const std::string securitytoken);
    static bool openSession(const sessionCredentials&);

    // accessors
   static const std::string& getConnectedAppToken() {return connectedAppToken;}
   static const std::string& getDomain() {return domain;}

};
