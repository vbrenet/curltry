//
//  bulkSession.cpp
//  curltry
//
//  Created by Vincent Brenet on 08/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "bulkSession.hpp"
#include <iostream>
#include <sstream>

extern size_t WriteCallback(void *, size_t , size_t , void *);
//
//  open a bulk session with Salesforce
//
bool bulkSession::openBulkSession(bool isSandbox, const std::string username, const std::string password) {
    bool result {false};
    
    std::stringstream ssurl;
    ssurl << "https://" << ((isSandbox) ? "test." : "login.") << "salesforce.com/services/Soap/u/39.0";
    
    std::stringstream ssbody;
    ssbody << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    << "<env:Envelope xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n"
    << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    << "xmlns:env=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    << "<env:Body>\n"
    << "<n1:login xmlns:n1=\"urn:partner.soap.sforce.com\">\n"
    << "<n1:username>" << username << "</n1:username>\n"
    << "<n1:password>" << password << "</n1:password>\n"
    <<  "</n1:login>\n"
    <<  "</env:Body>\n"
    <<  "</env:Envelope>\n";
    
    struct WriteThis {
        const char *readptr;
        size_t sizeleft;
    } wt;
    
    wt.readptr = ssbody.str().c_str();
    wt.sizeleft = strlen(wt.readptr);
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ssurl.str().c_str());
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS,urlParameters.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        
        curl_easy_cleanup(curl);
    }
    else
        return false;
    
    if (res != CURLE_OK) {
        std::cerr << "openBulkSession : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "openBulkSession : http error: " << http_code << std::endl;
        return false;
    }
    

    
    
    return result;
}
