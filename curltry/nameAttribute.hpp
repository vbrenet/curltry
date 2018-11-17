//
//  nameAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef nameAttribute_hpp
#define nameAttribute_hpp

#include <stdio.h>
#include "genericAttribute.hpp"

class nameAttribute : public genericAttribute {
private:
public:
    nameAttribute(std::string s);
    ~nameAttribute() {}

    const std::string getCsvValue() const;
};

#endif /* nameAttribute_hpp */
