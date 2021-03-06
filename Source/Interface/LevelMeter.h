#include <JuceHeader.h>

struct LevelMeter  : public Component, public Timer
{
	LevelMeter ()
	{
		startTimerHz (20);
	}

	~LevelMeter() override
	{
	}
    
    void setLevel(float lvl) {
        level = lvl;
    }

	void timerCallback() override
	{
		if (isShowing())
		{
            if(!std::isfinite(level.load()))  {
                level = 0;
                return;
            }
            
			if (std::abs (level - oldLevel) > 0.005f)
			{
				repaint();
			}
            
            oldLevel = level.load();
		}
		else
		{
			level = 0;
		}
	}

	void paint (Graphics& g) override
	{
		int width = getWidth();
		int height = getHeight();
		float lvl = (float) std::exp (std::log (level) / 3.0) * (level > 0.002);
		auto outerCornerSize = 3.0f;
		auto outerBorderWidth = 2.0f;
		auto totalBlocks = 20;
		auto spacingFraction = 0.03f;
		g.setColour (findColour (ResizableWindow::backgroundColourId));
		g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height), outerCornerSize);
		auto doubleOuterBorderWidth = 2.0f * outerBorderWidth;
		auto numBlocks = roundToInt (totalBlocks * lvl);
		auto blockWidth = (width - doubleOuterBorderWidth) / static_cast<float> (totalBlocks);
		auto blockHeight = height - doubleOuterBorderWidth;
		auto blockRectWidth = (1.0f - 2.0f * spacingFraction) * blockWidth;
		auto blockRectSpacing = spacingFraction * blockWidth;
		auto blockCornerSize = 0.1f * blockWidth;
		auto c = findColour (Slider::thumbColourId);

		for (auto i = 0; i < totalBlocks; ++i)
		{
			if (i >= numBlocks)
				g.setColour (c.withAlpha (0.5f));
			else
				g.setColour (i < totalBlocks - 1 ? c : Colours::red);

			g.fillRoundedRectangle (outerBorderWidth + (i * blockWidth) + blockRectSpacing,
			                        outerBorderWidth,
			                        blockRectWidth,
			                        blockHeight,
			                        blockCornerSize);
		}
	}

    std::atomic<float> oldLevel = 0;
	std::atomic<float> level = 0;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
