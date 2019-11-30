//
//  fieldBook.cpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include "fieldBook.hpp"
#include "utils.hpp"
//
//
void fieldBook::parseFieldDefinitionBuffer(const std::string &buffer) {
    /* ex:
     "{"totalSize":122,"done":true,"records":[{"attributes":{"type":"FieldDefinition","url":"/services/data/v47.0/sobjects/FieldDefinition/Lead.Id"},"BusinessOwnerId":null,"BusinessStatus":null,"ComplianceGroup":null,"DataType":"Lookup()","Description":null,"ExtraTypeInfo":null,"IsAiPredictionField":false,"IsApiFilterable":true,"IsApiGroupable":true,"IsApiSortable":true,"IsCalculated":false,"IsCompactLayoutable":false,"IsCompound":false,"IsFieldHistoryTracked":false,"IsHighScaleNumber":false,"IsHtmlFormatted":false,"IsIndexed":true,"IsListFilterable":true,"IsListSortable":true,"IsListVisible":true,"IsNameField":false,"Label":"Lead ID","LastModifiedById":null,"LastModifiedDate":null,"Length":18,"MasterLabel":"Lead ID","NamespacePrefix":null,"Precision":0,"QualifiedApiName":"Id","Scale":0,"SecurityClassification":null},{"attributes":{"type":"FieldDefinition","url":"/services/data/v47.0/sobjects/FieldDefinition/Lead.IsDeleted"},"BusinessOwnerId":null,"BusinessStatus":null,"ComplianceGroup":null,"DataType":"Checkbox",""...
     */
    
}
//
//
void fieldBook::makeFieldDefinitionQuery(const std::string durableId, std::string & query) const {
    // query FieldDefinition table using EntityDefinitionId
    // ex : SELECT DataType ... FROM FieldDefinition WHERE EntityDefinitionId = 'Case'

    query = "?q=SELECT";
    query += "+BusinessOwnerId+,";
    query += "+BusinessStatus+,";
    query += "+ComplianceGroup+,";
    query += "+DataType+,";
    query += "+Description+,";
    query += "+ExtraTypeInfo+,";
    query += "+IsAiPredictionField+,";
    query += "+IsApiFilterable+,";
    query += "+IsApiGroupable+,";
    query += "+IsApiSortable+,";
    query += "+IsCalculated+,";
    query += "+IsCompactLayoutable+,";
    query += "+IsCompound+,";
    query += "+IsFieldHistoryTracked+,";
    query += "+IsHighScaleNumber+,";
    query += "+IsHtmlFormatted+,";
    query += "+IsIndexed+,";
    query += "+IsListFilterable+,";
    query += "+IsListSortable+,";
    query += "+IsListVisible+,";
    query += "+IsNameField+,";
    query += "+Label+,";
    query += "+LastModifiedById+,";
    query += "+LastModifiedDate+,";
    query += "+Length+,";
    query += "+MasterLabel+,";
    query += "+NamespacePrefix+,";
    query += "+Precision+,";
    query += "+QualifiedApiName+,";
    query += "+Scale+,";
    query += "+SecurityClassification";
    query += "+FROM+FieldDefinition+where+EntityDefinitionId+=+'";
    query += durableId;
    query += "'";
}
//
//
//
std::string fieldBook::getDurableId (const std::string &buffer) const {
    /* ex:
     buffer = "{"totalSize":1,"done":true,"records":[{"attributes":{"type":"EntityDefinition","url":"/services/data/v47.0/sobjects/EntityDefinition/Lead"},"DurableId":"Lead"}]}"
     */
    std::string durableId {};
    size_t begintoken = buffer.find("DurableId");
    if (begintoken != std::string::npos) {
        size_t endtoken = buffer.find("\"}", begintoken);
        if (endtoken != std::string::npos)
            durableId = buffer.substr(begintoken+12,endtoken-begintoken-12);
    }
    return durableId;
}
//
bool fieldBook::setFieldBook(const std::string objectName) {
    entityName = objectName;
    
    // query entityDefinition table to get the durable id
    // ex : SELECT DurableId FROM EntityDefinition WHERE QualifiedApiName = 'Sinistre__c'
    std::string query = "?q=SELECT+DurableId+FROM+EntityDefinition+where+QualifiedApiName+=+'" + entityName + "'";
    std::string buffer;
    bool result = restQuery(query, buffer);
    
    if (result) {
        std::string durableId = getDurableId(buffer);
        std::string fieldDefinitionQuery {};
        makeFieldDefinitionQuery(durableId,fieldDefinitionQuery);
        result = restQuery(fieldDefinitionQuery, buffer);
        if (result) {
            parseFieldDefinitionBuffer(buffer);
        }
    }
    
    return result;
}
//
//
//
void fieldBook::outputFieldBook() const {
    
}
