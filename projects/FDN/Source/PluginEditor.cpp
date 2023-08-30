/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FDNAudioProcessorEditor::FDNAudioProcessorEditor (FDNAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), genericParameterEditor(audioProcessor.getParameterManager())
{
    int height = static_cast<int>(audioProcessor.getParameterManager().getParameters().size())
        * genericParameterEditor.parameterWidgetHeight;
    setSize (400, 300);
    addAndMakeVisible(genericParameterEditor);
}

FDNAudioProcessorEditor::~FDNAudioProcessorEditor()
{
}

//==============================================================================
void FDNAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void FDNAudioProcessorEditor::resized()
{
    genericParameterEditor.setBounds(getLocalBounds());
}
