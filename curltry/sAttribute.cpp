//
//  sAttribute.cpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "sAttribute.hpp"

sAttribute::sAttribute( const sAttribute &attr) {
    name = attr.name;
    type = attr.type;
    defaultValue = attr.defaultValue;
    excluded = attr.excluded;
    custom = attr.custom;
    picklist = attr.picklist;
}
