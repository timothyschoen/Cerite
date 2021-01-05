#pragma once

#include <JuceHeader.h>
#include "GUIContainer.h"
#include "../Canvas/ProgramState.h"
#include "../../Engine/Worker/Source/Message.h"

enum TransportState
{
    Starting,
    Stopping,
    Reset,
    LoopOn,
    LoopOff
};
/*
struct PlayerBase : public ExternalProcessor, public ValueTree::Listener
{
	bool playing = false;
    
	int numSamples;
	int currentSample = 0;
    int oversamp = 0;
    int osfactor = pow(2, oversamp);

	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> readerSource;

    int varId;
    Cerite::Object* destination;
    
    double* target;
    
	int channels;
    
	AudioBuffer<float> buf;
    Array<float> bigbuf;
	AudioSourceChannelInfo audiochannel;
    std::unique_ptr<dsp::Oversampling<float>> oversampler;


     //dsp::AudioBlock<float> oversampledBlock;
    
    PlayerBase(std::string paramname, ValueTree tree, int nChannels = 1) : ExternalProcessor(paramname, tree), readerSource(nullptr)
	{
        paramTree.addListener(this);
		channels = nChannels;
		formatManager.registerBasicFormats();
		buf = AudioBuffer<float>(nChannels, 1024);
		audiochannel = AudioSourceChannelInfo(buf);
	}
    
    void init(Cerite::Object* obj) override {
         destination = obj;
         target = destination->getVariablePtr(param);
    }
    

	int loadFile(String path)
	{
        //oversamp = m.oversample;
        //osfactor = pow(2, oversamp);
        //oversampler.reset(new dsp::Oversampling<float>(channels, oversamp, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
        //oversampler->initProcessing(1024);
        
		File file = File(path);
		auto* reader = formatManager.createReaderFor (file);

		if (reader)
		{
			readerSource.reset (new AudioFormatReaderSource (reader, true));
			readerSource->setLooping(true);
			readerSource->prepareToPlay (1024, 44100.);
            numSamples = reader->lengthInSamples * osfactor;
            currentSample = 0;
            //paramTree.setProperty()
            setParameter("CurrentSample", currentSample);
            readerSource->setNextReadPosition(currentSample);
            readerSource->getNextAudioBlock(audiochannel);
            //oversampledBlock = oversampler->processSamplesUp(buf);
            return numSamples;
		}

		return 0;
	}

	void changeState(int newState)
	{
		switch (newState)
		{
		case Reset:
			playing = false;
                if(readerSource.get()) readerSource->setNextReadPosition(0);
			break;

		case Starting:
			playing = true;
			break;

		case Stopping:
			playing = false;
			break;
		}
	}

	void setPosition(int numSamples)
	{
        if (readerSource.get())
            readerSource->setNextReadPosition(numSamples);
        currentSample = numSamples;
	}

	long long getPosition()
	{
        if (!readerSource.get()) return 0;
		return readerSource->getNextReadPosition();
	}

    void tick(int channel = 0) override {
        if (!playing || !readerSource.get()) return;
    
        int bufSample = currentSample & 1023;
        
        if (bufSample == 0) {
            setParameter("CurrentSample", currentSample);
            readerSource->getNextAudioBlock(audiochannel);
            //oversampledBlock = oversampler->processSamplesUp(buf);
        }
        
        channel %= channels;
        currentSample++;
        
        if(currentSample >= numSamples) {
            setParameter("CurrentSample", currentSample + 1);
            currentSample = 0;
            
        }
        *target = buf.getSample(channel, bufSample);
    }
     void valueTreePropertyChanged(ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override
     {
          // Some fake properties that i call manually to synchronize two audio outputs
          if(property == Identifier("Update"))
          {
               paramTree.getChildWithName("CurrentSample").setPropertyExcludingListener(this, "Value", getPosition(), nullptr);
               paramTree.sendPropertyChangeMessage("Sync");
          }
          else if(property == Identifier("Sync"))
          {
               setPosition(int(paramTree.getChildWithName("CurrentSample").getProperty("Value")));
          }

          if(property != Identifier("Value"))
               return;

          if(treeWhosePropertyHasChanged.hasType("Playing"))
          {
               changeState(bool(treeWhosePropertyHasChanged.getProperty("Value")) ? Starting : Stopping);
          }
          else if(treeWhosePropertyHasChanged.hasType("Looping"))
          {
               changeState(bool(treeWhosePropertyHasChanged.getProperty("Value")) ? LoopOn : LoopOff);
          }
          else if(treeWhosePropertyHasChanged.hasType("Path"))
          {
               numSamples = loadFile(treeWhosePropertyHasChanged.getProperty("Value").toString());
               setParameter("NumSamples", String(numSamples));
          }
          else if(treeWhosePropertyHasChanged.hasType("SetSample"))
          {
               setPosition(treeWhosePropertyHasChanged.getProperty("Value"));
          }
     }
}; */

class AudioFileList : public FileListComponent
{

    AudioFileList(DirectoryContentsList& d) :  FileListComponent(d)
    {
    };
};

class AudioPlayerContainer : public GUIContainer, public ChangeListener, public FileBrowserListener, private Timer
{

public:


    std::unique_ptr<DirectoryContentsList> directorylist;
    std::unique_ptr<FileListComponent> filelist;

    TextButton playbutton;
    TextButton loopbutton;

    graphicsButton glook;

    std::unique_ptr<ResizableCornerComponent> resizableCorner;

    ComponentBoundsConstrainer constrainer;

    Slider audioslider;

    TimeSliceThread thread;

    TransportState state = Stopping;


    Box* parent;

    audioSliderLook alook;
    
    AudioFormatManager formatManager;

    std::atomic<int> currentsamp = 0;
    int totalsamps = 0;
    int nextSample = 0;

    ~AudioPlayerContainer();

    AudioPlayerContainer(ValueTree boxtree, Box* box);

    void resized() override;

    void changeListenerCallback (ChangeBroadcaster *source) override;

    void selectionChanged() override  {};

    void timerCallback() override;

    void setPosition(int positionInSamples) {
        currentsamp = positionInSamples;
    }
    
    void setID(int newID) override;

    void fileClicked (const File &file, const MouseEvent &e) override;

    void fileDoubleClicked (const File &file) override {};

    void browserRootChanged (const File &newRoot) override {};

    void changeState (TransportState newState);
    
    Point<int> getBestSize() override;
    
    void sendMessage(MemoryOutputStream& m);
};


