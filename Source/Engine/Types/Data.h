#pragma once

enum Type {
    tBang,
    tNumber,
    tString,
    tList
};

struct Data{
    
    Type type;
    double number;
    const char* string;
    double* list;
    unsigned int listlen;
    
};
