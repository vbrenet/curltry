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
#include "idAttribute.hpp"
#include "checkboxAttribute.hpp"
#include <fstream>
#include <vector>
#include <stdio.h>


//
//
const std::string recordGenerator::getCsvHeader() const {
    std::string header {};
    
    for (std::vector<genericAttribute *>::const_iterator it = attributes.begin() ; it != attributes.end(); ++it) {
        genericAttribute *pt = *it;
        header += (pt->getCsvName() + ',');

    }

    size_t found = header.find_last_of(',');
    
    if (found != std::string::npos)
        header.erase(found, 1);
    
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
    
    size_t found = value.find_last_of(',');
    
    if (found != std::string::npos)
        value.erase(found, 1);

    
    return value;
}
//
//
std::string recordGenerator::removeQuotes (const std::string& input) {
    std::string result {input};
    std::size_t found;
    
    while ((found = result.find("\"")) != std::string::npos) {
        result.erase(found, 1);
    }
    
    return result;
}
//
void recordGenerator::fillValues(std::string source, std::vector<std::string>& target) {
    std::string delim = ",";
    size_t start {0};
    size_t end = source.find(delim);
    while (end != std::string::npos) {
        std::string token = source.substr(start, end - start);
        target.push_back(removeQuotes(token));
        start = end + delim.length();
        end = source.find(delim, start);
    }
    std::string token = source.substr(start, end);
    target.push_back(removeQuotes(token));

}
//
void recordGenerator::processLine(const std::string& line) {
    if (line[0] == '#') return; // skip comments
    size_t firstcolon = line.find_first_of(':');
    if (firstcolon != std::string::npos) {
        std::string attributeName = line.substr(0,firstcolon);
        size_t secondcolon = line.find_first_of(':',firstcolon+1);
        std::string attributeType {};
        if (secondcolon == std::string::npos)
            attributeType = line.substr(firstcolon+1);
        else
            attributeType = line.substr(firstcolon+1,secondcolon-firstcolon-1);
        if (attributeType.compare("nameAttribute") == 0) {
            nameAttribute *att = new nameAttribute(attributeName);
            attributes.push_back(att);
        } else if (attributeType.compare("text") == 0) {
            std::string textLength = line.substr(secondcolon+1);
            textAttribute *att = new textAttribute(attributeName, std::atoi(textLength.c_str()));
            attributes.push_back(att);
        } else if (attributeType.compare("integer") == 0) {
            integerAttribute *att = new integerAttribute(attributeName);
            attributes.push_back(att);
        } else if (attributeType.compare("picklist") == 0) {
            std::vector<std::string> values {};
            fillValues(line.substr(secondcolon+1), values);
            picklistAttribute *att = new picklistAttribute(attributeName, values);
            attributes.push_back(att);
        } else if (attributeType.compare("id") == 0) {
            idAttribute *att = new idAttribute(attributeName, line.substr(secondcolon+1));
            attributes.push_back(att);
        } else if (attributeType.compare("checkbox") == 0) {
            checkBoxAttribute *att = new checkBoxAttribute(attributeName, line.substr(secondcolon+1));
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
    for (auto p : attributes)
        delete p;
    attributes.clear();
}
