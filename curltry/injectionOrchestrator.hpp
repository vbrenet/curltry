//
//  injectionOrchestrator.hpp
//  curltry
//
//  Created by Vincent Brenet on 16/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef injectionOrchestrator_hpp
#define injectionOrchestrator_hpp

#include <stdio.h>
#include <string>
#include "sObject.hpp"
#include "sessionCredentials.hpp"

class injectionOrchestrator {
private:
    sObject theObject ;
    sessionCredentials credentials;

public:
    injectionOrchestrator(const std::string sObjectName, const sessionCredentials& c): theObject {sObjectName}, credentials{c} {}
    bool execute(int);

};


#endif /* injectionOrchestrator_hpp */
