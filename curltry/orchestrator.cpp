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
#include "globals.hpp"

//
extern bool getDescribeAttributesBuffer(const std::string objName, std::string& buffer);

//
//
bool orchestrator::describeObject() {
    std::string rawAttributeList;
    std::vector<std::string> excludedAttributes {};
    config::getExcludedAttributes(theObject.getName(), excludedAttributes);
    
    if (config::useFileForAttrList()) {
        std::vector<std::string> attrList;
        config::getAttributeList(globals::workingDirectory + "/", theObject.getName(), attrList);
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
        bool picklist {false};
        size_t offset = beginFields;
        do {
            picklist = false;
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
                            if (attributeType.compare("picklist") == 0)
                                picklist = true;
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
                            size_t begintrue = rawAttributeList.find ("true", begincustom);
                            if (begintrue != std::string::npos && begintrue < endcustom) {
                                isCustom = true;
                            }
                        }
                    } // end begincustom found
                    theObject.addAttribute({rawAttributeList.substr(beginName+7+1,endName-beginName-7-2),excluded, isCustom});
                    
                    if (picklist && globals::picklistAnalysis) {
                        /* example:
                         "picklistValues" : [ {
                           "active" : true,
                           "defaultValue" : false,
                           "label" : "Artisan-commerçant",
                           "validFor" : null,
                           "value" : "11"
                         }, {
                           "active" : true,
                           "defaultValue" : false,
                           "label" : "Commerçant",
                           "validFor" : null,
                           "value" : "12"
                         }, {

                         */
                        size_t beginPicklistValues = rawAttributeList.find("picklistValues", endName);
                        size_t beginArray = rawAttributeList.find_first_of('[', beginPicklistValues);
                        size_t endArray = rawAttributeList.find_first_of(']',beginArray);
                        bool picklistTerminated {false};
                        size_t picklistOffset = beginArray;
                        
                        while (!picklistTerminated) {
                            size_t beginlabel = rawAttributeList.find("\"label\"", picklistOffset);
                            if (beginlabel == std::string::npos || beginlabel > endArray)
                                break;
                            size_t endLabel = rawAttributeList.find("\"validFor\"", beginlabel);
                            std::string picklistLabel = rawAttributeList.substr(beginlabel+9,endLabel-beginlabel-9-2);
                            size_t beginvalue = rawAttributeList.find("\"value\"", endLabel);
                            size_t endvalue = rawAttributeList.find_first_of('}',beginvalue);
                            std::string picklistValue = rawAttributeList.substr(beginvalue+9,endvalue-beginvalue-9-1);
                            if (globals::veryverbose) {
                                std::cout << "picklistLabel :'" << picklistLabel << "' picklistValue : '" << picklistValue << "'" << std::endl;
                            }
                            picklistOffset = endvalue;
                        }   // end !picklistTerminated
                    }   // end picklist and picklistAnalysis
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
        theObject.initializeAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
    }
    
    //
    if (globals::verbose) {
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
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion(), config::getSecurityToken()))
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
                theObject.outputAttributeCounters(globals::workingDirectory + "/result");
            } else {
                long nbrec = theObject.computeCsvRecords(result);
                totalRecords += nbrec;
                std::cout << "Nb records: " << nbrec << " Total: " << totalRecords << std::endl;
                theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
                theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
                
                if (!restartManager::isAlreadyRead(resultid))
                    restartManager::saveBatchId(resultid);
            }
        }
    } while (moreResult);

    // close the job
    if (!bulkQuery::closeJob())
        return false;
   
    if (config::getFormat() == config::dataformat::XML)
        theObject.outputAttributeCounters(globals::workingDirectory + "/result");
    else {
        theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
        theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
    }

    if (globals::verbose)
        theObject.printAttributeCounters();
    
    return true;
};
//
//
//
bool orchestrator::getResultFromJobId(const std::string& jobid) {
    
    long totalRecords {0};
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion(), config::getSecurityToken()))
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
                theObject.outputAttributeCounters(globals::workingDirectory + "/result");

            } else {
                long nbrec = theObject.computeCsvRecords(result);
                totalRecords += nbrec;
                std::cout << "Nb records: " << nbrec << " Total: " << totalRecords << std::endl;
                theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
                theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
                
                if (!restartManager::isAlreadyRead(resultid))
                    restartManager::saveBatchId(resultid);

            }
        }
    } while (moreResult);
    
    if (config::getFormat() == config::dataformat::XML)
        theObject.outputAttributeCounters(globals::workingDirectory + "/result");
    else {
        theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
        theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
    }
    
    if (globals::verbose)
        theObject.printAttributeCounters();
    
    return true;
}

