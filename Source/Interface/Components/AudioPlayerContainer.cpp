#include "AudioPlayerContainer.h"

AudioPlayerContainer::~AudioPlayerContainer()
{
	audioslider.setLookAndFeel(nullptr);
	playbutton.setLookAndFeel(nullptr);
	loopbutton.setLookAndFeel(nullptr);
}

AudioPlayerContainer::AudioPlayerContainer(ValueTree boxtree, Box* box) : thread("DirectoryReader")
{
	parent = box;
	boxTree = boxtree;
    
	programState = boxTree.getParent().getParent();
	parameters = boxTree.getChildWithName("Parameters");
	transportTree = parameters.getOrCreateChildWithName("Playing", nullptr);
	loopTree = parameters.getOrCreateChildWithName("Looping", nullptr);
    
	directorylist = std::unique_ptr<DirectoryContentsList>(new DirectoryContentsList(nullptr, thread));
	directorylist->addChangeListener(this);
	directorylist->setDirectory(FSManager::media, false, true);
    
	filelist = std::unique_ptr<FileListComponent>(new FileListComponent(*directorylist));
	filelist->addListener(this);
	filelist->setColour(ListBox::backgroundColourId, Colour(25, 25, 25));
	filelist->setColour(DirectoryContentsDisplayComponent::ColourIds::highlightColourId, Colour(35, 35, 35));
	filelist->setRowHeight(17);
    
	playbutton.setButtonText("g");
	playbutton.setClickingTogglesState(true);
	playbutton.setLookAndFeel(&glook);
    
	loopbutton.setButtonText("x");
	loopbutton.setClickingTogglesState(true);
	loopbutton.setLookAndFeel(&glook);
    
	audioslider.setTextBoxStyle(Slider::NoTextBox, 0, 0, 0);
	audioslider.setRange(0, 1, 0.0001);
	audioslider.setLookAndFeel(&alook);
    
	addAndMakeVisible(audioslider);
	addAndMakeVisible(playbutton);
	addAndMakeVisible(loopbutton);
	addAndMakeVisible(filelist.get());
    
	audioslider.onDragStart = [this]
	{
        stopTimer();
		if(playbutton.getToggleState())
		{
			transportTree.setProperty("Value", false, nullptr);
		}
	};
	audioslider.onDragEnd = [this]
	{
		float position = audioslider.getValue();
		int newsample = int(position * totalsamps);
		currentsamp = newsample;

		parameters.getOrCreateChildWithName("SetSample", nullptr).setProperty("Value", newsample, nullptr);

		if(playbutton.getToggleState())
		{
			transportTree.setProperty("Value", true, nullptr);
			startTimerHz(5);
		}
	};
	playbutton.onClick = [this]()
	{
        bool isready = parameters.getChildWithName("Path").getProperty("Value").toString().containsNonWhitespaceChars() && bool(programState.getProperty("Power"));
		bool newstate = playbutton.getToggleState() && isready;
		changeState(newstate ? Starting : Stopping);

		if(newstate != playbutton.getToggleState()) playbutton.setToggleState(newstate, dontSendNotification);

		playbutton.setButtonText(newstate ? "h" : "g");
	};
	loopbutton.onClick = [this]()
	{
		bool newstate = loopbutton.getToggleState() && bool(programState.getProperty("Power"));
		changeState(newstate ? LoopOn : LoopOff);

		if(newstate != loopbutton.getToggleState()) loopbutton.setToggleState(newstate, dontSendNotification);
	};
	constrainer.setMaximumSize(1200, 800);
	constrainer.setMinimumSize(180, 75);
	resizableCorner = std::make_unique<ResizableCornerComponent>(this, &constrainer);
	resizableCorner->setAlwaysOnTop(true);
	addAndMakeVisible(resizableCorner.get());
    
    startTimerHz(5);
	changeState(Stopping);
}

void AudioPlayerContainer::resized()
{
	parent->resized();
	playbutton.setBounds(5, 8, 30, 30);
	loopbutton.setBounds(40, 8, 30, 30);
	audioslider.setBounds(75, 8, getWidth() - 85, 30);
	filelist->setBounds(5, 43, getWidth() - 10, getHeight() - 65);
	resizableCorner->setBounds(getWidth() - 10, getHeight() - 10, 10, 10);
}

void AudioPlayerContainer::changeListenerCallback (ChangeBroadcaster *source)
{
	if(!parameters.isValid()) parameters = boxTree.getChildWithName("Parameters");

	thread.startThread(3);

	while(directorylist->isStillLoading())
	{
		Time::waitForMillisecondCounter(Time::getMillisecondCounter() + 10);
	}

	removeChildComponent(filelist.get());
	filelist.reset();
	filelist = std::unique_ptr<FileListComponent>(new FileListComponent(*directorylist));
	filelist->addListener(this);
	filelist->setColour(ListBox::backgroundColourId, Colour(25, 25, 25));
	filelist->setColour(DirectoryContentsDisplayComponent::ColourIds::highlightColourId, findColour(ScrollBar::ColourIds::thumbColourId));
	filelist->setRowHeight(17);
	addAndMakeVisible(filelist.get());
	resized();
}

void AudioPlayerContainer::timerCallback()
{
    bool power = bool(programState.getProperty("Power"));
	if (!power || state == Stopping)
	{
        playbutton.setToggleState(false, dontSendNotification);
        playbutton.setButtonText("g");
		changeState(power ? Stopping : Reset);
		return;
	}

	if(parameters.isValid() && audioslider.getThumbBeingDragged() == -1)
	{
        
        int lastsample = currentsamp;
		totalsamps = int(parameters.getChildWithName("NumSamples").getProperty("Value"));
        currentsamp = int(parameters.getChildWithName("CurrentSample").getProperty("Value"));

		if(!std::isfinite(totalsamps))
		{
			totalsamps = 1;
			return;
		}

		float position = (float)currentsamp / (float)totalsamps;

		if(lastsample > currentsamp)
		{
			position = position - 1.;

			if(!loopbutton.getToggleState())
			{
				playbutton.setToggleState(false, dontSendNotification);
				playbutton.setButtonText("g");
				changeState(Reset);
			}
			else
			{
				changeState(Reset);
				changeState(Starting);
			}
		}

		currentsamp *= (currentsamp < totalsamps);
        
        position = std::clamp<double>(position, 0, 1);
        if(!std::isfinite(position)) position = 0;
		audioslider.setValue(position, dontSendNotification);
	}
}


void AudioPlayerContainer::fileClicked (const File &file, const MouseEvent &e)
{
	if(parameters.isValid())
	{
		parameters.getOrCreateChildWithName("Path", nullptr).setProperty("Value", file.getFullPathName(), nullptr);
		playbutton.setToggleState(false, dontSendNotification);
		playbutton.setButtonText("g");
		changeState(Reset);
	}
};

Point<int> AudioPlayerContainer::getBestSize()
{
	return Point<int>(180, 110);
}

void AudioPlayerContainer::changeState (TransportState newState)
{
	if(newState == Reset)
	{
		state = Stopping;
		currentsamp = 0;
		audioslider.setValue(0.0, dontSendNotification);
		transportTree.setProperty("Value", false, nullptr);
	}
	else if (newState == LoopOn || newState == LoopOff)
	{
		loopTree.setProperty("Value", newState == LoopOn, nullptr);
	}
	else if ((newState == Starting || newState == Stopping) && state != newState)
	{
		state = newState;
		//state == Starting ? startTimerHz(5) : state == Stopping ? stopTimer() : void();
		transportTree.setProperty("Value", state == Starting, nullptr);
	}
}

