//
//  checkboxAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 14/01/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef checkboxAttribute_hpp
#define checkboxAttribute_hpp

#include <stdio.h>
#include "genericAttribute.hpp"

class checkBoxAttribute : public genericAttribute {
private:
    std::string checkBoxValue;
public:
    checkBoxAttribute(std::string s, std::string b) : genericAttribute(s), checkBoxValue{b} {}
    ~checkBoxAttribute() {}
    std::string getCsvValue() const {return "\"" + checkBoxValue + "\"";}
};
#endif /* checkboxAttribute_hpp */
