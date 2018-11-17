//
//  textAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef textAttribute_hpp
#define textAttribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "genericAttribute.hpp"

class textAttribute : public genericAttribute {
private:
    int size;
public:
    textAttribute(const std::string& n, int s) : genericAttribute(n) {size = s;}
    const std::string getCsvValue() const;
};
#endif /* textAttribute_hpp */
