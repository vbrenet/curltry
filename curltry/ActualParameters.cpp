//
//  ActualParameters.cpp
//  getparams
//
//  Created by Vincent Brenet on 24/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include "ActualParameters.hpp"

//
//
//
bool ActualParameters::areValid(const expectedParameters& expected) const {
    
    if (expected.isLimitedNbOfValues()) {
        if (values.size() > expected.getMaxValues()) {
            std::cerr << "too many parameter values" << std::endl;
            return false;
        }
    }
    
    const std::map<std::string, expectedParameters::descriptor> descriptors = expected.getDescriptors();
    
    // scan parameters to check if they are defined in the descriptor
    for (NamedParameter n : namedParameters) {
        
        auto d = descriptors.find(n.getName());
        if (d != descriptors.end()) {
            if (d->second.isValued && n.getValue().size() == 0) {
                std::cerr << "parameter " << n.getName() << " must have a value" << std::endl;
                return false;
            }
        }
        else {
            std::cerr << "parameter " << n.getName() << " not allowed" << std::endl;
            return false;
        }
        
    } // end for n in nameParameters
    
    // scan descriptor to check that mandatory parameters are existing
    for (auto d : descriptors) {
        if (d.second.isMandatory) {
            bool found {false};
            for (auto n : namedParameters) {
                if (n.getName().compare(d.first) == 0) {
                    found = true;
                    break;
                }
            }   // end for n in namedParameters
            if (!found) {
                std::cerr << "missing mandatory parameter: " << d.first << std::endl;
                return false;
            }
        }
    }
    
    return true;
}
//
//
//
bool ActualParameters::set(int argc, const char * argv[], const expectedParameters& expected) {
    std::vector<std::string> parameters {};
    
    for (int i=0; i < argc; i++) {
        parameters.push_back(argv[i]);
    }
    
    enum class status {
        READ_VALUE,
        READ_NAMED,
        READ_VALUE_AFTER_NAMED,
        READ_VALUE_LOCKED
    };
    
    status status = status::READ_VALUE;
    std::string currNamed {};
    
    for (std::string curr : parameters) {
        bool isNamed {false};
        if (curr.size() > 0 && curr[0] == '-')
            isNamed = true;
        switch (status) {
            case status::READ_VALUE:
                if (isNamed) {
                    currNamed = curr;
                    status = status::READ_NAMED;
                } else
                    values.push_back(curr);
                break;
            case status::READ_NAMED:
                if (isNamed) {
                    namedParameters.push_back({currNamed,{}});
                    currNamed = curr;
                } else {
                    if (expected.isValued(currNamed)) {
                        namedParameters.push_back({currNamed,curr});
                        status = status::READ_VALUE_AFTER_NAMED;
                    } else {
                        namedParameters.push_back({currNamed,{}});
                        values.push_back(curr);
                        status = status::READ_VALUE_LOCKED;
                    }
                }
                break;
            case status::READ_VALUE_AFTER_NAMED:
                if (isNamed) {
                    currNamed = curr;
                    status = status::READ_NAMED;
                } else {
                    values.push_back(curr);
                    status = status::READ_VALUE_LOCKED;
                }
                break;
            case status::READ_VALUE_LOCKED:
                if (isNamed) {
                    return false;
                } else {
                    values.push_back(curr);
                }
                break;

            default:
                break;
        } // end switch
    } // end for
    
    if (status == status::READ_NAMED)
        namedParameters.push_back({currNamed,{}});

    auto it = values.begin();
    currExecName = *it;
    values.erase(it);
    
    return areValid(expected);
}
//
//
void ActualParameters::print() const {
    std::cout << "exec name: " << currExecName << std::endl;
    if (namedParameters.size() == 0) {
        std::cout << "zero valued parameters" << std::endl;
    } else {
        for (NamedParameter curr : namedParameters) {
            std::cout << curr.getName() << " : " << curr.getValue() << std::endl;
        }
    }
    
    if (values.size() == 0) {
        std::cout << "zero values" << std::endl;
    } else {
        for (std::string v : values) {
            std::cout << "value: " << v << std::endl;
        }
    }
}
