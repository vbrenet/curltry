//
//  buckets.hpp
//  curltry
//
//  Created by Vincent Brenet on 25/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef buckets_hpp
#define buckets_hpp

#include <string>
#include <map>

class buckets {
private:
    static const std::string theBuckets[];
    static std::map<int,std::string> bucketMap;
    static bool mapEnabled;
    
    static void processBucketLine(const std::string &line);

public:
    static std::string getBucket(double d);
    static void initBucketsFromFile(const std::string &inputfile);
};

#endif /* buckets_hpp */
