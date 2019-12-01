//
//  SObjectOutputs.cpp
//  curltry
//
//  Created by Vincent Brenet on 25/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  // for g++ compatibility
#include "sObject.hpp"
#include "utils.hpp"
#include "buckets.hpp"
#include "config.hpp"

//
//
void sObject::printPicklistCounters() const {
    std::cout << "**printPicklistCounters**" << std::endl;
    for (auto it = picklistCounters.begin(); it != picklistCounters.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            std::cout << it->first << ":" << it2->first << "," << it2->second << std::endl;
        }
    }
}
//
void sObject::printPicklistDescriptors() const {
    std::cout << "**printPicklistDescriptors**" << std::endl;
    for (auto it = picklistDescriptors.begin(); it != picklistDescriptors.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            std::cout << it->first << ":" << it2->first << "," << it2->second << std::endl;
        }
    }
}
//
void sObject::outputPicklistCounters()  {
    std::ofstream ofs { "picklists" + getName() + ".csv"};
    
    // header
    ofs << "Date,sObject,PicklistName,FromPackage,DefaultValue,PicklistLabel,PicklistValue,Usage,PercentUsage" << std::endl;

    for (auto it = picklistCounters.begin(); it != picklistCounters.end(); ++it) {
        
        std::string fromPackage {};
        if (attributeMap[it->first].isCustom()) {
            if (attributeMap[it->first].isPackage())
                fromPackage = attributeMap[it->first].getPackage();
            else
                fromPackage = config::getCustomer();
        }
        else
            fromPackage = "Standard";
        
        // first compute total valued fields
        double total {0};
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            total += it2->second;

        // then output counters
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            
            std::string label = picklistDescriptors[it->first][it2->first] ;
            
            double percentUsage = ((total == 0) ? 0 : (((double)it2->second / total)*100));
            
            ofs << analysisDate << ","; // date
            ofs << getName() << ",";    // object name
            ofs << it->first << ",";    // picklist name
            ofs << fromPackage << ",";  // origin
            ofs << attributeMap[it->first].getDefaultValue() << ",";    // default value
            ofs << "\"" << label << "\",";  // picklist label
            ofs << "\"" << it2->first << "\","; // picklist value
            ofs << it2->second << "," ;     // usage
            ofs << std::setprecision (2) << std::fixed << percentUsage << std::endl;    // percent usage
        }
    }
    
    ofs.close();
}
//
//
void sObject::outputRecordTypePicklistCounters() {
    std::ofstream ofs { "picklistsMatrix" + getName() + ".csv"};

    // header
    ofs << "Date,sObject,RecordTypeId,RecordType,PicklistName,FromPackage,DefaultValue,PicklistLabel,PicklistValue,Usage" << std::endl;

    // map of picklist counters by recordtypeId, by attributeName, by values
    //std::map<std::string, std::map<std::string, std::map<std::string, long>>> recordTypePicklistCounters

    for (auto it = recordTypePicklistCounters.begin(); it != recordTypePicklistCounters.end(); ++it) {
        // it->first : recordtype id
        // it->second : map [attributes, map[picklist value, counter]]
        std::string recordtypename;
        std::string recordtypeid;
        if (it->first.size() == 0) {
            recordtypename = "null";
            recordtypeid = "null";
        }
        else
        {
            recordtypename = getnamebyid(it->first);
            recordtypeid = it->first;
        }

        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            // it2->first : picklist name
            // it2->second : map[picklist value, counter]
            std::string fromPackage {};
            if (attributeMap[it2->first].isCustom()) {
                if (attributeMap[it2->first].isPackage())
                    fromPackage = attributeMap[it2->first].getPackage();
                else
                    fromPackage = config::getCustomer();
            }
            else
                fromPackage = "Standard";

            for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3) {
                // it3->first : picklist value
                // it3->second : counter
                ofs << analysisDate << ",";
                ofs << getName() << ",";
                ofs << recordtypeid << ",";
                ofs << "\"" << removeCommas(recordtypename) << "\",";
                ofs << it2->first << ",";           // picklist name
                ofs << fromPackage << ",";          // origin
                ofs << attributeMap[it2->first].getDefaultValue() << ","; //  default value
                ofs << "\"" << picklistDescriptors[it2->first][it3->first] << "\","; // picklist label
                ofs << "\"" << it3->first << "\","; // picklist value
                ofs << it3->second << std::endl;
            }
        }
    }
    
    ofs.close();
}
//
//
void sObject::outputAttributeCounters(const std::string &outputfile) {
    
    double nbRecords {0};
    auto idit = attributeCounters.find("Id");
    if (idit != attributeCounters.end()) {
        nbRecords = idit->second;
    }
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "Date,sObject,FieldName,FromPackage,FieldType,DefaultValue,FieldUsage,PercentUsage,UsageBucket" << std::endl;
        
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++) {
        
        std::string fromPackage {};
        if (attributeMap[it->first].isCustom()) {
            if (attributeMap[it->first].isPackage())
                fromPackage = attributeMap[it->first].getPackage();
            else
                fromPackage = config::getCustomer();
        }
        else
            fromPackage = "Standard";
        
        ofs << analysisDate << ","; // date
        ofs << getName() << ",";    // object name
        ofs << it->first << ",";    // field name
        ofs << fromPackage << ",";  // field origine
        ofs << attributeMap[it->first].getType() << ",";            // field type
        ofs << attributeMap[it->first].getDefaultValue() << ",";    // default value
        ofs << it->second << ",";   // field usage
        
        double percentUsage = ((nbRecords == 0) ? 0 : (((double)it->second / nbRecords)*100));
        ofs << std::setprecision (1) << std::fixed << percentUsage << ","; // percent usage
        ofs  << buckets::getBucket(percentUsage) << std::endl;;    // usage bucket

    }

    ofs.close();
}
//
//
void sObject::outputMatrixCounters(const std::string &outputfile) {
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "Date,sObject,RecordTypeId,RecordType,FieldName,FromPackage,FieldType,DefaultValue,FieldUsage,PercentRecordTypeUsage,UsageBucket" << std::endl;
        
    for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
        std::string recordtypeid = it->first.first;
        std::string recordtypename;
        if (recordtypeid.size() == 0) {
            recordtypename = "null";
        }
        else
            recordtypename = getnamebyid(recordtypeid);

        std::string fromPackage {};
        if (attributeMap[it->first.second].isCustom()) {
            if (attributeMap[it->first.second].isPackage())
                fromPackage = attributeMap[it->first.second].getPackage();
            else
                fromPackage = config::getCustomer();
        }
        else
            fromPackage = "Standard";
        
        double currNbRec = recordTypeMatrixCounters[std::pair<std::string,std::string>(recordtypeid,"Id")];
        double percentUsage = ((currNbRec == 0) ? 0 : (((double)it->second / currNbRec)*100));
        
        if (recordtypeid.size() == 0) {
            recordtypeid = "null";
        }
        
        ofs << analysisDate << ",";     // date
        ofs << getName() << ",";        // sobject name
        ofs << recordtypeid << ",";     // record type id
        ofs << "\"" << removeCommas(recordtypename) << "\","; // record type name
        ofs << it->first.second << ","; // field name
        ofs << fromPackage << ",";    // field origine
        ofs << attributeMap[it->first.second].getType() << ","; // field type
        ofs << attributeMap[it->first.second].getDefaultValue() << ","; // field default value
        ofs << it->second << ",";   // usage
        ofs << std::setprecision (1) << std::fixed << percentUsage << ",";
        ofs << buckets::getBucket(percentUsage) << std::endl; // percent usage
    }
    
    ofs.close();
}
