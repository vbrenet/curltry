//
//  orchestrator.cpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include "orchestrator.hpp"
#include "SalesforceSession.hpp"
#include "bulkSession.hpp"
#include "bulkQuery.hpp"
#include "config.hpp"
#include "restartManager.hpp"
//
extern bool getDescribeAttributesBuffer(const std::string objName, std::string& buffer);
extern std::string workingDirectory;
extern bool verbose;

//
//
bool orchestrator::describeObject() {
    std::string rawAttributeList;
    std::vector<std::string> excludedAttributes {};
    config::getExcludedAttributes(theObject.getName(), excludedAttributes);
    
    if (config::useFileForAttrList()) {
        std::vector<std::string> attrList;
        config::getAttributeList(workingDirectory + "/", theObject.getName(), attrList);
        if (attrList.size() > 0) {
            for (auto it = attrList.begin(); it != attrList.end(); it++) {
                theObject.addAttribute({*it,false});
            }
        }
        return true;
    }
    
    // launch describe request (produce a buffer)
    if (!getDescribeAttributesBuffer(theObject.getName(), rawAttributeList))
        return false;
    
    
    //std::cout << "raw describe object:\n" << rawAttributeList << std::endl;
    
    // parse each attribute in the buffer
    size_t beginFields = rawAttributeList.find("\"fields\":");
    if (beginFields != std::string::npos) {
        // chercher       "name":"curren__c","nameField":
        bool terminated {false};
        size_t offset = beginFields;
        do {
            size_t beginName = rawAttributeList.find("\"name\":",offset);
            if (beginName != std::string::npos) {
                bool excluded {false};
                size_t endName = rawAttributeList.find(",\"nameField\":",beginName);
                if (endName != std::string::npos) {
                    offset = endName+13;
                    // search type
                    size_t beginType = rawAttributeList.find("\"type\":",offset);
                    if (beginType != std::string::npos) {
                        size_t endType = rawAttributeList.find(",\"unique\":",beginType);
                        if (endType != std::string::npos) {
                            std::string attributeType = rawAttributeList.substr(beginType+7+1,endType-beginType-7-2);
                            if ((attributeType.compare("address") == 0) || (attributeType.compare("location") ==0))
                                excluded = true;
                        }   // end endType found
                    }   // end beginType found
                    // also check if the attribute is excluded in the configuration
                    std::string theAttribute = rawAttributeList.substr(beginName+7+1,endName-beginName-7-2);
                    for (auto itv=excludedAttributes.begin(); itv != excludedAttributes.end(); itv++) {
                        if (theAttribute.compare(*itv) == 0) {
                            excluded = true;
                            break;
                        }
                    }
                    
                    // get the custom attribute
                    bool isCustom {false};
                    size_t begincustom = rawAttributeList.rfind("\"custom\"", endName);
                    if (begincustom != std::string::npos) {
                        size_t endcustom = rawAttributeList.find("\"defaultValue\"", begincustom);
                        if (endcustom != std::string::npos) {
                            size_t begintrue = rawAttributeList.rfind ("true", begincustom);
                            if (begintrue != std::string::npos && begintrue < endcustom) {
                                isCustom = true;
                            }
                        }
                    } // end begincustom found
                    theObject.addAttribute({rawAttributeList.substr(beginName+7+1,endName-beginName-7-2),excluded, isCustom});
                } // end endName found
                else {
                    terminated = true;
                }
            } // end beginName found
            else
                terminated = true;
        } while (terminated == false);
    }
    
    // initialize record types
    if (!theObject.initializeRecordTypes()) {
        std::cerr << "orchestrator::describeObject : initializeRecordTypes error" << std::endl;
    }

    return true;
}
//
//
bool orchestrator::getObjectInfo() {
    // open REST session
    if (!SalesforceSession::openSession(credentials))
        return false ;

    // describe object (get all attributes)
    if (!describeObject())
        return false;
    
    //theObject.print();
    std::string thequery = theObject.makeAllAttributeQuery();
    
    // if restart mode, initialize counters from csvResult file
    if (restartManager::isRestartMode()) {
        theObject.initializeAttributeCounters(workingDirectory + "/result" + theObject.getName() + ".csv");
    }
    
    //
    if (verbose) {
        std::cout << "the query:" << std::endl;
        std::cout << thequery << std::endl;
    }
    
    return true;
};
//
//
bool orchestrator::execute(int chunksize) {
    long totalRecords {0};

    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion()))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkQuery::createJob(theObject.getName(), chunksize, config::getFormat()))
        return false;

    // bulkQuery::addQuery(const std::string& query)
    if (!bulkQuery::addQuery(theObject.makeAllAttributeQuery()))
        return false;

    // bulkQuery::waitCompletion()
    if (!bulkQuery::waitCompletion())
        return false;

    std::string result;
    std::string resultid;

    bool moreResult {false};
    bool allResultsRead {true};

    do {
        moreResult = bulkQuery::getResult(result,allResultsRead, resultid);
            
            // treat result
        if ((moreResult && !allResultsRead)|| (chunksize == 0)) {
            if (config::getFormat() == config::dataformat::XML) {
                theObject.computerecords(result);
                theObject.outputAttributeCounters(workingDirectory + "/result");
            } else {
                long nbrec = theObject.computeCsvRecords(result);
                totalRecords += nbrec;
                std::cout << "Nb records: " << nbrec << " Total: " << totalRecords << std::endl;
                theObject.outputAttributeCounters(workingDirectory + "/result" + theObject.getName() + ".csv");
                theObject.outputMatrixCounters(workingDirectory + "/matrix" + theObject.getName() + ".csv");
                
                if (!restartManager::isAlreadyRead(resultid))
                    restartManager::saveBatchId(resultid);
            }
        }
    } while (moreResult);

    // close the job
    if (!bulkQuery::closeJob())
        return false;
   
    if (config::getFormat() == config::dataformat::XML)
        theObject.outputAttributeCounters(workingDirectory + "/result");
    else {
        theObject.outputAttributeCounters(workingDirectory + "/result" + theObject.getName() + ".csv");
        theObject.outputMatrixCounters(workingDirectory + "/matrix" + theObject.getName() + ".csv");
    }

    if (verbose)
        theObject.printAttributeCounters();
    
    return true;
};
//
//
//
bool orchestrator::getResultFromJobId(const std::string& jobid) {
    
    long totalRecords {0};
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion()))
        return false;
    
    bulkQuery::setJobId(jobid);
    
    // bulkQuery::waitCompletion()
    if (!bulkQuery::waitCompletion())
        return false;

    std::string result;
    std::string resultid;
    bool moreResult {false};
    bool allResultsRead {true};
    do {
        moreResult = bulkQuery::getResult(result,allResultsRead, resultid);
            
        // treat result
        if (moreResult && !allResultsRead) {
            if (config::getFormat() == config::dataformat::XML) {
                theObject.computerecords(result);
                theObject.outputAttributeCounters(workingDirectory + "/result");

            } else {
                long nbrec = theObject.computeCsvRecords(result);
                totalRecords += nbrec;
                std::cout << "Nb records: " << nbrec << " Total: " << totalRecords << std::endl;
                theObject.outputAttributeCounters(workingDirectory + "/result" + theObject.getName() + ".csv");
                theObject.outputMatrixCounters(workingDirectory + "/matrix" + theObject.getName() + ".csv");
                
                if (!restartManager::isAlreadyRead(resultid))
                    restartManager::saveBatchId(resultid);

            }
        }
    } while (moreResult);
    
    if (config::getFormat() == config::dataformat::XML)
        theObject.outputAttributeCounters(workingDirectory + "/result");
    else {
        theObject.outputAttributeCounters(workingDirectory + "/result" + theObject.getName() + ".csv");
        theObject.outputMatrixCounters(workingDirectory + "/matrix" + theObject.getName() + ".csv");
    }
    
    if (verbose)
        theObject.printAttributeCounters();
    
    return true;
}

