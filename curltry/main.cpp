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
#include "buckets.hpp"

const std::string curltryVersion = "curltry v2.2.7";
//
//
void runGetResultFromId(const std::string& theObj,  const std::string& theId) {
    // credentials creation
    sessionCredentials credentials {config::isSandbox(),
        config::getDomain(),
        config::getClientId(),
        config::getClientSecret(),
        config::getUsername(),
        config::getPassword(),
        config::getSecurityToken()
    };
    
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
        config::getPassword(),
        config::getSecurityToken()
    };

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
        config::getPassword(),
        config::getSecurityToken()
    };
    
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
    std::cerr << "Syntax : curltry -o <object name> [-help] [-version] [-sz <chunksize>] [-j <jobid>] [-bookonly] [-picklists] [-picklistsonly] [-r] [-v] [-vv] workingDirectory" << std::endl;
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
    std::cout << "SYNTAX : curltry -o <object name> [-sz <chunksize>] [-j <jobid>] [-bookonly] [-picklists] [-picklistsonly] [-r] [-v] [-vv] workingDirectory" << std::endl;
    std::cout << "curltry -help" << std::endl;
    std::cout << "curltry -version" << std::endl << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "-o <object name> : specify the sObject to analyze, e.g. -o Opportunity" << std::endl;
    std::cout << "-sz <chunksize> : use the pkchunking option and specify the chunsize (max : 250000)" << std::endl;
    std::cout << "-j <jobid> : get results from a bulk job already run, e.g. -j 7503N00000009gn" << std::endl;
    std::cout << "-bookonly : produce only the field book of the sObject to analyze" << std::endl;
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
            {"-bookonly",{false,false}},
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
        else if (curr.getName().compare("-bookonly") == 0) {
            globals::bookOnly = true;
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
    
    if (!chunksizeProvided && !globals::bookOnly && !getResultFromJobId)
        std::cout << "chunksize defaulted to " << chunksize << std::endl;
    
    globals::workingDirectory = values[0];
    
    corpNameGenerator::init();
    
    textGenerator::init();
        
    if (!config::getConfig(globals::workingDirectory + "/config")) {
        std::cerr << "config file not found or empty" << std::endl;
        exitWithSyntaxError();
    }

    if (!config::checkConfig())
        exit(-1);
    
    buckets::initBucketsFromFile(globals::workingDirectory + "/buckets");
    
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
