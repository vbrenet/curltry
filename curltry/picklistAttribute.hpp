//
//  picklistAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef picklistAttribute_hpp
#define picklistAttribute_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "genericAttribute.hpp"

class picklistAttribute : public genericAttribute {
private:
    std::vector<std::string> values;
public:
    picklistAttribute(const std::string& n, const std::vector<std::string>& v);
    ~picklistAttribute() {}
    const std::string getCsvValue() const {return "\"" + values[rand() % values.size()] + "\"";};
};

#endif /* picklistAttribute_hpp */
