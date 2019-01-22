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

extern std::string workingDirectory;
extern bool caseAnalysis;

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
void sObject::outputAttributeCounters(const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        ofs << it->first << " : " << it->second << std::endl;

    ofs.close();
}
//
void sObject::outputMatrixCounters(const std::string &outputfile) {
    recordTypeMap rtm {workingDirectory + "/recordTypes"};
    
    std::ofstream ofs {outputfile};
    
    for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
        std::string recordtypeid = it->first.first;
        std::string recordtypename;
        if (recordtypeid.size() == 0)
            recordtypename = "null";
        else
            recordtypename = rtm.getnamebyid(recordtypeid);
        ofs << recordtypename << "," << it->first.second << " : " << it->second << std::endl;
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
    int typeFieldNumber {0};
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
                                    typeFieldMap.insert(std::pair<std::string,long>({token},0));
                                    typeFieldMap[token]++;
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
                                    typeFieldMap.insert(std::pair<std::string,long>({token},0));
                                    typeFieldMap[token]++;
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
