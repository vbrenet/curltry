//
//  bulkQuery.cpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include "bulkQuery.hpp"
#include "bulkSession.hpp"
//
bool bulkQuery::firstTime = true;
std::string bulkQuery::body;
std::string bulkQuery::jobId;
std::string bulkQuery::mainBatchId;
jobStatusInfo bulkQuery::jobInfo;

//
//
extern size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
extern std::string extractXmlToken(const std::string& inputbuffer, const std::string& token);
//
//
//
void jobStatusInfo::print() const {
    std::cout << "***jobStatusInfo:" << std::endl;
    std::cout << "status: " <<  status <<std::endl;
    std::cout << "numberBatchesTotal: " <<  numberBatchesTotal <<std::endl;
    std::cout << "numberBatchesQueued: " <<  numberBatchesQueued <<std::endl;
    std::cout << "numberBatchesInProgress: " <<  numberBatchesInProgress <<std::endl;
    std::cout << "numberBatchesCompleted: " <<  numberBatchesCompleted <<std::endl;
    std::cout << "numberBatchesFailed: " <<  numberBatchesFailed <<std::endl;
    std::cout << "concurrencyMode: " <<  concurrencyMode <<std::endl;
    std::cout << "numberRetries: " <<  numberRetries <<std::endl;
    std::cout << "numberRecordsProcessed: " <<  numberRecordsProcessed <<std::endl;
    std::cout << "numberRecordsFailed: " <<  numberRecordsFailed <<std::endl;
    std::cout << "apiActiveProcessingTime: " <<  apiActiveProcessingTime <<std::endl;
    std::cout << "apexProcessingTime: " <<  apexProcessingTime <<std::endl;
    std::cout << "totalProcessingTime: " <<  totalProcessingTime <<std::endl;
}
//
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
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/csv; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        if (chunksize > 0) {
            list = curl_slist_append(list, ("Sforce-Enable-PKChunking: chunksize=" + std::to_string(chunksize)).c_str());
        }
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
    if (http_code >= 400) {
        std::cerr << "bulkQuery::createJob : http error: " << http_code << std::endl;
        return false;
    }
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;
    
    jobId = extractXmlToken(readBuffer, "<id>");

    return true;
}
//
//
//
bool bulkQuery::addQuery(const std::string& query){
    
    body = query;
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId+"/batch").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/csv; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        /* we want to use our own read function */
        firstTime = true;
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        
        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body.c_str()));
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::addQuery : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkQuery::addQuery : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400) {
        std::cerr << "bulkQuery::addQuery : http error: " << http_code << std::endl;
        return false;
    }
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;
    
    mainBatchId = extractXmlToken(readBuffer, "<id>");
    
    return true;
}
//
//
//
bool bulkQuery::waitCompletion(bool isPKChunking) {
    
    bool terminated {false};
    
    while (!terminated) {
        
        getJobStatus();
        
        jobInfo.print();
        
        int processed = jobInfo.numberBatchesTotal - jobInfo.numberBatchesQueued - jobInfo.numberBatchesInProgress;
        
        if (isPKChunking)
            terminated = ((jobInfo.numberBatchesTotal - processed) == 1);
        else
            terminated = ((jobInfo.numberBatchesTotal - processed) == 0);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    };
    
    return true;
}
//
//
//
void bulkQuery::extractJobStatusInfo (const std::string& input, jobStatusInfo& target) {
    target.status = extractXmlToken(input,"<state>");
    target.concurrencyMode = extractXmlToken(input,"<concurrencyMode>");
    target.numberBatchesQueued = std::stoi(extractXmlToken(input,"<numberBatchesQueued>"));
    target.numberBatchesInProgress = std::stoi(extractXmlToken(input,"<numberBatchesInProgress>"));
    target.numberBatchesCompleted = std::stoi(extractXmlToken(input,"<numberBatchesCompleted>"));
    target.numberBatchesFailed = std::stoi(extractXmlToken(input,"<numberBatchesFailed>"));
    target.numberBatchesTotal = std::stoi(extractXmlToken(input,"<numberBatchesTotal>"));
    target.numberRecordsProcessed = std::stoi(extractXmlToken(input,"<numberRecordsProcessed>"));
    target.numberRetries = std::stoi(extractXmlToken(input,"<numberRetries>"));
    target.numberRecordsFailed = std::stoi(extractXmlToken(input,"<numberRecordsFailed>"));
    target.totalProcessingTime = std::stoi(extractXmlToken(input,"<totalProcessingTime>"));
    target.apiActiveProcessingTime = std::stoi(extractXmlToken(input,"<apiActiveProcessingTime>"));
    target.apexProcessingTime = std::stoi(extractXmlToken(input,"<apexProcessingTime>"));
}
//
//
bool bulkQuery::getJobStatus() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId).c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/csv; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::getJobStatus : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkQuery::getJobStatus : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code >= 400) {
        std::cerr << "bulkQuery::getJobStatus : http error: " << http_code << std::endl;
        return false;
    }

    extractJobStatusInfo(readBuffer, jobInfo);
    
    return true;
}

