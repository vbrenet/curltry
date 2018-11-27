//
//  SalesforceSession.cpp
//  curltry
//
//  Created by Vincent Brenet on 03/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <string>
#include <curl/curl.h>

#include "SalesforceSession.hpp"

std::string SalesforceSession::connectedAppToken;
std::string SalesforceSession::domain;

extern size_t WriteCallback(void *, size_t , size_t , void *);
//
//  extract token to be used to authenticate further calls
//
std::string SalesforceSession::extractToken(const std::string buffer) {
    size_t beginindex = buffer.find("access_token") + 15;
    size_t endindex = buffer.find('"', beginindex);
    
    return buffer.substr(beginindex,endindex-beginindex);
}
//
//  open connected app session
//
bool SalesforceSession::openSession(const std::string thedomain, const std::string client_id, const std::string client_secret, const std::string username, const std::string password) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    domain = thedomain;
    
    std::string urlParameters = "grant_type=password&client_id=" + client_id + "&client_secret=" + client_secret + "&username=" + username + "&password=" + password;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + domain + "/services/oauth2/token").c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,urlParameters.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    else
        return false;
    
    if (res != CURLE_OK) {
        std::cerr << "openSession : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "openSession : http error: " << http_code << std::endl;
        return false;
    }
    
   connectedAppToken = SalesforceSession::extractToken(readBuffer);
    
    return true;
}
//
//  open connected app session
//
bool SalesforceSession::openSession(const sessionCredentials &credentials) {
    return openSession(credentials.domain,
                       credentials.clientId,
                       credentials.clientSecret,
                       credentials.username,
                       credentials.password);
}
