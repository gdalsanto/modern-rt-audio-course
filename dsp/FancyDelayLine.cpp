#include "FancyDelayLine.h"

#include <cmath>

namespace mrta
{

FancyDelayLine::FancyDelayLine(unsigned int maxLengthSamples, unsigned int numChannels)
{
    for (unsigned int ch = 0; ch < numChannels; ++ch)
        delayBuffer.emplace_back(maxLengthSamples, 0.f);
}

FancyDelayLine::~FancyDelayLine()
{
}

void FancyDelayLine::clear()
{
    for (auto& b : delayBuffer)
        std::fill(b.begin(), b.end(), 0.f);
}

void FancyDelayLine::prepare(unsigned int maxLengthSamples, unsigned int numChannels)
{
    delayBuffer.clear();
    for (unsigned int ch = 0; ch < numChannels; ++ch)
        delayBuffer.emplace_back(maxLengthSamples, 0.f);
}


void FancyDelayLine::setDelaySamples(unsigned int newDelaySamplesA, unsigned int newDelaySamplesB)
{
    delaySamplesA = std::max(std::min(newDelaySamplesA, static_cast<unsigned int>(delayBuffer[0].size() - 1u)), 1u);
    delaySamplesB = std::max(std::min(newDelaySamplesB, static_cast<unsigned int>(delayBuffer[0].size() - 1u)), 1u);

}


void FancyDelayLine::process(float* const* audioOutput, const float* const* audioInput, const float* const* modInput, unsigned int numChannels, unsigned int numSamples)
{
    const unsigned int delayBufferSize{ static_cast<unsigned int>(delayBuffer[0].size()) };

    numChannels = std::min(numChannels, static_cast<unsigned int>(delayBuffer.size()));
    // for (unsigned int ch = 0; ch < numChannels; ++ch)
    const int ch = 0;  // take first channel only, for now
        // Calculate base indices based on fixed delay time
    unsigned int workingWriteIndexA { writeIndexA };
    unsigned int workingReadIndexA { (workingWriteIndexA + delayBufferSize - delaySamplesA) % delayBufferSize };
    
    unsigned int workingWriteIndexB { writeIndexB };
    unsigned int workingReadIndexB { (workingWriteIndexB + delayBufferSize - delaySamplesB) % delayBufferSize };
    
    for (unsigned int n = 0; n < numSamples; ++n)
    {
        // Linear interpolation coefficients
        const float m { std::fmax(modInput[ch][n], 0.f) };
        const float mFloor { std::floor(m) };
        const float mFrac0 { m - mFloor };
        const float mFrac1 { 1.f - mFrac0 };

        // Calculate read indices A
        const unsigned int readIndexA0 { (workingReadIndexA + delayBufferSize - static_cast<unsigned int>(mFloor)) % delayBufferSize };
        const unsigned int readIndexA1 { (readIndexA0 + delayBufferSize - 1u) % delayBufferSize };

        // Read from delay line A
        const float readA0 = delayBuffer[ch][readIndexA0];
        const float readA1 = delayBuffer[ch][readIndexA1];
        
        // Calculate read indices B
        const unsigned int readIndexB0 { (workingReadIndexB + delayBufferSize - static_cast<unsigned int>(mFloor)) % delayBufferSize };
        const unsigned int readIndexB1 { (readIndexB0 + delayBufferSize - 1u) % delayBufferSize };
        
        // Read from delay line B
        const float readB0 = delayBuffer[ch][readIndexB0];
        const float readB1 = delayBuffer[ch][readIndexB1];

        // Read audio input
        const float x { audioInput[ch][n] };

        // Interpolate stereo output
        audioOutput[0][n] = readA0 * mFrac1 + readA1 * mFrac0;
        audioOutput[1][n] = readB0 * mFrac1 + readB1 * mFrac0;
        
        // Write input
        delayBuffer[ch][workingWriteIndexA] = x;

        // Increament indices A
        ++workingWriteIndexA; workingWriteIndexA %= delayBufferSize;
        ++workingReadIndexA; workingReadIndexA %= delayBufferSize;

        // Increament indices B
        ++workingWriteIndexB; workingWriteIndexB %= delayBufferSize;
        ++workingReadIndexB; workingReadIndexB %= delayBufferSize;
    }

    // Update persistent write index
    writeIndexA += numSamples; writeIndexA %= delayBufferSize;
    writeIndexB += numSamples; writeIndexB %= delayBufferSize;
}

void FancyDelayLine::process(float* audioOutput, const float* audioInput, const float* modInput, unsigned int numChannels)
{
    const unsigned int delayBufferSize{ static_cast<unsigned int>(delayBuffer[0].size()) };

    // Calculate base indices based on fixed delay time
    unsigned int workingWriteIndex { writeIndex };
    unsigned int workingReadIndex { (workingWriteIndex + delayBufferSize - delaySamples) % delayBufferSize };

    numChannels = std::min(numChannels, static_cast<unsigned int>(delayBuffer.size()));
    for (unsigned int ch = 0; ch < numChannels; ++ch)
    {
        // Linear interpolation coefficients
        const float m { std::fmax(modInput[ch], 0.f) };
        const float mFloor { std::floor(m) };
        const float mFrac0 { m - mFloor };
        const float mFrac1 { 1.f - mFrac0 };

        // Calculate read indeces
        const unsigned int readIndex0 { (workingReadIndex + delayBufferSize - static_cast<unsigned int>(mFloor)) % delayBufferSize };
        const unsigned int readIndex1 { (readIndex0 + delayBufferSize - 1u) % delayBufferSize };

        // Read from delay line
        const float read0 = delayBuffer[ch][readIndex0];
        const float read1 = delayBuffer[ch][readIndex1];

        // Read audio input
        const float x { audioInput[ch] };

        // Interpolate output
        audioOutput[ch] = read0 * mFrac1 + read1 * mFrac0;

        // Write input
        delayBuffer[ch][workingWriteIndex] = x;
    }

    // Update persistent write index
    ++writeIndex; writeIndex %= delayBufferSize;
}



}

