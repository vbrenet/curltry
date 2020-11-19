//
//  utils.cpp
//  curltry
//
//  Created by Vincent Brenet on 08/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#include <string>
#include <ctime>
#include <sstream>
#include <iostream>

#include "utils.hpp"
#include "config.hpp"
#include <curl/curl.h>
#include "SalesforceSession.hpp"
#include "globals.hpp"

//
// produce current date in the format JJ/MM/AAAA
//
std::string getDateString() {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buffer,80,"%d/%m/%Y",timeinfo);
    
    std::string ret(buffer, 10);

    return ret;
    
}
//
bool getBooleanValue (const std::string& input) {
    if (input.compare("true") == 0)
        return true;
    else
        return false;
}
//
void removeDoubleQuote  (std::string& input) {
    std::size_t found;
    
    while ((found = input.find("\"")) != std::string::npos) {
        input.erase(found, 1);
    }

}
//
std::string removeCommas (const std::string& input) {
    std::string result {input};
    std::size_t found;
    
    while ((found = result.find(",")) != std::string::npos) {
        result.replace(found, 1, 1, ' ');
    }

    return result;
}
//
//
std::string replaceDoubleQuotes (const std::string& input) {
    std::string result {input};
    std::size_t found;
    
    while ((found = result.find("\"")) != std::string::npos) {
        result.replace(found, 1, 1, '\'');
    }

    return result;
}
//
bool isStringNumeric (const std::string input) {
    return (input.find_first_not_of( "0123456789" ) == std::string::npos);
}
//
std::string extractXmlToken(const std::string& inputbuffer, size_t pos, const std::string& token) {
    std::string endtoken = token;
    endtoken.insert(1,1,'/');
    
    size_t beginpos = inputbuffer.find(token, pos);
    size_t endpos = inputbuffer.find(endtoken, pos);
    
    return inputbuffer.substr(beginpos+token.size(),endpos-beginpos-token.size());
}
//
//
std::string extractXmlToken(const std::string& inputbuffer, const std::string& token) {
    std::string endtoken = token;
    endtoken.insert(1,1,'/');
    
    size_t beginpos = inputbuffer.find(token);
    size_t endpos = inputbuffer.find(endtoken);
    
    return inputbuffer.substr(beginpos+token.size(),endpos-beginpos-token.size());
}
//
//
//
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
//
//
bool restQuery(const std::string& query, std::string& readBuffer) {
    CURL *curl;
    CURLcode res;

    readBuffer.clear();
    
    curl = curl_easy_init();

    if(curl) {
        
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SalesforceSession::getDomain() + "/services/data/v" + config::getApiVersion() + "/query/" + query).c_str());
        
        struct curl_slist *chunk = NULL;
        
        chunk = curl_slist_append(chunk, ("Authorization: Bearer " + SalesforceSession::getConnectedAppToken()).c_str());
        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_setopt() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        
        curl_slist_free_all(chunk); /* free the list  */
        
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code >= 400) {
            std::cerr << "restQuery : http error: " << http_code << std::endl;
            return false;
        }

        curl_easy_cleanup(curl);
        
    }
    else
        return false;

    return (res == CURLE_OK);
}
//
std::string getFrenchDate(const std::string input) {
    //input : 2019-11-25T10:21:32.000Z
    std::stringstream target;
    
    target  << input[8]
            << input[9]
            << '/'
            << input[5]
            << input[6]
            << '/'
            << input[0]
            << input[1]
            << input[2]
            << input[3];
    
    return target.str();
}

