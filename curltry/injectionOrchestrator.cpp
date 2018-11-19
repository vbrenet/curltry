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
#include "recordGenerator.hpp"

bool injectionOrchestrator::execute() {
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkInject::createJob(theObject.getName()))
        return false;

    // prepare records
    recordGenerator recgen ("/Users/vbrenet/Documents/Pocs/curltry/" + theObject.getName() + "inject");
    
    char cr = 13;
    char lf = 10;
    std::string content = recgen.getCsvHeader();
    content += cr;
    content += lf;

    for (auto i=1; i != 10; i++) {
        content += recgen.getCsvRecord();
        content += cr;
        content += lf;
    }
    
    // put records
    if (!bulkInject::addRecords(content))
        return false;
    
    // close job
    if (!bulkInject::closeJob())
        return false;

    return true;
}
