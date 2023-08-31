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

void FancyDelayLine::process(float* audioOutput, const float* audioInput, const float* modInput, unsigned int numChannels)
{
    const unsigned int delayBufferSize{ static_cast<unsigned int>(delayBuffer[0].size()) };

    // Calculate base indices based on fixed delay time
    unsigned int workingWriteIndexA { writeIndexA };
    unsigned int workingReadIndexA { (workingWriteIndexA + delayBufferSize - delaySamplesA) % delayBufferSize };

    // unsigned int workingWriteIndexB { writeIndexB };
    unsigned int workingReadIndexB { (workingWriteIndexA + delayBufferSize - delaySamplesB) % delayBufferSize };

    numChannels = std::min(numChannels, static_cast<unsigned int>(delayBuffer.size()));
//    for (unsigned int ch = 0; ch < numChannels; ++ch)
//    {
//    }
    // let's work on the first input channel only
    const int ch = 0;
    // Linear interpolation coefficients
    const float mA { std::fmax(modInput[0], 0.f) };
    const float mFloorA { std::floor(mA) };
    const float mFrac0A { mA - mFloorA };
    const float mFrac1A { 1.f - mFrac0A };

    const float mB { std::fmax(modInput[1], 0.f) };
    const float mFloorB { std::floor(mB) };
    const float mFrac0B { mB - mFloorB };
    const float mFrac1B { 1.f - mFrac0B };

    // Calculate read indeces A
    const unsigned int readIndexA0 { (workingReadIndexA + delayBufferSize - static_cast<unsigned int>(mFloorA)) % delayBufferSize };
    const unsigned int readIndexA1 { (readIndexA0 + delayBufferSize - 1u) % delayBufferSize };
    
    // Calculate read indeces B
    const unsigned int readIndexB0 { (workingReadIndexB + delayBufferSize - static_cast<unsigned int>(mFloorB)) % delayBufferSize };
    const unsigned int readIndexB1 { (readIndexB0 + delayBufferSize - 1u) % delayBufferSize };
    
    // Read from delay line A
    const float readA0 = delayBuffer[ch][readIndexA0];
    const float readA1 = delayBuffer[ch][readIndexA1];

    // Read from delay line B
    const float readB0 = delayBuffer[ch][readIndexB0];
    const float readB1 = delayBuffer[ch][readIndexB1];
    
    // Read audio input
    const float x { audioInput[ch] };

    // Interpolate output
    audioOutput[0] = readA0 * mFrac1A + readA1 * mFrac0A;
    audioOutput[1] = readB0 * mFrac1B + readB1 * mFrac0B;

    // Write input
    delayBuffer[ch][workingWriteIndexA] = x;

    // Update persistent write index
    ++writeIndexA; writeIndexA %= delayBufferSize;
}

}
