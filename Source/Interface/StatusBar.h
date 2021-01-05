#pragma once
#include <JuceHeader.h>
#include "Looks.h"
#include "LevelMeter.h"

class StatusBar : public Component, public Timer
{
	Component* parent;

	volumeSliderLook vlook;

public:
	StatusBar(Component* maincomponent);
	~StatusBar();

    std::unique_ptr<LevelMeter> lvlmeter;
    
	Slider volumeSlider;
    
    roundButton rlook;
    
    TextButton powerButton;

	void paint(Graphics & g) override;

	void resized() override;

	void timerCallback() override
	{
		repaint();
	}
};
