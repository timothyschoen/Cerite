//
//  AudioPlayer.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#pragma once


#define NO_CERITE_MACROS
#include "/Users/timschoen/Documents/Cerite/.exec/libcerite.h"

#include <boost/interprocess/ipc/message_queue.hpp>

#include <JuceHeader.h>
#include "NodeConverter.hpp"
#include "concurrentqueue.h"

struct IPCMain
{
    const size_t msg_size = 1 << 10;
    
    std::unique_ptr<message_queue> sendQueue = nullptr;
    std::unique_ptr<message_queue> receiveQueue = nullptr;
    
    // Fill this with callbacks by index
    std::vector<std::function<void(Data)> callbacks;
    
    IPCWorker() {
        sendQueue.reset(new message_queue(create_only ,"cerite_send", 100, msg_size));
        receiveQueue.reset(new message_queue(create_only, "cerite_receive",  100, msg_size));
        
        
    }
    
    
}
