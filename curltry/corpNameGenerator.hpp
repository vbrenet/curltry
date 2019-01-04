//
//  corpNameGenerator.hpp
//  curltry
//
//  Created by Vincent Brenet on 16/11/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef corpNameGenerator_hpp
#define corpNameGenerator_hpp

#include <stdio.h>
#include <vector>
#include <string>
#include <ctime>

class corpNameGenerator {
private:
    static std::vector<std::string> names;
    static std::string corpname;
public:
    static void init();
    static const std::string& getName();
};
#endif /* corpNameGenerator_hpp */
