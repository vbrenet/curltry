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
    packageName = attr.packageName;
    package = attr.package;
}
//
//
void sAttribute::setPackageName() {
    if (isCustom()) {
        size_t firstDoubleUnderscore = name.find("__");
        if (firstDoubleUnderscore != std::string::npos) {
            size_t secondDoubleUnderscore = name.find("__", firstDoubleUnderscore+2);
            if (secondDoubleUnderscore != std::string::npos) {
                this->package = true;
                this->packageName = name.substr(0,firstDoubleUnderscore);
            }
        }
    }
}

