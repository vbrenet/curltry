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
#include "config.hpp"
#include "corpNameGenerator.hpp"
#include "textGenerator.hpp"
#include "nameAttribute.hpp"
#include "textAttribute.hpp"
#include "integerAttribute.hpp"
#include "picklistAttribute.hpp"
#include "recordGenerator.hpp"
#include "injectionOrchestrator.hpp"


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
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
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
void testConfig() {
 //
    config::getConfig("/Users/vbrenet/Documents/Pocs/curltry/config");
    std::cout << "domain : " << config::getDomain() << std::endl;
    std::cout << "clientid : " << config::getClientId() << std::endl;
    std::cout << "clientsecret : " << config::getClientSecret() << std::endl;
    std::cout << "username : " << config::getUsername() << std::endl;
    std::cout << "password : " << config::getPassword() << std::endl;
    config::printMap();
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
//
void runOrchestration(const std::string& theObj, int chunksize) {
    // credentials creation
    sessionCredentials credentials {config::isSandbox(),
        config::getDomain(),
        config::getClientId(),
        config::getClientSecret(),
        config::getUsername(),
        config::getPassword()};

    //
    orchestrator theOrchestrator {theObj, credentials};
    //
    if (!theOrchestrator.getObjectInfo()) {
       std::cerr << "theOrchestrator.getObjectInfo failure" << std::endl;
    } else {
        if (!theOrchestrator.execute(chunksize)) {
            std::cerr << "theOrchestrator.execute failure" << std::endl;
        }
    }
}
//
//
void runInjection(const std::string& theObj, int nbrec) {
    // credentials creation
    sessionCredentials credentials {config::isSandbox(),
        config::getDomain(),
        config::getClientId(),
        config::getClientSecret(),
        config::getUsername(),
        config::getPassword()};
    
    //
    injectionOrchestrator theOrchestrator {theObj, credentials};
    //
    if (!theOrchestrator.execute(nbrec)) {
        std::cerr << "injectionOrchestrator.execute failure" << std::endl;

    }
}
//
//
void terminate() {
    exit(0);
}
//
void testrun() {
    
    recordGenerator recgen ("/Users/vbrenet/Documents/Pocs/curltry/Account.inject");

    std::cout << "rec header: " << recgen.getCsvHeader() << std::endl;
    std::cout << "rec values: " << recgen.getCsvRecord() << std::endl;
    
    terminate();
}
//
int main(int argc, const char * argv[]) {
    
    std::cout << "curltry : version 6Dec2018 V1" << std::endl;
    corpNameGenerator::init();
    textGenerator::init();
    
    bool injection {false};
    
    if (argc < 3) {
        std::cerr << "Syntax : curltry <object name> <chunksize> [injection=true|false>]" << std::endl;
        exit(-1);
    }
    
    if (argc == 4) {
        std::string injectionParam {argv[3]};
        if (injectionParam.compare("injection=true") == 0)
            injection = true;
        else if (injectionParam.compare("injection=false") == 0)
            injection = false;
        else {
            std::cerr << "Syntax : curltry <object name> <chunksize> [injection=true|false>]" << std::endl;
            exit(-1);
        }
    }
    
    std::string theObject {argv[1]};
    std::string arg2 {argv[2]};

    int chunksize = std::stoi(arg2);
    if (chunksize < 0) {
        std::cerr << "Error : chunksize invalid" << std::endl;
        exit(-1);
    }
            
    config::getConfig("/Users/vbrenet/Documents/Pocs/curltry/config");

    if (injection) {
        runInjection(theObject,chunksize);
    } else {
        runOrchestration(theObject,chunksize);
    }
    
    return 0;
}
