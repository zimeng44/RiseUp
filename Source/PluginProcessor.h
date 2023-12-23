/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RiserLine.h"

//==============================================================================
/**
*/
class RiseUpAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RiseUpAudioProcessor();
    ~RiseUpAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    //==============================================================================
    
    float getDelayTime() const { return delayTime; }
    void setDelayTime(float newDelayTime) { delayTime = newDelayTime; }
//    void setDelayBufferSize(float newDelayTime);

    float getFeedback() const { return feedback; }
    void setFeedback(float newFeedback) { feedback = newFeedback; }
    
    float getWetDryRatio() const { return wetDryRatio; }
    void setWetDryRatio(float newWetDryRatio) { wetDryRatio = newWetDryRatio; }
    
    float getRiserLength() const { return riserLength; }
    void setRiserLength(float newRiserLength) { riserLength = newRiserLength; }
//    void setRiserBufferSize(float newRiserLength);
    
    float getAccelerateCap() const { return accelerateCap; }
    void setAccelerateCap(float newAccelerateCap) { accelerateCap = newAccelerateCap; }
    
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    juce::ParameterID delayTimeId = juce::ParameterID("delayTime", 1);
    juce::ParameterID feedbackId = juce::ParameterID("feedback", 1);
    juce::ParameterID wetDryRatioId = juce::ParameterID("wetDryRatio", 1);
    juce::ParameterID riserLengthId = juce::ParameterID("riserLength", 1);
    juce::ParameterID accelerateCapId = juce::ParameterID("accelerateCap", 1);

private:
    float delayTime; // mapped to notes with setDelayBufferSize();
    float riserLength; // mapped to notes with setRiserBufferSize();
    float feedback; 
    float accelerateCap = 0.5f;
    float wetDryRatio;
    double hostBPM = 120;

    std::unique_ptr<RiserLine> riserLine;
    
    juce::AudioProcessorValueTreeState apvts;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiseUpAudioProcessor)
};

//==============================================================================
