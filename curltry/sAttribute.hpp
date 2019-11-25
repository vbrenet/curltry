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
    std::string type;
    std::string defaultValue;
    bool excluded {false};
    bool custom {false};
    bool picklist {false};
public:
    // constructors
    sAttribute() {};
    sAttribute (const std::string attr, bool excl) : name {attr}, excluded{excl} {}
    sAttribute (const std::string attr, bool excl, bool cust) : name {attr}, excluded{excl}, custom{cust} {}
    sAttribute (const std::string attr, bool excl, bool cust, bool pick) : name {attr}, excluded{excl}, custom{cust}, picklist{pick} {}
    sAttribute (const std::string attr, bool excl, bool cust, bool pick, const std::string t) : name {attr}, excluded{excl}, custom{cust}, picklist{pick}, type{t} {}

    // copy constructor
    sAttribute(const sAttribute &attr);
    
    const std::string &getName() const {return name;};
    const std::string &getType() const {return type;};
    const std::string &getDefaultValue() const {return defaultValue;};

    bool isExcluded() const {return excluded;};
    bool isCustom() const {return custom;};
    bool isPicklist() const {return picklist;};
};
