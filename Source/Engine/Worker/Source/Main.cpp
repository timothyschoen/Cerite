/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"




String getCommandLineParameters(int argc, char* argv[])
{
    String argString;

    for (int i = 1; i < argc; ++i)
    {
        String arg (argv[i]);

        if (arg.containsChar (' ') && ! arg.isQuotedString())
            arg = arg.quoted ('"');

        argString << arg << ' ';
    }
    

    return argString.trim();
}

int main(int argc, char* argv[]) {
    ScopedJuceInitialiser_GUI libraryInitialiser;
    MainComponent* main = MainComponent::getInstance();

    while(true) {
    JUCE_TRY
    {
        // loop until a quit message is received..
        MessageManager::getInstance()->runDispatchLoopUntil(1000);
    }
    JUCE_CATCH_EXCEPTION
    }
    
    /*
    std::condition_variable cv;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, []{return false;});
     */
    
    return 0;
}
