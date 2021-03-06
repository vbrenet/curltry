//
//  genericAttribute.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef genericAttribute_hpp
#define genericAttribute_hpp

#include <stdio.h>
#include <string>


class genericAttribute {
    
protected:
    std::string name;
    std::string csvName;

public:
    genericAttribute(std::string n);
    virtual ~genericAttribute() {}
    const std::string& getCsvName() const {return csvName;}
    const std::string& getName() const {return name;}
    virtual std::string getCsvValue() const = 0;
    
};

#endif /* genericAttribute_hpp */
