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
/*

*/

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
    
        std::cout << readBuffer << std::endl;
    return 0;
}
