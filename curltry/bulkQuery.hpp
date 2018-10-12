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
class bulkQuery {
private:
    static bool firstTime;
    static std::string body;
    static std::string jobId;
    static std::string mainBatchId;
    static jobStatusInfo jobInfo;
    
    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp);
    static bool getJobStatus();
    static void extractJobStatusInfo (const std::string&, jobStatusInfo&);

public:
    static bool createJob(const std::string objectName, int chunksize);
    static bool addQuery(const std::string& query);
    static bool waitCompletion(bool isPKChunking);
    static bool getResult(std::string& result);
};

#endif /* bulkQuery_hpp */
