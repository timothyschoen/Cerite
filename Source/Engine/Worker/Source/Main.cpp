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
    
    freopen("output.txt","w",stdout);
    
    printf("test");
    
    MainComponent* main = MainComponent::getInstance();

    main->initialiseFromCommandLine(getCommandLineParameters(argc, argv), "CeritePort");

    std::condition_variable cv;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, []{return false;});
    
    return 0;
}
