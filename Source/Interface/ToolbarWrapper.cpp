#include "ToolbarWrapper.h"
#include "../MainComponent.h"


ToolbarWrapper::ToolbarWrapper(MainComponent* main)
{
	setStyle(Toolbar::ToolbarItemStyle::textOnly);
	setVertical(false);
	setAlwaysOnTop(true);
	cnv = &(main->canvas);
    
    functions.resize(8);
	// Functions when the buttons are clicked
	functions[0] = [this, main]()->void
	{
        main->appcmds.invokeDirectly(AppCommands::keyCmd::New, true);
	};
    
    functions[1] = [this, main]()->void
	{
        main->appcmds.invokeDirectly(AppCommands::keyCmd::Open, true);
	};
    
    functions[2] = [this, main]()->void
	{
        main->appcmds.invokeDirectly(AppCommands::keyCmd::Save, true);
	};
    functions[3] = [this, main]()->void
	{
        main->appcmds.invokeDirectly(AppCommands::keyCmd::Undo, true);
	};
    
    functions[4] = [this, main]()->void
	{
		main->appcmds.invokeDirectly(AppCommands::keyCmd::Redo, true);
	};
    
    
    functions[5] = [this, main]()->void
    {

        
        PopupMenu subMenu;
        subMenu.addItem (4, "Numbox");
        subMenu.addItem (5, "Message");
        subMenu.addItem (6, "Bang");
        subMenu.addItem (7, "Toggle");
        subMenu.addItem (8, "HSlider");
        subMenu.addItem (9, "VSlider");
        subMenu.addItem (10, "HRadio");
        subMenu.addItem (11, "VRadio");
     
        PopupMenu mainMenu;
        mainMenu.addItem (1, "Empty Box");
        mainMenu.addItem (2, "Point");
        mainMenu.addSubMenu ("GUI Objects", subMenu);
     
        //mainMenu.setLookAndFeel(&clook);
        const int result = mainMenu.showAt(getItemComponent(5)->getScreenBounds());
        
        if(result == 1) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewBox, true);
        }
        else if(result == 2) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::newEdge, true);
        }
        else if(result == 4) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewNBX, true);
        }
        else if(result == 5) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewMSG, true);
        }
        else if(result == 6) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewBNG, true);
        }
        else if(result == 7) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewTGL, true);
        }
        else if(result == 8) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewHSL, true);
        }
        else if(result == 9) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewVSL, true);
        }
        else if(result == 10) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewHRAD, true);
        }
        else if(result == 11) {
            main->appcmds.invokeDirectly(AppCommands::keyCmd::NewVRAD, true);
        }

    };
    
    functions[6] = [this, main]()->void
    {
        PopupMenu menu;
        menu.addItem (1, "Export Audio");
        menu.addItem (2, "Export Code");
    
        const int result = menu.showAt(getItemComponent(6)->getScreenBounds());
        if(result == 1) {
            main->saveAudioFile();
        }
        else if (result == 2){
            
        }
    };
    
    
	addItem(*this, 1);
	addItem(*this, 2);
	addItem(*this, 3);
	addItem(*this, -2);
	addItem(*this, 4);
	addItem(*this, 5);
    addItem(*this, -2);
    addItem(*this, 6);
    addItem(*this, 7);
}


ToolbarItemComponent* ToolbarWrapper::createItem(int itemId)
{
	DrawableText text;
	ToolbarItemComponent* newitem;
	newitem = new ToolbarButton(itemId, names[itemId - 1], text.createCopy(), 0);
	newitem->setLookAndFeel(&clook);
	newitem->onClick = functions[itemId - 1];
	return newitem;
}

void ToolbarWrapper::getAllToolbarItemIds (Array< int > & ids)
{
	ids.addArray({1, 2, 3, -2, 4, 5, -2, 6, 7});
}

void ToolbarWrapper::getDefaultItemSet (Array< int > & ids)
{
	ids.addArray({1, 2, 3, -2, 4, 5, -2, 6, 7});
}

void ToolbarWrapper::setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo)
{
    
    if(setUndo)
        getItemComponent(4)->setEnabled(canUndo);
    
    if(setRedo)
        getItemComponent(5)->setEnabled(canRedo);
}

void ToolbarWrapper::checkUndoState()
{
	if(cnv->connectingEdge == nullptr)
	{
		getItemComponent(4)->setEnabled(cnv->undoManager.canUndo());
		getItemComponent(5)->setEnabled(cnv->undoManager.canRedo());
	}
	else
	{
		getItemComponent(4)->setEnabled(false);
		getItemComponent(5)->setEnabled(false);
	}
}
