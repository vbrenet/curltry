//
//  nameAttribute.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "nameAttribute.hpp"
#include "corpNameGenerator.hpp"


 std::string nameAttribute::getCsvValue() const {
    return "\"" + corpNameGenerator::getName() + "\"";
}
