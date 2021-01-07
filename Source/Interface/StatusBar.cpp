#include "StatusBar.h"
#include "../MainComponent.h"

StatusBar::StatusBar(Component* maincomponent)
{
	parent = maincomponent;
	MainComponent* main = static_cast<MainComponent*>(parent);
	volumeSlider.setLookAndFeel(&vlook);
	addAndMakeVisible(volumeSlider);
    
    /*
	volumeSlider.textFromValueFunction = [](double value)
	{
		return Decibels::toString (value);
	};
	volumeSlider.valueFromTextFunction = [](const String & text)
	{
		auto decibelText = text.upToFirstOccurrenceOf ("dB", false, false).trim();
		auto minusInfinitydB = -100.0;
		return decibelText.equalsIgnoreCase ("-INF") ? minusInfinitydB : decibelText.getDoubleValue();
	};
    */
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
	volumeSlider.setRange(-60, 6);
	volumeSlider.setValue(-6);
	volumeSlider.setAlwaysOnTop(true);
	volumeSlider.onValueChange = [this, main] { main->setVolume( Decibels::decibelsToGain ((float) volumeSlider.getValue())); };
    
	lvlmeter = std::make_unique<LevelMeter>();
	addAndMakeVisible(lvlmeter.get());

    powerButton.setLookAndFeel(&rlook);

    powerButton.setButtonText("J");

    powerButton.setClickingTogglesState(true);
    
    addAndMakeVisible(powerButton);
    
    powerButton.setColour(ComboBox::outlineColourId, Colour(0));
    powerButton.setColour(TextButton::buttonColourId, Colour(35, 35, 35));
    powerButton.setColour(TextButton::buttonOnColourId, Colour(25, 25, 25));
    powerButton.onClick = [this, main]()
    {
        if(powerButton.getToggleState())
        {
            powerButton.setColour(TextButton::textColourOnId, Colours::orange);
            main->startAudio(44100, ((EngineSettings*)(main->sidebar.content[0]))->settings);
            powerButton.setColour(TextButton::textColourOnId, Colours::green);
        }
        else
        {
            powerButton.setColour(TextButton::textColourOnId, Colours::red);
            main->stopAudio();
        }
    };
    
	startTimerHz(1);
}

StatusBar::~StatusBar()
{
	volumeSlider.setLookAndFeel(nullptr);
    powerButton.setLookAndFeel(nullptr);
}


String roundToString(double number)
{
	int len = (number >= 1.) ? 4 : 3;
	String num_text = String(number * 100);
	return num_text.substring(0, num_text.lastIndexOf(".") + len);
}

void StatusBar::paint(Graphics & g)
{
    String cpuUsage = "0"; //roundToString(main->deviceManager.getCpuUsage());
	float alpha = 0.8;
	g.setColour(Colour(41, 41, 41).withAlpha(alpha));
	g.fillRect(getLocalBounds());
	g.setColour(Colours::white);
	g.drawText("CPU Load: " + cpuUsage + "% ", 10, 0, 120, 30, 1);
	//g.drawText("Audio Status: " + main->deviceManager.getCurrentAudioDeviceType(), 130, 2, 330, 30, 1);
}

void StatusBar::resized()
{
	lvlmeter->setBounds(getWidth() - 135, 4, 110, 22);
	volumeSlider.setBounds(getWidth() - 139, 2, 119, 26);
    powerButton.setBounds(getWidth() - 163, 5, 30, 20);
}
