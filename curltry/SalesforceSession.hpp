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

#endif /* SalesforceSession_hpp */

class SalesforceSession {
private:
    static std::string connectedAppToken;
    static std::string domain;
public:
    static bool openSession(const std::string domain, const std::string client_id, const std::string client_secret, const std::string username, const std::string password);
    // accessors
    static const std::string& getConnectedAppToken() {return connectedAppToken;}
    static const std::string& getDomain() {return domain;}

};
