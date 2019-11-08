//
//  sObject.hpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef sObject_hpp
#define sObject_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <tuple>

#include "sAttribute.hpp"

class sObject{
private:
    std::string name;
    std::vector<sAttribute> attributeList {};
    std::map<std::string,long> attributeCounters {};
    std::map<std::pair<std::string,std::string>,long> recordTypeMatrixCounters {};//key: (record type id, attribute name)
    std::map<int,std::string> csvAttributeMap {};
    std::map<std::string,std::string> recordTypes {}; // key : id, value : name
    
    void computeAttributes(const std::string &record, int);

    void initRecordTypeMatrixCounters();
    void processCsvLine(const std::string &inputline);
    void processMatrixLine(const std::string &inputline);
    void initializeCounter(const std::string& attribute, const std::string& countervalue);
    void parseRecordTypeBuffer(const std::string&);
    std::string getnamebyid(const std::string id);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    bool isAttributeCustom(const std::string) const;
public:
    sObject (const std::string objname)  : name {objname} {}
    const std::string& getName() {return name;};
    void addAttribute(sAttribute a) {attributeList.push_back({a});}
    std::string makeAllAttributeQuery();
    void print() const;
    void computerecords(const std::string &xmlresult);
    long computeCsvRecords(const std::string &);
    void printAttributeCounters();
    void initializeAttributeCounters(const std::string &inputfile);
    bool initializeRecordTypes();
    void initializeMatrixCountersFromFile(const std::string &inputfile);
    void outputAttributeCounters(const std::string &);
    void outputMatrixCounters(const std::string &outputfile);

};

#endif /* sObject_hpp */
