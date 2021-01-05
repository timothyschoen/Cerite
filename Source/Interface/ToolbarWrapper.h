#pragma once

#include <JuceHeader.h>
#include "Looks.h"
#include "Canvas/Canvas.h"

class MainComponent;
class ToolbarWrapper :  private ToolbarItemFactory, public Toolbar
{
public:

	ToolbarWrapper(MainComponent* main);

	~ToolbarWrapper()
	{
		setLookAndFeel(nullptr);
		getItemComponent(0)->setLookAndFeel(nullptr);
		getItemComponent(1)->setLookAndFeel(nullptr);
		getItemComponent(2)->setLookAndFeel(nullptr);
		getItemComponent(3)->setLookAndFeel(nullptr);
		getItemComponent(4)->setLookAndFeel(nullptr);
		getItemComponent(5)->setLookAndFeel(nullptr);
		getItemComponent(6)->setLookAndFeel(nullptr);
        getItemComponent(7)->setLookAndFeel(nullptr);
        getItemComponent(8)->setLookAndFeel(nullptr);
	};


    void setUndoState(bool setUndo, bool canUndo, bool setRedo, bool canRedo);
	void checkUndoState();

private:

	Canvas* cnv;

	CeriteLookAndFeel clook;


	std::vector<std::function<void(void)>> functions;

	Array<String> names = {"p", "o",  "O", "x", "6", "s", "j"};

	AudioAppComponent* manager;

	ToolbarItemComponent* createItem(int itemId) override;

	void getAllToolbarItemIds (Array< int > & ids) override;

	void getDefaultItemSet (Array< int > & ids) override;


};
