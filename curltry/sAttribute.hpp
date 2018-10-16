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
    bool excluded {false};
public:
    sAttribute (const std::string attr, bool excl) : name {attr}, excluded{excl} {}
    const std::string &getName() const {return name;};
    bool isExcluded() const {return excluded;};
    //void setExcluded(bool excl) {excluded=excl;}
};
