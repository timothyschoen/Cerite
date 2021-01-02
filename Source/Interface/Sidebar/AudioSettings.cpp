#include <JuceHeader.h>
#include "AudioSettings.h"
#include "../../MainComponent.h"




AudioSettings::AudioSettings()
{
	setOpaque (true);
	setBounds(getLocalBounds());
	//AudioSettings* audiosettings = static_cast<AudioSettings*>(sidebar.content[1]);
	setSize (500, 600);
}

void AudioSettings::init(MainComponent* main)
{
	audioSetupComp.reset (new AudioDeviceSelectorComponent (main->deviceManager, 0, 256, 0, 256, true, true, true, false));
	addAndMakeVisible(audioSetupComp.get());
	resized();
}

void AudioSettings::resized()
{
	if(audioSetupComp != nullptr)
		audioSetupComp->setBounds(getLocalBounds());
}
void AudioSettings::paint (Graphics& g)
{
	g.fillAll (findColour(SidePanel::ColourIds::backgroundColour));
}

String AudioSettings::getListOfActiveBits (const BigInteger& b)
{
	StringArray bits;

	for (int i = 0; i <= b.getHighestBit(); ++i)
		if (b[i])
			bits.add (String (i));

	return bits.joinIntoString (", ");
}

