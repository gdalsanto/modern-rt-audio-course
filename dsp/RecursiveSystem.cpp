#include "RecursiveSystem.h"

// Windows does not have Pi constants
#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#define D_SIZE 0.002
namespace mrta
{
RecursiveSystem::RecursiveSystem(float maxTimeMs, unsigned int numChannels) :
delayLine(static_cast<unsigned int>(std::ceil(std::fmax(maxTimeMs, 1.f) * sampleRate)), numChannels),
offsetRamp(0.05f),
modDepthRamp(0.05f),
feedbackRampA(0.05f),
feedbackRampB(0.05f)
{    
}

void RecursiveSystem::prepare(double newSampleRate, float maxTimeMs, unsigned int numChannels)
{
    sampleRate = newSampleRate;

    delayLine.prepare(static_cast<unsigned int>(std::round(maxTimeMs * static_cast<float>(D_SIZE * sampleRate))), numChannels);
    
    // set fixed delay time to max
    delayLine.setDelaySamplesA(static_cast<unsigned int>(std::ceil(D_SIZE * sampleRate)));
    delayLine.setDelaySamplesB(static_cast<unsigned int>(std::ceil(D_SIZE * sampleRate)));
    offsetRamp.prepare(sampleRate, true, offsetMs * static_cast<float>(D_SIZE * sampleRate));
    modDepthRamp.prepare(sampleRate, true, modDepthMs * static_cast<float>(D_SIZE * sampleRate));
    feedbackRampA.prepare(sampleRate, true, feedbackA);
    feedbackRampB.prepare(sampleRate, true, feedbackB);
    
    phaseState[0] = 0.f;
    phaseState[1] = static_cast<float>(M_PI / 2.0);
    phaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * modRate;

    feedbackStateA[0] = 0.f;
    feedbackStateA[1] = 0.f;
    feedbackStateB[0] = 0.f;
    feedbackStateB[1] = 0.f;
}

void RecursiveSystem::clear()
{
    // clear delay line
    delayLine.clear();
    // clear feedback states
    feedbackStateA[0] = 0.f;
    feedbackStateA[1] = 0.f;
    feedbackStateB[0] = 0.f;
    feedbackStateB[1] = 0.f;
}


void RecursiveSystem::process(float* const* output, const float* const* input, unsigned int numChannels, unsigned int numSamples)
{
    for (unsigned int n = 0; n < numSamples; ++n)
    {
        // Process LFO acording to mod type
        float lfo[2] { 0.f, 0.f };
        switch (modType)
        {
        case Saw:
            lfo[0] = phaseState[0] * static_cast<float>(0.5 / M_PI);
            lfo[1] = phaseState[1] * static_cast<float>(0.5 / M_PI);
            break;

        case Tri:
            lfo[0] = std::fabs((phaseState[0] - static_cast<float>(M_PI)) / static_cast<float>(M_PI));
            lfo[1] = std::fabs((phaseState[1] - static_cast<float>(M_PI)) / static_cast<float>(M_PI));
            break;

        case Sin:
            lfo[0] = 0.5f + 0.5f * std::sin(phaseState[0]);
            lfo[1] = 0.5f + 0.5f * std::sin(phaseState[1]);
            break;
        }

        // Increment and wrap phase states
        phaseState[0] = std::fmod(phaseState[0] + phaseInc, static_cast<float>(2 * M_PI));
        phaseState[1] = std::fmod(phaseState[1] + phaseInc, static_cast<float>(2 * M_PI));

        // Apply mod depth and offset ramps
        modDepthRamp.applyGain(lfo, numChannels);
        offsetRamp.applySum(lfo, numChannels);

        // Process feedback gain ramp
        feedbackRampA.applyGain(feedbackStateA, numChannels);
        feedbackRampB.applyGain(feedbackStateB, numChannels);
        
        // Read inputs
//        float x[2] { 0.f, 0.f };
//        for (unsigned int ch = 0; ch < numChannels; ++ch)
//            x[ch] = input[ch][n] + feedbackState[ch];
        // for now we use only the first state since
        float x[2] { 0.f, 0.f};
        x[0] = input[0][n] + feedbackStateB[0] + feedbackStateA[0];
        x[1] = x[0];

        // Process delay
        float feedbackState[2] { feedbackStateA[0], feedbackStateB[0] };
        delayLine.process(feedbackState, x, lfo, numChannels);

        // Write to output buffers
        output[0][n] = feedbackStateA[0];
        output[1][n] = feedbackStateB[0];
    }
}



void RecursiveSystem::setOffset(float newOffsetMs)
{
    // Since the fixed delay is set to 1ms
    // We can deduct that from the offset ramp
    offsetMs = std::fmax(newOffsetMs - 1.f, 0.f);
    offsetRamp.setTarget(offsetMs * static_cast<float>(D_SIZE * sampleRate));
}

void RecursiveSystem::setDepth(float newDepthMs)
{
    modDepthMs = std::fmax(newDepthMs, 0.f);
    modDepthRamp.setTarget(modDepthMs * static_cast<float>(D_SIZE * sampleRate));
}

void RecursiveSystem::setFeedbackGainA(float newFeedbackA)
{
    feedbackA = std::fmin(std::fmax(newFeedbackA, -1.f), 1.f);
    feedbackRampA.setTarget(feedbackA);
}

void RecursiveSystem::setFeedbackGainB(float newFeedbackB)
{
    feedbackB = std::fmin(std::fmax(newFeedbackB, -1.f), 1.f);
    feedbackRampB.setTarget(feedbackB);
}

void RecursiveSystem::setModulationRate(float newModRateHz)
{
    modRate = std::fmax(newModRateHz, 0.f);
    phaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * modRate;
}

void RecursiveSystem::setModulationType(ModulationType newModType)
{
    modType = newModType;
}
void RecursiveSystem::setDelaySamplesA(float time)
{
    int newDelaySamplesA = static_cast<int>(time/1000 * sampleRate);
    
    delayLine.setDelaySamplesA(newDelaySamplesA);
}
void RecursiveSystem::setDelaySamplesB(float time)
{
    int newDelaySamplesB = static_cast<int>(time/1000 * sampleRate);

    delayLine.setDelaySamplesB(newDelaySamplesB);
}

}

