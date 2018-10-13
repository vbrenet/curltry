//
//  sObject.cpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include "sObject.hpp"

void sObject::print() const {
    for (sAttribute s : attributeList)
        std::cout << "Attribute: " << s.getName() << std::endl;
}
//
//
std::string sObject::makeAllAttributeQuery() {
    std::string query {};
    
    std::cout <<  "makeAllAttributeQuery: attributeList.size = " <<attributeList.size()<< std::endl;
    
    query = "Select ";
    
    for (auto i=0; i < attributeList.size(); i++) {
        if (attributeList[i].getName().find("__c") == std::string::npos)
            continue;

        query += attributeList[i].getName();
        if (i != (attributeList.size() -1))
            query += ",";
    }
    
    query += " from " + getName();
    
    return query;
}
