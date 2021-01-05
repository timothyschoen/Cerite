#include <utility>
#include <string>
#include "AppCommands.h"
#include "Canvas/Connection.h"
#include "Canvas/Box.h"
#include "SaveDialog.h"
#include "../MainComponent.h"



AppCommands::AppCommands (Canvas* toBeModified, MainComponent* maincomponent)
{
	cnv = toBeModified;
	main = maincomponent;
}

void AppCommands::getAllCommands (Array<CommandID>& commands)
{
    if(cnv->codeEditor) return;
    
	commands.add (keyCmd::New);
	commands.add (keyCmd::Open);
	commands.add (keyCmd::Save);
	commands.add (keyCmd::Cut);
	commands.add (keyCmd::Copy);
	commands.add (keyCmd::Paste);
	commands.add (keyCmd::Duplicate);
	commands.add (keyCmd::Undo);
	commands.add (keyCmd::Redo);
	commands.add (keyCmd::ZoomIn);
	commands.add (keyCmd::ZoomOut);
	commands.add (keyCmd::NewBox);
	commands.add (keyCmd::newEdge);
	commands.add (keyCmd::RemoveSelection);
	commands.add (keyCmd::SelectAll);
	commands.add (keyCmd::SnapToGrid);
    
    commands.add (keyCmd::NewNBX);
    commands.add (keyCmd::NewVSL);
    commands.add (keyCmd::NewHSL);
    commands.add (keyCmd::NewMSG);
    commands.add (keyCmd::NewTGL);
    commands.add (keyCmd::NewBNG);
    commands.add (keyCmd::NewHRAD);
    commands.add (keyCmd::NewVRAD);
}

void AppCommands::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
	if (commandID == keyCmd::New)
	{
		result.setInfo (TRANS("New"),
		                TRANS("Create a new document"),
		                "File", 8);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('n', ModifierKeys::commandModifier, 'n'));
	}

	if (commandID == keyCmd::Open)
	{
		result.setInfo (TRANS("Open"),
		                TRANS("Open a project"),
		                "File", 8);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('o', ModifierKeys::commandModifier, 'o'));
	}

	if (commandID == keyCmd::Save)
	{
		result.setInfo (TRANS("Save"),
		                TRANS("Save a project"),
		                "File", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 's'));
	}

	if (commandID == keyCmd::Cut)
	{
		result.setInfo (TRANS("Cut"),
		                TRANS("Cut Selection"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('x', ModifierKeys::commandModifier, 'x'));
	}

	if (commandID == keyCmd::Copy)
	{
		result.setInfo (TRANS("Copy"),
		                TRANS("Copy Selection"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('c', ModifierKeys::commandModifier, 'c'));
	}

	if (commandID == keyCmd::Paste)
	{
		result.setInfo (TRANS("Paste"),
		                TRANS("Paste Clipboard"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('v', ModifierKeys::commandModifier, 'v'));
	}

	if (commandID == keyCmd::Duplicate)
	{
		result.setInfo (TRANS("Duplicate"),
		                TRANS("Duplicate Selection"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('d', ModifierKeys::commandModifier, 'd'));
	}

	if (commandID == keyCmd::Undo)
	{
		result.setInfo (TRANS("Undo"),
		                TRANS("Undo last action"),
		                "Edit", 0);
		result.setActive(cnv->undoManager.canUndo());
		result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier, 'z'));
	}

	if (commandID == keyCmd::Redo)
	{
		result.setInfo (TRANS("Redo"),
		                TRANS("Redo"),
		                "Edit", 0);
		result.setActive(cnv->undoManager.canRedo());
		result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::commandModifier, 'r'));
	}

	if (commandID == keyCmd::ZoomIn)
	{
		result.setInfo (TRANS("Zoom In"),
		                TRANS("Zooms in"),
		                "View", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('=', ModifierKeys::commandModifier, '=')); // plus key but without shift
	}

	if (commandID == keyCmd::ZoomOut)
	{
		result.setInfo (TRANS("Zoom Out"),
		                TRANS("Zooms out"),
		                "View", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('-', ModifierKeys::commandModifier, '-'));
	}

	if (commandID == keyCmd::NewBox)
	{
		result.setInfo (TRANS("New Box"),
		                TRANS("Create a new box"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('n', ModifierKeys::noModifiers, 'n'));
	}

	if (commandID == keyCmd::newEdge)
	{
		result.setInfo (TRANS("New Point"),
		                TRANS("Create a new connection point"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('p', ModifierKeys::noModifiers, 'p'));
	}

	if (commandID == keyCmd::RemoveSelection)
	{
		result.setInfo (TRANS("Remove Selection"),
		                TRANS("Removes selected components"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey
		                                        , ModifierKeys::noModifiers, KeyPress::backspaceKey));
	}

	if (commandID == keyCmd::SelectAll)
	{
		result.setInfo (TRANS("Select All"),
		                TRANS("Selects all components"),
		                "Edit", 0);
		result.setActive(true);
		result.defaultKeypresses.add (KeyPress ('a'
		                                        , ModifierKeys::commandModifier, 'a'));
	}

	if (commandID == keyCmd::SnapToGrid)
	{
		result.setInfo (TRANS("Snap to Grid"),
		                TRANS("Snap objects to a grid based on connection angles"),
		                "View", 0);
		result.setTicked(cnv->snapToGrid);
	}
    
    if (commandID >= 50)
    {
        
        result.setInfo (TRANS(names[commandID - 50]),
                        TRANS("Create a new box"),
                        "Edit", 0);
        result.setActive(true);
    }
    
    //if(cnv->codeEditor) result.setActive(false);
    
}


ApplicationCommandTarget* AppCommands::getNextCommandTarget()
{
    return nullptr;
}

bool AppCommands::perform (const InvocationInfo& info)
{

    // let the code editor handle its own key commands
    if(cnv->codeEditor) {
        return false;
    }
	if (info.commandID == StandardApplicationCommandIDs::quit)
	{
		//systemRequestedQuit();
		return true;
	}

	if (info.commandID == keyCmd::New)
	{
		std::vector<int> positions = {100, 260, 360};

		if(main->askToSave())
		{
            main->sidebar.inspector->deselect();
			cnv->clearState();
			cnv->projectFile = File();
			main->setTitle("Untitled | Cerite");
			return true;
		}
	}

	if (info.commandID == keyCmd::Open)
	{
		if (main->askToSave() && main->openChooser.browseForFileToOpen())
		{
            main->sidebar.inspector->deselect();
            File openedfile = main->openChooser.getResult();
            if(openedfile.exists()) {
                cnv->projectFile = openedfile;
                
                main->setTitle(cnv->projectFile.getFileName().toStdString() + " | Cerite");
                cnv->clearState();

                try
                {
                    cnv->setState(cnv->projectFile.loadFileAsString());
                    cnv->addToHistory(cnv->projectFile);
                }
                catch (...)
                {
                    cnv->clearState();
                    std::cout << "Failed to open project" << std::endl;
                }
            }
		}

		return true;
	}

	if (info.commandID == keyCmd::Save)
	{
		if (cnv->projectFile.exists() && cnv->projectFile.getFileExtension().equalsIgnoreCase(".clab") && !cnv->projectFile.getFileName().equalsIgnoreCase("autosave.clab"))
		{
			cnv->changedSinceSave = false;
		}
		else if (main->saveChooser.browseForFileToSave(true))
		{
			cnv->projectFile = main->saveChooser.getResult();
			main->setTitle(cnv->projectFile.getFileName().toStdString() + " | Cerite");
			cnv->changedSinceSave = false;
		}
		else
			cnv->changedSinceSave = true;

		if (!cnv->changedSinceSave)
			cnv->projectFile.replaceWithText(Base64::toBase64(cnv->programState.toXmlString()));

		return true;
	}

	if (info.commandID == keyCmd::Cut)
	{
        main->sidebar.inspector->deselect();
		Array<Box*> boxarray = cnv->getSelectedBoxes();
		Array<LooseEdge*> letarray = cnv->getSelectedEdges();

		for(int i = 0; i < boxarray.size(); i++)
			cnv->boxNode.removeChild(boxarray[i]->state, &cnv->undoManager);

		for(int i = 0; i < letarray.size(); i++)
		{
			letarray[i]->clearConnections();
			cnv->edgeNode.removeChild(letarray[i]->state, &cnv->undoManager);
		}

		return true;
	}

	if (info.commandID == keyCmd::Copy)
	{
		copyTextToClipboard(cnv->getState(cnv->getSelectedBoxes(), cnv->getSelectedEdges()));
		return true;
	}

	if (info.commandID == keyCmd::Paste)
	{
		cnv->addState(getTextFromClipboard());
		return true;
	}

	if (info.commandID == keyCmd::Duplicate)
	{
		cnv->addState(cnv->getState(cnv->getSelectedBoxes(), cnv->getSelectedEdges()));
		return true;
	}

	if (info.commandID == keyCmd::Undo)
	{
		cnv->undoManager.undo();
		cnv->updateSystemState();
		cnv->updateUndoState();
		return true;
	}

	if (info.commandID == keyCmd::Redo)
	{
		cnv->undoManager.redo();
		cnv->updateSystemState();
		cnv->updateUndoState();
		return true;
	}

	if (info.commandID == keyCmd::ZoomIn)
	{
		zoom = zoom.scaled(1.15);

		// reset to 1 if it's close to 1: this prevents ugly lines
		if(std::sqrt (std::abs (zoom.getDeterminant())) > 0.95 && std::sqrt (std::abs (zoom.getDeterminant())) < 1.05)
			zoom = zoom.scale(1);

		cnv->setTransform(zoom);
		return true;
	}

	if (info.commandID == keyCmd::ZoomOut)
	{
		zoom = zoom.scaled(1 / 1.15);

		if(std::sqrt (std::abs (zoom.getDeterminant())) > 0.95 && std::sqrt (std::abs (zoom.getDeterminant())) < 1.05)
			zoom = zoom.scale(1);

		cnv->setTransform(zoom);
		return true;
	}

	if (info.commandID == keyCmd::NewBox)
	{
		cnv->addBox(String(), cnv->getMousePosition());
		return true;
	}

	if (info.commandID == keyCmd::newEdge)
	{
		cnv->addEdge(cnv->getMousePosition());
		return true;
	}

	if (info.commandID == keyCmd::RemoveSelection)
	{
		Array<Box*> boxarray = cnv->getSelectedBoxes();
		Array<LooseEdge*> letArray = cnv->getSelectedEdges();

		for(int i = boxarray.size() - 1; i >= 0 ; i--) cnv->removeBox(boxarray[i]);

		for(int i = letArray.size() - 1; i >= 0 ; i--) cnv->removeEdge(letArray[i]);

		Array<Connection*> connArray = cnv->getSelectedConns();

		for(int i = connArray.size() - 1; i >= 0 ; i--) cnv->removeConnection(connArray[i]);

		return true;
	}

	if (info.commandID == keyCmd::SelectAll)
	{
		for(auto element : cnv->boxmanager->objects) cnv->setSelected(static_cast<TextButton*>(element), true);

		for(auto element : cnv->edgemanager->objects) cnv->setSelected(static_cast<TextButton*>(element), true);

		return true;
	}

	if (info.commandID == keyCmd::SnapToGrid)
	{
		
		cnv->snapToGrid = !cnv->snapToGrid;
		main->commandManager.commandStatusChanged();
		main->topmenu.menuItemsChanged();
		return true;
	}

    if (info.commandID == keyCmd::NewNBX)
    {
        cnv->addBox("nbx", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewHSL)
    {
        cnv->addBox("hsl", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewVSL)
    {
        cnv->addBox("vsl", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewMSG)
    {
        cnv->addBox("msg", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewTGL)
    {
        cnv->addBox("tgl", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewBNG)
    {
        cnv->addBox("bng", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewHRAD)
    {
        cnv->addBox("hradio", cnv->getMousePosition());
        return true;
    }
    if (info.commandID == keyCmd::NewVRAD)
    {
        cnv->addBox("vradio", cnv->getMousePosition());
        return true;
    }
    
	return false;
}
