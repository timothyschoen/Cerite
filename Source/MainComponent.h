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

/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/




class MainComponent final : public Component, public FileDragAndDropTarget, public ChildProcessMaster, public Timer
{
public:
    
    ApplicationCommandManager commandManager;
    
	MainComponent();
	~MainComponent();
    
    Array<SafePointer<GUIContainer>> guiComponents;
    
    int oversample = 0;
    int osfactor = pow(2, oversample);
    
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


	int totalNodes = 0;

	std::atomic<bool> playing = false;
    std::atomic<bool> crashed = false;
    
    bool bypass = true;
	int fadecounter = 0;

	//MNASystem* getMNASystem();

	//==============================================================================
    /*
	void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
	{
        samplesPerBlock = samplesPerBlockExpected;
        oversampler.reset(new dsp::Oversampling<float>(2, oversample, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR));
        
        oversampler->initProcessing(samplesPerBlockExpected);
		//deviceManager.addMidiInputDeviceCallback("from Max 1", this);
		audioLock = &deviceManager.getAudioCallbackLock();
        midiCollector.reset(sampleRate * osfactor);
        //pd->setCallbackLock(audioLock);
        //pd->prepareToPlay(sampleRate, samplesPerBlockExpected, oversample);
		stopAudio();
	};
	void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

	void releaseResources() override;
     */
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

    void handleMessageFromSlave (const MemoryBlock &) override;

    void handleConnectionLost () override;
    
    void timerCallback() override;
    

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

	bool askToSave();
    
    JUCE_DECLARE_SINGLETON (MainComponent, true)

private:

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
