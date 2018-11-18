//
//  recordGenerator.cpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "recordGenerator.hpp"
#include "nameAttribute.hpp"
#include "textAttribute.hpp"
#include "integerAttribute.hpp"
#include "picklistAttribute.hpp"
#include <fstream>
#include <vector>

//
//
const std::string recordGenerator::getCsvHeader() const {
    std::string header {};
    
    for (std::vector<genericAttribute *>::const_iterator it = attributes.begin() ; it != attributes.end(); ++it) {
        genericAttribute *pt = *it;
        header += (pt->getCsvName() + ',');

    }
    
    return header;
}
//
//
const std::string recordGenerator::getCsvRecord() const {
    std::string value {};
    
    for (std::vector<genericAttribute *>::const_iterator it = attributes.begin() ; it != attributes.end(); ++it) {
        genericAttribute *pt = *it;
        value += (pt->getCsvValue() + ',');
    }
    
    return value;
}
//
//
void recordGenerator::fillValues(const std::string source, std::vector<std::string>& target) {
    
}
//
void recordGenerator::processLine(const std::string& line) {
    if (line[0] == '#') return; // skip comments
    size_t firstcolon = line.find_first_of(':');
    if (firstcolon != std::string::npos) {
        std::string attributeName = line.substr(0,firstcolon);
        size_t secondcolon = line.find_first_of(':',firstcolon);
        std::string attributeType {};
        if (secondcolon == std::string::npos)
            attributeType = line.substr(firstcolon);
        else
            attributeType = line.substr(firstcolon,secondcolon-firstcolon);
        if (attributeType.compare("nameAttribute") == 0) {
            nameAttribute *att = new nameAttribute(attributeName);
            attributes.push_back(att);
        } else if (attributeType.compare("text") == 0) {
            std::string textLength = line.substr(secondcolon);
            textAttribute *att = new textAttribute(attributeName, std::atoi(textLength.c_str()));
            attributes.push_back(att);
        } else if (attributeType.compare("integer") == 0) {
            integerAttribute *att = new integerAttribute(attributeName);
            attributes.push_back(att);
        } else if (attributeType.compare("picklist") == 0) {
            std::vector<std::string> values {};
            fillValues(line.substr(secondcolon), values);
            picklistAttribute *att = new picklistAttribute(attributeName, values);
            attributes.push_back(att);
        }
    } // first colon found
}
//
recordGenerator::recordGenerator(std::string pathname) {
    std::ifstream configFile {pathname};
    std::string currentLine;
    
    while (getline(configFile,currentLine)) {
        processLine(currentLine);
    }
}
//
//
recordGenerator::~recordGenerator() {
//  clean attributes vector
}
