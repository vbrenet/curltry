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
#include "sObject.hpp"

class orchestrator {
private:
    sObject theObject;
public:
    orchestrator(std::string sObjectName);
    bool getObjectInfo();
    bool execute(int chunsize);
    bool getResult(std::string& result);
};


#endif /* orchestrator_hpp */
