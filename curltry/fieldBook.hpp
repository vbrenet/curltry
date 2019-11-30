//
//  fieldBook.hpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef fieldBook_hpp
#define fieldBook_hpp

#include <stdio.h>
#include <string>
#include <map>
#include "fieldDefinition.hpp"

class fieldBook {
private:
    std::string entityName;
    std::map<std::string, fieldDefinition> fieldDefinitionMap;
public:
    fieldBook(const std::string objectName);
    void outputFieldBook() const;
};

#endif /* fieldBook_hpp */
