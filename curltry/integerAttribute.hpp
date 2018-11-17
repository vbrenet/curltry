//
//  integerAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef integerAttribute_hpp
#define integerAttribute_hpp

#include <stdio.h>
#include <string>

#include "genericAttribute.hpp"

class integerAttribute : public genericAttribute {
private:
public:
    const std::string getCsvValue() const {return "\"" + std::to_string(rand()%100000) + "\"";};
};

#endif /* integerAttribute_hpp */
