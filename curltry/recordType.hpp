//
//  recordType.hpp
//  curltry
//
//  Created by Vincent Brenet on 14/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef recordType_hpp
#define recordType_hpp

#include <stdio.h>
#include <string>
class recordType {
private:
    std::string id {};
    std::string name {};
public:
    recordType(std::string i,std::string n) : id {i}, name {n} {};
    const std::string& getId() const {return id;};
    const std::string& getName() const {return name;}
};
#endif /* recordType_hpp */
