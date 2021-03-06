//
//  bulkQuery.hpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef bulkQuery_hpp
#define bulkQuery_hpp

#include <stdio.h>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include "config.hpp"

//
//  job info provided by Salesforce during job execution
//
struct jobStatusInfo {
    std::string status;
    std::string concurrencyMode;
    std::string createdDate;
    int numberBatchesQueued;
    int numberBatchesInProgress;
    int numberBatchesCompleted;
    int numberBatchesFailed;
    int numberBatchesTotal;
    int numberRecordsProcessed;
    int numberRetries;
    int numberRecordsFailed;
    int totalProcessingTime;
    int apiActiveProcessingTime;
    int apexProcessingTime;
    void print() const;     // print helper method (standard output)
};
//
//
//
struct batchInfo {
    std::string status; // info provided by Salesforce (e.g. "completed")
    bool isRead;        // true if results (read data) have already been provided by bulkQuery class
    std::string resultId; // result id used to retrieve actual result of the batch
    std::map<std::string,bool> resultMap;   // batch result ids, along with a bool stating if the result is read
    batchInfo(std::string s, bool b) : status{s}, isRead{b} {}
};
//
//  class bulkQuery: encapsulate bulk API calls for queries, both in PKChunking mode or not
//
class bulkQuery {
private:
    static bool firstTime;      // used by the callback providing data to libcurl
    static std::string body;    // buffer containing POST request bodies
    static std::string jobId;   // jobID of the bulkQuery session
    static std::string mainBatchId; // main batch id of the bulkQuery session, never completed in case of PK chunking
    static jobStatusInfo jobInfo;   // job info provided at the begining by the bulk API, related to the session's job
    static jobStatusInfo closedJobInfo;   // job info provided at the closing by the bulk API, related to the session's job
    static std::map<std::string,batchInfo> batches; // map of ids and batch info of each batch related to the job
    static bool pkchunking;     // true if we are in PK chunking mode
    
    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp); // callback called by libcurl to send data in POST requests
    static bool getJobStatus(); // get job status, using bulk API resource
    static bool getBatchResultId(const std::string& batchid, std::string& resultid); // get result id of a given batch
    static bool getBatchResultIdNew(const std::string& batchid, std::map<std::string,bool>&); // get result IDS of a given batch
    static bool getBatchResult(const std::string& batchid, const std::string& resultid, std::string& result);

    static void extractJobStatusInfo (const std::string&, jobStatusInfo&);  // helper method to extract job info
    static bool getBatchesInfo();   // populate the batches map by all batch ids and status, using bulk API resource
    static void extractBatchesInfo(const std::string&); // helper method to extract batch info
    static void extractBatchResults (const std::string& input, std::map<std::string,bool>& results);

public:
    static bool createJob(const std::string objectName, int chunksize); // first step to use the bulk API
    static bool addQuery(const std::string& query); // second step : create a query batch
    static bool waitCompletion(std::string& jobDate, long& totalNbRecordProcessed); // third step : wait for completion of batches
    static bool getResult(std::string& result, bool&, std::string& resultid);     // fourth step : get data result
    static bool closeJob();                         // fifth step : close the job
    static void setJobId(const std::string id) {jobId=id;pkchunking=true;};
};

#endif /* bulkQuery_hpp */
