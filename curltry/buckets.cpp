//
//  buckets.cpp
//  curltry
//
//  Created by Vincent Brenet on 25/11/2019.
//  Copyright © 2019 Vincent Brenet. All rights reserved.
//
#include <iostream>
#include <fstream>
#include "buckets.hpp"
#include "utils.hpp"

bool buckets::mapEnabled {false};

std::map<int,std::string> buckets::bucketMap;

const std::string buckets::theBuckets[] =
{
"00",//0
"01-05",//1
"01-05",//2
"01-05",//3
"01-05",//4
"01-05",//5
"06-10",//6
"06-10",//7
"06-10",//8
"06-10",//9
"06-10",//10
"11-20",//11
"11-20",//12
"11-20",//13
"11-20",//14
"11-20",//15
"11-20",//16
"11-20",//17
"11-20",//18
"11-20",//19
"11-20",//20
"21-30",//21
"21-30",//22
"21-30",//23
"21-30",//24
"21-30",//25
"21-30",//26
"21-30",//27
"21-30",//28
"21-30",//29
"21-30",//30
"31-40",//31
"31-40",//32
"31-40",//33
"31-40",//34
"31-40",//35
"31-40",//36
"31-40",//37
"31-40",//38
"31-40",//39
"31-40",//40
"41-50",//41
"41-50",//42
"41-50",//43
"41-50",//44
"41-50",//45
"41-50",//46
"41-50",//47
"41-50",//48
"41-50",//49
"41-50",//50
"51-60",//51
"51-60",//52
"51-60",//53
"51-60",//54
"51-60",//55
"51-60",//56
"51-60",//57
"51-60",//58
"51-60",//59
"51-60",//60
"61-70",//61
"61-70",//62
"61-70",//63
"61-70",//64
"61-70",//65
"61-70",//66
"61-70",//67
"61-70",//68
"61-70",//69
"61-70",//70
"71-80",//71
"71-80",//72
"71-80",//73
"71-80",//74
"71-80",//75
"71-80",//76
"71-80",//77
"71-80",//78
"71-80",//79
"71-80",//80
"81-90",//81
"81-90",//82
"81-90",//83
"81-90",//84
"81-90",//85
"81-90",//86
"81-90",//87
"81-90",//88
"81-90",//89
"81-90",//90
"91-99",//91
"91-99",//92
"91-99",//93
"91-99",//94
"91-99",//95
"91-99",//96
"91-99",//97
"91-99",//98
"91-99",//99
"All",  //100
"Zero"  //101
};

//
std::string buckets::getBucket(double d) {
    if (d < 0 || d > 100)
        return ("undefined");
    if (!mapEnabled) {
        if (d == 0)
            return theBuckets[101];
        return (theBuckets[(int)d]);
    }
    else {
        if (d == 0)
            return bucketMap[101];
        return bucketMap[(int)d];
    }
}
//
//
void buckets::processBucketLine(const std::string &line) {
    // 0:"00"
    // 1-20:"01-20"
    // 21:"21"
    // 22-50: "22-50"
    // etc
    
    size_t semicolon = line.find_first_of(':');
    if (semicolon != std::string::npos) {
        std::string index = line.substr(0,semicolon);
        if (isStringNumeric(index)) {
            int key = std::stoi(index);
            std::string value = line.substr(semicolon+1);
            if (key == 0) {
                bucketMap[101] = value;
            }
            else if (key > 0 && key <= 100) {
                bucketMap[key] = value;
            }
            else {
                std::cerr << "buckets initialization: key out of bounds: " << key << std::endl;
            }
        }
        else {
            size_t dash = index.find_first_of('-');
            if (dash != std::string::npos) {
                std::string left = index.substr(0,dash);
                std::string right = index.substr(dash+1);
                if (isStringNumeric(left) && isStringNumeric(right)) {
                    int leftbound = std::stoi(left);
                    int rightbound = std::stoi(right);
                    if (leftbound >= 0 && leftbound <= 100 && rightbound >= 0 && rightbound <= 100 && leftbound < rightbound) {
                        std::string value = line.substr(semicolon+1);
                        for (auto i = leftbound; i <= rightbound; ++i)
                            bucketMap[i] = value;
                    }
                    else
                        std::cerr << "buckets initialization: invalid line: " << line << std::endl;
                }
                else
                    std::cerr << "buckets initialization: invalid line: " << line << std::endl;
            }
            else
                std::cerr << "buckets initialization: invalid line: " << line << std::endl;
        }
    }
    else {
        std::cerr << "buckets initialization: invalid line: " << line << std::endl;
    }
}

//
//
void buckets::initBucketsFromFile(const std::string &inputfilename) {
    std::ifstream bucketsFile {inputfilename};
    
    std::string currentLine;
    
    while (getline(bucketsFile,currentLine)) {
        // skip comments
        if (currentLine[0] == '#')
            continue;
        
        processBucketLine(currentLine);
    }
    
    bucketsFile.close();

    // check that all map positions are filled
    bool errorFound {false};
    for (int i=0; i <= 100; i++) {
        if (bucketMap.find(i) == bucketMap.end()) {
            errorFound = true;
            std::cerr << "buckets initialization: not all % values are filled: " << i << std::endl;
            break;
        }
    }
    
    mapEnabled = !errorFound;
}
