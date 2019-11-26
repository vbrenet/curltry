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
    ofs << "Date,sObject,PicklistName,PicklistLabel,PicklistValue,Usage,PercentUsage" << std::endl;

    for (auto it = picklistCounters.begin(); it != picklistCounters.end(); ++it) {
        // first compute total valued fields
        double total {0};
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            total += it2->second;

        // then output counters
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            
            std::string label = picklistDescriptors[it->first][it2->first] ;
            
            double percentUsage = ((total == 0) ? 0 : (((double)it2->second / total)*100));
            
            ofs << analysisDate << "," << getName() << "," << it->first << ",\"" << label << "\",\"" << it2->first << "\"," << it2->second << "," ;
            ofs << std::setprecision (2) << std::fixed << percentUsage << std::endl;
        }
    }
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
    ofs << "date,sobject,fieldName,fieldType,fieldUsage,percentUsage,usageBucket,fromPackage" << std::endl;
        
    for (auto it=attributeCounters.begin(); it != attributeCounters.end(); it++) {
        
        ofs << analysisDate << "," << getName() << ",";
        ofs << it->first << "," << attributeMap[it->first].getType() << "," << it->second << ",";
        double percentUsage = ((nbRecords == 0) ? 0 : (((double)it->second / nbRecords)*100));
        ofs << std::setprecision (1) << std::fixed << percentUsage ;
        
        ofs << "," << buckets::getBucket(percentUsage);
        
        std::string fromPackage {};
        if (attributeMap[it->first].isCustom()) {
            if (attributeMap[it->first].isPackage())
                fromPackage = attributeMap[it->first].getPackage();
            else
                fromPackage = "Custom";
        }
        else
            fromPackage = "Standard";
        
        ofs << "," << fromPackage << std::endl;

    }

    ofs.close();
}
//
//
void sObject::outputMatrixCounters(const std::string &outputfile) {
    
    std::ofstream ofs {outputfile};
    
    // header
    ofs << "Date,sObject,RecordTypeId,RecordType,Field,FieldType,NbRecords,PercentRecordTypeUsage,UsageBucket,FromPackage" << std::endl;
        
    for (auto it=recordTypeMatrixCounters.begin(); it != recordTypeMatrixCounters.end(); it++) {
        std::string recordtypeid = it->first.first;
        std::string recordtypename;
        if (recordtypeid.size() == 0)
            recordtypename = "null";
        else
            recordtypename = getnamebyid(recordtypeid);

        double currNbRec = recordTypeMatrixCounters[std::pair<std::string,std::string>(recordtypeid,"Id")];
        double percentUsage = ((currNbRec == 0) ? 0 : (((double)it->second / currNbRec)*100));
        
        ofs << analysisDate << "," << getName() << ",";
        ofs << recordtypeid << "," << removeCommas(recordtypename) << "," << it->first.second << ",";
        ofs << attributeMap[it->first.second].getType()  << "," << it->second << ",";
        ofs << std::setprecision (1) << std::fixed << percentUsage;
        ofs << "," << buckets::getBucket(percentUsage);
        
        std::string fromPackage {};
        if (attributeMap[it->first.second].isCustom()) {
            if (attributeMap[it->first.second].isPackage())
                fromPackage = attributeMap[it->first.second].getPackage();
            else
                fromPackage = "Custom";
        }
        else
            fromPackage = "Standard";
        
        ofs << "," << fromPackage << std::endl;
    }
    
    ofs.close();
}

