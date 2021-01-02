#pragma once
#include <JuceHeader.h>

/*
struct Theme
{
    Colour border =  Colour()l
    
    
};
*/

class CeriteLookAndFeel : public LookAndFeel_V4
{
public:

	CeriteLookAndFeel();

	// Icon font
	Font Entypo = (Font (Typeface::createSystemTypefaceFor (BinaryData::CLFont_ttf, BinaryData::CLFont_ttfSize)));

    Font getComboBoxFont(ComboBox &) override {
        return Font(13);
    }
    
    void drawComboBox (Graphics& g, int width, int height, bool,
                       int, int, int, int, ComboBox& box) override;
    
    void drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
	void drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g, int w, int h, int titleSpaceX, int titleSpaceW, const Image* icon, bool drawTitleTextOnLeft) override;

	// Toolbar buttons
	void paintToolbarButtonLabel (Graphics & g, int x, int y, int width, int height, const String & text, ToolbarItemComponent & button) override;

	void drawFileBrowserRow (Graphics& g, int width, int height, const File&, const String& filename, Image* icon, const String& fileSizeDescription, const String& fileTimeDescription, bool isDirectory, bool isItemSelected, int /*itemIndex*/, DirectoryContentsDisplayComponent& dcc) override;
	// For tooltips
	static TextLayout layoutTooltipText (const String& text, Colour colour) noexcept;

	void drawTooltip(Graphics&g, const String& text, int width, int height) override;
};

class roundButton : public LookAndFeel_V4
{
	Font Entypo = (Font (Typeface::createSystemTypefaceFor (BinaryData::CLFont_ttf, BinaryData::CLFont_ttfSize)));

public:
	roundButton();

	Font getTextButtonFont (TextButton&, int buttonHeight) override;

	void drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

class graphicsButton : public LookAndFeel_V4
{
	Font Entypo = (Font (Typeface::createSystemTypefaceFor (BinaryData::CLFont_ttf, BinaryData::CLFont_ttfSize)));

public:
	graphicsButton();

	Font getTextButtonFont (TextButton&, int buttonHeight) override;

	void drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

class audioSliderLook : public LookAndFeel_V4
{

public:
	audioSliderLook()
	{
		setColour (Slider::backgroundColourId, Colours::white);
		setColour (Slider::trackColourId, Colours::white);
		setColour (Slider::thumbColourId, Colours::white);
	};

	void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override;

	int getSliderThumbRadius(Slider &) override;

};

class volumeSliderLook : public LookAndFeel_V4
{

public:

	void drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider) override;

	int getSliderThumbRadius(Slider &) override;

};



class SidebarLookAndFeel : public LookAndFeel_V2
{
public:
	SidebarLookAndFeel();

	~SidebarLookAndFeel() {}

	Font Entypo = (Font (Typeface::createSystemTypefaceFor (BinaryData::CLFont_ttf, BinaryData::CLFont_ttfSize)));

	Font getTabButtonFont (TabBarButton& b, float height);

	void drawTabButtonText (TabBarButton & button, Graphics & g, bool isMouseOver, bool isMouseDown);

	void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown);

	void drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h);

};




class nameSuggestionsButtons : public LookAndFeel_V4
{
public:
	nameSuggestionsButtons();

	void drawButtonText (Graphics& g, TextButton& button, bool isMouseOverButton, bool isButtonDown) override;


	void drawButtonBackground(Graphics & g, Button & button, const Colour & backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

};
