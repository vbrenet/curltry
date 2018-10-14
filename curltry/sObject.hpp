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
#include <map>

#include "sAttribute.hpp"


#endif /* sObject_hpp */

class sObject{
private:
    std::string name;
    std::vector<sAttribute> attributeList {};
    std::map<std::string,int> attributeCounters {};
    void computeAttributes(const std::string &record, int);

public:
    sObject (const std::string objname)  : name {objname} {}
    const std::string& getName() {return name;};
    void addAttribute(sAttribute a) {attributeList.push_back({a});}
    std::string makeAllAttributeQuery();
    void print() const;
    void computerecords(const std::string &xmlresult);
    void printAttributeCounters();
};
