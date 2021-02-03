#include "GUIContainer.h"
#include "../../MainComponent.h"

void GUIContainer::send(Data d) {
    MemoryOutputStream memstream(false);
    memstream.writeInt(MessageID::SetParam);
    memstream.writeInt(ID);
    DataStream::writeToStream(d, memstream);
    MainComponent::getInstance()->sendMessage(memstream.getMemoryBlock());
}
