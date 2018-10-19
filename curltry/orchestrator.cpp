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
//
extern bool getDescribeAttributesBuffer(const std::string objName, std::string& buffer);

//
//
bool orchestrator::describeObject() {
    std::string rawAttributeList;
    std::vector<std::string> excludedAttributes {};
    config::getExcludedAttributes(theObject.getName(), excludedAttributes);
    
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
                    theObject.addAttribute({rawAttributeList.substr(beginName+7+1,endName-beginName-7-2),excluded});
                } // end endName fournd
                else {
                    terminated = true;
                }
            } // end beginName found
            else
                terminated = true;
        } while (terminated == false);
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
    
    std::cout << "the query:" << std::endl;
    std::cout << thequery << std::endl;
    
    return true;
};
//
//
bool orchestrator::execute(int chunksize) {
        
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkQuery::createJob(theObject.getName(), chunksize))
        return false;

    // bulkQuery::addQuery(const std::string& query)
    if (!bulkQuery::addQuery(theObject.makeAllAttributeQuery()))
        return false;

    // bulkQuery::waitCompletion()
    if (!bulkQuery::waitCompletion())
        return false;
    
    std::string result;
    bool moreResult {false};
    do {
        moreResult = bulkQuery::getResult(result);
        
        // treat result
        if (moreResult || (chunksize == 0)) {
            theObject.computerecords(result);
            theObject.outputAttributeCounters("/Users/vbrenet/Documents/Pocs/curltry/result");
            //std::cout << "***result :" << std::endl;
            //std::cout << result << std::endl;
        }
    } while (moreResult);

    // close the job
    if (!bulkQuery::closeJob())
        return false;
   
    theObject.outputAttributeCounters("/Users/vbrenet/Documents/Pocs/curltry/result");
    theObject.printAttributeCounters();
    
    return true;
};
