#include <JuceHeader.h>
#include "TopMenu.h"
#include "../MainComponent.h"


TopMenu::TopMenu(Canvas* cnv, Component* main)
{
	parent = main;
	canvas = cnv;
	addAndMakeVisible (menuBar = new MenuBarComponent (this));
	popupButton.setButtonText ("Show Popup Menu");
	popupButton.setTriggeredOnMouseDown (true);
	popupButton.addListener (this);
	addAndMakeVisible (popupButton);
#if JUCE_MAC
	MenuBarModel::setMacMainMenu (this);
#endif
	// fix later
	//setApplicationCommandManagerToWatch (&MainAppWindow::getApplicationCommandManager());
}

TopMenu::~TopMenu()
{
#if JUCE_MAC
	MenuBarModel::setMacMainMenu (nullptr);
#endif
	PopupMenu::dismissAllActiveMenus();
	popupButton.removeListener (this);
}

void TopMenu::resized()
{
	Rectangle<int> area (getLocalBounds());
	menuBar->setBounds (area.removeFromTop (LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));
	area.removeFromTop (20);
	area = area.removeFromTop (33);
	popupButton.setBounds (area.removeFromLeft (200).reduced (5));
}

//==============================================================================
StringArray TopMenu::getMenuBarNames()
{
	const char* const names[] = { "File", "Edit", "View", "Options",  nullptr };
	return StringArray (names);
}

PopupMenu TopMenu::getMenuForIndex (int menuIndex, const String& /*menuName*/)
{
	MainComponent* main = static_cast<MainComponent*>(parent);
	PopupMenu menu;

	if (menuIndex == 0)
	{
		menu.addCommandItem(main, AppCommands::keyCmd::New);
		menu.addCommandItem(main, AppCommands::keyCmd::Open);
		PopupMenu subMenu;
		XmlElement* recent = FSManager::meta->getChildByName("recentlyOpened");

		for (int s = 0; s < recent->getNumChildElements(); s++)
		{
			XmlElement* file = recent->getChildElement(s);
			paths[s] = file->getStringAttribute("Path").toStdString();
			subMenu.addItem (100 + s, file->getTagName());
		}

		menu.addSubMenu ("Open Recent", subMenu);
		menu.addCommandItem(main, AppCommands::keyCmd::Save);
		menu.addItem(998, "Save As");
		//menu.addItem(6, "Quit");
	}
	else if (menuIndex == 1)
	{
		menu.addCommandItem(main, AppCommands::keyCmd::NewBox);
		menu.addCommandItem(main, AppCommands::keyCmd::newEdge);
		menu.addSeparator();
		menu.addCommandItem(main, AppCommands::keyCmd::Cut);
		menu.addCommandItem(main, AppCommands::keyCmd::Copy);
		menu.addCommandItem(main, AppCommands::keyCmd::Paste);
		menu.addCommandItem(main, AppCommands::keyCmd::Duplicate);
		menu.addCommandItem(main, AppCommands::keyCmd::SelectAll);
		menu.addCommandItem(main, AppCommands::keyCmd::RemoveSelection);
		menu.addSeparator();
		menu.addCommandItem(main, AppCommands::keyCmd::Undo);
		menu.addCommandItem(main, AppCommands::keyCmd::Redo);
		menu.addSeparator();
	}
	else if (menuIndex == 2)
	{
		menu.addCommandItem(main, AppCommands::keyCmd::ZoomIn);
		menu.addCommandItem(main, AppCommands::keyCmd::ZoomOut);
		menu.addItem(999, "Show At 100%");
		menu.addCommandItem(main, AppCommands::keyCmd::SnapToGrid);
	}
	else if (menuIndex == 3)
	{
		menu.addItem(996, "Audio/MIDI Settings");
	}

	return menu;
}

void TopMenu::menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
{
	MainComponent* m = static_cast<MainComponent*>(parent);
	// most of our menu items are invoked automatically as commands, but we can handle the
	// other special cases here..

	if (menuItemID == 998)
	{
		canvas->projectFile = File(); // unload the current file
		m->appcmds.invokeDirectly(AppCommands::keyCmd::Save, false); // Save in a new location
		canvas->addToHistory(canvas->projectFile);
	}

	if (menuItemID == 999)
	{
		m->appcmds.zoom = m->appcmds.zoom.scale(1);
		canvas->setTransform(m->appcmds.zoom);
	}
	else if (menuItemID >= 100 && menuItemID < 200)
	{
		if (m->askToSave())
		{
			canvas->clearState();

			try
			{
                m->sidebar.inspector->deselect();
				File fileToOpen(paths[menuItemID - 100]);
                if(fileToOpen.exists()) {
                    m->setTitle(fileToOpen.getFileName().toStdString() + " | Cerite");
                    canvas->setState(fileToOpen.loadFileAsString());
                    canvas->addToHistory(File(paths[menuItemID - 100]));
                    canvas->projectFile = fileToOpen;
                }
			}
			catch(...)
			{
				canvas->clearState();
			}
		}
	}
}
