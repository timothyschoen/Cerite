//
//  AudioPlayer.hpp
//  Cerite_Light - ConsoleApp
//
//  Created by Timothy Schoen on 27/04/2021.
//

#pragma once


#define NO_CERITE_MACROS
#include "/Users/timschoen/Documents/Cerite/.exec/libcerite.h"

#include <JuceHeader.h>

#define BOOST_INTERPROCESS_MSG_QUEUE_CIRCULAR_INDEX
#include <boost/interprocess/ipc/message_queue.hpp>

#include "NodeConverter.hpp"
#include "concurrentqueue.h"

using namespace boost::interprocess;

enum Messages
{
    tInitialize,
    tSe
}

struct IPCWorker
{

    
    
    
};
