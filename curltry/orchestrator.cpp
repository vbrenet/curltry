//
//  orchestrator.cpp
//  curltry
//
//  Created by Vincent Brenet on 11/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "orchestrator.hpp"
#include "SalesforceSession.hpp"
#include "bulkSession.hpp"
#include "bulkQuery.hpp"
//
extern bool getDescribeAttributesBuffer(const std::string objName, std::string& buffer);

//
bool orchestrator::describeObject() {
    std::string rawAttributeList;
    
    // launch describe request (produce a buffer)
    if (!getDescribeAttributesBuffer(theObject.getName(), rawAttributeList))
        return false;
    
    // parse each attribute in the buffer
    size_t beginFields = rawAttributeList.find("\"fields\":");
    if (beginFields != std::string::npos) {
        // chercher       "name":"curren__c","nameField":
        bool terminated {false};
        size_t offset = beginFields;
        do {
            size_t beginName = rawAttributeList.find("\"name\":",offset);
            if (beginName != std::string::npos) {
                size_t endName = rawAttributeList.find(",\"nameField\":",beginName);
                if (endName != std::string::npos) {
                    theObject.addAttribute({rawAttributeList.substr(beginName+7+1,endName-beginName-7-2)});
                    offset = endName+13;
                } else {
                    terminated = true;
                }
            } else
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
    
    return true;
};
//
//
bool orchestrator::execute(int chunksize) {
    // open bulk session bulkSession::openBulkSession
    if (!bulkSession::openBulkSession(credentials.isSandbox, credentials.username, credentials.password))
        return false;
    
    // bulkQuery::createJob(const std::string objectName, int chunksize)
    if (!bulkQuery::createJob(theObject.getName(), 0))
        return false;

    // bulkQuery::addQuery(const std::string& query)
    if (!bulkQuery::addQuery("Select id,name from account"))
        return false;

    // bulkQuery::waitCompletion()
    
    // while moreResult, bulkQuery::getResult(std::string& result);
    return true;
};
