#pragma once

#include <JuceHeader.h>
#include "Looks.h"

class SaveDialog : public Component
{

	CeriteLookAndFeel clook;

	Label savelabel = Label("savelabel", "Save Changes?");

	TextButton cancel = TextButton("Cancel");
	TextButton dontsave = TextButton("Don't Save");
	TextButton save = TextButton("Save");

	void resized() override;

	void paint(Graphics & g) override;

public:

	SaveDialog();

	~SaveDialog();


};
