#pragma once

#include <JuceHeader.h>
#include "Canvas/Canvas.h"

class MainComponent;
class AppCommands : public SystemClipboard, public ApplicationCommandTarget
{
public:

    
    FileChooser saveChooser =  FileChooser("Select a save file", FSManager::home.getChildFile("Saves"), "*.clab");
    FileChooser openChooser = FileChooser("Choose file to open", FSManager::home.getChildFile("Saves"), "*.clab");
    
	Canvas* cnv;
    MainComponent* main;

	AffineTransform zoom = AffineTransform();

	enum keyCmd {none, New, Open, Save, Cut, Copy, Paste, RemoveSelection, SelectAll, Duplicate, Undo, Redo, ZoomIn, ZoomOut, NewBox, newEdge, SnapToGrid, NewNBX = 50, NewVSL, NewHSL, NewMSG, NewTGL, NewBNG, NewHRAD, NewVRAD};
    
    Array<String> names = {"Numbox", "Vertical Slider", "Horizontal Slider", "Message", "Toggle", "Bang", "Horizontal Radio", "Vertical Radio" };
    
	AppCommands(Canvas* toBeModified, MainComponent* main);

	bool perform (const InvocationInfo& info) override;

	void getAllCommands (Array<CommandID>& commands) override;

	void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result) override;

	ApplicationCommandTarget* getNextCommandTarget() override;
};

