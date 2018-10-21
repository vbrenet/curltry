//
//  threadBucket.hpp
//  curltry
//
//  Created by Vincent Brenet on 21/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef threadBucket_hpp
#define threadBucket_hpp

#include <stdio.h>
#include <string>
#include <map>
//
//  all info needed to execute safely a batch get result into a thread
//
class threadBucket {
private:
public:
    std::string buffer; //  buffer to analyze
    std::map<std::string,long> attributeCounters {};    // results
};


#endif /* threadBucket_hpp */
