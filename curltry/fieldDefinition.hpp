//
//  fieldDefinition.hpp
//  curltry
//
//  Created by Vincent Brenet on 30/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef fieldDefinition_hpp
#define fieldDefinition_hpp

#include <stdio.h>
#include <string>

class fieldDefinition {
    friend class fieldBook;
private:
    std::string EntityName; // field's sObject name
    std::string BusinessOwnerId;
    std::string BusinessStatus;
    std::string ComplianceGroup;
    std::string DataType;
    std::string ExtraTypeInfo;
    bool IsAiPredictionField;
    bool IsApiFilterable;
    bool IsApiGroupable;
    bool IsApiSortable;
    bool IsCalculated;
    bool IsCompactLayoutable;
    bool IsCompound;
    bool IsFieldHistoryTracked;
    bool IsHighScaleNumber;
    bool IsHtmlFormatted;
    bool IsIndexed;
    bool IsListFilterable;
    bool IsListSortable;
    bool IsListVisible;
    bool IsNameField;
    std::string Label;
    std::string LastModifiedById;
    std::string LastModifiedDate;
    long Length;
    std::string MasterLabel;
    std::string NamespacePrefix;
    std::string Precision;
    std::string QualifiedApiName;   // field API name
    std::string Scale;
    std::string SecurityClassification;

public:
    fieldDefinition(std::string name) : EntityName{name} {};
};

#endif /* fieldDefinition_hpp */
