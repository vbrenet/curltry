//
//  textGenerator.hpp
//  curltry
//
//  Created by Vincent Brenet on 17/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef textGenerator_hpp
#define textGenerator_hpp

#include <stdio.h>
#include <vector>
#include <string>

class textGenerator {
private:
    static std::vector<std::string> words;
    static std::string text;
public:
    static void init();
    static const std::string& gettext();
};
#endif /* textGenerator_hpp */
