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
