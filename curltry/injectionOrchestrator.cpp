//
//  injectionOrchestrator.cpp
//  curltry
//
//  Created by Vincent Brenet on 16/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "injectionOrchestrator.hpp"
#include "bulkInject.hpp"
#include "bulkSession.hpp"

bool injectionOrchestrator::execute() {
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkInject::createJob(theObject.getName()))
        return false;

    return true;
}
