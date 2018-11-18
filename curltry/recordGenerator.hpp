//
//  recordGenerator.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef recordGenerator_hpp
#define recordGenerator_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include "genericAttribute.hpp"

class recordGenerator {
private:
    std::vector<genericAttribute *> attributes;
    void processLine(const std::string&);
    void fillValues(const std::string, std::vector<std::string>&);
public:
    recordGenerator(std::string pathname);
    recordGenerator(std::vector<genericAttribute *>& attlist) : attributes {attlist} {}
    ~recordGenerator();
    const std::string getCsvHeader() const;
    const std::string getCsvRecord() const;
};

#endif /* recordGenerator_hpp */
