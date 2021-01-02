#pragma once

#include <JuceHeader.h>
#include "../Looks.h"
#include "Inspector/Inspector.h"
#include "SidebarDialog.h"

class Sidebar :  public Component
{
public:

    TabbedButtonBar tabs;

	int selectedTab = 0;

	OwnedArray<Component> content;

	Component tabbar;

    TabbedButtonBar hidebar;

	Inspector* inspector;
    
    inline static Console* console = nullptr;

	SidebarLookAndFeel clook;

	ComponentDragger myDragger;

	Label title;

	Array<String> titles = {"Engine Settings", "Audio Settings", "Console", "Inspector:    None"};

	Component* main;

	Sidebar(Component* parent);

	~Sidebar();

	void resized() override;

	void paint(Graphics & g) override;

	void mouseUp(const MouseEvent& e) override
	{
		repaint();
	}
	void mouseDrag (const MouseEvent& e) override;
    
    int createDialog (DialogBase* comp, Component* parent);
    
	void mouseDown (const MouseEvent& e) override;
    
    void logConsole (const char* msg);

};
