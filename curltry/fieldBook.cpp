//
//  fieldBook.cpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include "fieldBook.hpp"

//
fieldBook::fieldBook(const std::string objectName) {
    entityName = objectName;
    
    // query entityDefinition table to get the durable id
    // ex : SELECT DurableId FROM EntityDefinition WHERE QualifiedApiName = 'Sinistre__c'
    
    
    
    // query FieldDefinition table using EntityDefinitionId
    // ex : SELECT DataType ... FROM FieldDefinition WHERE EntityDefinitionId = 'Case'
}
//
//
//
void fieldBook::outputFieldBook() const {
    
}
