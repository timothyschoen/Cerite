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

#include "Engine/Cerite/src/Interface/Patch.h"

/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/




class MainComponent final : public AudioAppComponent, public MidiInputCallback, public ApplicationCommandManager, public FileDragAndDropTarget
{
public:

	MainComponent();
	~MainComponent();
    
    int oversample = 0;
    int osfactor = pow(2, oversample);
    
    OwnedArray<ExternalProcessor> external;
    
    std::unique_ptr<dsp::Oversampling<float>> oversampler;
    
    //std::unique_ptr<CeriteAudioProcessor> pd;

	juce::MidiMessageCollector midiCollector;

	CriticalSection* audioLock;

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

    std::unique_ptr<Cerite::Patch> processor = nullptr;
    
    double* input;
    double* output;
	//IProcessor* net = nullptr;
	//IProcessor* newnet = nullptr;

	//MNASystem* msystem = nullptr;

	std::vector<int> lastMidiSelection;
    
	int totalNodes = 0;

	bool playing = false;

    bool bypass = true;
	int fadecounter = 0;
	double volume = 0.5;
    
    int samplesPerBlock = 512;

	FileChooser saveChooser =  FileChooser("Select a save file", FSManager::home.getChildFile("Saves"), "*.clab");
	FileChooser openChooser = FileChooser("Choose save location", FSManager::home.getChildFile("Saves"), "*.clab");

	//MNASystem* getMNASystem();

	//==============================================================================
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

	//==============================================================================
	void paint (Graphics& g) override;
	void resized() override;
    
    static void logMessage(const char* str);

	//==============================================================================

	void startAudio(double sampleRate, std::vector<double> settings);
	void stopAudio();

	void updateSystem();

	//void ExportWavFile(IProcessor* proc, double sampleRate);

	bool saveAudioFile();
    
    void saveCode();

	void setTitle(std::string newTitle);

	void loadLibrary(std::string path);

	File chosenFile;

	AudioDeviceManager* getAudioManager()
	{
		return &deviceManager;
	};

	void updateMidiInputs();

	void handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message) override
	{
        midiCollector.addMessageToQueue(message);
		std::vector<int> midiMessage(3, 0);
		midiMessage[0] = message.isNoteOn() ? 144 : message.isNoteOff() ? 128     : message.isController() ? 176 : 0;
		midiMessage[1] = message.isController()   ? message.getControllerNumber() : message.getNoteNumber();
		midiMessage[2] = message.isController()   ? message.getControllerValue () : message.getVelocity();

	}

    void setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo)
    {
        toolbar.setUndoState(setUndo, canUndo, setRedo, canRedo);
        topmenu.menuItemsChanged();
    }
	void updateUndoState()
	{
		toolbar.checkUndoState();
		commandStatusChanged();
		topmenu.menuItemsChanged();
	}

	bool askToSave();

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
