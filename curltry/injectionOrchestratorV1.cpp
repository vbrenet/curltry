//
//  injectionOrchestratorV1.cpp
//  curltry
//
//  Created by Vincent Brenet on 15/01/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include "injectionOrchestratorV1.hpp"
#include "bulkInjectV1.hpp"
#include "bulkSession.hpp"
#include "recordGenerator.hpp"

bool injectionOrchestratorV1::execute(int nbrec) {
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkInjectV1::createJob(theObject.getName()))
        return false;
    
    // prepare records
//    recordGenerator recgen ("/Users//Documents/Pocs/curltry/" + theObject.getName() + ".inject");
//
//    char lf = 10;
//    std::string content = recgen.getCsvHeader();
//    content += lf;
//
//    for (auto i=0; i != nbrec; i++) {
//        content += recgen.getCsvRecord();
//        content += lf;
//    }
//
//    std::ofstream csvlog {"/Users//Documents/Pocs/curltry/output.csv"};
//    csvlog << content;
//
//    // put records
//    if (!bulkInject::addRecords(content))
//        return false;
//
//    //    std::this_thread::sleep_for(std::chrono::milliseconds(15000));
//
//    // close job
//    if (!bulkInject::closeJob())
//        return false;
    
    return true;
}
