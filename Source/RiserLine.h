/*
  ==============================================================================

    RiserLine.h
    Created: 17 Dec 2023 5:07:38pm
    Author:  Zi Meng

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class RiserLine
{
public:
    RiserLine(float delayTime, float riserLength);
    ~RiserLine();
    
    // set up the parameters
    void prepare(float delayTime, float riserLength, float accelerateCap, float feedbak, double tempo, double sampleRate);
    
    // take in a new sample and return a delayed sample
    float getNextSample(float inputSample, float currentDelayTime, float currentRiserLength, float currentFeedback, float currentAccelerateCap, double currentTempo);
    
    // convert delayTime indices ('1' - '5' corresponding to 1/32, 1/16, 1/8, 1/4, 1/2 notes) to delaybuffer size in samples
    void setDelayBufferSize(float newDelayTime);
    
    // convert riserLength indices ('3' - '7' corresponding to 1/8, 1/4, 1/2, 1, 2 notes) to riserbuffer size in samples
    void setRiserBufferSize(float newRiserLength);
    
private:
    
    juce::AudioBuffer<float> delayBuffer; // the delay line
    
//     delaybBuffer reads from the 2 riserbuffers alternatively (delayBuffer reads from one riserBuffer while the input sample from processBlock() is written into the other).
//    Since both riserbuffers are of same length,
//    when one is finished writing the other will be finished reading then the roles switch.
    juce::AudioBuffer<float> riserBuffer1;
    juce::AudioBuffer<float> riserBuffer2;
    
    int sampleRate;
    float feedback;
    
    int delayBufferSize; // in samples
    int riserBufferSize; // in samples
    
//    the step delayBuffer uses to read out next delay sample and the step increases from 1 to accelerateCap 
//    as delayBuffer reads out delay samples.
    float accelerateBase = 1.0f;
    
//     the largest step delayBuffer uses to read out next delay sample
//    ( '2' means reading out at twice the speed of the original signal which is also one octave higher)
    float accelerateCap = 2.0f;
    
    bool riserSwitch = false; //switch between riserBuffer1 and riserBuffer2

    double dlyWritePtr = 0; // the pointer where input signal(from the riserBuffers) is written into delayBuffer
    double dlyPlayPtr = 0; // the pointer where the output reads from delayBuffer
    int riserWritePtr1 = 0; // the pointer where input signal(from the processBlock()) is written into riserWritePtr1
    int riserPlayPtr1 = 0; // the pointer where the delayBuffer reads from riserWritePtr1
    int riserWritePtr2 = 0; // the pointer where input signal(from the processBlock()) is written into riserWritePtr2
    int riserPlayPtr2 = 0; // the pointer where the delayBuffer reads from riserWritePtr2
    double tempo = 120; // the BPM from the host (default value 120)
};
