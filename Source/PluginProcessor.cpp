/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "RiserLine.h"

//==============================================================================
RiseUpAudioProcessor::RiseUpAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), delayTime(3.0f), riserLength(5.0f), feedback(0.3f), wetDryRatio(0.5f), apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    
    riserLine.reset(new RiserLine(delayTime, riserLength));

}

RiseUpAudioProcessor::~RiseUpAudioProcessor()
{
}

//==============================================================================
const juce::String RiseUpAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RiseUpAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RiseUpAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RiseUpAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RiseUpAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RiseUpAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RiseUpAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RiseUpAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RiseUpAudioProcessor::getProgramName (int index)
{
    return {};
}

void RiseUpAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RiseUpAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    riserLine->prepare(delayTime, riserLength, accelerateCap, feedback, hostBPM, getSampleRate());
}

void RiseUpAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RiseUpAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void RiseUpAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    delayTime = *apvts.getRawParameterValue("delayTime");
    feedback = *apvts.getRawParameterValue("feedback");
    wetDryRatio = *apvts.getRawParameterValue("wetDryRatio");
    riserLength = *apvts.getRawParameterValue("riserLength");
    accelerateCap = *apvts.getRawParameterValue("accelerateCap");
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm())
        hostBPM = bpmFromHost;
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        if (channel > 0) {
            buffer.copyFrom(channel, 0, buffer, 0, 0, buffer.getNumSamples());
            break;
        
        }

        auto channelData = buffer.getWritePointer(0);
        auto channelReader = buffer.getReadPointer(0);
        
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            
            float wetSample = riserLine->getNextSample(channelReader[i], delayTime, riserLength, feedback, accelerateCap, hostBPM);
            channelData[i] = wetSample * wetDryRatio + channelReader[i] * (1.0f - wetDryRatio);
            
        }

    }
}

//==============================================================================
bool RiseUpAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RiseUpAudioProcessor::createEditor()
{
    return new RiseUpAudioProcessorEditor (*this);
}

//==============================================================================
void RiseUpAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RiseUpAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout RiseUpAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Add delay time parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(delayTimeId, "Delay Time",
                                                           juce::NormalisableRange<float>(1.0, 5.0, 1.0),
                                                           3.0f));

    // Add feedback parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(feedbackId, "Feedback",
                                                           juce::NormalisableRange<float>(0.0, 1.0, 0.01),
                                                           0.3f));

    // Add wet/dry ratio parameter
    layout.add(std::make_unique<juce::AudioParameterFloat>(wetDryRatioId, "Wet/Dry",
                                                           juce::NormalisableRange<float>(0.0, 1.0, 0.01),
                                                           0.5f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(riserLengthId, "Riser Length",
                                                           juce::NormalisableRange<float>(3.0, 7.0, 1.0),
                                                           5.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(accelerateCapId, "Accelerate Cap",
                                                           juce::NormalisableRange<float>(1.1, 4.0, 0.1),
                                                           4.0f));

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RiseUpAudioProcessor();
}
