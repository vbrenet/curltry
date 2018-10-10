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

#endif /* bulkSession_hpp */

class bulkSession {
private:
    static std::string sessionId;   // returned at login
    static std::string serverUrl;   // returned at login
public:
    static bool firstTime;
    static std::string body;

    static bool openBulkSession(bool isSandbox, const std::string username, const std::string password);
    //
    //  accessors
    static std::string &getSessionId() {return sessionId;}
    static std::string &getServerUrl() {return serverUrl;}
};
