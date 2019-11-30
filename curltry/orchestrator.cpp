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
    if (!theObject.getDescribeAttributesBuffer(rawAttributeList))
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
            std::string attributeType;
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
                            attributeType = rawAttributeList.substr(beginType+7+1,endType-beginType-7-2);
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
                    
                    // get the custom and defaultValue attributes
                    bool isCustom {false};
                    std::string defaultValue {};
                    size_t begincustom = rawAttributeList.rfind("\"custom\"", endName);
                    if (begincustom != std::string::npos) {
                        size_t endcustom = rawAttributeList.find("\"defaultValue\"", begincustom);
                        if (endcustom != std::string::npos) {
                            size_t begintrue = rawAttributeList.find ("true", begincustom);
                            if (begintrue != std::string::npos && begintrue < endcustom) {
                                isCustom = true;
                            }
                            size_t comma = rawAttributeList.find_first_of(',',endcustom);
                            defaultValue = rawAttributeList.substr(endcustom+15,comma-endcustom-15);
                        }
                    } // end begincustom found
                    
                    std::string currentAttributeName = rawAttributeList.substr(beginName+7+1,endName-beginName-7-2);
                    theObject.addAttribute({currentAttributeName,excluded, isCustom, picklist, attributeType, defaultValue});
                    
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
                        size_t endArray = rawAttributeList.find("],",beginArray);
                        bool picklistTerminated {false};
                        size_t picklistOffset = beginArray;
                        std::string picklistValue {};
                        std::string picklistLabel {};
                        
                        while (!picklistTerminated) {
                            size_t beginlabel = rawAttributeList.find("\"label\"", picklistOffset);
                            if (beginlabel == std::string::npos || beginlabel > endArray)
                                break;
                            size_t endLabel = rawAttributeList.find("\"validFor\"", beginlabel);
                            picklistLabel = rawAttributeList.substr(beginlabel+9,endLabel-beginlabel-9-2);
                            size_t beginvalue = rawAttributeList.find("\"value\"", endLabel);
                            size_t endvalue = rawAttributeList.find_first_of('}',beginvalue);
                            picklistValue = rawAttributeList.substr(beginvalue+9,endvalue-beginvalue-9-1);
                            theObject.addPicklistDescriptor (currentAttributeName, picklistValue, picklistLabel);

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
    
    if (globals::veryverbose) {
        theObject.printPicklistCounters();
        theObject.printPicklistDescriptors();
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

    // get object field book
    theObject.getFieldBook();
    
    // output field book
    theObject.outputFieldBook();
    
    if (globals::bookOnly)
        exit(0);
    
    // describe object (get all attributes)
    if (!describeObject())
        return false;
    
    //theObject.print();
    std::string thequery = theObject.makeAllAttributeQuery();
    
    // if restart mode, initialize counters from csvResult files
    if (restartManager::isRestartMode()) {
        theObject.initializeAttributeCountersFromFile(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
        if (globals::picklistAnalysis) {
            theObject.initializePicklistCountersFromFile(globals::workingDirectory + "/picklists" + theObject.getName() + ".csv");
            theObject.initializeMatrixPicklistCountersFromFile(globals::workingDirectory + "/picklistsMatrix" + theObject.getName() + ".csv");
        }
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
    long totalRecordProcessedByBatches {0};

    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion(), credentials.userSecurityToken))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkQuery::createJob(theObject.getName(), chunksize))
        return false;

    // bulkQuery::addQuery(const std::string& query)
    if (!bulkQuery::addQuery(theObject.makeAllAttributeQuery()))
        return false;

    // bulkQuery::waitCompletion()
    std::string jobDate {};
    if (!bulkQuery::waitCompletion(jobDate, totalRecordProcessedByBatches))
        return false;

    theObject.setAnalysisDate(jobDate);
    
    std::string result;
    std::string resultid;

    bool moreResult {false};
    bool allResultsRead {true};

    do {
        moreResult = bulkQuery::getResult(result,allResultsRead, resultid);
            
            // treat result
        if ((moreResult && !allResultsRead)|| (chunksize == 0)) {
            
            long nbrec = theObject.computeCsvRecords(result);
            totalRecords += nbrec;
            double percentProgress = ((double)totalRecords/(double)totalRecordProcessedByBatches)*100;
            std::cout << "Nb records: " << nbrec;
            std::cout << " Total processed: " << totalRecords;
            if (!restartManager::isRestartMode())
                std::cout << " Progress: " << std::setprecision (1) << std::fixed << percentProgress << "%";
            std::cout << std::endl;
            
            if (globals::picklistOnly) {
                theObject.outputPicklistCounters();
            }
            else {
                theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
                theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
                if (globals::picklistAnalysis) {
                    theObject.outputPicklistCounters();
                    theObject.outputRecordTypePicklistCounters();
                }
            }
                
            if (!restartManager::isAlreadyRead(resultid))
                restartManager::saveBatchId(resultid);
        }
    } while (moreResult);

    // close the job
    if (!bulkQuery::closeJob())
        return false;
   
        if (globals::picklistOnly) {
            theObject.outputPicklistCounters();
        }
        else {
            theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
            theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
            if (globals::picklistAnalysis) {
                theObject.outputPicklistCounters();
                theObject.outputRecordTypePicklistCounters();
            }
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
    long totalRecordProcessedByBatches {0};
    
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password, config::getApiVersion(), credentials.userSecurityToken))
        return false;
    
    bulkQuery::setJobId(jobid);
    
    std::string jobDate {};
    
    if (!bulkQuery::waitCompletion(jobDate, totalRecordProcessedByBatches))
        return false;
    
    theObject.setAnalysisDate(jobDate);

    std::string result;
    std::string resultid;
    bool moreResult {false};
    bool allResultsRead {true};
    do {
        moreResult = bulkQuery::getResult(result,allResultsRead, resultid);
            
        // treat result
        if (moreResult && !allResultsRead) {
                long nbrec = theObject.computeCsvRecords(result);
                totalRecords += nbrec;
                double percentProgress = ((double)totalRecords/(double)totalRecordProcessedByBatches)*100;
                std::cout << "Nb records: " << nbrec;
                std::cout << " Total processed: " << totalRecords;
                if (!restartManager::isRestartMode())
                    std::cout << " Progress: " << std::setprecision (1) << std::fixed << percentProgress << "%";
                std::cout << std::endl;

                if (globals::picklistOnly) {
                    theObject.outputPicklistCounters();
                }
                else {
                    theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
                    theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
                    if (globals::picklistAnalysis) {
                        theObject.outputPicklistCounters();
                        theObject.outputRecordTypePicklistCounters();
                    }
                }

                if (!restartManager::isAlreadyRead(resultid))
                    restartManager::saveBatchId(resultid);
        }
    } while (moreResult);
    
        if (globals::picklistOnly) {
            theObject.outputPicklistCounters();
        }
        else {
            theObject.outputAttributeCounters(globals::workingDirectory + "/result" + theObject.getName() + ".csv");
            theObject.outputMatrixCounters(globals::workingDirectory + "/matrix" + theObject.getName() + ".csv");
            if (globals::picklistAnalysis) {
                theObject.outputPicklistCounters();
                theObject.outputRecordTypePicklistCounters();
            }
        }
    
    if (globals::verbose)
        theObject.printAttributeCounters();
    
    return true;
}

