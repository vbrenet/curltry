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
#include "expectedParameters.hpp"
#include "ActualParameters.hpp"
#include "restartManager.hpp"
#include "utils.hpp"
#include "globals.hpp"

const std::string curltryVersion = "curltry v1.2.8";

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
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
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SalesforceSession::getDomain() + "/services/data/v" + config::getApiVersion() + "/sobjects/" + objName + "/describe").c_str());
        
        if (globals::veryverbose)
            std::cout << "getDescribeAttributesBuffer: " << "https://" <<  SalesforceSession::getDomain() << "/services/data/v" + config::getApiVersion() + "/sobjects/" << objName << "/describe" << std::endl;
        
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
//
void runGetResultFromId(const std::string& theObj,  const std::string& theId) {
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
        if (!theOrchestrator.getResultFromJobId(theId)) {
            std::cerr << "theOrchestrator.getResultFromJobId failure" << std::endl;
        }
    }
}
//
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
    
        {
        injectionOrchestrator theOrchestrator {theObj, credentials};
        if (!theOrchestrator.execute(nbrec)) {
            std::cerr << "injectionOrchestrator.execute failure" << std::endl;
        }
    }
}
//
//
void terminate() {
    exit(0);
}
//
//
void exitWithSyntaxError() {
    std::cerr << "Syntax : curltry -o <object name> [-help] [-version] [-sz <chunksize>] [-j <jobid>] [-picklists] [-picklistsonly] [-r] [-v] [-vv] workingDirectory" << std::endl;
    exit(-1);
}
//
//
void exitWithVersion() {
    std::cout << curltryVersion << std::endl;
    exit(0);
}
//
//
void exitWithHelp() {
    std::cout << "SYNTAX : curltry -o <object name> [-sz <chunksize>] [-j <jobid>] [-picklists] [-picklistsonly] [-r] [-v] [-vv] workingDirectory" << std::endl;
    std::cout << "curltry -help" << std::endl;
    std::cout << "curltry -version" << std::endl << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "-o <object name> : specify the sObject to analyze, e.g. -o Opportunity" << std::endl;
    std::cout << "-sz <chunksize> : use the pkchunking option and specify the chunsize (max : 250000)" << std::endl;
    std::cout << "-j <jobid> : get results from a bulk job already run, e.g. -j 7503N00000009gn" << std::endl;
    std::cout << "-picklists : produce a file picklists<object> containing picklists stats" << std::endl;
    std::cout << "-picklistsonly : only produce a file picklists<object> containing picklists stats" << std::endl;
    std::cout << "-r : restart from the last curltry run session - this option requires the -j option" << std::endl;
    std::cout << "-v : verbose mode" << std::endl;
    std::cout << "-vv : very verbose mode (trace level)" << std::endl;
    std::cout << "workingDirectory (mandatory) : working directory which must contain the config file" << std::endl;
    exit(0);
    
}
//
//
//
int main(int argc, const char * argv[]) {
    
    expectedParameters ep {
        true,
        1,
        {
            {"-o",{true,true}},     // ismandatory, isvalued
            {"-sz",{false,true}},
            {"-i",{false,false}},
            {"-r",{false,false}},
            {"-v",{false,false}},
            {"-vv",{false,false}},
            {"-help",{false,false}},
            {"-version",{false,false}},
            {"-picklists",{false,false}},
            {"-picklistsonly",{false,false}},
            {"-j",{false,true}}
        }
    };
    
    ActualParameters ap;
        
    ap.set(argc, argv, ep);
    
    const std::vector<NamedParameter> parameters = ap.getNamedParameters();
    
    for (auto curr : parameters) {
        if (curr.getName().compare("-version") == 0)
            exitWithVersion();

        if (curr.getName().compare("-help") == 0)
            exitWithHelp();
    }
    
    if (!ap.areValid(ep))
        exitWithSyntaxError();

    bool injection {false};
    bool getResultFromJobId {false};
    std::string paramJobId {};
    std::string theObject {};
    int chunksize {10000};
    
    bool chunksizeProvided {false};
    for (auto curr : parameters) {
        
         if (curr.getName().compare("-o") == 0)
             theObject = curr.getValue();
         else if (curr.getName().compare("-sz") == 0) {
             if (!isStringNumeric(curr.getValue())) {
                 std::cerr << "Error : chunksize must be numeric" << std::endl;
                 exit(-1);
             }
             chunksize = std::stoi(curr.getValue());
             if (chunksize < 0 || chunksize > 250000) {
                 std::cerr << "Error : chunksize invalid" << std::endl;
                 exit(-1);
             }
            chunksizeProvided = true;
         }
         else if (curr.getName().compare("-i") == 0)
             injection = true;
         else if (curr.getName().compare("-j") == 0) {
             paramJobId = curr.getValue();
             getResultFromJobId = true;
         }
         else if (curr.getName().compare("-r") == 0) {
            restartManager::setRestartMode();
        }
         else if (curr.getName().compare("-v") == 0) {
             globals::verbose = true;
         }
        else if (curr.getName().compare("-vv") == 0) {
            globals::verbose = true;
            globals::veryverbose = true;
        }
        else if (curr.getName().compare("-picklists") == 0) {
            globals::picklistAnalysis = true;
        }
        else if (curr.getName().compare("-picklistsonly") == 0) {
            globals::picklistAnalysis = true;
            globals::picklistOnly = true;
        }



     }  // end for parameters

    const std::vector<std::string> values = ap.getValues();
    if (values.size() != 1) {
        std::cerr << "Missing parameter : workingDirectory" << std::endl;
        exitWithSyntaxError();
    }
    
    if (!getResultFromJobId && restartManager::isRestartMode()) {
        std::cerr << "Restart mode not allowed without option -j <jobid>" << std::endl;
        exitWithSyntaxError();
    }
    
    if (!chunksizeProvided)
        std::cout << "chunksize defaulted to " << chunksize << std::endl;
    
    globals::workingDirectory = values[0];
    
    corpNameGenerator::init();
    
    textGenerator::init();
        
    config::getConfig(globals::workingDirectory + "/config");

    restartManager::init();
    
    if (injection) {
        runInjection(theObject,chunksize);
    } else if (!getResultFromJobId ){
        runOrchestration(theObject,chunksize);
    } else {
        runGetResultFromId(theObject,paramJobId);
    }
    
    return 0;
}
