#include "Inspector.h"
#include "../Sidebar.h"

Inspector::Inspector(Canvas* cnv)
{
    setOpaque (true);
    setWantsKeyboardFocus(true);
    canvas = cnv;
    
    cnv->programState.addListener(this);
    
    keys.resize(80);
    values.resize(80);
    
    addChildComponent(pcomp);
    
    CollapseablePanel* appearance = panels.add(new CollapseablePanel("Appearance"));
    CollapseablePanel* arguments = panels.add(new CollapseablePanel("Arguments"));
    CollapseablePanel* advparams = panels.add(new CollapseablePanel("Advanced Parameters"));
    
    arguments->filter = [this](ValueTree p) {
        return int(p.getProperty("Position")) > 0;
    };
    
    advparams->filter = [this](ValueTree p) {
        return int(p.getProperty("Position")) == 0;
    };

    parameterView.addAndMakeVisible(appearance);
    parameterView.addAndMakeVisible(arguments);
    parameterView.addAndMakeVisible(advparams);
    
    parameterViewport.setScrollBarsShown(true, false);
    parameterViewport.setViewedComponent(&parameterView, false);
    addChildComponent(parameterViewport);
}

void Inspector::paint (Graphics& g)
{
    g.fillAll (findColour(SidePanel::ColourIds::backgroundColour));
}


void Inspector::resized()
{
    pcomp.setBounds(0, 0, getWidth(), 80);
    
    parameterViewport.setBounds(getLocalBounds().withTrimmedTop(80));
    
    int totalheight = 0;
    for(auto panel : panels) {
        int increment = panel->getBestHeight();
        panel->setBounds(0, totalheight, getWidth(), increment);
        panel->resized();
        totalheight += increment;
    }
    
    parameterView.setBounds(0, 0, getWidth(), totalheight + 100);
}

void Inspector::deselect()
{
    Sidebar* sbar = findParentComponentOfClass<Sidebar>();
    if(sbar != nullptr && sbar->selectedTab == 3)
        sbar->title.setText("Inspector:   None", sendNotification);
    
    pcomp.setVisible(false);
    parameterViewport.setVisible(false);
    
}


void Inspector::select(ValueTree boxTree)
{
    if(!boxTree.isValid()) {
        deselect();
        return;
    }
    
    currentBox = boxTree;
    parameters = boxTree.getOrCreateChildWithName("Parameters", nullptr);
    Sidebar* sbar = findParentComponentOfClass<Sidebar>();
    
    panels[1]->addValueTree(parameters);
    panels[2]->addValueTree(parameters);
    
    panels[0]->clear();
    panels[0]->addItem(currentBox, "Orientation", ParameterType::tBool);
    
    if(sbar->selectedTab == 3)
        sbar->title.setText("Inspector:   " + boxTree.getProperty("Name").toString(), sendNotification);
     
    String name  = boxTree.getProperty("Name").toString();
    StringArray typeextract;
    typeextract.addTokens(name, " ", "");
    
    name = typeextract[0];
    
    pcomp.loadTree(name, boxTree);

    pcomp.setVisible(true);
    parameterViewport.setVisible(true);

    resized();
}


void Inspector::update() {
    
    for(auto& panel : panels)
        panel->update();
}


void Inspector::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)  {
    
    
    // Lock static parameters when the system is turned on
    if(property == Identifier("Power")) {
        for(auto panel : panels)
            panel->setPower(bool(treeWhosePropertyHasChanged.getProperty("Power")));
    }
    
};
