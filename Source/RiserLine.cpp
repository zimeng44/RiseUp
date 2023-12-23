/*
  ==============================================================================

    RiserLine.cpp
    Created: 17 Dec 2023 5:07:49pm
    Author:  Zi Meng

  ==============================================================================
*/

#include "RiserLine.h"

RiserLine::RiserLine(float delayTime, float riserLength){
    setDelayBufferSize(delayTime);
    setRiserBufferSize(riserLength);
    
    delayBuffer = juce::AudioBuffer<float>(1,  delayBufferSize);
    riserBuffer1 = juce::AudioBuffer<float>(1,  riserBufferSize);
    riserBuffer2 = juce::AudioBuffer<float>(1,  riserBufferSize);
}

RiserLine::~RiserLine(){
    
}

void RiserLine::prepare(float delayTime, float riserLength, float newAccelerateCap, float newFeedback, double newTempo, double newSampleRate)
{
    setDelayBufferSize(delayTime);
    setRiserBufferSize(riserLength);
    accelerateCap = newAccelerateCap;
    feedback = newFeedback;
    tempo = newTempo;
    sampleRate = newSampleRate;
    
    dlyWritePtr = 0.0;
    riserWritePtr1 = 0;
    riserPlayPtr1 = 0;
    riserWritePtr2 = 0;
    riserPlayPtr2 = 0;
    
    dlyPlayPtr = dlyWritePtr  - delayBufferSize;
    while (dlyPlayPtr < 0) { dlyPlayPtr += delayBufferSize; }
    
    delayBuffer.setSize(1, delayBufferSize, true, true, false);
    delayBuffer.clear();
    riserBuffer1.setSize(1, riserBufferSize, true, true, false);
    riserBuffer1.clear();
    riserBuffer2.setSize(1, riserBufferSize, true, true, false);
    riserBuffer2.clear();
    
    riserSwitch = false;
    
    accelerateBase = 1.0f;
}

float RiserLine::getNextSample(float inputSample, float currentDelayTime, float currentRiserLength, float currentFeedback, float currentAccelerateCap, double currentTempo){
    
    feedback = currentFeedback;
    accelerateCap = currentAccelerateCap;
    tempo = currentTempo;
    
//    update the buffers and do fade-outs to the buffers to avoid clipping
    setDelayBufferSize(currentDelayTime);
    setRiserBufferSize(currentRiserLength);
    delayBuffer.setSize(1, delayBufferSize, true, true, false);
    delayBuffer.applyGainRamp(floor(delayBufferSize * 0.99), floor(delayBufferSize * 0.01), 1.0f, 0.0f);
    riserBuffer1.setSize(1, riserBufferSize, true, true, false);
    riserBuffer1.applyGainRamp(floor(riserBufferSize * 0.99), floor(riserBufferSize * 0.01), 1.0f, 0.0f);
    riserBuffer2.setSize(1, riserBufferSize, true, true, false);
    riserBuffer2.applyGainRamp(floor(riserBufferSize * 0.99), floor(riserBufferSize * 0.01), 1.0f, 0.0f);
    
//    initiate wetSample for output
    float wetSample = 0.0f;
    
//    when riserSwitched is true, delayBuffer read from riserBuffer1 while input sample from processBlock() being written into riserBuffer2
    if (riserSwitch) {
        
//         write the input sample into riserBuffer2
        riserBuffer2.setSample(0, riserWritePtr2++, inputSample);
        
//        circular delayBuffer needs to circulate the pointer
        while (dlyPlayPtr < 0)
            dlyPlayPtr += delayBufferSize-1;
        
//        circular delayBuffer needs to circulate the pointers
        if (dlyWritePtr >= delayBufferSize){
            dlyWritePtr = 0;
        }
        if (dlyPlayPtr >= delayBufferSize-1){ // 'dlyPlayPtr' only goes to 'delayBufferSize-1' due to interpolation
            dlyPlayPtr = 0;
        }
        
//        interpolate for the fractional delay value between integer samples
        double a = dlyPlayPtr - (double)floor(dlyPlayPtr);
        float interplolate = a * delayBuffer.getSample(0, floor(dlyPlayPtr)) + (1-a) * delayBuffer.getSample(0, floor(dlyPlayPtr)+1);
        
//        adding the current sample from riserBuffer1 and a delayed sample from the delayBuffer at the delay play pointer
//        the feedback is larger than 1 so the delayed samples will create a riser effect as they come back from the delayBuffer (the feedback rate is tested to avoid system overload and crash)
        float feedbackSample = riserBuffer1.getSample(0, riserPlayPtr1++)
                                                + 0.5 * interplolate * (1 + 0.01*feedback);
        
//        put the feedbackSample into the delayBuffer at the current delay write pointer
        delayBuffer.setSample(0, dlyWritePtr, feedbackSample);
        
//        read out the delayed sample from delayBuffer at the delay play pointer.
        wetSample = delayBuffer.getSample(0, dlyPlayPtr);
        
//        since the feedback rate is larger than 1 the output sample is hard clipped at 0.99 to avoid clipping
        if (wetSample > 0.99)
            wetSample = 0.99;
        
//        move the delay write pointer by 1 and move the delay play pointer by the increment 'accelerateBase'
        ++dlyWritePtr;
        dlyPlayPtr+= accelerateBase;
        
        // if riserBuffer2 is filled up and riserBuffer1 is read up, the two riserBuffers switch
        if (riserWritePtr2 >= riserBufferSize and riserPlayPtr1 >= riserBufferSize) {
//                    riserBuffer.reverse(0, 0, riserBufferSize);
            
//            Apply fade-in and fade-out to riserBuffer2
            riserBuffer2.applyGainRamp(0, riserBufferSize, 0.0f, 1.0f);
            riserBuffer2.applyGainRamp(floor(riserBufferSize * 0.9), floor(riserBufferSize * 0.1), 1.0f, 0.0f);
            
//            change the riserBuffer switch and reset the pointers
            riserSwitch = false;
            riserWritePtr2 = 0;
            dlyPlayPtr = dlyWritePtr - delayBufferSize;
            riserPlayPtr1 = 0;
            riserBuffer1.clear();
            accelerateBase = 1.0f;
        }
        
        // delayBuffer reader pointer increment also increases
        accelerateBase += ((accelerateCap - 1.0) / riserBufferSize);
        
        if (accelerateBase >= accelerateCap){
            accelerateBase = 1.0f;
        }
        
    }else{ // when riserSwitched is false, delayBuffer read from riserBuffer2 while input sample from processBlock() being written into riserBuffer1
        
        // write the input sample into riserBuffer1
        riserBuffer1.setSample(0, riserWritePtr1++, inputSample);
        
        while (dlyPlayPtr < 0)
            dlyPlayPtr += delayBufferSize-1;
        
        if (dlyWritePtr >= delayBufferSize){
            dlyWritePtr = 0;
        }
        if (dlyPlayPtr >= delayBufferSize-1){
            dlyPlayPtr = 0;
        }
        
        double a = dlyPlayPtr - (double)floor(dlyPlayPtr);
        float interplolate = a * delayBuffer.getSample(0, floor(dlyPlayPtr)) + (1-a) * delayBuffer.getSample(0, floor(dlyPlayPtr) + 1);
        
        float feedbackSample = riserBuffer2.getSample(0, riserPlayPtr2++) + interplolate * feedback;
        
        delayBuffer.setSample(0, dlyWritePtr, feedbackSample);
        
        wetSample = delayBuffer.getSample(0, dlyPlayPtr);
        
        if (wetSample > 0.99)
            wetSample = 0.99;
        
//        move both the delay write pointer and the delay play pointer by 1,
        ++dlyWritePtr;
//        ++dlyPlayPtr;
        dlyPlayPtr+= accelerateBase;
        
        // if riserBuffer1 is filled up, the two riserBuffers switch
        if (riserWritePtr1 >= riserBufferSize and riserPlayPtr2 >= riserBufferSize) {
//                    riserBuffer.reverse(0, 0, riserBufferSize);
            
//            Apply fade-in and fade-out to riserBuffer1
            riserBuffer1.applyGainRamp(0, riserBufferSize, 0.0f, 1.0f);
            riserBuffer1.applyGainRamp(floor(riserBufferSize * 0.9), floor(riserBufferSize * 0.1), 1.0f, 0.0f);
            
//            Change the riserBuffer switch and reset the pointers
            riserSwitch = true;
            riserWritePtr1 = 0;
            accelerateBase = 1.0f;
            dlyPlayPtr = dlyWritePtr - delayBufferSize;
            riserPlayPtr2 = 0;
            riserBuffer2.clear();
            accelerateBase = 1.0f;
        }
        
        // delayBuffer reader pointer increment also increases
        accelerateBase += ((accelerateCap - 1.0) / riserBufferSize);
        
        if (accelerateBase >= accelerateCap){
            accelerateBase = 1.0f;
        }

    }
    
    return wetSample;
}

void RiserLine::setDelayBufferSize(float newDelayTime) {
    switch ((int)newDelayTime) {
        case 1:
            delayBufferSize = 60 / tempo /8 * sampleRate; // 1/32 note
            break;
        case 2:
            delayBufferSize =  60 / tempo /4 * sampleRate; // 1/16 note
            break;
        case 3:
            delayBufferSize =  60 / tempo /2 * sampleRate; // 1/8 note
            break;
        case 4:
            delayBufferSize =  60 / tempo * sampleRate; // 1/4 note
            break;
        case 5:
            delayBufferSize =  60 / tempo * 2 * sampleRate; // 1/2 note
            break;
        case 6:
            delayBufferSize =  60 / tempo * 4 * sampleRate; // 1 bar
            break;
        case 7:
            delayBufferSize =  60 / tempo * 8 * sampleRate; // 2 bard
            break;
            
        default:
            delayBufferSize = 60 / tempo * sampleRate;
            break;
    }
}

void RiserLine::setRiserBufferSize(float newRiserLength) {
    switch ((int)newRiserLength) {
        case 1:
            riserBufferSize = 60 / tempo /8 * sampleRate; // 1/32 note
            break;
        case 2:
            riserBufferSize =  60 / tempo /4 * sampleRate; // 1/16 note
            break;
        case 3:
            riserBufferSize =  60 / tempo /2 * sampleRate; // 1/8 note
            break;
        case 4:
            riserBufferSize =  60 / tempo * sampleRate; // 1/4 note
            break;
        case 5:
            riserBufferSize =  60 / tempo * 2 * sampleRate; // 1/2 note
            break;
        case 6:
            riserBufferSize =  60 / tempo * 4 * sampleRate; // 1 bar
            break;
        case 7:
            riserBufferSize =  60 / tempo * 8 * sampleRate; // 2 bard
            break;
            
        default:
            riserBufferSize = 60 / tempo * sampleRate;
            break;
    }
    
//    if riserBufferSize is changed the delayBuffer play pointer increment is reset to '1'
    if (riserBufferSize != riserBuffer1.getNumSamples()){
        accelerateBase = 1.0f;
        riserPlayPtr1 = 0;
        riserWritePtr1 = 0;
        riserPlayPtr2 = 0;
        riserWritePtr2 = 0;
        dlyWritePtr = 0;
    }
}
