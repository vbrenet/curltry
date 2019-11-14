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

std::string getBucket(double d);
std::string getDateString();
std::string removeCommas (const std::string& input);
bool isStringNumeric (const std::string input);
std::string extractXmlToken(const std::string& inputbuffer, size_t pos, const std::string& token);
std::string extractXmlToken(const std::string& inputbuffer, const std::string& token);

#endif /* utils_hpp */
