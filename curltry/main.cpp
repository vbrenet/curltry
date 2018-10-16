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
#include "SalesforceSession.hpp"
#include "bulkSession.hpp"
#include "orchestrator.hpp"
#include "sessionCredentials.hpp"


size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
//
std::string extractXmlToken(const std::string& inputbuffer, size_t pos, const std::string& token) {
    std::string endtoken = token;
    endtoken.insert(1,1,'/');
    
    size_t beginpos = inputbuffer.find(token, pos);
    size_t endpos = inputbuffer.find(endtoken, pos);
    
    return inputbuffer.substr(beginpos+token.size(),endpos-beginpos-token.size());
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
//
//
void runBulkSession() {
    if (bulkSession::openBulkSession(false, "vbrlight@brenet.com", "Petrosian0"))
        std::cout << "openBulkSession successfull" << std::endl;
    else
        std::cerr << "openBulkSession error" << std::endl;

}
//
//
void runOrchestration() {
    // credentials creation
    sessionCredentials credentials {false,
                                    "vbrlight-dev-ed.my.salesforce.com",
                                    "3MVG98_Psg5cppyaViFlqbC.qo_drqk_L1ZWJnB4UB.NmykHpAvz.3wxbx23DBjgnccMNsZVfBF8UgvovtfYh",
                                    "8703187062703750250",
                                    "vbrlight@brenet.com",
                                    "Petrosian0"};
    //
    orchestrator theOrchestrator {"Account", credentials};
    //
    if (!theOrchestrator.getObjectInfo()) {
       std::cerr << "theOrchestrator.getObjectInfo failure" << std::endl;
    } else {
        if (!theOrchestrator.execute(0)) {
            std::cerr << "theOrchestrator.execute failure" << std::endl;
        }
    }
}
//
int main(int argc, const char * argv[]) {

 //   runBulkSession();
    
    runOrchestration();
    
    return 0;
}
