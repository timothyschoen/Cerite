#pragma once
#include <JuceHeader.h>
#include "Looks.h"
#include "Canvas/Canvas.h"

class MainComponent;
class TopMenu : public Component,
	public MenuBarModel,
	public ChangeBroadcaster,
	private Button::Listener
{
public:

	Canvas* canvas;
    MainComponent* parent;

	std::vector<std::string> paths = std::vector<std::string>(10);

	TopMenu(Canvas* cnv, MainComponent* main);

	~TopMenu();

	void resized() override;

	//==============================================================================
	StringArray getMenuBarNames() override;

	PopupMenu getMenuForIndex (int menuIndex, const String& /*menuName*/) override;

	void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/) override;

private:
	TextButton popupButton;
	ScopedPointer<MenuBarComponent> menuBar;


	//==============================================================================
	void buttonClicked (Button* button) override {};

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopMenu)
};

