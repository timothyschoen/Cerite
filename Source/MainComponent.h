/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <mutex>
#include <thread>
#include <JuceHeader.h>

#include "Interface/Looks.h"
#include "Interface/ToolbarWrapper.h"
#include "Interface/AppCommands.h"
#include "Interface/Sidebar/Sidebar.h"
#include "Interface/StatusBar.h"
#include "Interface/TopMenu.h"

#include "Interface/Components/GUIContainer.h"

#include "Interface/ExportDialog.h"
#include "Interface/Canvas/Canvas.h"
#include "Interface/Canvas/Box.h"
#include "Utility/FSManager.h"

#include <boost/interprocess/ipc/message_queue.hpp>

/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

using namespace boost::interprocess;


class MainComponent final : public Component, public FileDragAndDropTarget, public Timer
{
public:
    
    ApplicationCommandManager commandManager;
    
	MainComponent();
	~MainComponent();
    
    Array<SafePointer<GUIContainer>> guiComponents;
    
    std::unique_ptr<message_queue> sendQueue = nullptr;
    std::unique_ptr<message_queue> receiveQueue = nullptr;
    
    int oversample = 0;
    int osfactor = pow(2, oversample);
    
    ChildProcess worker;
    
    Canvas canvas;
    AppCommands appcmds;

	Viewport viewport;
    Sidebar sidebar;

    ToolbarWrapper toolbar;

	roundButton rlook;
	CeriteLookAndFeel clook;

    TopMenu topmenu;
    StatusBar statusbar;

	Box* selection;
    
    bool pingReceived = true;


	int totalNodes = 0;

	std::atomic<bool> playing = false;
    std::atomic<bool> crashed = false;
    
    bool bypass = true;
	int fadecounter = 0;

	//==============================================================================
	void paint (Graphics& g) override;
	void resized() override;
    
    static void logMessage(const char* str);

	//==============================================================================

    void setVolume(float level);
    
	void startAudio(double sampleRate, std::vector<double> settings);
    
    void attachParameters();
    
	void stopAudio();

	void updateSystem();

	bool saveAudioFile();
    
    void saveCode();

	void setTitle(std::string newTitle);

	void loadLibrary(std::string path);

    void handleMessage(const MemoryBlock & m);
    
    void setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo)
    {
        toolbar.setUndoState(setUndo, canUndo, setRedo, canRedo);
        topmenu.menuItemsChanged();
    }
	void updateUndoState()
	{
		toolbar.checkUndoState();
		commandManager.commandStatusChanged();
		topmenu.menuItemsChanged();
	}
    
    const size_t msg_size = 1 << 10;
    
    void sendMessage(MemoryBlock m, unsigned int priority = 0) {
        
        if(!sendQueue) return;
        
        if(m.getSize() > msg_size)
        {
            std::cout << "message too big!" << std::endl;
        }
       
        m.ensureSize(msg_size);
        // Use try to prevent blocking the audio thread
        sendQueue->send(m.getData(), msg_size, priority);
    }
    
    int updateCount = 0;
    void timerCallback() override {
        
          MemoryBlock memblock;
          while(receiveMessage(memblock)) {
              handleMessage(memblock);
              memblock.reset();
          }
        
        if(updateCount > 350) {
            if(!pingReceived) {
                //std::cout << "NO RESPONSE!" << std::endl;
            }
            pingReceived = false;
            updateCount = 0;
        }
        updateCount++;
        
    }
    
    bool receiveMessage(MemoryBlock& m)
    {
        if(!receiveQueue) return false;
        
        message_queue::size_type recvd_size;
        
        unsigned int priority;
        
        if(receiveQueue->try_receive(receivedData, msg_size, recvd_size, priority))
        {
            m.replaceWith(receivedData, msg_size);
            return true;
        }
        
        return false;
    }

	bool askToSave();
    
    JUCE_DECLARE_SINGLETON (MainComponent, true)

private:

    void* receivedData;
    
	bool somethingIsBeingDraggedOver = false;

	bool isInterestedInFileDrag (const StringArray & files) override
	{
		if(files.size() == 1)
			return true;
		else
			return false;
	}

	void fileDragEnter (const StringArray& files, int /*x*/, int /*y*/) override;
	void fileDragMove (const StringArray&, int /*x*/, int /*y*/) override {};
	void fileDragExit (const StringArray& /*files*/) override;
	void filesDropped (const StringArray& files, int x, int y) override;

    
};
