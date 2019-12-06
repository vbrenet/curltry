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

#include "utils.hpp"
#include "config.hpp"
#include <curl/curl.h>
#include "SalesforceSession.hpp"
#include "globals.hpp"


/*
 =IF(NUMBERVALUE(E12;".")=100;"All";IF(NUMBERVALUE(E12;".")>90;"91-99";IF(NUMBERVALUE(E12;".")>80;"81-90";IF(NUMBERVALUE(E12;".")>70;"71-80";IF(NUMBERVALUE(E12;".")>60;"61-70";IF(NUMBERVALUE(E12;".")>50;"51-60";IF(NUMBERVALUE(E12;".")>40;"41-50";IF(NUMBERVALUE(E12;".")>30;"31-40";IF(NUMBERVALUE(E12;".")>20;"21-30";IF(NUMBERVALUE(E12;".")>10;"11-20";IF(NUMBERVALUE(E12;".")>5;"06-10";IF(NUMBERVALUE(E12;".")>0;"01-05";"00"))))))))))))
 */

static const std::string buckets[] =
{
"00",//0
"01-05",//1
"01-05",//2
"01-05",//3
"01-05",//4
"01-05",//5
"06-10",//6
"06-10",//7
"06-10",//8
"06-10",//9
"06-10",//10
"11-20",//11
"11-20",//12
"11-20",//13
"11-20",//14
"11-20",//15
"11-20",//16
"11-20",//17
"11-20",//18
"11-20",//19
"11-20",//20
"21-30",//21
"21-30",//22
"21-30",//23
"21-30",//24
"21-30",//25
"21-30",//26
"21-30",//27
"21-30",//28
"21-30",//29
"21-30",//30
"31-40",//31
"31-40",//32
"31-40",//33
"31-40",//34
"31-40",//35
"31-40",//36
"31-40",//37
"31-40",//38
"31-40",//39
"31-40",//40
"41-50",//41
"41-50",//42
"41-50",//43
"41-50",//44
"41-50",//45
"41-50",//46
"41-50",//47
"41-50",//48
"41-50",//49
"41-50",//50
"51-60",//51
"51-60",//52
"51-60",//53
"51-60",//54
"51-60",//55
"51-60",//56
"51-60",//57
"51-60",//58
"51-60",//59
"51-60",//60
"61-70",//61
"61-70",//62
"61-70",//63
"61-70",//64
"61-70",//65
"61-70",//66
"61-70",//67
"61-70",//68
"61-70",//69
"61-70",//70
"71-80",//71
"71-80",//72
"71-80",//73
"71-80",//74
"71-80",//75
"71-80",//76
"71-80",//77
"71-80",//78
"71-80",//79
"71-80",//80
"81-90",//81
"81-90",//82
"81-90",//83
"81-90",//84
"81-90",//85
"81-90",//86
"81-90",//87
"81-90",//88
"81-90",//89
"81-90",//90
"91-99",//91
"91-99",//92
"91-99",//93
"91-99",//94
"91-99",//95
"91-99",//96
"91-99",//97
"91-99",//98
"91-99",//99
"All"   //100
};

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

