//
//  textAttribute.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "textAttribute.hpp"
#include "textGenerator.hpp"

 std::string textAttribute::getCsvValue() const {
    std::string s = textGenerator::gettext();
    s = s.substr(0,size);
    return "\"" + s + "\"";
}
//

