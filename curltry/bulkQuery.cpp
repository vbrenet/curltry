//
//  bulkQuery.cpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <sstream>
#include "bulkQuery.hpp"
#include "bulkSession.hpp"
//
bool bulkQuery::firstTime;
std::string bulkQuery::body;
std::string bulkQuery::jobId;
//
//
extern size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
extern std::string extractXmlToken(const std::string& inputbuffer, const std::string& token);
//
size_t bulkQuery::read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    char *thedest = (char *)dest;
    if (bulkQuery::firstTime) {
        bulkQuery::firstTime = false;
        for (auto i=0; i < bulkQuery::body.size(); i++)
            thedest[i] = bulkQuery::body[i];
        return bulkQuery::body.size();
    }
    return 0;
}
//
//
//
bool bulkQuery::createJob(const std::string objectName, int chunksize) {
    
    std::stringstream ssbody;
    ssbody <<   "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    <<          "<jobInfo xmlns=\"http://www.force.com/2009/06/asyncapi/dataload\">\n"
    <<          "<operation>query</operation>\n"
    <<          "<object>" << objectName << "</object>\n"
    <<          "<concurrencyMode>Parallel</concurrencyMode>\n"
    <<          "<contentType>CSV</contentType>\n"
    <<          "</jobInfo>\n";
    
    body = ssbody.str();
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, bulkSession::getServerUrl().c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        if (chunksize > 0) {
            list = curl_slist_append(list, ("Sforce-Enable-PKChunking: chunksize=" + std::to_string(chunksize)).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
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
    else {
        std::cerr << "bulkQuery::createJob : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkQuery::createJob : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "bulkQuery::createJob : http error: " << http_code << std::endl;
        return false;
    }
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;
    
    jobId = extractXmlToken(readBuffer, "<id>");

    return true;
}
