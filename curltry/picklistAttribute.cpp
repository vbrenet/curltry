//
//  picklistAttribute.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "picklistAttribute.hpp"

picklistAttribute::picklistAttribute(const std::string& n, const std::vector<std::string>& v) {
    name = n;
    csvName = "\"" + name + "\"";
    values = v;
}
