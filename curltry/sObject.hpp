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
#include "sAttribute.hpp"


#endif /* sObject_hpp */

class sObject{
private:
    std::string name;
    std::vector<sAttribute> attributeList {};
public:
    sObject (const std::string objname)  : name {objname} {}
    void addAttribute(sAttribute a) {attributeList.push_back({a});}
    void print() const;
};
