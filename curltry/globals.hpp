//
//  globals.hpp
//  curltry
//
//  Created by Vincent Brenet on 11/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef globals_hpp
#define globals_hpp

#include <string>
#include <stdio.h>

class globals {
public:
    static std::string workingDirectory;
    static bool verbose;
    static bool veryverbose;
    static bool picklistAnalysis;
};

#endif /* globals_hpp */
