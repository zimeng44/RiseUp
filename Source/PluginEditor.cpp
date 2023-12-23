/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "RiserLine.h"

//==============================================================================
RiseUpAudioProcessorEditor::RiseUpAudioProcessorEditor (RiseUpAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setSize (400, 300);
    
    delayTimeSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    delayTimeSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 80, 20);
    delayTimeSlider.setNormalisableRange(juce::NormalisableRange<double>(1.0, 5.0, 1.0));
    delayTimeSlider.setValue (audioProcessor.getDelayTime());
    delayTimeSlider.addListener (this);
//    addAndMakeVisible (delayTimeSlider);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), "delayTime", delayTimeSlider);

    feedbackSlider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 80, 20);
    feedbackSlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    feedbackSlider.setValue (audioProcessor.getFeedback());
    feedbackSlider.addListener (this);
    addAndMakeVisible (feedbackSlider);
    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), "feedback", feedbackSlider);
    
    wetDrySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    wetDrySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    wetDrySlider.setNormalisableRange(juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    wetDrySlider.setValue(audioProcessor.getWetDryRatio());
    wetDrySlider.addListener(this);
    addAndMakeVisible(wetDrySlider);
    wetDryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), "wetDryRatio", wetDrySlider);
    
    riserLengthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    riserLengthSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 80, 20);
    riserLengthSlider.setNormalisableRange(juce::NormalisableRange<double>(3.0, 7.0, 1.0));
    riserLengthSlider.setValue(audioProcessor.getRiserLength());
    riserLengthSlider.addListener(this);
    addAndMakeVisible(riserLengthSlider);
    riserLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), "riserLength", riserLengthSlider);
    
    accelerateCapSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    accelerateCapSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
    accelerateCapSlider.setNormalisableRange(juce::NormalisableRange<double>(1.1, 4.0, 0.1));
    accelerateCapSlider.setValue(audioProcessor.getAccelerateCap());
    accelerateCapSlider.addListener(this);
    addAndMakeVisible(accelerateCapSlider);
    accelerateCapAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getAPVTS(), "accelerateCap", accelerateCapSlider);

//    addAndMakeVisible(delayTimeLabel);
    delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
//    delayTimeLabel.attachToComponent(&delayTimeSlider, false);

    addAndMakeVisible(feedbackLabel);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
//    feedbackLabel.attachToComponent(&feedbackSlider, false);

    addAndMakeVisible(wetDryLabel);
    wetDryLabel.setText("Wet/Dry", juce::dontSendNotification);
//    wetDryLabel.attachToComponent(&wetDrySlider, false);
    
    addAndMakeVisible(riserLengthLabel);
    riserLengthLabel.setText("Riser Length", juce::dontSendNotification);
//    riserLengthLabel.attachToComponent(&riserLengthSlider, false);
    
    addAndMakeVisible(accelerateCapLabel);
    accelerateCapLabel.setText("Accelerate Cap", juce::dontSendNotification);
//    accelerateCapLabel.attachToComponent(&accelerateCapSlider, false);
    
    addAndMakeVisible(riserNoteLabel);
    riserNoteLabel.setText("1/4", juce::dontSendNotification);
    
    addAndMakeVisible(noteLabel);
    noteLabel.setText("Note", juce::dontSendNotification);
    
}

RiseUpAudioProcessorEditor::~RiseUpAudioProcessorEditor()
{
}

//==============================================================================
void RiseUpAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
    
    juce::Image myImage = juce::ImageCache::getFromMemory(BinaryData::RiseUp_BG_png, BinaryData::RiseUp_BG_pngSize);
    
    g.drawImage(myImage, getLocalBounds().toFloat());

//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void RiseUpAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    int sliderWidth = 100;
    int sliderHeight = 100;
    int labelHeight = 20;
    
    delayTimeSlider.setBounds(20, 20, sliderWidth, sliderHeight);
//    delayTimeLabel.setBounds(25, 20 + sliderHeight, sliderWidth, labelHeight);

    feedbackSlider.setBounds(25, 190, sliderWidth, sliderHeight);
    feedbackLabel.setBounds(40, 175, sliderWidth, labelHeight);

    wetDrySlider.setBounds(285, 190, sliderWidth, sliderHeight);
    wetDryLabel.setBounds(305, 175, sliderWidth, labelHeight);
    
    riserLengthSlider.setBounds(155, 110, 85, 85);
    riserLengthLabel.setBounds(155, 90, 85, labelHeight);
    
    accelerateCapSlider.setBounds(285, 20, sliderWidth, sliderHeight);
    accelerateCapLabel.setBounds(285, 5, sliderWidth, labelHeight);
    
    riserNoteLabel.setBounds(riserLengthSlider.getX()+23, riserLengthSlider.getY() + 30, 40, 10);
    riserNoteLabel.setJustificationType(juce::Justification::centred);
    noteLabel.setBounds(riserLengthSlider.getX()+23, riserLengthSlider.getY() + 45, 40, 10);
    
    riserLengthSlider.toFront(true);
}

void RiseUpAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    // Handle slider value changes and update processor parameters
    if (slider == &delayTimeSlider)
    {
        audioProcessor.setDelayTime(static_cast<float>(delayTimeSlider.getValue()));
        riserLengthSlider.setValue(delayTimeSlider.getValue()+2.0);
        audioProcessor.setRiserLength(static_cast<float>(riserLengthSlider.getValue()));
    }
    else if (slider == &feedbackSlider)
    {
        audioProcessor.setFeedback(static_cast<float>(feedbackSlider.getValue()));
    }
    else if (slider == &wetDrySlider)
    {
        audioProcessor.setWetDryRatio(static_cast<float>(wetDrySlider.getValue()));
    }
    else if (slider == &riserLengthSlider)
    {
        audioProcessor.setRiserLength(static_cast<float>(riserLengthSlider.getValue()));
        delayTimeSlider.setValue(riserLengthSlider.getValue()-2.0);
        audioProcessor.setDelayTime(static_cast<float>(delayTimeSlider.getValue()));
        setNoteWithLength(static_cast<float>(riserLengthSlider.getValue()));
    }
    else if (slider == &accelerateCapSlider)
    {
        audioProcessor.setAccelerateCap(static_cast<float>(accelerateCapSlider.getValue()));
    }
    
}

void RiseUpAudioProcessorEditor::setNoteWithLength(float newRiserLength){
    switch ((int)newRiserLength) {
        case 1:
            riserNoteLabel.setText("1/32", juce::dontSendNotification); // 1/32 note
            break;
        case 2:
            riserNoteLabel.setText("1/16", juce::dontSendNotification); // 1/16 note
            break;
        case 3:
            riserNoteLabel.setText("1/8", juce::dontSendNotification); // 1/8 note
            break;
        case 4:
            riserNoteLabel.setText("1/4", juce::dontSendNotification); // 1/4 note
            break;
        case 5:
            riserNoteLabel.setText("1/2", juce::dontSendNotification); // 1/2 note
            break;
        case 6:
            riserNoteLabel.setText("1", juce::dontSendNotification);// 1 bar
            break;
        case 7:
            riserNoteLabel.setText("2", juce::dontSendNotification); // 2 bard
            break;
            
        default:
            riserNoteLabel.setText("1/4", juce::dontSendNotification);
            break;
    }
}
