//
//  packages.hpp
//  curltry
//
//  Created by Vincent Brenet on 02/12/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef packages_hpp
#define packages_hpp

#include <stdio.h>
#include <string>
#include <map>

class packages {
private:
    // map of raw value (e.g. FinServ) and commercial value
    static std::map <std::string, std::string> packageMap;

    static void processLine (const std::string &inputline);

public:
    static void initPackagesFromFile (const std::string &inputfile);
    static std::string getPackageName(const std::string );
};

#endif /* packages_hpp */
