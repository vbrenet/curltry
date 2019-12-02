//
//  fieldBook.cpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//
#include <fstream>
#include <iostream>
#include "fieldBook.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "fieldDefinition.hpp"
#include "config.hpp"
//
//
std::string fieldBook::getValue(const std::string attributeName, const std::string &buffer, size_t token, const std::string endDelimeter) {
    std::string value {};
    size_t begin = buffer.find(attributeName, token);
    
    if (begin != std::string::npos) {
        size_t semicolon = buffer.find_first_of(':',begin);
        size_t end = buffer.find(endDelimeter,semicolon);
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
    
    std::string QualifiedApiName = getValue("QualifiedApiName", buffer, token, ",\"");
    fieldDefinitionMap.insert((std::pair<std::string,fieldDefinition>){QualifiedApiName,{entityName}});
    auto it = fieldDefinitionMap.find(QualifiedApiName);

    if (it != fieldDefinitionMap.end()) {
        it->second.QualifiedApiName = QualifiedApiName;
        it->second.BusinessOwnerId = getValue("BusinessOwnerId", buffer, token, ",\"");
        it->second.BusinessStatus = getValue("BusinessStatus", buffer, token, ",\"");
        it->second.ComplianceGroup = getValue("ComplianceGroup", buffer, token, ",\"");
        it->second.DataType = getValue("DataType", buffer, token, ",\"");
        it->second.ExtraTypeInfo = getValue("ExtraTypeInfo", buffer, token, ",\"");
        it->second.IsAiPredictionField = getBooleanValue(getValue("IsAiPredictionField", buffer, token, ",\""));
        it->second.IsApiFilterable = getBooleanValue(getValue("IsApiFilterable", buffer, token, ",\""));
        it->second.IsApiGroupable = getBooleanValue(getValue("IsApiGroupable", buffer, token, ",\""));
        it->second.IsApiSortable = getBooleanValue(getValue("IsApiSortable", buffer, token, ",\""));
        it->second.IsCalculated = getBooleanValue(getValue("IsCalculated", buffer, token, ",\""));
        it->second.IsCompactLayoutable = getBooleanValue(getValue("IsCompactLayoutable", buffer, token, ",\""));
        it->second.IsCompound = getBooleanValue(getValue("IsCompound", buffer, token, ",\""));
        it->second.IsFieldHistoryTracked = getBooleanValue(getValue("IsFieldHistoryTracked", buffer, token, ",\""));
        it->second.IsHighScaleNumber = getBooleanValue(getValue("IsHighScaleNumber", buffer, token, ",\""));
        it->second.IsHtmlFormatted = getBooleanValue(getValue("IsHtmlFormatted", buffer, token, ",\""));
        it->second.IsIndexed = getBooleanValue(getValue("IsIndexed", buffer, token, ",\""));
        it->second.IsListFilterable = getBooleanValue(getValue("IsListFilterable", buffer, token, ",\""));
        it->second.IsListSortable = getBooleanValue(getValue("IsListSortable", buffer, token, ",\""));
        it->second.IsListVisible = getBooleanValue(getValue("IsListVisible", buffer, token, ",\""));
        it->second.IsNameField = getBooleanValue(getValue("IsNameField", buffer, token, ",\""));
        it->second.Label = getValue("Label", buffer, token, ",\"");
        it->second.LastModifiedById = getValue("LastModifiedById\":", buffer, token, ",\"");
        it->second.LastModifiedDate = getValue("LastModifiedDate\":", buffer, token, ",\"");
        std::string length = getValue("Length", buffer, token, ",\"");
        if (isStringNumeric(length))
            it->second.Length = std::stol(length);
        it->second.MasterLabel = getValue("MasterLabel", buffer, token, ",\"");
        it->second.NamespacePrefix = getValue("NamespacePrefix", buffer, token, ",\"");
        it->second.Precision = getValue("Precision", buffer, token, ",\"");
        it->second.Scale = getValue("Scale", buffer, token, ",\"");
        it->second.SecurityClassification = getValue("SecurityClassification", buffer, token, "}");
    }
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
            offset = token+1;
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
    
    if (globals::verbose)
        std::cout << "Number of " << entityName << " fields: " << fieldDefinitionMap.size() << std::endl;
    
    return result;
}
//
//
//
void fieldBook::outputFieldBook() const {
    std::ofstream ofs { "fieldBook" + entityName + ".csv"};
    
    // output header
    ofs << "Date,";
    ofs << "sObject,";
    ofs << "fieldAPIName,";
    ofs << "BusinessOwnerId,";
    ofs << "BusinessStatus,";
    ofs << "ComplianceGroup,";
    ofs << "DataType,";
    ofs << "ExtraTypeInfo,";
    ofs << "IsAiPredictionField,";
    ofs << "IsApiFilterable,";
    ofs << "IsApiGroupable,";
    ofs << "IsApiSortable,";
    ofs << "IsCalculated,";
    ofs << "IsCompactLayoutable,";
    ofs << "IsCompound,";
    ofs << "IsFieldHistoryTracked,";
    ofs << "IsHighScaleNumber,";
    ofs << "IsHtmlFormatted,";
    ofs << "IsIndexed,";
    ofs << "IsListFilterable,";
    ofs << "IsListSortable,";
    ofs << "IsListVisible,";
    ofs << "IsNameField,";
    ofs << "Label,";
    ofs << "LastModifiedById,";
    ofs << "LastModifiedDate,";
    ofs << "Length,";
    ofs << "MasterLabel,";
    ofs << "NamespacePrefix,";
    ofs << "Precision,";
    ofs << "Scale,";
    ofs << "SecurityClassification" << std::endl;

    ofs << std::boolalpha;
    
    // output lines
    for (auto it = fieldDefinitionMap.begin(); it != fieldDefinitionMap.end(); ++it) {
        if (config::getDateOutput().compare("") == 0)
            ofs << getDateString() << ",";
        else
            ofs << config::getDateOutput() << ",";
        ofs << entityName << ",";
        ofs << "\"" << it->second.QualifiedApiName << "\"" << ",";
        ofs << "\"" << it->second.BusinessOwnerId << "\"" << ",";
        ofs << "\"" << it->second.BusinessStatus << "\"" << ",";
        ofs << "\"" << it->second.ComplianceGroup << "\"" << ",";
        ofs << "\"" << it->second.DataType << "\"" << ",";
        ofs << "\"" << it->second.ExtraTypeInfo << "\"" << ",";
        ofs << "\"" << it->second.IsAiPredictionField << "\"" << ",";
        ofs << "\"" << it->second.IsApiFilterable << "\"" << ",";
        ofs << "\"" << it->second.IsApiGroupable << "\"" << ",";
        ofs << "\"" << it->second.IsApiSortable << "\"" << ",";
        ofs << "\"" << it->second.IsCalculated << "\"" << ",";
        ofs << "\"" << it->second.IsCompactLayoutable << "\"" << ",";
        ofs << "\"" << it->second.IsCompound << "\"" << ",";
        ofs << "\"" << it->second.IsFieldHistoryTracked << "\"" << ",";
        ofs << "\"" << it->second.IsHighScaleNumber << "\"" << ",";
        ofs << "\"" << it->second.IsHtmlFormatted << "\"" << ",";
        ofs << "\"" << it->second.IsIndexed << "\"" << ",";
        ofs << "\"" << it->second.IsListFilterable << "\"" << ",";
        ofs << "\"" << it->second.IsListSortable << "\"" << ",";
        ofs << "\"" << it->second.IsListVisible << "\"" << ",";
        ofs << "\"" << it->second.IsNameField << "\"" << ",";
        ofs << "\"" << it->second.Label << "\"" << ",";
        ofs << "\"" << it->second.LastModifiedById << "\"" << ",";
        ofs << "\"" << it->second.LastModifiedDate.substr(0,10) << "\"" << ",";
        ofs << "\"" << it->second.Length << "\"" << ",";
        ofs << "\"" << it->second.MasterLabel << "\"" << ",";
        ofs << "\"" << it->second.NamespacePrefix << "\"" << ",";
        ofs << "\"" << it->second.Precision << "\"" << ",";
        ofs << "\"" << it->second.Scale << "\"" << ",";
        ofs << "\"" << it->second.SecurityClassification << "\"" << std::endl;
    }
    
    ofs.close();
}
