#include "GUIContainer.h"
#include "../../MainComponent.h"

void GUIContainer::send(Data d) {
    MemoryOutputStream memstream(false);
    memstream.writeInt(MessageID::SetParam);
    memstream.writeInt(ID);
    DataStream::writeToStream(d, memstream);
    
    freeData(d);
    MainComponent::getInstance()->sendMessage(memstream.getMemoryBlock());
}

void GUIContainer::freeData(Data& d)
{
    if(d.type == tString) {
        delete[] d.string;
    }

    for(int i = 0; i < d.listlen; i++)
    {
        freeData(d.list[i]);
    }
    
    if(d.listlen > 0) {
        delete[] d.list;
        d.listlen = 0;
    }
    
    
}
