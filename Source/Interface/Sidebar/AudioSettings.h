#include <JuceHeader.h>
#include "../../MainComponent.h"

class AudioSettings  : public Component
{
public:

	std::unique_ptr<AudioDeviceSelectorComponent> audioSetupComp;

	void init(MainComponent* main);

	AudioSettings();

	~AudioSettings()
	{
		audioSetupComp->setLookAndFeel(nullptr);
	};


private:

	void paint (Graphics& g) override;

	void resized () override;

	static String getListOfActiveBits (const BigInteger& b);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSettings)
};
