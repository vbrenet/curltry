//
//  main.cpp
//  curltry
//
//  Created by Vincent Brenet on 25/09/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include <iostream>
#include <string>
#include <curl/curl.h>
#include "sObject.hpp"
#include "SalesforceSession.hpp"
#include "bulkSession.hpp"


size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
//
std::string extractXmlToken(const std::string& inputbuffer, const std::string& token) {
    std::string endtoken = token;
    endtoken.insert(1,1,'/');
    
    size_t beginpos = inputbuffer.find(token);
    size_t endpos = inputbuffer.find(endtoken);
    
    return inputbuffer.substr(beginpos+token.size(),endpos-beginpos-token.size());
}
//
//
bool getDescribeAttributesBuffer(const std::string objName, std::string& buffer){
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    readBuffer.clear();
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SalesforceSession::getDomain() + "/services/data/v43.0/sobjects/" + objName + "/describe").c_str());
        
        struct curl_slist *chunk = NULL;
        
        chunk = curl_slist_append(chunk, ("Authorization: Bearer " + SalesforceSession::getConnectedAppToken()).c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_setopt() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_easy_cleanup(curl);
        
        buffer = readBuffer;
    }
    else
        return false;
    
    return true;
}
//
void describeSObject( sObject& obj) {
    std::string rawAttributeList;
    
    // launch describe request (produce a buffer)
    if (!getDescribeAttributesBuffer(obj.getName(), rawAttributeList))
        return;
    
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
                    obj.addAttribute({rawAttributeList.substr(beginName+7+1,endName-beginName-7-2)});
                    offset = endName+13;
                } else {
                    terminated = true;
                }
            } else
                terminated = true;
            } while (terminated == false);
        }
}
//
//
void runRestSession() {
    if (!SalesforceSession::openSession("vbrlight-dev-ed.my.salesforce.com", "3MVG98_Psg5cppyaViFlqbC.qo_drqk_L1ZWJnB4UB.NmykHpAvz.3wxbx23DBjgnccMNsZVfBF8UgvovtfYh", "8703187062703750250", "vbrlight@brenet.com", "Petrosian0"))
        return ;
    
    // get account attributes and print them
    sObject account {"Opportunity"};
    describeSObject (account);
    account.print();
}
//
//
void runBulkSession() {
    if (bulkSession::openBulkSession(false, "vbrlight@brenet.com", "Petrosian0"))
        std::cout << "openBulkSession successfull" << std::endl;
    else
        std::cerr << "openBulkSession error" << std::endl;

}
//
int main(int argc, const char * argv[]) {

    runBulkSession();
    
    return 0;
}
