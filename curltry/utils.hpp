//
//  utils.hpp
//  curltry
//
//  Created by Vincent Brenet on 08/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#include <stdio.h>

std::string getDateString();
std::string removeCommas (const std::string& input);
bool isStringNumeric (const std::string input);
std::string extractXmlToken(const std::string& inputbuffer, size_t pos, const std::string& token);
std::string extractXmlToken(const std::string& inputbuffer, const std::string& token);
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
bool restQuery(const std::string& query, std::string& result);

#endif /* utils_hpp */
