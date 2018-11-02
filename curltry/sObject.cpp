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

void sObject::print() const {
    for (sAttribute s : attributeList)
        std::cout << "Attribute: " << s.getName() << std::endl;
}
//
//
std::string sObject::makeAllAttributeQuery() {
    std::string query {};
    std::vector<std::string> actualList {};
    
    for (auto it = attributeList.begin(); it != attributeList.end(); it++)
        if (!it->isExcluded())
            actualList.push_back(it->getName());
        else
            std::cout << "excluded : " << it->getName() << std::endl;
    
    std::cout <<  "makeAllAttributeQuery: attributeList.size = " <<attributeList.size()<< std::endl;
    std::cout <<  "makeAllAttributeQuery: actualList = " << actualList.size() << std::endl;

    if (actualList.size() == 0) {
        // try to get attribute list from file
        config::getAttributeList("/Users/vbrenet/Documents/Pocs/curltry/", name, actualList);
        std::cout <<  "makeAllAttributeQuery: actualList from file = " << actualList.size() << std::endl;
        if (actualList.size() > 0) {
            for (auto it = actualList.begin(); it != actualList.end(); it++) {
                addAttribute({*it,false});
            }
        }
    }
    
    query = "Select ";
    
    for (auto i=0; i < actualList.size(); i++) {
        
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
void sObject::outputAttributeCounters(const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++)
        ofs << it->first << " : " << it->second << std::endl;

    ofs.close();
}
//
//
//
void sObject::computeCsvRecords(const std::string &csvString, const std::string &outputfile) {
    std::ofstream ofs {outputfile};
    bool firstRecord {true};
    bool errorFound {false};

    std::string token {};
    std::string record {};
    
    enum class state {START_TOKEN, QUOTE_RECEIVED, TOKEN_IN_PROGRESS, RETURN_IN_PROGRESS, PARSING_ERROR};
    state currentState {state::START_TOKEN};
    
    for (char c : csvString) {
        switch (currentState) {
            case state::START_TOKEN:
                break;
            case state::QUOTE_RECEIVED:
                break;
            case state::TOKEN_IN_PROGRESS:
                break;
            case state::RETURN_IN_PROGRESS:
                break;
            case state::PARSING_ERROR:
                std::cerr << "computeCsvRecords : parsing error" << std::endl;
                errorFound = true;
                break;
            default:
                break;
        }
        if (errorFound)
            break;
    }
    
    ofs.close();

}
