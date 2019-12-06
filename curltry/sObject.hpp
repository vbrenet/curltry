//
//  sObject.hpp
//  curltry
//
//  Created by Vincent Brenet on 02/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef sObject_hpp
#define sObject_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <map>

#include "sAttribute.hpp"
#include "fieldBook.hpp"

class sObject{
    private:
    std::string name;   // sObject name
    std::string label;  // sObject label
    std::string query;  // query string
    bool queryStringConstructed {false}; // true if query string constructed
    
    std::string analysisDate;
    
    // field book
    fieldBook objectFieldBook;
    
    // map of attributes by attribute name
    std::map<std::string, sAttribute> attributeMap {};
    
    // map of counters, key = attribute name
    std::map<std::string,long> attributeCounters {};
    // map of counters, key = (record type id, attribute name)
    std::map<std::pair<std::string,std::string>,long> recordTypeMatrixCounters {};

    // map of attributes in CSV file, key = position number in the CSV line
    std::map<int,std::string> csvAttributeMap {};
    // map of record type names by ids
    std::map<std::string,std::string> recordTypes {};
    
    // map of picklist labels by values (value, label) by attribute name
    std::map<std::string, std::map<std::string,std::string>> picklistDescriptors {};
    // map of picklist counters by values (value, counter) by attribute name
    std::map<std::string, std::map<std::string, long>> picklistCounters {};
    // map of picklist counters by recordtypeId, by attributeName, by values
    std::map<std::string, std::map<std::string, std::map<std::string, long>>> recordTypePicklistCounters {};
    //
    //  private methods
    std::string getAnalysisDate() const;
    //
    // object configuration
    void parseRecordTypeBuffer(const std::string&);

    // initialization of counters
    void initRecordTypeMatrixCounters();
    void initRecordTypePicklistMatrixCounters();
    void processAttributeCountersLine(const std::string &inputline);
    void processMatrixLine(const std::string &inputline);
    void processPicklistLine(const std::string &inputline);
    void processMatrixPicklistLine(const std::string &inputline);

    // record analysis
    void incrementCounters(const std::string &recordTypeId, int counter, const std::string &token);
    
    // utils
    void initializeCounter(const std::string& attribute, const std::string& countervalue);
    std::string getnamebyid(const std::string id) const;
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    
public:
    // constructor and accessors
    sObject (const std::string objname)  : name {objname} {}
    const std::string& getName() const {return name;} ;
    const std::string& getLabel() const {return label;} ;

    // object configuration
    bool getFieldBook() {return objectFieldBook.setFieldBook(name, label);};
    void outputFieldBook() {objectFieldBook.outputFieldBook();};
    void addAttribute(sAttribute a) {attributeMap.insert(std::pair<std::string,sAttribute>(a.getName(),a));}
    bool getDescribeAttributesBuffer(std::string& buffer);
    std::string makeAllAttributeQuery();
    bool initializeRecordTypes();
    void setAnalysisDate(const std::string);
    void setLabel(const std::string l) {label = l;}

    // initialization of counters
    void initializeAttributeCountersFromFile(const std::string &inputfile);
    void initializeMatrixCountersFromFile(const std::string &inputfile);
    void initializePicklistCountersFromFile(const std::string &inputfile);
    void initializeMatrixPicklistCountersFromFile(const std::string &inputfile);

    void addPicklistDescriptor (std::string picklistName, std::string value, std::string label);

    // record analysis
    long computeCsvRecords(const std::string &);

    // outputs
    void printAttributeCounters() const;
    void printPicklistCounters() const;
    void printPicklistDescriptors() const;
    void outputAttributeCounters(const std::string &);
    void outputMatrixCounters(const std::string &outputfile);
    void outputPicklistCounters() ;
    void outputRecordTypePicklistCounters() ;
};

#endif /* sObject_hpp */
