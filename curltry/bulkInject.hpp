//
//  bulkInject.hpp
//  curltry
//
//  Created by Vincent Brenet on 19/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef bulkInject_hpp
#define bulkInject_hpp

#include <stdio.h>
#include <string>

class bulkInject {
private:
    static bool firstTime;      // used by the callback providing data to libcurl
    static std::string body;    // buffer containing POST request bodies
    static std::string jobId;   // jobID of the bulkQuery session
    
    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp); // callback called by libcurl to send data in POST requests
    static bool getJobStatus(); // get job status, using bulk API resource
    
public:
    static bool createJob(const std::string objectName); // first step to use the bulk API
    static bool waitCompletion();                   // third step : wait for completion of batches
    static bool getResult(std::string& result);     // fourth step : get data result
    static bool closeJob();                         // fifth step : close the job
};


#endif /* bulkInject_hpp */
