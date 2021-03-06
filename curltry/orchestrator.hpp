//
//  orchestrator.hpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef orchestrator_hpp
#define orchestrator_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "sObject.hpp"
#include "sessionCredentials.hpp"

class orchestrator {
private:
    sObject theObject ;
    sessionCredentials credentials;
    
    bool describeObject();
public:
    orchestrator(const std::string sObjectName, const std::string where, const sessionCredentials& c): theObject {sObjectName, where}, credentials{c} {}
    orchestrator(const std::string sObjectName, const sessionCredentials& c): theObject {sObjectName}, credentials{c} {}
    bool getObjectInfo();
    bool execute(int chunksize);
    bool getResultFromJobId(const std::string&);
};


#endif /* orchestrator_hpp */
