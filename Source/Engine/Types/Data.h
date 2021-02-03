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
    
    /*
    for(int i = 0; i < data.listlen; i++)
        stream.writeDouble(data.list[i]);
    
    stream.writeInt(data.listlen); */
    
    return stream.getMemoryBlock();
}

static inline Data readFromStream(MemoryInputStream& stream) {
    Data data;
    data.type = (Type)stream.readInt();
    data.number = stream.readDouble();
    data.string = stream.readString().toRawUTF8();

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
