//
//  expectedParameters.cpp
//  getparams
//
//  Created by Vincent Brenet on 24/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "expectedParameters.hpp"

bool expectedParameters::isValued(const std::string parameterName) const {    
    auto d = descriptors.find(parameterName);
    if (d != descriptors.end())
        return d->second.isValued;
    return false;
}
