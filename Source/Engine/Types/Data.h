#pragma once
#include <JuceHeader.h>

enum Type {
    tBang,
    tNumber,
    tString,
    tList
};

struct Data {
    
    Type type;
    double number;
    const char* string;
    Data* list;
    unsigned int listlen = 0;
    
};


struct DataStream {

static inline MemoryBlock writeToStream(const Data& data, MemoryOutputStream& stream) {
    stream.writeInt(data.type);
    stream.writeDouble(data.number);
    
    stream.writeString(data.string);
    stream.writeInt(data.listlen);
    
    for(int i = 0; i < data.listlen; i++) {
        writeToStream(data.list[i], stream);
    }
    
    
    
    return stream.getMemoryBlock();
}

static inline Data readFromStream(MemoryInputStream& stream) {
    Data data;
    data.type = (Type)stream.readInt();
    data.number = stream.readDouble();
    
    String data_str = stream.readString();
    if(data.type == tString) {
        data.string = (const char*)malloc(data_str.length() + 1);
        strcpy(const_cast<char*>(data.string), data_str.toRawUTF8());
    }
    
    data.listlen = stream.readInt();
    
    data.list = new Data[data.listlen];
    for(int i = 0; i < data.listlen; i++) {
        data.list[i] = readFromStream(stream);
    }
    
    return data;
    /*
    int startpos = stream.getPosition();
    
    stream.setPosition(stream.getDataSize() - 32);
    data.listlen = stream.readInt();
    
    stream.setPosition(startpos);
    
    for(int i = 0; i < data.listlen; i++)
        stream.writeDouble(data.list[i]);
    
    stream.writeInt(data.listlen); */
}

};
