//
//  fieldBook.cpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include "fieldBook.hpp"
#include "utils.hpp"
#include "fieldDefinition.hpp"
//
//
std::string fieldBook::getValue(const std::string attributeName, const std::string &buffer, size_t token, const char endDelimeter) {
    std::string value {};
    size_t begin = buffer.find(attributeName, token);
    if (begin != std::string::npos) {
        size_t semicolon = buffer.find_first_of(':',begin);
        size_t end = buffer.find_first_of(endDelimeter,begin);
        value = buffer.substr(semicolon+1,end-semicolon-1);
        // strip " in value
        removeDoubleQuote(value);
    }
    return value;
}
//
//
void fieldBook::parseSingleFieldDefinition(const std::string &buffer, size_t token) {
    /* ex:
     {"attributes":{"type":"FieldDefinition","url":"/services/data/v47.0/sobjects/FieldDefinition/Lead.Id"},"BusinessOwnerId":null,"BusinessStatus":null,"ComplianceGroup":null,"DataType":"Lookup()","Description":null,"ExtraTypeInfo":null,"IsAiPredictionField":false,"IsApiFilterable":true,"IsApiGroupable":true,"IsApiSortable":true,"IsCalculated":false,"IsCompactLayoutable":false,"IsCompound":false,"IsFieldHistoryTracked":false,"IsHighScaleNumber":false,"IsHtmlFormatted":false,"IsIndexed":true,"IsListFilterable":true,"IsListSortable":true,"IsListVisible":true,"IsNameField":false,"Label":"Lead ID","LastModifiedById":null,"LastModifiedDate":null,"Length":18,"MasterLabel":"Lead ID","NamespacePrefix":null,"Precision":0,"QualifiedApiName":"Id","Scale":0,"SecurityClassification":null}
     */
    
    fieldDefinition theField {entityName};
    
    theField.BusinessOwnerId = getValue("BusinessOwnerId", buffer, token, ',');
    theField.BusinessStatus = getValue("BusinessStatus", buffer, token, ',');
    theField.ComplianceGroup = getValue("ComplianceGroup", buffer, token, ',');
    theField.DataType = getValue("DataType", buffer, token, ',');
    theField.Description = getValue("Description", buffer, token, ',');
    theField.ExtraTypeInfo = getValue("ExtraTypeInfo", buffer, token, ',');
    theField.IsAiPredictionField = getBooleanValue(getValue("IsAiPredictionField", buffer, token, ','));
    theField.IsApiFilterable = getBooleanValue(getValue("IsApiFilterable", buffer, token, ','));
    theField.IsApiGroupable = getBooleanValue(getValue("IsApiGroupable", buffer, token, ','));
    theField.IsApiSortable = getBooleanValue(getValue("IsApiSortable", buffer, token, ','));
    theField.IsCalculated = getBooleanValue(getValue("IsCalculated", buffer, token, ','));
    theField.IsCompactLayoutable = getBooleanValue(getValue("IsCompactLayoutable", buffer, token, ','));
    theField.IsCompound = getBooleanValue(getValue("IsCompound", buffer, token, ','));
    theField.IsFieldHistoryTracked = getBooleanValue(getValue("IsFieldHistoryTracked", buffer, token, ','));
    theField.IsHighScaleNumber = getBooleanValue(getValue("IsHighScaleNumber", buffer, token, ','));
    theField.IsHtmlFormatted = getBooleanValue(getValue("IsHtmlFormatted", buffer, token, ','));
    theField.IsIndexed = getBooleanValue(getValue("IsIndexed", buffer, token, ','));
    theField.IsListFilterable = getBooleanValue(getValue("IsListFilterable", buffer, token, ','));
    theField.IsListSortable = getBooleanValue(getValue("IsListSortable", buffer, token, ','));
    theField.IsListVisible = getBooleanValue(getValue("IsListVisible", buffer, token, ','));
    theField.IsNameField = getBooleanValue(getValue("IsNameField", buffer, token, ','));
    theField.IsNameField = getBooleanValue(getValue("IsNameField", buffer, token, ','));
    theField.Label = getValue("Label", buffer, token, ',');
    theField.LastModifiedById = getValue("LastModifiedById", buffer, token, ',');
    theField.LastModifiedDate = getValue("LastModifiedDate", buffer, token, ',');
    std::string length = getValue("Length", buffer, token, ',');
    if (isStringNumeric(length))
        theField.Length = std::stol(length);
    theField.MasterLabel = getValue("MasterLabel", buffer, token, ',');
    theField.NamespacePrefix = getValue("NamespacePrefix", buffer, token, ',');
    theField.Precision = getValue("Precision", buffer, token, ',');
    theField.QualifiedApiName = getValue("QualifiedApiName", buffer, token, ',');
    theField.Scale = getValue("Scale", buffer, token, ',');
    theField.SecurityClassification = getValue("SecurityClassification", buffer, token, '}');
    
    // put the created field in the field map
}

//
void fieldBook::parseFieldDefinitionBuffer(const std::string &buffer) {
    /* ex:
     "{"totalSize":122,"done":true,"records":[{"attributes":{"type":"FieldDefinition","url":"/services/data/v47.0/sobjects/FieldDefinition/Lead.Id"},"BusinessOwnerId":null,"BusinessStatus":null,"ComplianceGroup":null,"DataType":"Lookup()","Description":null,"ExtraTypeInfo":null,"IsAiPredictionField":false,"IsApiFilterable":true,"IsApiGroupable":true,"IsApiSortable":true,"IsCalculated":false,"IsCompactLayoutable":false,"IsCompound":false,"IsFieldHistoryTracked":false,"IsHighScaleNumber":false,"IsHtmlFormatted":false,"IsIndexed":true,"IsListFilterable":true,"IsListSortable":true,"IsListVisible":true,"IsNameField":false,"Label":"Lead ID","LastModifiedById":null,"LastModifiedDate":null,"Length":18,"MasterLabel":"Lead ID","NamespacePrefix":null,"Precision":0,"QualifiedApiName":"Id","Scale":0,"SecurityClassification":null},{"attributes":{"type":"FieldDefinition","url":"/services/data/v47.0/sobjects/FieldDefinition/Lead.IsDeleted"},"BusinessOwnerId":null,"BusinessStatus":null,"ComplianceGroup":null,"DataType":"Checkbox",""...
     */
    bool terminated {false};
    size_t offset {0};
    while (!terminated) {
        size_t token = buffer.find("{\"attributes\":", offset);
        if (token != std::string::npos) {
            offset += token+1;
            parseSingleFieldDefinition(buffer, token);
        }
        else {
            terminated = true;
        }
    }
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
