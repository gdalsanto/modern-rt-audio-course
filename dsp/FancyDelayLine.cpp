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


void FancyDelayLine::setDelaySamples(unsigned int newDelaySamples)
{
    delaySamples = std::max(std::min(newDelaySamples, static_cast<unsigned int>(delayBuffer[0].size() - 1u)), 1u);
}
}
