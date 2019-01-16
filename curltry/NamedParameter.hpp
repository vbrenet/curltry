//
//  NamedParameter.hpp
//  getparams
//
//  Created by Vincent Brenet on 24/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef NamedParameter_hpp
#define NamedParameter_hpp

#include <stdio.h>
#include <string>

class NamedParameter {
private:
    std::string name;
    std::string value;
public:
    NamedParameter(const std::string n, const std::string v) : name {n}, value {v} {}
    const std::string getName() const {return name;}
    const std::string getValue() const {return value;}
};

#endif /* NamedParameter_hpp */
