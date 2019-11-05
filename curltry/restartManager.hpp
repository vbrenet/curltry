//
//  restartManager.hpp
//  curltry
//
//  Created by Vincent Brenet on 04/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//

#ifndef restartManager_hpp
#define restartManager_hpp

#include <stdio.h>
#include <string>
#include <vector>

class restartManager {
private:
    static bool restartMode;
    static std::vector<std::string> batchids;
public:
    static void setRestartMode() {restartMode = true;}
    static bool isRestartMode() {return restartMode;};
    static void init();
    static void saveBatchId(const std::string batchid);
    static bool isAlreadyRead(const std::string batchid);
};
#endif /* restartManager_hpp */
