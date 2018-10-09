//
//  bulkSession.cpp
//  curltry
//
//  Created by Vincent Brenet on 08/10/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#include "bulkSession.hpp"
#include <iostream>
#include <sstream>

extern size_t WriteCallback(void *, size_t , size_t , void *);
//

struct WriteThis {
    const char *readptr;
    size_t sizeleft;
};

static size_t read_callback(void *dest, size_t size, size_t nmemb, void *userp)
{
    struct WriteThis *wt = (struct WriteThis *)userp;
    size_t buffer_size = size*nmemb;
    
    if(wt->sizeleft) {
        /* copy as much as possible from the source to the destination */
        size_t copy_this_much = wt->sizeleft;
        if(copy_this_much > buffer_size)
            copy_this_much = buffer_size;
        memcpy(dest, wt->readptr, copy_this_much);
        
        wt->readptr += copy_this_much;
        wt->sizeleft -= copy_this_much;
        return copy_this_much; /* we copied this many bytes */
    }
    
    return 0; /* no more data left to deliver */
}
//
//
//  open a bulk session with Salesforce
//
bool bulkSession::openBulkSession(bool isSandbox, const std::string username, const std::string password) {
    bool result {false};
    
    std::stringstream ssurl;
    ssurl << "https://" << ((isSandbox) ? "test." : "login.") << "salesforce.com/services/Soap/u/39.0";
    
    std::stringstream ssbody;
    ssbody << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
    << "<env:Envelope xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n"
    << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
    << "xmlns:env=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
    << "<env:Body>\n"
    << "<n1:login xmlns:n1=\"urn:partner.soap.sforce.com\">\n"
    << "<n1:username>" << username << "</n1:username>\n"
    << "<n1:password>" << password << "</n1:password>\n"
    <<  "</n1:login>\n"
    <<  "</env:Body>\n"
    <<  "</env:Envelope>\n";
    
    
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    struct WriteThis wt;
    
    wt.readptr = ssbody.str().c_str();
    wt.sizeleft = strlen(ssbody.str().c_str());
    
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, ssurl.str().c_str());
        
        std::cout << "URL: " << ssurl.str() << std::endl;
        std::cout << "BODY size: " << strlen(ssbody.str().c_str());
        std::cout << "\nBODY: \n" << ssbody.str() << std::endl;

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // set header
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: text/xml; charset=UTF-8");
        list = curl_slist_append(list, "SOAPAction: login");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        //curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

        /* Now specify we want to POST data */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        //curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ssbody.str().c_str());
        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        
        /* pointer to pass to our read function */
        curl_easy_setopt(curl, CURLOPT_READDATA, &wt);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)wt.sizeleft);
        
        res = curl_easy_perform(curl);
        curl_slist_free_all(list); /* free the list  */

        curl_easy_cleanup(curl);
    }
    else
        return false;
    
    if (res != CURLE_OK) {
        std::cerr << "openBulkSession : curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cerr << "openBulkSession : http error: " << http_code << std::endl;
        return false;
    }
    

    
    
    return result;
}
