/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "FancyDelayLine.h"
#include "RecursiveSystem.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Time_L { "time_l" };
        static const juce::String Time_R { "time_r" };
        static const juce::String Feedback_Gain_L { "feedback_gain_l" };
        static const juce::String Feedback_Gain_R { "feedback_gain_r" };

    }

    namespace Name
    {
        static const juce::String Enabled{ "Enabled" };
        static const juce::String Time_L{ "Time L" };
        static const juce::String Time_R{ "Time R" };
        static const juce::String Feedback_Gain_L{ "Feedback Gain L" };
        static const juce::String Feedback_Gain_R{ "Feedback Gain R" };

    }

    namespace Ranges
    {
        static constexpr float OffsetMin { 1.f };
        static constexpr float OffsetMax { 2000.f };
        static constexpr float OffsetInc { 0.1f };
        static constexpr float OffsetSkw { 0.5f };

        static constexpr float FeedbackMin { -1.f };
        static constexpr float FeedbackMax { 1.f };
        static constexpr float FeedbackInc { 0.1f };
        static constexpr float FeedbackSkw { 1.f };

        static constexpr float TimeMin{ 20.f };
        static constexpr float TimeMax{ 2000.f };
        static constexpr float TimeInc{ 1.f };
        static constexpr float TimeSkw{ 0.5f };

        static const juce::String EnabledOff { "Off" };
        static const juce::String EnabledOn { "On" };
    }

    namespace Units
    {
        static const juce::String ms { "ms" };
        static const juce::String hz { "Hz" };
        static const juce::String none { " " };
    }
}
//==============================================================================
/**
*/
class FDNAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FDNAudioProcessor();
    ~FDNAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    //bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    mrta::ParameterManager& getParameterManager() {return parameterManager;}
    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    static const unsigned int MaxDelaySizeSamples { 1 << 12 };
    static const unsigned int MaxChannels { 2 };
    static const unsigned int MaxProcessBlockSamples{ 32 };


private:
    mrta::ParameterManager parameterManager;
    //mrta::FancyDelayLine fancyDelayLine;
    mrta::RecursiveSystem recSys;
    mrta::Ramp<float> enableRamp;

    juce::AudioBuffer<float> fxBuffer;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FDNAudioProcessor)
};
