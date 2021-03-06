//
//  bulkQuery.cpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
//
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <functional>
#include "bulkQuery.hpp"
#include "bulkSession.hpp"
#include "restartManager.hpp"
#include "utils.hpp"
#include "globals.hpp"
//
bool bulkQuery::firstTime = true;
bool bulkQuery::pkchunking = true;
std::string bulkQuery::body;
std::string bulkQuery::jobId;
std::string bulkQuery::mainBatchId;
jobStatusInfo bulkQuery::jobInfo;
jobStatusInfo bulkQuery::closedJobInfo;
std::map<std::string,batchInfo> bulkQuery::batches {};
//
//
//  jobStatusInfo::print
//      print a jobStatusInfo instance content on the standard output
//
void jobStatusInfo::print() const {
    std::cout << "***jobStatusInfo:" << std::endl;
    std::cout << "status: " <<  status <<std::endl;
    std::cout << "createdDate: " <<  createdDate <<std::endl;
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::read_callback
//      callback used by libcurl to fill POST bodies
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::createJob - create a new bulk API job
//      objectName : name of sObject to be queried (e.g. "account")
//      chunksize : size of chunks - if >0, PKChunking is used, otherwise no
//      theformat : xml or csv
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool bulkQuery::createJob(const std::string objectName, int chunksize) {
    
    pkchunking = (chunksize > 0);
    
    std::stringstream ssbody;
    ssbody <<   "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    <<          "<jobInfo xmlns=\"http://www.force.com/2009/06/asyncapi/dataload\">\n"
    <<          "<operation>query</operation>\n"
    <<          "<object>" << objectName << "</object>\n"
    <<          "<concurrencyMode>Parallel</concurrencyMode>\n"
    <<          "<contentType>" << "CSV" << "</contentType>\n"
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
        list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
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
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkQuery::createJob : http error: " << http_code << std::endl;
            return false;
        }
        
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
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;
    
    jobId = extractXmlToken(readBuffer, "<id>");

    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::addQuery - set the query clause of the existing job
//      query : the query clause, which must respect bulk API limitations
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkQuery::addQuery : http error: " << http_code << std::endl;
            return false;
        }

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
    
    
    //std::cout << "Received buffer: " << readBuffer << std::endl;
    
    mainBatchId = extractXmlToken(readBuffer, "<id>");
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::waitCompletion - wait completion of all batches of the job, AND THEN get batches info, chiefly batch ids
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool bulkQuery::waitCompletion(std::string& jobDate, long& totalNbRecordProcessed) {
    
    bool terminated {false};
    
    while (!terminated) {
        
        getJobStatus();
        
        jobInfo.print();
        
        jobDate = jobInfo.createdDate;
        totalNbRecordProcessed = jobInfo.numberRecordsProcessed;
        
        int processed = jobInfo.numberBatchesTotal - jobInfo.numberBatchesQueued - jobInfo.numberBatchesInProgress;
        
        if (pkchunking)
            terminated = (((jobInfo.numberBatchesTotal != 1) && (jobInfo.numberBatchesQueued == 0) && (jobInfo.numberBatchesInProgress == 0)) || (jobInfo.numberBatchesCompleted == jobInfo.numberBatchesTotal));
        else
            terminated = ((jobInfo.numberBatchesTotal - processed) == 0);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    };
    
    if (pkchunking) {
        if (batches.size() == 0)
            if (!getBatchesInfo())
                return false;
        
        // print info
        for (auto it= batches.begin(); it != batches.end(); ++it)
            std::cout << it->first << " => " << it->second.status << std::endl;
    }
    
    return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::extractBatchesInfo - populate the batches map with batch info provided by the bulk API
//      input : buffer containing batches info
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bulkQuery::extractBatchesInfo (const std::string& input) {
    bool terminated {false};
    size_t curr = 0;
    while (!terminated) {
        // positionnement sur next batch info
        size_t next = input.find("<batchInfo>", curr);
        if (next == std::string::npos)
            terminated = true;
        else {
            curr = next + 11;
            std::string batchId = extractXmlToken(input, next, "<id>");
            std::string status = extractXmlToken(input, next, "<state>");
            if (batchId.compare(mainBatchId) != 0)
                batches.insert(std::pair<std::string,batchInfo>{batchId,{status,false}});
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bulkQuery::extractBatchResults -
//      input : buffer containing batch result info
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bulkQuery::extractBatchResults (const std::string& input, std::map<std::string,bool>& results) {
    bool terminated {false};
    size_t curr = 0;
    while (!terminated) {
        // positionnement sur next batch info
        size_t next = input.find("<result>", curr);
        if (next == std::string::npos)
            terminated = true;
        else {
            curr = next + 11;
            std::string batchId = extractXmlToken(input, next, "<result>");
            results.insert(std::pair<std::string,bool>{batchId,false});
        }
    }
}
//
//
//
void bulkQuery::extractJobStatusInfo (const std::string& input, jobStatusInfo& target) {
    target.status = extractXmlToken(input,"<state>");
    target.concurrencyMode = extractXmlToken(input,"<concurrencyMode>");
    target.createdDate = extractXmlToken(input, "<createdDate>");
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
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId).c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkQuery::getJobStatus : http error: " << http_code << std::endl;
            return false;
        }

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

    if (globals::veryverbose) {
        std::cout << "**Job Status raw buffer**" << std::endl;
        std::cout << readBuffer << std::endl;
    }
        
    extractJobStatusInfo(readBuffer, jobInfo);
    
    return true;
}
//
//
bool bulkQuery::getBatchesInfo() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId+"/batch").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "bulkQuery::getBatchesInfo : http error: " << http_code << std::endl;
            return false;
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::getBatchesInfo : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkQuery::getBatchesInfo : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    

    extractBatchesInfo(readBuffer);
    
    return true;
}
//
// get result id of a given batch
//
bool bulkQuery::getBatchResultId(const std::string& batchid, std::string& resultid) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    //https://instance.salesforce.com/services/async/39.0/job/jobId/batch/batchId/result
    if (curl) {
        std::cout << bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result" << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        //list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        if (res != CURLE_OK) {
            std::cerr << "bulkQuery::getBatchResultId : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::getBatchResultId : curl_easy_init failure" << std::endl;
        return false;
    }
    
    resultid = extractXmlToken(readBuffer, "<result>");
//    std::cout << "getBatchResultId : " << readBuffer << std::endl;
    return true;

}
//
//
//
bool bulkQuery::getBatchResultIdNew(const std::string& batchid, std::map<std::string,bool>& results) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    //https://instance.salesforce.com/services/async/39.0/job/jobId/batch/batchId/result
    if (curl) {
        std::cout << bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result" << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result").c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        //list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        if (res != CURLE_OK) {
            std::cerr << "bulkQuery::getBatchResultId : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::getBatchResultId : curl_easy_init failure" << std::endl;
        return false;
    }
    
    extractBatchResults (readBuffer, results);

    return true;
}
//
// get result  of a given batch
//
bool bulkQuery::getBatchResult(const std::string& batchid, const std::string& resultid, std::string& result) {
    CURL *curl;
    CURLcode res;
    //std::string readBuffer;
    
    curl = curl_easy_init();
    
    if (curl) {
        std::cout << bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result/"+resultid << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, (bulkSession::getServerUrl()+"/job/"+jobId+"/batch/"+batchid+"/result/"+resultid).c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, "Accept-Encoding: gzip");

        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());
        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */
        
        if (res != CURLE_OK) {
            std::cerr << "bulkQuery::getBatchResultId : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
            return false;
            }
        curl_easy_cleanup(curl);

    }
        
    else {
        std::cerr << "bulkQuery::getBatchResultId : curl_easy_init failure" << std::endl;
        return false;
    }

    //result = readBuffer;
    
    return true;
    
}
//
//
//
bool bulkQuery::getResult(std::string& result, bool& allResultsRead, std::string& resultid) {
    bool moreResult {false};

    if (pkchunking) {
        for (auto it=batches.begin(); it!=batches.end(); ++it) {
            if (!it->second.isRead) {
                // call read batch
                if (!getBatchResultIdNew(it->first, it->second.resultMap))
                    return false;
                
                allResultsRead = true;
                for (auto it2 = it->second.resultMap.begin(); it2!= it->second.resultMap.end(); ++it2){
                    if (it2->second == false) {
                        allResultsRead = false;
                        if (!restartManager::isRestartMode() || !restartManager::isAlreadyRead(it2->first)) {
                            result.clear();
                            if (!getBatchResult(it->first, it2->first, result))
                                return false;
                        }
                        
                        // save result id in case of a future restart
                        resultid = it2->first;
                        
                        it2->second = true;
                        break;
                    }
                }

                // set isread to true
                if (allResultsRead)
                    it->second.isRead = true;
                moreResult = true;
                break;
            }
        }
    }
    
    return moreResult;
}
//
//
//
bool bulkQuery::closeJob() {
    std::stringstream ssbody;
    ssbody <<   "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    <<          "<jobInfo xmlns=\"http://www.force.com/2009/06/asyncapi/dataload\">\n"
    <<          "<state>Closed</state>\n"
    <<          "</jobInfo>\n";
    
    body = ssbody.str();
    
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
        list = curl_slist_append(list, "Content-Type: application/xml; charset=UTF-8");
        list = curl_slist_append(list, ("X-SFDC-Session: " + bulkSession::getSessionId()).c_str());

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
            std::cerr << "bulkQuery::closeJob : http error: " << http_code << std::endl;
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "bulkQuery::closeJob : curl_easy_init failure" << std::endl;
        return false;
    }
    
    if (res != CURLE_OK) {
        std::cerr << "bulkQuery::closeJob : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    extractJobStatusInfo(readBuffer, closedJobInfo);

    std::cout << "Final job status : " << std::endl;
    closedJobInfo.print();
    
    return true;

}
