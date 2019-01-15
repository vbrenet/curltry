//
//  bulkInjectV1.hpp
//  curltry
//
//  Created by Vincent Brenet on 15/01/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef bulkInjectV1_hpp
#define bulkInjectV1_hpp

#include <stdio.h>
#include <string>
#include <cstring>

class bulkInjectV1 {
private:
    static bool firstTime;      // used by the callback providing data to libcurl
    static std::string body;    // buffer containing POST request bodies
    static size_t sizeleft;
    static int currindex;
    
    static std::string jobId;   // jobID of the bulkQuery session
    
    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp); // callback called by libcurl to send data in POST requests
    static size_t read_callback_inject(void *dest, size_t size, size_t nmemb, void *userp); // callback called by libcurl to send data in POST requests
    
    static std::string extractJobId(const std::string &);
    //    static bool getJobStatus(); // get job status, using bulk API resource
    
public:
    static bool createJob(const std::string objectName); // first step to use the bulk API
    static bool addRecords(const std::string& content);
    static bool closeJob();                         // third step : close the job
};


#endif /* bulkInjectV1_hpp */
