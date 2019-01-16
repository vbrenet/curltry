//
//  expectedParameters.hpp
//  getparams
//
//  Created by Vincent Brenet on 24/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef expectedParameters_hpp
#define expectedParameters_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include "NamedParameter.hpp"

class expectedParameters {
public:
    struct descriptor {
        bool isMandatory;
        bool isValued;
    };
private:
    bool limitedNbOfValues;
    int maxvalues;
    std::map<std::string, descriptor> descriptors;
public:
    expectedParameters() {};
    expectedParameters(bool b, int i, std::map<std::string, descriptor> vp) : limitedNbOfValues{b}, maxvalues{i}, descriptors{vp} {}
    bool isLimitedNbOfValues() const {return limitedNbOfValues;};
    int getMaxValues() const {return maxvalues;};
    const std::map<std::string, descriptor>& getDescriptors() const {return descriptors;};
    bool isValued(const std::string parameterName) const;
};

#endif /* expectedParameters_hpp */
