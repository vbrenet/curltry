//
//  bulkInject.cpp
//  curltry
//
//  Created by Vincent Brenet on 19/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <curl/curl.h>
#include <iostream>
#include <sstream>
#include "bulkInject.hpp"
#include "bulkSession.hpp"

//
bool bulkInject::firstTime = true;
std::string bulkInject::body;
std::string bulkInject::jobId;
//
extern size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::extractJsonId
//      extractJobId from the response received to create job request
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string bulkInject::extractJobId(const std::string & input) {
    //    bulkInject::createJob : received buffer: {"id":"7505800000Ltf5GAAR","operation":"insert","object":"Account","createdById":"00558000000jlbAAAQ","createdDate":"2018-11-19T13:19:49.000+0000","systemModstamp":"2018-11-19T13:19:49.000+0000","state":"Open","concurrencyMode":"Parallel","contentType":"CSV","apiVersion":41.0,"contentUrl":"services/data/v41.0/jobs/ingest/7505800000Ltf5GAAR/batches","lineEnding":"CRLF","columnDelimiter":"COMMA"}
    std::string id {};
    size_t index = input.find("\"id\":\"");
    if (index != std::string::npos) {
        id = input.substr(index+5+1,18);
    }
    return id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::read_callback
//      callback used by libcurl to fill POST bodies
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t bulkInject::read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    char *thedest = (char *)dest;
    if (bulkInject::firstTime) {
        bulkInject::firstTime = false;
        for (auto i=0; i < bulkInject::body.size(); i++)
            thedest[i] = bulkInject::body[i];
        return bulkInject::body.size();
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::createJob - create a new bulk API job
//      objectName : name of sObject to be queried (e.g. "account")
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool bulkInject::createJob(const std::string objectName) {
    
//    {
//        "object" : "Contact",
//        "contentType" : "CSV",
//        "operation" : "insert",
//        "lineEnding": "CRLF"
//    }
    
    
    std::stringstream ssbody;
    ssbody << "{\n";
    ssbody << "\"object\" : \"" << objectName << "\",\n";
    ssbody << "\"contentType\" : \"CSV\",\n";
    ssbody << "\"operation\" : \"insert\",\n";
    ssbody << "\"lineEnding\": \"CRLF\"";
    ssbody << "}\n";
    
    body = ssbody.str();
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        
        ///services/data/vXX.X/jobs/ingest
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getInjectUrl()+"/jobs/ingest").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json; charset=UTF-8");
        list = curl_slist_append(list, "Accept: application/json");
        list = curl_slist_append(list, ("Authorization: Bearer " + bulkSession::getSessionId()).c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        /* we want to use our own read function */
        firstTime = true;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        
        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, ssbody.str().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(ssbody.str().c_str()));
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkInject::createJob : http error: " << http_code << std::endl;
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkInject::createJob : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkInject::createJob : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    std::cout << "bulkInject::createJob : received buffer: " << readBuffer << std::endl;
    
    jobId = extractJobId(readBuffer);
    std::cout << "bulkInject::createJob : id: " << jobId << std::endl;

    return true;
}
