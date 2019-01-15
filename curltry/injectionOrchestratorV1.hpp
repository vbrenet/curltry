//
//  injectionOrchestratorV1.hpp
//  curltry
//
//  Created by Vincent Brenet on 15/01/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef injectionOrchestratorV1_hpp
#define injectionOrchestratorV1_hpp

#include <stdio.h>
#include <string>
#include "sObject.hpp"
#include "sessionCredentials.hpp"

class injectionOrchestratorV1 {
private:
    sObject theObject ;
    sessionCredentials credentials;
    
public:
    injectionOrchestratorV1(const std::string sObjectName, const sessionCredentials& c): theObject {sObjectName}, credentials{c} {}
    bool execute(int);
    
};

#endif /* injectionOrchestratorV1_hpp */
