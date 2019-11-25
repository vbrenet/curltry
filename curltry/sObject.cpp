//
//  sObject.cpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  // for g++ compatibility
#include "sObject.hpp"
#include "config.hpp"
#include <curl/curl.h>
#include <sys/stat.h>
#include "SalesforceSession.hpp"
#include "restartManager.hpp"
#include "utils.hpp"
#include "globals.hpp"

//
std::string sObject::makeAllAttributeQuery() {
    
    if (queryStringConstructed)
        return query;
    
    std::vector<std::string> actualList {};
    bool isRecordTypeIdFound {false};
    
    for (auto it = attributeMap.begin(); it != attributeMap.end(); it++) {
        if (!it->second.isExcluded()) {
            if (globals::picklistOnly) {
                if (it->second.isPicklist())
                    actualList.push_back(it->second.getName());
            }
            else
                actualList.push_back(it->second.getName());
        }
        else
            std::cout << "excluded : " << it->second.getName() << std::endl;
        if (it->second.getName().compare("RecordTypeId") ==0)
            isRecordTypeIdFound = true;
    }
        
    std::cout <<  "makeAllAttributeQuery: attributeMap.size = " <<attributeMap.size()<< std::endl;
    std::cout <<  "makeAllAttributeQuery: actualList = " << actualList.size() << std::endl;

    if (actualList.size() == 0) {
        // try to get attribute list from file (RecordTypeId must be in first)
        config::getAttributeList(globals::workingDirectory + "/", name, actualList);
        std::cout <<  "makeAllAttributeQuery: actualList from file = " << actualList.size() << std::endl;
        if (actualList.size() > 0) {
            for (auto it = actualList.begin(); it != actualList.end(); it++) {
                addAttribute({*it,false});
            }
        }
    }
    
    query = "Select ";
    
    if (isRecordTypeIdFound) {
        attributeCounters.insert(std::pair<std::string,int>({"RecordTypeId",0}));
        query += "RecordTypeId";
        if (actualList.size() > 1)
            query += ",";
    }
    
    for (auto i=0; i < actualList.size(); i++) {
        
        if (isRecordTypeIdFound && (actualList[i].compare("RecordTypeId") == 0))
            continue;
        
        attributeCounters.insert(std::pair<std::string,int>({actualList[i],0}));
        query += actualList[i];
        if (i != (actualList.size() -1))
            query += ",";
    }
    
    query += " from " + getName();
    
    queryStringConstructed = true;
    
    return query;
}
//
//
void sObject::printAttributeCounters() const {
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        std::cout << it->first << " : " << it->second << std::endl;
}
//
//
void sObject::incrementCounters(const std::string &recordTypeId, int counter, const std::string &token) {
    attributeCounters[csvAttributeMap[counter]]++;
    std::pair<std::string,std::string> key {{recordTypeId},{csvAttributeMap[counter]}};
    recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
    recordTypeMatrixCounters[key]++;
    if (globals::picklistAnalysis) {
        if (attributeMap[csvAttributeMap[counter]].isPicklist())
            picklistCounters[csvAttributeMap[counter]][token]++;
    }
}
//
//
long sObject::computeCsvRecords(const std::string &csvString) {
    
    if (globals::verbose)
        std::cout << "Processing records (buffer size: " << csvString.length() << ") ..." << std::endl;
    
    bool firstRecord {true};
    bool errorFound {false};

    std::string token {};
    std::string record {};
    
    enum class state {START_TOKEN, QUOTE_RECEIVED, TOKEN_IN_PROGRESS, RETURN_IN_PROGRESS};
    state currentState {state::START_TOKEN};
    int counter {0};
    int recordtypeidnumber {0};

    int nbRecords {0};
    std::string currentRecordTypeId;
    
    for (char c : csvString) {
        switch (currentState) {
            case state::START_TOKEN:
                if (c == '"') {
                    currentState = state::TOKEN_IN_PROGRESS;
                }
                else if (c == 10) { // line feed
                    // end of record
                    currentState = state::START_TOKEN;
                }
                else {
                    std::cerr << "Parsing error : START_TOKEN, received a character different of \" or LF" << std::endl;
                    errorFound = true;
                }
                break;
            case state::QUOTE_RECEIVED:
                if (c == '"') {
                    token.push_back(c);
                    currentState = state::TOKEN_IN_PROGRESS;
                }
                else if (c == ',') {
                    // end of token
                    if (firstRecord) {
                        counter++;
                        csvAttributeMap.insert(std::pair<int,std::string>({counter},{token}));
                        if (token.compare("RecordTypeId") == 0)
                            recordtypeidnumber = counter;
                        token.clear();
                    }
                    else {
                        counter++;
                        if (counter == recordtypeidnumber)
                            currentRecordTypeId = token;
                        if (token.size() > 0) {
                            incrementCounters(currentRecordTypeId, counter, token);
                        }
                        token.clear();
                    }
                    currentState = state::START_TOKEN;
                }
                else if (c == 10) {
                    // end of token, end of record
                    if (firstRecord) {
                        counter++;
                        csvAttributeMap.insert(std::pair<int,std::string>({counter},{token}));
                        if (token.compare("RecordTypeId") == 0)
                            recordtypeidnumber = counter;
                                                
                        token.clear();
                        counter = 0;
                        firstRecord = false;
                    }
                    else {
                        nbRecords++;
                        counter++;
                        if (counter == recordtypeidnumber)
                            currentRecordTypeId = token;
                        if (token.size() > 0) {
                            incrementCounters(currentRecordTypeId, counter, token);
                        }
                        counter = 0;
                        token.clear();
                    }
                    currentState = state::START_TOKEN;
                }
                else {
                    std::cerr << "Parsing error : QUOTE_RECEIVED, received a character different of \" or , or LF" << std::endl;
                    errorFound = true;
                }
                break;
            case state::TOKEN_IN_PROGRESS:
                if (c == '"') {
                    currentState = state::QUOTE_RECEIVED;
                }
                else {
                    // accumulate current token
                    token.push_back(c);
                }
                break;
            case state::RETURN_IN_PROGRESS:
                if (c == 10) {  // line feed
                    token.push_back(c);
                    currentState = state::TOKEN_IN_PROGRESS;
                    // add carriage return in token
                }
                else {
                    std::cerr << "Parsing error : RETURN_IN_PROGRESS, received a character different of LF" << std::endl;
                    errorFound = true;
                }
                break;
            default:
                std::cerr << "Parsing error, unknown state" << std::endl;
                errorFound = true;
                break;
        }
        if (errorFound)
            break;
    }
    return nbRecords;
}
//
//
void sObject::initializeCounter(const std::string& attribute, const std::string& countervalue) {
    
    auto it = attributeCounters.find(attribute);
    
    if (it != attributeCounters.end()) {
        it->second = std::stol(countervalue);
    }
}
//
//
void sObject::processCsvLine(const std::string &inputline) {
    // example:
    //date,sobject,fieldName,fieldUsage,percentUsage,usageBucket,fromPackage
    //08/11/2019,Lead,Aconvertir__c,331,100.0,All,Custom
    //08/11/2019,Lead,AdressePostaleComplementDestinataire__c,0,0.0,00,Custom

    size_t firstcomma = inputline.find_first_of(',');
    size_t secondcomma = inputline.find_first_of(',',firstcomma+1);
    size_t thirdcomma = inputline.find_first_of(',',secondcomma+1);
    size_t fourthcomma = inputline.find_first_of(',',thirdcomma+1);

    if (firstcomma == std::string::npos ||
        secondcomma == std::string::npos ||
        thirdcomma == std::string::npos ||
        fourthcomma == std::string::npos) {
        std::cerr << "sObject::processCsvLine parsing error" << std::endl;
        return;
    }

    std::string attributeName = inputline.substr(secondcomma+1,thirdcomma-secondcomma-1);
    std::string counterValue = inputline.substr(thirdcomma+1,fourthcomma-thirdcomma-1);

    initializeCounter(attributeName,counterValue);
    
    if (globals::veryverbose) {
        std::cout << "attribute counters initialization" << std::endl;
        std::cout << "name: " << attributeName << " value: " << counterValue << std::endl;
        }
}
//
//
void sObject::initializeAttributeCountersFromFile(const std::string &inputfile) {
    std::ifstream csvResult {inputfile};
    
    std::string currentLine;
    
    int lineCounter = 0;
    
    while (getline(csvResult,currentLine)) {
        // skip header
        if (++lineCounter == 1) continue;
        
        processCsvLine(currentLine);
    }
    
    csvResult.close();
}
//
//  initialize the record type map for the current object from a REST query
//
//
void sObject::parseRecordTypeBuffer(const std::string&buffer) {
/* Example
 {"totalSize":7,"done":true,"records":[{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X0000009g6ZQAQ"},"Id":"0120X0000009g6ZQAQ","Name":"General"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X0000009g6aQAA"},"Id":"0120X0000009g6aQAA","Name":"Person Referral"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X0000009g6bQAA"},"Id":"0120X0000009g6bQAA","Name":"Retirement Planning"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X0000009gDKQAY"},"Id":"0120X0000009gDKQAY","Name":"Business Referral"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X000000sgJYQAY"},"Id":"0120X000000sgJYQAY","Name":"Personne connue"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X000000sgJZQAY"},"Id":"0120X000000sgJZQAY","Name":"Silhouette Part"},{"attributes":{"type":"RecordType","url":"/services/data/v47.0/sobjects/RecordType/0120X000000sgJaQAI"},"Id":"0120X000000sgJaQAI","Name":"Silhouette Pro/Ent"}]}
 */
    bool terminated = false;
    size_t cursor = 0;
    while (!terminated) {
        size_t idtoken = buffer.find("\"Id\":", cursor);
        if (idtoken != std::string::npos) {
            std::string currid = buffer.substr(idtoken+6,18);
            size_t nametoken = buffer.find("\"Name\":",idtoken);
            if (nametoken != std::string::npos) {
                size_t endnametoken = buffer.find("\"}",nametoken);
                if (endnametoken != std::string::npos) {
                    std::string currname = buffer.substr(nametoken+8,endnametoken-nametoken-8);
                    recordTypes.insert(std::pair<std::string,std::string>({currid},{currname}));
                    cursor = endnametoken;
                }
                else {
                    std::cerr << "parseRecordTypeBuffer error" << std::endl;
                    terminated = true;
                }
            }
            else {
                std::cerr << "parseRecordTypeBuffer error" << std::endl;
                terminated = true;
            }
        }
        else {
            terminated = true;
        }
    }   // end while
    
    //
    recordTypes.insert(std::pair<std::string,std::string>({"","null"}));
    
    //  output map
    if (globals::verbose) {
        std::cout << "Record Type map from REST" << std::endl;
        for (auto it=recordTypes.begin(); it != recordTypes.end(); ++it) {
            std::cout << it->first << ":" << it->second << std::endl;
        }
    }
}
//
size_t sObject::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
bool sObject::initializeRecordTypes() {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    readBuffer.clear();
    curl = curl_easy_init();
    // /services/data/v47.0/query/?q=SELECT+ID+,+Name+FROM+RECORDTYPE+where+sobjecttype+=+'opportunity'
    if(curl) {
        std::string query = "?q=SELECT+ID+,+Name+FROM+RECORDTYPE+where+sobjecttype+=+'" + getName() + "'";
        
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SalesforceSession::getDomain() + "/services/data/v" + config::getApiVersion() + "/query/" + query).c_str());
        
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
        
        //buffer = readBuffer;
    }
    else
        return false;

    if (globals::verbose) {
        std::cout << "record types query: " << std::endl;
        std::cout << readBuffer << std::endl;
    }
    
    parseRecordTypeBuffer(readBuffer);
    
    if (restartManager::isRestartMode())
        initializeMatrixCountersFromFile(globals::workingDirectory + "/matrix" + getName() + ".csv");
    else
        initRecordTypeMatrixCounters();
    
    return true;
}
//
//
std::string sObject::getnamebyid(const std::string id) const {
std::string result;
auto it = recordTypes.find(id);
if (it == recordTypes.end())
    result = "unknown";
else
    result = it->second;
return result;
}
//
//
//
void sObject::initRecordTypeMatrixCounters() {
    if (globals::veryverbose)
        std::cout << "initRecordTypeMatrixCounters" << std::endl;

    for (auto it = recordTypes.begin(); it != recordTypes.end(); ++it) {
        for (auto itattr = attributeMap.begin(); itattr != attributeMap.end(); ++itattr) {
            if (itattr->second.isExcluded())
                continue;
            std::pair<std::string,std::string> key {it->first,itattr->second.getName()};
            recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
            if (globals::veryverbose) {
                std::cout << key.first << ":" << key.second << std::endl;
            }
        }
    }
}
//
//
void sObject::processMatrixLine(const std::string &inputline) {
  /* example:
   Date,sObject,RecordTypeId,RecordType,Field,NbRecords,PercentRecordTypeUsage,UsageBucket,FromPackage
   08/11/2019,Lead,,null,Aconvertir__c,17,100.0,All,Custom
   08/11/2019,Lead,,null,AdressePostaleComplementDestinataire__c,0,0.0,00,Custom
   */

    std::string recordTypeId, attributeName, counterValue;
    
    size_t firstcomma = inputline.find_first_of(',');
    size_t secondcomma = inputline.find_first_of(',',firstcomma+1);
    size_t thirdcomma = inputline.find_first_of(',',secondcomma+1);
    size_t fourthcomma = inputline.find_first_of(',',thirdcomma+1);
    size_t fifthcomma = inputline.find_first_of(',',fourthcomma+1);
    size_t sixthcomma = inputline.find_first_of(',',fifthcomma+1);

    if (firstcomma == std::string::npos ||
        secondcomma == std::string::npos ||
        thirdcomma == std::string::npos ||
        fourthcomma == std::string::npos ||
        fifthcomma == std::string::npos ||
        sixthcomma == std::string::npos) {
        std::cerr << "sObject::processMatrixLine parsing error" << std::endl;
        return;
    }
        
        if ((thirdcomma-secondcomma) == 1)
            recordTypeId = "";
        else
            recordTypeId = inputline.substr(secondcomma+1,thirdcomma-secondcomma-1);

        attributeName = inputline.substr(fourthcomma+1,fifthcomma-fourthcomma-1);
        counterValue = inputline.substr(fifthcomma+1,sixthcomma-fifthcomma-1);
    
    if (globals::veryverbose) {
        std::cout << "recordTypeId: "<< recordTypeId << " attributeName: " << attributeName << " counterValue: " << counterValue << std::endl;
    }
    
    std::pair<std::string,std::string> key {recordTypeId,attributeName};
    recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{std::stol(counterValue)}));
}
//
//
void sObject::initializeMatrixCountersFromFile(const std::string &inputfile) {
    std::ifstream matrixFile {inputfile};
    
    std::string currentLine;
    
    int lineCounter = 0;
    
    while (getline(matrixFile,currentLine)) {
        // skip header
        if (++lineCounter == 1) continue;
        
        processMatrixLine(currentLine);
    }
    
    matrixFile.close();

}
//
//
//
void sObject::processPicklistLine(const std::string &inputline) {
/*
 15/11/2019,Account,AccountSource,"Partner Referral",Partner Referral,64,34.59
 15/11/2019,Account,AccountSource,"Phone Inquiry",Phone Inquiry,56,30.27
 15/11/2019,Account,AccountSource,"Purchased List",Purchased List,65,35.14
 15/11/2019,Account,AccountSource,"Web",Web,0,0.00
 */
    std::string picklistName, picklistValue, counterValue;
    
    size_t firstcomma = inputline.find_first_of(',');
    size_t secondcomma = inputline.find_first_of(',',firstcomma+1);
    size_t thirdcomma = inputline.find_first_of(',',secondcomma+1);
    if (firstcomma == std::string::npos ||
        secondcomma == std::string::npos ||
        thirdcomma == std::string::npos) {
        std::cerr << "sObject::processPicklistLine parsing error" << std::endl;
        return;
    }

    picklistName = inputline.substr(secondcomma+1,thirdcomma-secondcomma-1);
    
    size_t lastcomma = inputline.find_last_of(',');
    size_t lastcommaminus1 = inputline.rfind(',',lastcomma-1);
    size_t lastcommaminus2 = inputline.rfind(',',lastcommaminus1-1);

    if (lastcomma == std::string::npos ||
        lastcommaminus1 == std::string::npos ||
        lastcommaminus2 == std::string::npos) {
        std::cerr << "sObject::processPicklistLine parsing error" << std::endl;
        return;
    }
    
    picklistValue = inputline.substr(lastcommaminus2+1,lastcommaminus1-lastcommaminus2-1);
    counterValue = inputline.substr(lastcommaminus1+1,lastcomma-lastcommaminus1-1);
    
    if (globals::veryverbose) {
        std::cout << "sObject::processPicklistLine: picklistName: "<< picklistName << " picklistValue: " << picklistValue << " counterValue: " << counterValue << std::endl;
    }

    picklistCounters[picklistName][picklistValue] = std::stol(counterValue);
}
//
//
void sObject::initializePicklistCountersFromFile(const std::string &inputfile) {
    std::ifstream picklistFile {inputfile};
    
    std::string currentLine;
    
    int lineCounter = 0;
    
    while (getline(picklistFile,currentLine)) {
        // skip header
        if (++lineCounter == 1) continue;
        
        processPicklistLine(currentLine);
    }
    
    picklistFile.close();
}
//
//
void sObject::addPicklistDescriptor (std::string picklistName, std::string value, std::string label) {
    if (globals::veryverbose) {
        std::cout << "addPicklistDescriptor: " << picklistName << "," << value << "," << label << std::endl;
    }

    picklistDescriptors[picklistName][value] = label;

    picklistCounters[picklistName][value] = 0L;
}
//

//
//
bool sObject::getDescribeAttributesBuffer(std::string& buffer){
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    readBuffer.clear();
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SalesforceSession::getDomain() + "/services/data/v" + config::getApiVersion() + "/sobjects/" + getName() + "/describe").c_str());
        
        if (globals::veryverbose)
            std::cout << "getDescribeAttributesBuffer: " << "https://" <<  SalesforceSession::getDomain() << "/services/data/v" + config::getApiVersion() + "/sobjects/" << getName() << "/describe" << std::endl;
        
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
void sObject::setAnalysisDate(const std::string jobDate) {
    //input : 2019-11-25T10:21:32.000Z
    std::stringstream target;
    
    target  << jobDate[8]
            << jobDate[9]
            << '/'
            << jobDate[5]
            << jobDate[6]
            << '/'
            << jobDate[0]
            << jobDate[1]
            << jobDate[2]
            << jobDate[3];
    
    analysisDate = target.str();
}
