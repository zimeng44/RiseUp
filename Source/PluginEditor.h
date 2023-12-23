/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RiserLine.h"

//==============================================================================
/**
*/
class RiseUpAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    RiseUpAudioProcessorEditor (RiseUpAudioProcessor&);
    ~RiseUpAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RiseUpAudioProcessor& audioProcessor;
    
    juce::Slider delayTimeSlider;
    juce::Slider feedbackSlider;
    juce::Slider wetDrySlider;
    juce::Slider riserLengthSlider;
    juce::Slider accelerateCapSlider;
    
    juce::Label delayTimeLabel;
    juce::Label feedbackLabel;
    juce::Label wetDryLabel;
    juce::Label riserLengthLabel;
    juce::Label accelerateCapLabel;
    juce::Label riserNoteLabel;
    juce::Label noteLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wetDryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> riserLengthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accelerateCapAttachment;
    
    void setNoteWithLength(float riserLength);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiseUpAudioProcessorEditor)
};
