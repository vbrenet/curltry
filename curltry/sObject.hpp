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

#include "sAttribute.hpp"

class sObject{
private:
    std::string name;
    std::vector<sAttribute> attributeList {};
    std::map<std::string,long> attributeCounters {};
    std::map<std::pair<std::string,std::string>,long> recordTypeMatrixCounters {};//key: (record type id, attribute name)
    std::map<int,std::string> csvAttributeMap {};
    void computeAttributes(const std::string &record, int);

public:
    sObject (const std::string objname)  : name {objname} {}
    const std::string& getName() {return name;};
    void addAttribute(sAttribute a) {attributeList.push_back({a});}
    std::string makeAllAttributeQuery();
    void print() const;
    void computerecords(const std::string &xmlresult);
    long computeCsvRecords(const std::string &);
    void printAttributeCounters();
    void outputAttributeCounters(const std::string &);
    void outputMatrixCounters(const std::string &outputfile);

};

#endif /* sObject_hpp */
