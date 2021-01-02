#include "ExportDialog.h"
#include "../MainComponent.h"

ExportComponent::ExportComponent(AudioAppComponent* m) : main(m), exportSettings(false), Thread("audiowriter") {
    setLookAndFeel(&clook);
    
    addAndMakeVisible(&formatSel);
    addAndMakeVisible(&exportSettings);
    addAndMakeVisible(&exportButton);
    addAndMakeVisible(&startPlaying);
    addAndMakeVisible(&samplerateSel);
    addAndMakeVisible(&lengthEditor);
    addChildComponent(&progressbar);
    addChildComponent(&donelabel);
    
    progressbar.setColour(ProgressBar::backgroundColourId, Colour(50, 50, 50));
    
    progressbar.setColour(ProgressBar::foregroundColourId, findColour(ScrollBar::thumbColourId));
    
    samplerateSel.addItemList(samplerates, 1);
    
    lengthEditor.setText("10.");
    
    samplerateSel.setSelectedItemIndex(1);
    lengthEditor.setInputRestrictions(10, ".0123456789");
    
    formatSel.addItemList({"WAV", "AIF", "FLAC", "MP3", "OGG"}, 1);
    
    startPlaying.setToggleState(true, dontSendNotification);
    
    exportButton.onClick = [this]() {
        main->releaseResources();
        
        exportAudio();
    };
    
    formatSel.setSelectedItemIndex(0);
}

void ExportComponent::paint(Graphics & g) {
    
    g.fillAll (findColour(SidePanel::ColourIds::backgroundColour));
    g.setColour(Colours::white);
    
    g.drawFittedText("Format", 20, 20, 80, 20, Justification::left, 1);
    g.drawFittedText("Start Playback", 20, 50, 80, 20, Justification::left, 1);
    g.drawFittedText("Sample Rate", 20, 80, 80, 20, Justification::left, 1);
    g.drawFittedText("Export length", 20, 110, 80, 20, Justification::left, 1);
    
}

void ExportComponent::exportAudio() {
    MainComponent* m = static_cast<MainComponent*>(main);
    
    settings = exportSettings.settings;
    samplerate = samplerates[samplerateSel.getSelectedItemIndex()].getIntValue();
        
    formatidx = formatSel.getSelectedItemIndex();
    
    saveChooser.reset(new FileChooser("Export Audio", FSManager::home.getChildFile("Saves"), extensions[formatidx]));
    
    if (saveChooser->browseForFileToSave(true))
        destination = saveChooser->getResult();
    else return;
    
    int oversample = settings[5];
    int sampsPerBlock = 2048 * pow(2, oversample);
    int numSamples =  int(lengthEditor.getText().getFloatValue() * samplerate);
    
    m->shutdownAudio();
    
    //Cerite::Patch* circuit = m->canvas.compile();
    /*
    oversampler.reset(new dsp::Oversampling<float>(2, oversample, dsp::Oversampling<float>::filterHalfBandFIREquiripple));
    
    oversampler->initProcessing(2048);
    
    m->pd->setupMNA(m->canvas.createNetlist(), settings,  samplerate);
    
    m->pd->prepareToPlay(samplerate, sampsPerBlock, settings[5]);
    
    if(startPlaying.getToggleState())
        m->pd->netlist->startPlaying();
    
    m->canvas.programState.setProperty("Power", true, nullptr);
    
    numBlocks = numSamples / 2048;
    progress = 0;
    formatidx = formatSel.getSelectedItemIndex();
    
    donelabel.setVisible(false);
    progressbar.setVisible(true);
    
    
    donelabel.setText("Exported as " + destination.getFileName(), dontSendNotification);
    
    startThread();
    */
}

void ExportComponent::resized() {
    

    formatSel.setBounds(110, 20, 80, 22);
    exportSettings.setBounds(getWidth() / 2., 10, getWidth() / 2. - 20, 250);
    exportButton.setBounds(getWidth() - 110, getHeight() - 60, 80, 22);
    
    samplerateSel.setBounds(110, 80, 80, 22);
    lengthEditor.setBounds(110, 110, 80, 22);
    
    progressbar.setBounds(50, getHeight() - 100, getWidth() - 100, 20);
    donelabel.setBounds(20, getHeight() - 60, 200, 16);
    startPlaying.setBounds(110, 50, 80, 22);
}


void ExportComponent::run() {
        
    int nSamps = 2048 * pow(2, settings[5]);
    AudioBuffer<float> buf = AudioBuffer<float>(2, numBlocks * 2048);
    
    for(int i = 0; i < numBlocks; i++) {
        MidiBuffer mbuf;
                
        dsp::AudioBlock<float> block (buf.getArrayOfWritePointers(), 2, i * 2048, 2048);
        dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);
        
        //AudioBuffer<float> tempbuf = AudioBuffer<float>((float*[]) { oversampledBlock.getChannelPointer(0), oversampledBlock.getChannelPointer(1) }, 2, static_cast<int> (oversampledBlock.getNumSamples()));
        
        //p->processBlock(tempbuf, mbuf);
        //progress = (1. / numBlocks) * i;
        
        //oversampler->processSamplesDown(block);
        
        if(halt) return;
    }
    
    progress = 1;
    
    buf.applyGain(0.5);
    
    std::unique_ptr<AudioFormatWriter> writer;
    writer.reset (formats[formatidx]->createWriterFor (new FileOutputStream (destination),
                                          samplerate,
                                          buf.getNumChannels(),
                                          24,
                                          {},
                                          0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (buf, 0, buf.getNumSamples());
    

    startTimer(500);
}


ExportDialog::ExportDialog(AudioAppComponent* m)
{
    
    //getLookAndFeel().getColour(ResizableWindow::backgroundColourId);
    
    content.setOwned (new ExportComponent(m));
    content->setSize (400, 250);
    
    dialogTitle                   = "Export Audio";
    dialogBackgroundColour        = content->getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    escapeKeyTriggersCloseButton  = true;
    useNativeTitleBar             = true;
    resizable                     = false;
    
    DialogWindow* window = launchAsync();
    window->setVisible (true);
    
};

ExportDialog::~ExportDialog() {
    
};
