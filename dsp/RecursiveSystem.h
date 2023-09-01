#pragma once

#include "FancyDelayLine.h"
#include "Ramp.h"

namespace mrta
{

class RecursiveSystem
{
public:
    enum ModulationType : unsigned int
    {
        Sin = 0,
        Tri,
        Saw
    };

    RecursiveSystem(float maxTimeMs, unsigned int numChannels);
    ~RecursiveSystem() {};

    // No default ctor
    RecursiveSystem() = delete;

    // No copy semantics
    RecursiveSystem(const RecursiveSystem&) = delete;
    const RecursiveSystem& operator=(const RecursiveSystem&) = delete;

    // No move semantics
    RecursiveSystem(RecursiveSystem&&) = delete;
    const RecursiveSystem& operator=(RecursiveSystem&&) = delete;

    // Update sample rate, reallocates and clear internal buffers
    void prepare(double sampleRate, float maxTimeMs, unsigned int numChannels);

    // Clear contents of internal buffer
    void clear();

    // Process audio
    void process(float* const* output, const float* const* input, unsigned int numChannels, unsigned int numSamples);

    // Set delay offset in ms
    void setOffsetA(float newOffsetMs);

    // Set delay offset in ms
    void setOffsetB(float newOffsetMs);

    // Set modulation depth in ms
    void setDepth(float newDepthMs);

    // Set feedback to delay line, normalised [-1; 1]
    void setFeedbackGainA(float newFeedbackA);
    void setFeedbackGainB(float newFeedbackB);

    // Set delay time modsulation rate in Hz
    void setModulationRate(float newModRateHz);

    // Set delay time modulation waveform type
    void setModulationType(ModulationType newModType);
    
    void setDelaySamplesA(float time);
    void setDelaySamplesB(float time);
    
    // set delay cross feed
    void setDelayCrossFeed(float newCrossFeed);

private:
    double sampleRate { 48000.0 };

    mrta::FancyDelayLine delayLine;

    mrta::Ramp<float> offsetARamp;
    mrta::Ramp<float> offsetBRamp;
    mrta::Ramp<float> modDepthRamp;
    mrta::Ramp<float> feedbackRampA;
    mrta::Ramp<float> feedbackRampB;

    float phaseState[2] { 0.f, 0.f };
    float phaseInc { 0.f };

    float offsetAMs { 0.f };
    float offsetBMs { 0.f };
    float modDepthMs { 0.f };
    float feedbackA { 0.f };    // feedback from A to B
    float feedbackB { 0.f };    // feedback from B to A
    float modRate { 0.f };
    float delayCrossFeed { 0.f };

    ModulationType modType { Sin };

    float feedbackState[4] { 0.f, 0.f , 0.f, 0.f};

};

}
