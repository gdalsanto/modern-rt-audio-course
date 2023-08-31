/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static const std::vector<mrta::ParameterInfo> ParameterInfos
{
    { Param::ID::Enabled, Param::Name::Enabled, Param::Ranges::EnabledOff, Param::Ranges::EnabledOn, true},
    { Param::ID::Time_L,  Param::Name::Time_L, Param::Units::ms, 500.f, Param::Ranges::TimeMin, Param::Ranges::TimeMax, Param::Ranges::TimeInc, Param::Ranges::TimeSkw },
    { Param::ID::Time_R,  Param::Name::Time_R, Param::Units::ms, 1000.f, Param::Ranges::TimeMin, Param::Ranges::TimeMax, Param::Ranges::TimeInc, Param::Ranges::TimeSkw },
    { Param::ID::Offset_L,  Param::Name::Offset_L, Param::Units::ms, 1.f, Param::Ranges::OffsetMin, Param::Ranges::OffsetMax, Param::Ranges::OffsetInc, Param::Ranges::OffsetSkw },
    { Param::ID::Offset_R,  Param::Name::Offset_R, Param::Units::ms, 500.f, Param::Ranges::OffsetMin, Param::Ranges::OffsetMax, Param::Ranges::OffsetInc, Param::Ranges::OffsetSkw },
    { Param::ID::Feedback_Gain_R,  Param::Name::Feedback_Gain_L, Param::Units::Pct, 20.f, Param::Ranges::FeedbackMin, Param::Ranges::FeedbackMax, Param::Ranges::FeedbackInc, Param::Ranges::FeedbackSkw },
    { Param::ID::Feedback_Gain_L,  Param::Name::Feedback_Gain_R, Param::Units::Pct, 20.f, Param::Ranges::FeedbackMin, Param::Ranges::FeedbackMax, Param::Ranges::FeedbackInc, Param::Ranges::FeedbackSkw }
};
//==============================================================================

FDNAudioProcessor::FDNAudioProcessor() : parameterManager(*this, ProjectInfo::projectName, ParameterInfos), flanger(20.f,2), enableRamp(0.05f)
{
    parameterManager.registerParameterCallback(Param::ID::Enabled,
        [this](float newValue, bool force)
        {
            enableRamp.setTarget(std::fmin(std::fmax(newValue, 0.f), 1.f), force);
        });
    parameterManager.registerParameterCallback(Param::ID::Time_L,
        [this](float newValue, bool /*force*/)
        {
            flanger.setTime_L(newValue);
        });
    parameterManager.registerParameterCallback(Param::ID::Time_L,
        [this](float newValue, bool /*force*/)
        {
            flanger.setTime_R(newValue);
        });
    parameterManager.registerParameterCallback(Param::ID::Offset_L,
        [this](float newValue, bool /*force*/)
        {
            flanger.setOffset_L(newValue);
        });
    parameterManager.registerParameterCallback(Param::ID::Offset_R,
        [this](float newValue, bool /*force*/)
        {
            flanger.setOffset_R(newValue);
        });
    parameterManager.registerParameterCallback(Param::ID::Feedback_Gain_L,
        [this](float newValue, bool /*force*/)
        {
            flanger.setFeedbackGain_L(newValue);
        });
    parameterManager.registerParameterCallback(Param::ID::Feedback_Gain_R,
        [this](float newValue, bool /*force*/)
        {
            flanger.setFeedbackGain_R(newValue);
        });
}

FDNAudioProcessor::~FDNAudioProcessor()
{
}

//==============================================================================
const juce::String FDNAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FDNAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FDNAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FDNAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FDNAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FDNAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FDNAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FDNAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FDNAudioProcessor::getProgramName (int index)
{
    return {};
}

void FDNAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FDNAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    const unsigned int numChannels { static_cast<unsigned int>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };

    flanger.prepare(newSampleRate, 20.f, numChannels);
    enableRamp.prepare(newSampleRate);

    parameterManager.updateParameters(true);

    fxBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    fxBuffer.clear();
}

void FDNAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FDNAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FDNAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    parameterManager.updateParameters();

    const unsigned int numChannels { static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples { static_cast<unsigned int>(buffer.getNumSamples()) };

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        fxBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));

    flanger.process(fxBuffer.getArrayOfWritePointers(), fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);
    enableRamp.applyGain(fxBuffer.getArrayOfWritePointers(), numChannels, numSamples);

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        buffer.addFrom(ch, 0, fxBuffer, ch, 0, static_cast<int>(numSamples));
}

//==============================================================================
bool FDNAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FDNAudioProcessor::createEditor()
{
    return new FDNAudioProcessorEditor (*this);
}

//==============================================================================
void FDNAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FDNAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FDNAudioProcessor();
}