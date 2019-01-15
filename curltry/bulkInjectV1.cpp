//
//  bulkInjectV1.cpp
//  curltry
//
//  Created by Vincent Brenet on 15/01/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include "bulkInjectV1.hpp"

#include <curl/curl.h>
#include <iostream>
#include <sstream>

#include "bulkSession.hpp"

//
bool bulkInjectV1::firstTime = true;
std::string bulkInjectV1::body;
size_t bulkInjectV1::sizeleft;
int bulkInjectV1::currindex;

std::string bulkInjectV1::jobId;
//
extern size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::extractJsonId
//      extractJobId from the response received to create job request
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string bulkInjectV1::extractJobId(const std::string & input) {
    std::string id {};
    size_t index = input.find("<id>");
    if (index != std::string::npos) {
        id = input.substr(index+4,18);
    }
    return id;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::read_callback
//      callback used by libcurl to fill bodies
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t bulkInjectV1::read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    char *thedest = (char *)dest;
    if (bulkInjectV1::firstTime) {
        bulkInjectV1::firstTime = false;
        for (auto i=0; i < bulkInjectV1::body.size(); i++)
            thedest[i] = bulkInjectV1::body[i];
        return bulkInjectV1::body.size();
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::read_callback_inject
//      callback used by libcurl to fill bodies
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
size_t bulkInjectV1::read_callback_inject(void *dest, size_t size, size_t nmemb, void *userp)
{
    char *thedest = (char *)dest;
    size_t maxtocopy = size*nmemb;
    
    if (sizeleft > 0) {
        size_t sizeToCopy = sizeleft;
        if (sizeToCopy > maxtocopy)
            sizeToCopy = maxtocopy;
        for (auto j=0; j < sizeToCopy; j++) {
            thedest[j] = body[currindex];
            currindex++;
        }
        sizeleft -= sizeToCopy;
        return sizeToCopy;
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::createJob - create a new bulk API job
//      objectName : name of sObject to be queried (e.g. "account")
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool bulkInjectV1::createJob(const std::string objectName) {
    
//    <?xml version="1.0" encoding="UTF-8"?>
//    <jobInfo
//    xmlns="http://www.force.com/2009/06/asyncapi/dataload">
//    <operation>insert</operation>
//    <object>Account</object>
//    <contentType>CSV</contentType>
//    </jobInfo>
    
    std::stringstream ssbody;
    ssbody << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    ssbody << "<jobInfo\n xmlns=\"http://www.force.com/2009/06/asyncapi/dataload\">\n";
    ssbody << "<operation>insert</operation>\n";
    ssbody << "<object>" << objectName  << "</object>\n";
    ssbody << "<contentType>CSV</contentType>\n";
    ssbody << "</jobInfo>\n";

    body = ssbody.str();
    std::cout << "bulkInjectV1::createJob : body: " << body << std::endl;

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        std::cout << "bulkInjectV1::createJob : url: " << bulkSession::getServerUrl()+"/job" << std::endl;

        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
//        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        std::cout << "bulkInjectV1::createJob : sessionid: " << bulkSession::getSessionId() << std::endl;

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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkInject::addRecords -
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool bulkInjectV1::addRecords(const std::string& content){
    
    body = content;
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getInjectUrl()+"/jobs/ingest/"+jobId+"/batches").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/csv");
        list = curl_slist_append(list, "Accept: application/json");
        list = curl_slist_append(list, ("Authorization: Bearer " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        /* Now specify we want to PUT data */
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);
        
        /* we want to use our own read function */
        std::cout << "strlen(body.c_str): " << strlen(body.c_str()) << std::endl;
        firstTime = true;
        sizeleft = body.size();
        currindex = 0;
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback_inject);
        
        /* pointer to pass to our read function */
        
        curl_easy_setopt(curl, CURLOPT_READDATA, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body.c_str()));
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 201) {
            std::cerr << "bulkInject::addRecords : http error: " << http_code << std::endl;
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkInject::addRecords : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkInject::addRecords : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    return true;
}
//
//
bool bulkInjectV1::closeJob() {
    std::stringstream ssbody;
    ssbody << "{\n";
    ssbody << "\t\"state\" : \"UploadComplete\"\n";
    ssbody << "}";
    
    body = ssbody.str();
    
    std::cout << "closeJob, body: \n" << body << std::endl;
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getInjectUrl()+"/jobs/ingest/"+jobId).c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json; charset=UTF-8");
        list = curl_slist_append(list, "Accept: application/json");
        list = curl_slist_append(list, ("Authorization: Bearer " + bulkSession::getSessionId()).c_str());
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        /* Now specify we want to PATCH  */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
        
        /* we want to use our own read function */
        firstTime = true;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        
        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, ssbody.str().c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(ssbody.str().c_str()));
        
        //        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkInject::closeJob : http error: " << http_code << std::endl;
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkInject::closeJob : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkInject::closeJob : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    std::cout << "Final job status : " << readBuffer << std::endl;
    
    return true;
    
}

