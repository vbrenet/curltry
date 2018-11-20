//
//  idAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 20/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef idAttribute_hpp
#define idAttribute_hpp

#include "genericAttribute.hpp"
#include <stdio.h>
#include <string>

class idAttribute : public genericAttribute {
private:
    std::string id;
public:
    idAttribute(const std::string& n, std::string s) : genericAttribute(n), id{s} {}
    
    std::string getCsvValue() const {return "\"" + id + "\"";}
};

#endif /* idAttribute_hpp */
