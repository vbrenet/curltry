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
#include "globals.hpp"
#include "utils.hpp"

std::string bulkSession::sessionId;
std::string bulkSession::serverUrl;
std::string bulkSession::injectUrl;
std::string bulkSession::apiversion;

//
//
bool bulkSession::firstTime {true};
std::string bulkSession::body;
//
//
static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    char *thedest = (char *)dest;
    if (bulkSession::firstTime) {
        bulkSession::firstTime = false;
        for (auto i=0; i < bulkSession::body.size(); i++)
            thedest[i] = bulkSession::body[i];
        return bulkSession::body.size();
    }
    return 0;
}
//
//
void bulkSession::processResponse(const std::string& response) {
    std::string serverUrlToken = extractXmlToken(response, "<serverUrl>");
    // received : https://vbrlight-dev-ed.my.salesforce.com/services/Soap/u/39.0/00D58000000ZzE0
    // target : https://vbrlight-dev-ed.my.salesforce.com/services/async/39.0
    size_t lastslash = serverUrlToken.find_last_of('/');
    serverUrlToken.erase(lastslash);
    lastslash = serverUrlToken.find_last_of('/');
    std::string version = serverUrlToken.substr(lastslash+1);
    size_t soap = serverUrlToken.find("/Soap");
    serverUrl = serverUrlToken.substr(0,soap) + "/async/" + version;
    injectUrl = serverUrlToken.substr(0,soap) + "/data/v" + apiversion;

    sessionId = extractXmlToken(response, "<sessionId>");
}

//
//  open a bulk session with Salesforce
//
bool bulkSession::openBulkSession(bool isSandbox, const std::string username, const std::string password, const std::string theapiversion, const std::string securitytoken) {
    bool result {true};
    
    apiversion = theapiversion;
    
    std::stringstream ssurl;
    ssurl << "https://" << ((isSandbox) ? "test." : "login.") << "salesforce.com/services/Soap/u/" << apiversion;
    
    if (globals::verbose)
        std::cout << "session open url: " << ssurl.str() << std::endl;
    
    std::stringstream ssbody;
    ssbody << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    << "<env:Envelope xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n"
    << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    << "xmlns:env=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    << "<env:Body>\n"
    << "<n1:login xmlns:n1=\"urn:partner.soap.sforce.com\">\n"
    << "<n1:username>" << username << "</n1:username>\n"
    << "<n1:password>" << password << securitytoken << "</n1:password>\n"
    <<  "</n1:login>\n"
    <<  "</env:Body>\n"
    <<  "</env:Envelope>\n";
    
    body = ssbody.str();
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ssurl.str().c_str());
        
        //std::cout << "URL: " << ssurl.str() << std::endl;
        //std::cout << "BODY size: " << strlen(ssbody.str().c_str());
        //std::cout << "\nBODY: \n" << ssbody.str() << std::endl;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "SOAPAction: login");
        list = curl_slist_append(list, "Content-Type: text/xml; charset=UTF-8");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        
        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, ssbody.str().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(ssbody.str().c_str()));
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */

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
    if (http_code >= 400) {
        std::cerr << "openBulkSession : http error: " << http_code << std::endl;
        return false;
    }
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;

    processResponse(readBuffer);
    
    if (globals::veryverbose) {
        std::cout <<  "sessionid: " << sessionId << std::endl;
        std::cout <<  "serverurl: " << serverUrl << std::endl;
        std::cout <<  "injecturl: " << injectUrl << std::endl;
    }

    return result;
}
