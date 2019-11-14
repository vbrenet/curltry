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
        if (!it->isExcluded()) {
            if (globals::picklistOnly) {
                if (it->isPicklist())
                    actualList.push_back(it->getName());
            }
            else
                actualList.push_back(it->getName());
        }
        else
            std::cout << "excluded : " << it->getName() << std::endl;
        if (it->getName().compare("RecordTypeId") ==0)
            isRecordTypeIdFound = true;
    }
        
    std::cout <<  "makeAllAttributeQuery: attributeList.size = " <<attributeList.size()<< std::endl;
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
void sObject::printAttributeCounters() const {
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        std::cout << it->first << " : " << it->second << std::endl;
}
//
//
void sObject::outputAttributeCounters(const std::string &outputfile) {
    
    double nbRecords {0};
    auto idit = attributeCounters.find("Id");
    if (idit != attributeCounters.end()) {
        nbRecords = idit->second;
    }
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "date,sobject,fieldName,fieldUsage,percentUsage,usageBucket,fromPackage" << std::endl;
    
    std::string dateString = getDateString();
    
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++) {
        
        ofs << dateString << "," << getName() << ",";
        ofs << it->first << "," << it->second << ",";
        double percentUsage = ((nbRecords == 0) ? 0 : (((double)it->second / nbRecords)*100));
        ofs << std::setprecision (1) << std::fixed << percentUsage ;
        
        ofs << "," << getBucket(percentUsage);
        
        std::string fromPackage {};
        if (isAttributeCustom(it->first))
            fromPackage = "Custom";
        else
            fromPackage = "Standard";
        
        ofs << "," << fromPackage << std::endl;

        
        
    }

    ofs.close();
}
//
//
bool sObject::isAttributeCustom(const std::string name) const {
    
    for (auto it = attributeList.begin(); it != attributeList.end(); ++it) {
        if (it->getName().compare(name) == 0) {
            return it->isCustom();
        }
    }

    return false;
}
//
//
bool sObject::isAttributePicklist(const std::string name) const {
    
    for (auto it = attributeList.begin(); it != attributeList.end(); ++it) {
        if (it->getName().compare(name) == 0) {
            return it->isPicklist();
        }
    }

    return false;
}

//
//
void sObject::outputMatrixCounters(const std::string &outputfile) {
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "Date,sObject,RecordTypeId,RecordType,Field,NbRecords,PercentRecordTypeUsage,UsageBucket,FromPackage" << std::endl;
    
    std::string dateString = getDateString();
    
    for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
        std::string recordtypeid = it->first.first;
        std::string recordtypename;
        if (recordtypeid.size() == 0)
            recordtypename = "null";
        else
            recordtypename = getnamebyid(recordtypeid);

        double currNbRec = recordTypeMatrixCounters[std::pair<std::string,std::string>(recordtypeid,"Id")];
        double percentUsage = ((currNbRec == 0) ? 0 : (((double)it->second / currNbRec)*100));
        
        ofs << dateString << "," << getName() << ",";
        ofs << recordtypeid << "," << removeCommas(recordtypename) << "," << it->first.second << "," << it->second << ",";
        ofs << std::setprecision (1) << std::fixed << percentUsage;
        ofs << "," << getBucket(percentUsage);
        
        std::string fromPackage {};
        if (isAttributeCustom(it->first.second))
            fromPackage = "Custom";
        else
            fromPackage = "Standard";
        
        ofs << "," << fromPackage << std::endl;
    }
    
    ofs.close();
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
                            attributeCounters[csvAttributeMap[counter]]++;
                            std::pair<std::string,std::string> key {{currentRecordTypeId},{csvAttributeMap[counter]}};
                            recordTypeMatrixCounters.insert(std::pair<std::pair<std::string,std::string>,long>({key},{0}));
                            recordTypeMatrixCounters[key]++;
                            if (globals::picklistAnalysis) {
                                if (isAttributePicklist(csvAttributeMap[counter]))
                                    picklistCounters[csvAttributeMap[counter]][token]++;
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
                            if (globals::picklistAnalysis) {
                                if (isAttributePicklist(csvAttributeMap[counter]))
                                    picklistCounters[csvAttributeMap[counter]][token]++;
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
    if (globals::verbose) {
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
        for (auto itattr = attributeList.begin(); itattr != attributeList.end(); ++itattr) {
            if (itattr->isExcluded())
                continue;
            std::pair<std::string,std::string> key {it->first,itattr->getName()};
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
void sObject::addPicklistDescriptor (std::string picklistName, std::string value, std::string label) {
    if (globals::veryverbose) {
        std::cout << "addPicklistDescriptor: " << picklistName << "," << value << "," << label << std::endl;
    }

    picklistDescriptors[picklistName][value] = label;

    picklistCounters[picklistName][value] = 0L;
}
//
//
void sObject::outputPicklistCounters()  {
    std::ofstream ofs { "picklists" + getName() + ".csv"};
    
    // header
    ofs << "Date,sObject,PicklistName,PicklistLabel,PicklistValue,Usage,PercentUsage" << std::endl;

    for (auto it = picklistCounters.begin(); it != picklistCounters.end(); ++it) {
        // first compute total valued fields
        double total {0};
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            total += it2->second;

        // then output counters
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            
            std::string label = picklistDescriptors[it->first][it2->first] ;
            
            double percentUsage = ((total == 0) ? 0 : (((double)it2->second / total)*100));
            
            ofs << getDateString() << "," << getName() << "," << it->first << ",\"" << label << "\"," << it2->first << "," << it2->second << "," ;
            ofs << std::setprecision (2) << std::fixed << percentUsage << std::endl;
        }
    }
}
//
//
void sObject::printPicklistCounters() const {
    std::cout << "**printPicklistCounters**" << std::endl;
    for (auto it = picklistCounters.begin(); it != picklistCounters.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            std::cout << it->first << ":" << it2->first << "," << it2->second << std::endl;
        }
    }
}
//
void sObject::printPicklistDescriptors() const {
    std::cout << "**printPicklistDescriptors**" << std::endl;
    for (auto it = picklistDescriptors.begin(); it != picklistDescriptors.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            std::cout << it->first << ":" << it2->first << "," << it2->second << std::endl;
        }
    }
}

