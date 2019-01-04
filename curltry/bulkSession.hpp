//
//  bulkSession.hpp
//  curltry
//
//  Created by Vincent Brenet on 08/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef bulkSession_hpp
#define bulkSession_hpp

#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <cstring>

#endif /* bulkSession_hpp */

class bulkSession {
private:
    static std::string sessionId;   // returned at login
    static std::string serverUrl;   // calculated at login
    static std::string injectUrl;   // calculated at login
    static void processResponse(const std::string&);
public:
    static bool firstTime;
    static std::string body;

    static bool openBulkSession(bool isSandbox, const std::string username, const std::string password);
    //
    //  accessors
    static std::string &getSessionId() {return sessionId;}
    static std::string &getServerUrl() {return serverUrl;}
    static std::string &getInjectUrl() {return injectUrl;}

};
