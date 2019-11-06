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
#include "sObject.hpp"
#include "config.hpp"
#include "recordTypeMap.hpp"
#include <curl/curl.h>
#include "SalesforceSession.hpp"


extern std::string workingDirectory;
extern bool caseAnalysis;
extern bool verbose;

void sObject::print() const {
    for (sAttribute s : attributeList)
        std::cout << "Attribute: " << s.getName() << std::endl;
}
//
//
std::string sObject::makeAllAttributeQuery() {
    std::string query {};
    std::vector<std::string> actualList {};
    bool isRecordTypeIdFound {false};
    
    for (auto it = attributeList.begin(); it != attributeList.end(); it++) {
        if (!it->isExcluded())
            actualList.push_back(it->getName());
        else
            std::cout << "excluded : " << it->getName() << std::endl;
        if (it->getName().compare("RecordTypeId") ==0)
            isRecordTypeIdFound = true;
    }
    
    if (caseAnalysis) {
        actualList.clear();
        actualList.push_back("RecordTypeId");
        actualList.push_back("Type");
        actualList.push_back("objDem__c");
    }
    
    std::cout <<  "makeAllAttributeQuery: attributeList.size = " <<attributeList.size()<< std::endl;
    std::cout <<  "makeAllAttributeQuery: actualList = " << actualList.size() << std::endl;

    if (actualList.size() == 0) {
        // try to get attribute list from file (RecordTypeId must be in first)
        config::getAttributeList(workingDirectory + "/", name, actualList);
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
    
    return query;
}
//
void sObject::computeAttributes(const std::string &record, int recnumber) {
    for (sAttribute a : attributeList) {
        size_t beginAttr = record.find(a.getName());
        if (beginAttr != std::string::npos) {
            if (record[beginAttr+a.getName().size()] == '>') {
                //std::cout << "attribute " << a.getName() << " filled in rec " << recnumber << std::endl;
                attributeCounters[a.getName()]++;
            }
        }
    }
}
//
void sObject::computerecords(const std::string &xmlresult) {
    size_t cursor = 0;
    int nbrec {0};
    bool terminated {false};
    
    while (!terminated) {
        size_t nextRecord = xmlresult.find("<records xsi:type=\"sObject\">", cursor);
        terminated = (nextRecord == std::string::npos);
        if (!terminated) {
            nbrec++;
            size_t endOfRecord = xmlresult.find("</records>", nextRecord);
            std::string currentRecord = xmlresult.substr(nextRecord, endOfRecord-nextRecord);
            computeAttributes(currentRecord, nbrec);
            //std::cout << "record " << nbrec << " : " << currentRecord << std::endl;
            cursor = nextRecord + 30;
        }
    }

    //std::cout << "Total number of records: " << nbrec << std::endl;
}
//
//
void sObject::printAttributeCounters() {
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        std::cout << it->first << " : " << it->second << std::endl;
}
//
//
void sObject::outputTypeCounter(const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    
    for (auto it=typeFieldMap.begin(); it != typeFieldMap.end(); it++)
        ofs << it->first << " : " << it->second << std::endl;
    
    ofs.close();
}

//
void sObject::outputTypeObjDemMap(const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    
    for (auto it=typeObjDemMap.begin(); it != typeObjDemMap.end(); it++)
        ofs << it->first.first << "," << it->first.second << " : " << it->second << std::endl;
    
    ofs.close();
}

//
void sObject::outputAttributeCounters(const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "fieldName,fieldUsage" << std::endl;
    
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        ofs << it->first << "," << it->second << std::endl;

    ofs.close();
}
//
//
void sObject::outputTupleMap(const std::string &outputfile) {
    recordTypeMap rtm {workingDirectory + "/recordTypes"};
    
    std::ofstream ofs {outputfile};
    
    for (auto it=tupleMap.begin(); it != tupleMap.end(); it++) {
        std::string recordtypeid = std::get<2>(it->first);
        std::string recordtypename;
        if (recordtypeid.size() == 0)
            recordtypename = "null";
        else
            recordtypename = rtm.getnamebyid(recordtypeid);
        ofs << std::get<0>(it->first) << ":" << std::get<1>(it->first) << ":" << recordtypename << ":" << it->second << std::endl;
    }

}

//
//
void sObject::outputMatrixCounters(const std::string &outputfile) {
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "RecordTypeId,RecordType,FieldName,FieldUsage" << std::endl;
    
    for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
        std::string recordtypeid = it->first.first;
        std::string recordtypename;
        if (recordtypeid.size() == 0)
            recordtypename = "null";
        else
            recordtypename = getnamebyid(recordtypeid);
        ofs << recordtypeid << "," << recordtypename << "," << it->first.second << "," << it->second << std::endl;
    }
    
    ofs.close();
}

//
//
long sObject::computeCsvRecords(const std::string &csvString) {
    
    bool firstRecord {true};
    bool errorFound {false};

    std::string token {};
    std::string record {};
    
    enum class state {START_TOKEN, QUOTE_RECEIVED, TOKEN_IN_PROGRESS, RETURN_IN_PROGRESS};
    state currentState {state::START_TOKEN};
    int counter {0};
    int recordtypeidnumber {0};
    // case analysis
    int typeFieldNumber {0};
    int ObjDemandeFieldNumber {0};
    std::string currentCaseType;

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
                        if (caseAnalysis) {
                            if (token.compare("Type") == 0)
                                typeFieldNumber = counter;
                            if (token.compare("objDem__c") == 0)
                                ObjDemandeFieldNumber = counter;
                        }
                        token.clear();
                    }
                    else {
                        counter++;
                        if (counter == recordtypeidnumber)
                            currentRecordTypeId = token;
                        if (token.size() > 0) {
                            attributeCounters[csvAttributeMap[counter]]++;
                            std::pair<std::string,std::string> key {{currentRecordTypeId},{csvAttributeMap[counter]}};
                            recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
                            recordTypeMatrixCounters[key]++;
                            
                            // caseAnalysis
                            if (caseAnalysis) {
                                if (typeFieldNumber == counter) {
                                    currentCaseType = token;
                                    typeFieldMap.insert(std::pair<std::string,long>({token},0));
                                    typeFieldMap[token]++;
                                }
                                if (ObjDemandeFieldNumber == counter) {
                        typeObjDemMap.insert(std::pair<std::pair<std::string,std::string>,long>({{currentCaseType},{token}},0));
                                    typeObjDemMap[{currentCaseType,token}]++;
                                    auto currTuple = std::make_tuple(currentCaseType,token,currentRecordTypeId);
                                    tupleMap.insert(std::pair<std::tuple<std::string,std::string,std::string>,long>(currTuple,0));
                                    tupleMap[currTuple]++;
                                }
                            }
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
                        
                        if (caseAnalysis) {
                            if (token.compare("Type") == 0)
                                typeFieldNumber = counter;
                            if (token.compare("objDem__c") == 0)
                                ObjDemandeFieldNumber = counter;
                        }
                        
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
                            attributeCounters[csvAttributeMap[counter]]++;
                            std::pair<std::string,std::string> key {{currentRecordTypeId},{csvAttributeMap[counter]}};
                            recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
                            recordTypeMatrixCounters[key]++;
                            if (caseAnalysis) {
                                if (typeFieldNumber == counter) {
                                    currentCaseType = token;
                                    typeFieldMap.insert(std::pair<std::string,long>({token},0));
                                    typeFieldMap[token]++;
                                }
                                if (ObjDemandeFieldNumber == counter) {
                                    typeObjDemMap.insert(std::pair<std::pair<std::string,std::string>,long>({{currentCaseType},{token}},0));
                                    typeObjDemMap[{currentCaseType,token}]++;
                                    auto currTuple = std::make_tuple(currentCaseType,token,currentRecordTypeId);
                                    tupleMap.insert(std::pair<std::tuple<std::string,std::string,std::string>,long>(currTuple,0));
                                    tupleMap[currTuple]++;
                                }
                            }
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
    //TECH_ResponsableIdSf__c,0
    //TECH_StructureVisibilite__c,0
    size_t firstComma = inputline.find_first_of(',');
    if (firstComma != std::string::npos) {
        std::string attributeName = inputline.substr(0,firstComma);
        std::string counterValue = inputline.substr(firstComma+1, std::string::npos);
        initializeCounter(attributeName,counterValue);
        if (verbose) {
            std::cout << "attribute counters initialization" << std::endl;
            std::cout << "name: " << attributeName << " value: " << counterValue << std::endl;
        }
    }
    
}
//
//
void sObject::initializeAttributeCounters(const std::string &inputfile) {
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
    if (verbose) {
        std::cout << "Record Type map from REST" << std::endl;
        for (auto it=recordTypes.begin(); it != recordTypes.end(); ++it) {
            std::cout << it->first << ":" << it->second << std::endl;
        }
    }
}
//
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

    if (verbose) {
        std::cout << "record types query: " << std::endl;
        std::cout << readBuffer << std::endl;
    }
    
    parseRecordTypeBuffer(readBuffer);
    
    initRecordTypeMatrixCounters();
    
    return true;
}
//
//
std::string sObject::getnamebyid(const std::string id) {
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
    
    for (auto it = recordTypes.begin(); it != recordTypes.end(); ++it) {
        for (auto itattr = attributeList.begin(); itattr != attributeList.end(); ++itattr) {
            if (itattr->isExcluded())
                continue;
            std::pair<std::string,std::string> key {it->first,itattr->getName()};
            recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
            if (verbose) {
                //std::cout << "initRecordTypeMatrixCounters: inserted value" << std::endl;
                //std::cout << key.first << ":" << key.second << std::endl;
            }
        }
    }
    
    //
    if (verbose) {
        /*
        std::cout << "recordTypeMatrixCounters after initialization" << std::endl;
        for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
            std::string recordtypeid = it->first.first;
            std::string recordtypename;
            if (recordtypeid.size() == 0)
                recordtypename = "null";
            else
                recordtypename = getnamebyid(recordtypeid);
            std::cout << recordtypename << "," << it->first.second << " : " << it->second << std::endl;
        }
         */
    }

}
