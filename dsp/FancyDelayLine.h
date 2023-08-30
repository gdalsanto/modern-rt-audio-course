#pragma once
#include <vector>

namespace mrta
{

class FancyDelayLine
{
public:
    FancyDelayLine(unsigned int maxLengthSamples, unsigned int numChannels);
    ~FancyDelayLine();
    
    // No default ctor
    FancyDelayLine() = delete;

    // No copy semantics
    FancyDelayLine(const FancyDelayLine&) = delete;
    const FancyDelayLine& operator=(const FancyDelayLine&) = delete;

    // No move semantics
    FancyDelayLine(FancyDelayLine&&) = delete;
    const FancyDelayLine& operator=(FancyDelayLine&&) = delete;
    
    // Clear the contents of the delay buffer (fill with 0s)
    void clear();
    
    // Reallocate delay buffer for the new channel count and clear its contents
    void prepare(unsigned int maxLengthSamples, unsigned int numChannels);
    
    // Set the current delay time in samples
    void setDelaySamples(unsigned int samples);
    
private:
    std::vector<std::vector<float>> delayBuffer;
    unsigned int delaySamples { 0 };
    unsigned int writeIndex0 { 0 };
    unsigned int writeIndex1 { 0 };
    
};
}
