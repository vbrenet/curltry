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

struct jobStatusInfo {
    std::string status;
    std::string concurrencyMode;
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
    void print() const;
};
//
//
struct batchInfo {
    std::string status;
    bool isRead;
    batchInfo(std::string s, bool b) : status{s}, isRead{b} {}
};
//
//
class bulkQuery {
private:
    static bool firstTime;
    static std::string body;
    static std::string jobId;
    static std::string mainBatchId;
    static jobStatusInfo jobInfo;
    static std::map<std::string,batchInfo> batches; // id, infos
    static bool pkchunking;
    
    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp);
    static bool getJobStatus();
    static void extractJobStatusInfo (const std::string&, jobStatusInfo&);
    static bool getBatchesInfo();
    static void extractBatchesInfo(const std::string&);

public:
    static bool createJob(const std::string objectName, int chunksize);
    static bool addQuery(const std::string& query);
    static bool waitCompletion();
    static bool getResult(std::string& result);
};

#endif /* bulkQuery_hpp */
