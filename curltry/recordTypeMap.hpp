//
//  recordTypeMap.hpp
//  curltry
//
//  Created by Vincent Brenet on 14/12/2018.
//  Copyright © 2018 Vincent Brenet. All rights reserved.
//

#ifndef recordTypeMap_hpp
#define recordTypeMap_hpp

#include <stdio.h>
#include <map>

class recordTypeMap {
private:
    std::map<std::string,std::string> recordTypes;  // map id,name
public:
    recordTypeMap(const std::string filename);
    std::string getnamebyid(const std::string id);
};
#endif /* recordTypeMap_hpp */
