//
//  main.cpp
//  curltry
//
//  Created by Vincent Brenet on 25/09/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include <iostream>
#include <string>
#include <curl/curl.h>
#include "sObject.hpp"
#include "SalesforceSession.hpp"


size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
//
bool getDescribeAttributesBuffer(const std::string& token, const std::string orgurl, const std::string objName, std::string& buffer){
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    readBuffer.clear();
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + orgurl + "/services/data/v43.0/sobjects/" + objName + "/describe").c_str());
        
        struct curl_slist *chunk = NULL;
        
        chunk = curl_slist_append(chunk, ("Authorization: Bearer " + token).c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_setopt() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_easy_cleanup(curl);
        
        // search attribute list in readBuffer (substr) and put it in buffer
        buffer = readBuffer;
    }
    else
        return false;
    
    return true;
}
//
void describeSObject(const std::string& token, const std::string& orgurl, sObject& obj) {
    std::string rawAttributeList;
    // launch describe request (produce a buffer)
    if (!getDescribeAttributesBuffer(token, orgurl, obj.getName(), rawAttributeList))
        return;
    
    // parse each attribute in the buffer
}
//
//
//
int main(int argc, const char * argv[]) {
    // insert code here...
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string token;

    if (SalesforceSession::openSession("vbrlight-dev-ed.my.salesforce.com", "3MVG98_Psg5cppyaViFlqbC.qo_drqk_L1ZWJnB4UB.NmykHpAvz.3wxbx23DBjgnccMNsZVfBF8UgvovtfYh", "8703187062703750250", "vbrlight@brenet.com", "Petrosian0"))
        std::cout << "token:" << SalesforceSession::getConnectedAppToken() << std::endl;
    else
        return -1;

    // get account
    readBuffer.clear();
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://vbrlight-dev-ed.my.salesforce.com/services/data/v43.0/sobjects/Account/describe");

        struct curl_slist *chunk = NULL;
        
        chunk = curl_slist_append(chunk, ("Authorization: Bearer " + token).c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_setopt() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
    
    std::cout << readBuffer << std::endl;

    return 0;
}
