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
#include "config.hpp"
#include "globals.hpp"

bool injectionOrchestrator::execute(int nbrec) {
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password,config::getApiVersion(), credentials.userSecurityToken))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkInject::createJob(theObject.getName()))
        return false;

    // prepare records
    recordGenerator recgen (globals::workingDirectory + "/" + theObject.getName() + ".inject");
    
    char lf = 10;
    std::string content = recgen.getCsvHeader();
    content += lf;

    for (auto i=0; i != nbrec; i++) {
        content += recgen.getCsvRecord();
        content += lf;
    }
    
    std::ofstream csvlog {globals::workingDirectory + "/output.csv"};
    csvlog << content;

    // put records
    if (!bulkInject::addRecords(content))
        return false;
    
    // close job
    if (!bulkInject::closeJob())
        return false;

    return true;
}
