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


 size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
   ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
//
//
//
std::string extractToken(const std::string buffer) {
    size_t beginindex = buffer.find("access_token") + 15;
    size_t endindex = buffer.find('"', beginindex);

    return buffer.substr(beginindex,endindex-beginindex);
}
//
//
//
bool getToken(std::string& token, const std::string orgurl, const std::string clientId, const std::string clientSecret, const std::string userName, const std::string password) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string urlParameters = "grant_type=password&client_id=" + clientId + "&client_secret=" + clientSecret + "&username=" + userName + "&password=" + password;
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ("https://" + orgurl + "/services/oauth2/token").c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,urlParameters.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    else
        return false;
    
    if (res != CURLE_OK) {
        std::cerr << "getToken : curl_easy_perform error " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    token = extractToken(readBuffer);

    return true;

}
//
//
int main(int argc, const char * argv[]) {
    // insert code here...
    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    std::string urlParameters { "grant_type=password&client_id=3MVG98_Psg5cppyaViFlqbC.qo_drqk_L1ZWJnB4UB.NmykHpAvz.3wxbx23DBjgnccMNsZVfBF8UgvovtfYh&client_secret=8703187062703750250&username=vbrlight@brenet.com&password=Petrosian0"};

    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://vbrlight-dev-ed.my.salesforce.com/services/oauth2/token");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS,urlParameters.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    
    std::string token = extractToken(readBuffer);
    
    std::cout << "token:" << token << std::endl;

    // get account
    readBuffer.clear();
    curl = curl_easy_init();
    
    if(curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, "https://vbrlight-dev-ed.my.salesforce.com/services/data/v43.0/sobjects/account/0015800000fp2WF");
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
