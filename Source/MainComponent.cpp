#include <iostream>
#include <unistd.h>

#include "MainComponent.h"
#include "Interface/Sidebar/EngineSettings.h"
#include "Interface/Sidebar/Console.h"
#include "Utility/FSManager.h"
#include "Interface/SaveDialog.h"


//==============================================================================

MainComponent::MainComponent() : canvas(this), sidebar(this), topmenu(&canvas, this), statusbar(this), appcmds(&canvas, this), toolbar(this)
{
    setSize (1200, 720);
    setLookAndFeel(&clook);
    addAndMakeVisible(statusbar);
    updateMidiInputs();
    viewport.setViewedComponent(&canvas, false);
    addAndMakeVisible(viewport);
    addAndMakeVisible(toolbar);
    addAndMakeVisible(sidebar);
    addAndMakeVisible(topmenu);
    
    resized();
    ((Console*)sidebar.content[2])->clear();
    registerAllCommandsForTarget(&appcmds);
    setFirstCommandTarget(&appcmds);
    
    
    //pd.reset(new CeriteAudioProcessor);
    setAudioChannels (2, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    /*
    toolbar.setLookAndFeel(nullptr);
    sidebar.setLookAndFeel(nullptr);
    canvas.setLookAndFeel(nullptr); */
    setLookAndFeel(nullptr);
    /*
    viewport.removeChildComponent(&canvas);
    removeChildComponent(&statusbar);
    removeChildComponent(&sidebar);
    removeChildComponent(&canvas);
    removeChildComponent(&viewport);
    removeChildComponent(&toolbar); */
}


void MainComponent::updateSystem()
{
    if(bool(canvas.programState.getProperty("Power"))/*msystem != nullptr*/)
    {
        
        Cerite::Patch* patch = canvas.createPatch().compile<Cerite::Patch>([](const char* str) {
            logMessage(str);
        });
        processor.reset(patch);
        processor->init(1./44100.);
        
        for(auto& ext : external)
            ext->init(processor.get());
        
        output = processor->getVariablePtr("out");
        
        audioLock->exit();
        //startbutton.setColour(TextButton::textColourOnId, Colours::green);
    }
}
//==============================================================================
void MainComponent::startAudio (double sampleRate, std::vector<double> settings)
{
    audioLock->enter();
    updateMidiInputs();

    oversample = settings[5];
    osfactor = pow(2, oversample);
    
    //pd->prepareToPlay(sampleRate, samplesPerBlock, oversample);
    
    oversampler.reset(new dsp::Oversampling<float>(2, oversample, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    
    oversampler->initProcessing(samplesPerBlock);
    
    bypass = false;
    Cerite::Patch* patch = canvas.createPatch().compile<Cerite::Patch>([](const char* str) {
        logMessage(str);
    });
    processor.reset(patch);
    processor->init(1./44100.);
    
    for(auto& ext : external)
        ext->init(processor.get());
    
    output = processor->getVariablePtr("out");
    
    audioLock->exit();
    canvas.programState.setProperty("Power", true, nullptr);
}

void MainComponent::stopAudio()
{
    audioLock->enter();
    canvas.programState.setProperty("Power", false, nullptr);
    processor = nullptr;
    bypass = true;
    //if(pd.get()) pd->setBypass(true);
    audioLock->exit();
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    
    MidiBuffer mbuf;
    if(!processor)  {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    

    for(int i = 0; i < bufferToFill.numSamples; i++)
    {
        //*vinp = bufferToFill.buffer->getSample(0, i);
        for(auto& ext : external)
            ext->tick();
        
        processor->tick();
        
        //double output = processor->getVariable<double>("out");
        //processor->setVariable<double>("out", 0);
        
        //std::cout << processor->getVariable<double>("phase_sin1") << std::endl;
        bufferToFill.buffer->setSample(0, i, *output * volume);
        bufferToFill.buffer->setSample(1, i, *output * volume);
        *output = 0;

    }
    
    
    //pd->processBlock(buf, mbuf);
    //buf.applyGain(volume);
    
    //oversampler->processSamplesDown(block);
}


void MainComponent::releaseResources()
{
    audioLock->enter();
    
    //if(net != nullptr)
    //{
    //    msystem = nullptr;
    //    net = nullptr;
        
   // }
    //processor->clear();
    audioLock->exit();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    if (somethingIsBeingDraggedOver)
    {
        g.setColour(findColour(ScrollBar::ColourIds::thumbColourId));
        g.drawRect(viewport.getBounds(), 3);
    }
}

void MainComponent::resized()
{
    sidebar.setBounds (getLocalBounds().removeFromRight (sidebar.getWidth()));
    viewport.setBounds (0, 40, getWidth() - sidebar.getWidth(), getHeight() - 70);
    statusbar.setBounds(0, getHeight() - 30, getWidth() - sidebar.getWidth(), 30);
    // Only make space for a topmenu on windows and linux
#if !JUCE_MAC
    toolbar.setBounds(0, 20, sidebar.getX(), 40);
    topmenu.setBounds(0, 0, getWidth(), 20);
#else
    toolbar.setBounds(0, 0, sidebar.getX(), 40);
#endif

}

bool MainComponent::saveAudioFile()
{
    ExportDialog exporter(this);
    
    return true;
};


void MainComponent::updateMidiInputs()
{
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    
    for (auto input : midiInputs)
    {
        if (deviceManager.isMidiInputDeviceEnabled (input.identifier))
            deviceManager.addMidiInputDeviceCallback (input.identifier, this);
        else
            deviceManager.removeMidiInputDeviceCallback(input.identifier, this);
    }
}

void MainComponent::setTitle(std::string newTitle)
{
    DocumentWindow* window = findParentComponentOfClass<DocumentWindow>();
    
    if(window != nullptr)
        window->setName(newTitle);
}

void MainComponent::fileDragEnter (const StringArray& files, int /*x*/, int /*y*/)
{
    somethingIsBeingDraggedOver = true;
    repaint();
}

void MainComponent::fileDragExit (const StringArray& /*files*/)
{
    somethingIsBeingDraggedOver = false;
    repaint();
}

void MainComponent::filesDropped (const StringArray& files, int x, int y)
{
    File droppedfile(files[0]);
    
    if(droppedfile.getFileExtension().equalsIgnoreCase(".clab"))
    {
        //if (canvas.askToSave())
        //canvas.openProject(files[0].toStdString()); TODO: reimplement this
    }
    else if(droppedfile.getFileExtension().equalsIgnoreCase(".wav"))
    {
        canvas.addBox(String("input " + files[0].toStdString() + " 0.3"), Point<int>(x, y));
    }
    else
    {
        std::cout << "No action for this file type" << std::endl;
    }
    
    somethingIsBeingDraggedOver = false;
    repaint();
}

bool MainComponent::askToSave()
{
    if(canvas.boxNode.getNumChildren() > 0 && canvas.changedSinceSave)
    {
        std::unique_ptr<SaveDialog> sdialog = std::make_unique<SaveDialog>();
        sdialog->setBounds(viewport.getScreenX()  + (viewport.getWidth() / 2.) - 200., viewport.getScreenY(), 400, 130);
        int action = sdialog->runModalLoop(); // appearantly this is a bad method to use but it does what i want...
        sdialog->setVisible(false);
        
        if(action == 1)
        {
            appcmds.invokeDirectly(AppCommands::keyCmd::Save, true);
            return true;
        }
        
        if(action == 2)
            return true;
        
        return false;
    }
    else
        return true;
}

void MainComponent::saveCode() {
    
    //saveChooser.reset(new FileChooser("Export Audio", FSManager::home.getChildFile("Saves"), extensions[formatidx]));
    
   // if (saveChooser->browseForFileToSave(true))
   //     destination = saveChooser->getResult();
    //else return;
    
    std::unique_ptr<Cerite::Patch> patch;
    std::string ccode = Compiler::exportCode(canvas.createPatch());
    //return ccode;
}

void MainComponent::logMessage(const char* str) {
    // this is baadddd
    Canvas::mainCanvas->main->sidebar.console->logMessage(str);
    
}
