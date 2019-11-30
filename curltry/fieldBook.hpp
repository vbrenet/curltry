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
    
    std::string getDurableId (const std::string &buffer) const;
    void makeFieldDefinitionQuery(const std::string durableId, std::string & fieldDefinitionQuery) const;
    void parseFieldDefinitionBuffer(const std::string &buffer);
    void parseSingleFieldDefinition(const std::string &buffer, size_t token);
    std::string getValue(const std::string attributeName, const std::string &buffer, size_t token, const char end);

public:
    fieldBook() {};
    bool setFieldBook(const std::string objectName);
    void outputFieldBook() const;
};

#endif /* fieldBook_hpp */
