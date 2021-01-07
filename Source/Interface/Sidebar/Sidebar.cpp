#include "Sidebar.h"
#include "EngineSettings.h"
#include "AudioSettings.h"
#include "Console.h"
#include "Inspector/Inspector.h"
#include "../../MainComponent.h"

Sidebar::Sidebar(Component* main) : tabs(TabbedButtonBar::Orientation::TabsAtLeft), hidebar(TabbedButtonBar::Orientation::TabsAtLeft)
{
	tabbar.addAndMakeVisible(tabs);
	addAndMakeVisible(&tabbar);
	tabbar.setSize(30, getParentHeight() - 40);
	setSize(300, getParentHeight() - 40);
	setWantsKeyboardFocus (true);
	tabbar.addMouseListener(this, true);
	tabs.setLookAndFeel(&clook);
	hidebar.setLookAndFeel(&clook);
	content.add(std::make_unique<EngineSettings>());
	AudioSettings* audiosettings = static_cast<AudioSettings*>(content.add(std::make_unique<AudioSettings>()));
	content.add(std::make_unique<Console>());
    console = static_cast<Console*>(content.getLast());
	inspector = static_cast<Inspector*>(content.add(std::make_unique<Inspector>(&static_cast<MainComponent*>(main)->canvas)));
	audiosettings->init(static_cast<MainComponent*>(main));
	addAndMakeVisible(hidebar);
	addAndMakeVisible(title);
    
	for(int i = 0; i < content.size(); i++)
		addChildComponent(*content[i]);

	tabs.addTab("T", findColour(SidePanel::backgroundColour), 0);
	tabs.addTab("z", findColour(SidePanel::backgroundColour), 0);
	tabs.addTab("8", findColour(SidePanel::backgroundColour), 0);
	tabs.addTab("m", findColour(SidePanel::backgroundColour), 0);
	hidebar.addTab("b", findColour(SidePanel::backgroundColour), 0);
	hidebar.getTabButton(0)->setClickingTogglesState(true);
	tabs.setTabBackgroundColour(0, Colour::fromString("#FF333333"));
	tabs.setTabBackgroundColour(1, Colour::fromString("#FF333333"));
	tabs.setTabBackgroundColour(2, Colour::fromString("#FF333333"));
	tabs.setTabBackgroundColour(3, Colour::fromString("#FF333333"));
	hidebar.setTabBackgroundColour(0, Colour::fromString("#FF333333"));
    
    
    for(int i = 0; i < tabs.getNumTabs(); i++) {
        
        tabs.getTabButton(i)->onClick = [this, i]() mutable {
            if(selectedTab != -1) {
                (*content[selectedTab]).setVisible(false);
                selectedTab = i;
                (*content[selectedTab]).setVisible(true);
                title.setText(titles[selectedTab], sendNotification);
            }
        };
        
    }
    
    hidebar.getTabButton(0)->onClick = [this]() {
        
        if (hidebar.getTabButton(0)->getToggleState() == 0 && selectedTab != -1)
        {
            (*content[selectedTab]).setVisible(false);
            setBounds (getParentComponent()->getBounds().removeFromRight(30));
            selectedTab = -1;
            hidebar.getTabButton(0)->setButtonText("a");
            getParentComponent()->resized();
        };

        if (hidebar.getTabButton(0)->getToggleState() != 0 && selectedTab == -1)
        {
            selectedTab = tabs.getCurrentTabIndex();
            (*content[selectedTab]).setVisible(true);
            setBounds (getParentComponent()->getBounds().removeFromRight(300));
            hidebar.getTabButton(0)->setButtonText("b");
            getParentComponent()->resized();
        }
        
        resized();
    };
    
    tabs.getTabButton(2)->triggerClick();
    
}

Sidebar::~Sidebar()
{
	tabs.setLookAndFeel(nullptr);
	hidebar.setLookAndFeel(nullptr);
	removeChildComponent(&tabbar);
	removeChildComponent(&hidebar);
}


void Sidebar::paint(Graphics & g)
{
	float alpha = 0.9;
	auto background = Colour(41, 41, 41).withAlpha(alpha);
	g.fillAll(findColour(SidePanel::backgroundColour));
	g.setGradientFill ({ background, 0.0f, 0.0f,
	                     background.darker (0.2f),
	                     0.0f,
	                     getHeight() - 1.0f,
	                     false });
	g.fillRect(0, 0, getWidth(), 40);
	g.setColour(findColour(Toolbar::backgroundColourId));
	g.fillRect(tabbar.getBounds());
	Rectangle<int> outline = Rectangle<int>(tabbar.getWidth() - 1, 0, 1, getHeight());
	g.setColour (Colour (0x80000000));
	g.fillRect (outline);
}

void Sidebar::resized()
{
    
	tabs.setBounds(0, getHeight() / 2 - 60, 30, tabs.getNumTabs() * 40);
	tabbar.setBounds(0, 0, 30, getHeight());
	title.setBounds(40, 5, getWidth() - 80, 30);
	hidebar.setBounds(0, 40, 30, 50);

	for(int i = 0; i < content.size(); i++)
	{
		content[i]->setBounds(40 - ((i > 1) * 10), 40, getWidth() - 40 + ((i > 1) * 10), getHeight() - 40);
		content[i]->resized();
	}
    
}


void  Sidebar::mouseDrag (const MouseEvent& e)
{
	MainComponent* editor = findParentComponentOfClass<MainComponent>();

	if(selectedTab != -1)
	{
		myDragger.dragComponent (&tabbar, e, nullptr);
		tabbar.setTopLeftPosition(tabbar.getX(), 0);
	}

	setBounds (editor->getLocalBounds().removeFromRight (getWidth() - tabbar.getX()));

	if(getWidth() < 200 && selectedTab != -1) setSize(200, getHeight());

	if(getWidth() > editor->getWidth() / 2. && selectedTab != -1) setSize(editor->getWidth() / 2., getHeight());

	editor->resized();
}

void  Sidebar::mouseDown (const MouseEvent& e)
{
	myDragger.startDraggingComponent (&tabbar, e);
    repaint();
}

int Sidebar::createDialog (DialogBase* comp, Component* parent) {
    
    Rectangle<int> componentBounds = getLocalArea(parent, parent->getLocalBounds());
    int posY = componentBounds.getBottom();
    
    DocumentWindow* mainwindow = findParentComponentOfClass<DocumentWindow>();
    
    std::unique_ptr<SidebarDialog> sdialog = std::make_unique<SidebarDialog>(mainwindow, this, comp, Rectangle<int>(0, posY, getWidth() - 30, 140), componentBounds);
    
    sdialog->setBounds(getScreenBounds().withTrimmedLeft(30));
    int action = sdialog->runModalLoop(); // appearantly this is a bad method to use but it does what i want...
    sdialog->setVisible(false);
    
    return action;
    
}
