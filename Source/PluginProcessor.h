/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct ChainSettings
{
    float peakFreq{ 0 },
        peakGainInDecibels{ 0 },
        peakQuailty{ 1.f };

    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    int lowCutSlope{ 0 }, highCutSlope{ 0 };

};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& aptvs);
//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

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
    static juce::AudioProcessorValueTreeState::ParameterLayout
        createParameterLayout();
    juce::AudioProcessorValueTreeState apvts
    {
        *this,nullptr,"Parameters",createParameterLayout()
    };
private:
    //ÓÃÓÚ¹ýÂË¸´ÔÓµÄÇ¶Ì×ÃüÃû¿Õ¼ä
    //IIR£º±íÊ¾ÎÞÏÞÂö³åÏìÓ¦ÂË²¨Æ÷£¨Infinite Impulse Response£©¡£IIRÂË²¨Æ÷ÊÇÍ¨¹ýµÝ¹é¼ÆËãÊä³öµÄÂË²¨Æ÷ÀàÐÍ£¬¾ßÓÐ½Ï¸ßµÄÐ§ÂÊºÍ½ÏµÍµÄ¼ÆËã¸´ÔÓ¶È¡£
    using Filter = juce::dsp::IIR::Filter<float>;
    //ProcessorChain£ºProcessorChain ÊÇÒ»¸öÄ£°åÀà£¬ÓÃÓÚ½«¶à¸ö´¦ÀíÆ÷£¨processors£©Á¬½ÓÔÚÒ»Æð£¬ÒÔË³Ðò·½Ê½´¦ÀíÐÅºÅ¡£Ã¿¸ö´¦ÀíÆ÷ÒÀ´Î´¦ÀíÊäÈëÐÅºÅ²¢½«Æä´«µÝ¸øÏÂÒ»¸ö´¦ÀíÆ÷¡£
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    MonoChain leftChain, rightChain;
    enum ChainPosition
    {
        LowCut,
        Peak,
        HighCut
    };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
