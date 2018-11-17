//
//  recordGenerator.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "recordGenerator.hpp"

//
//
const std::string recordGenerator::getCsvHeader() const {
    std::string header;
    
    for (std::vector<genericAttribute>::const_iterator it = attributes.begin() ; it != attributes.end(); ++it)
        header += (it->getCsvName() + ',');
    
    return header;
}
//
//
const std::string recordGenerator::getCsvRecord() const {
    std::string value;
    
    for (std::vector<genericAttribute>::const_iterator it = attributes.begin() ; it != attributes.end(); ++it)
        value += (it->getCsvValue() + ',');
    
    return value;

}
