//
//  sAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef sAttribute_hpp
#define sAttribute_hpp

#include <stdio.h>
#include <string>


#endif /* sAttribute_hpp */

class sAttribute {
private:
    std::string name;
public:
    sAttribute (const std::string attr) : name {attr} {}
    const std::string &getName() const {return name;};
};
