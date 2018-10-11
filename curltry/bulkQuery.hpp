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

class bulkQuery {
private:
    static bool firstTime;
    static std::string body;
    static std::string jobId;

    static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp);

public:
    static bool createJob(const std::string objectName, int chunksize);
    static bool addQuery(const std::string& query);
    static bool waitCompletion();
    static bool getResult(std::string& result);
};

#endif /* bulkQuery_hpp */
