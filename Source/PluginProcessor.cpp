/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //sampleRate ²ÎÊý±íÊ¾ÒôÆµÑù±¾ÂÊ£¨Ã¿ÃëÑù±¾Êý£©¡£
    //samplesPerBlock ²ÎÊý±íÊ¾Ã¿¸öÒôÆµ¿éµÄÑù±¾Êý¡£
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    //specÊÇÒ»¸ö½á¹¹Ìå£¬°üº¬ÁËÅäÖÃÐÅÏ¢
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec);


    auto chainSettings = getChainSettings(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQuailty,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));



    *leftChain.get<ChainPosition::Peak>().coefficients=*peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    
 }

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    auto chainSettings = getChainSettings(apvts);

    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQuailty,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));



    *leftChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPosition::Peak>().coefficients = *peakCoefficients;

    juce::dsp::AudioBlock<float> block(buffer);


    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    //确保不会出现参数不存在而导致的程序崩溃的情况
    if (auto* param = apvts.getRawParameterValue("LowCut Freq"))
        settings.lowCutFreq = param->load();
    if (auto* param = apvts.getRawParameterValue("HighCut Freq"))
        settings.highCutFreq = param->load();
    if (auto* param = apvts.getRawParameterValue("PeakCut Freq"))
        settings.peakFreq = param->load();
    if (auto* param = apvts.getRawParameterValue("PeakCut Gain"))
        settings.peakGainInDecibels = param->load();
    if (auto* param = apvts.getRawParameterValue("Peak Quality"))
        settings.peakQuailty = param->load();
    if (auto* param = apvts.getRawParameterValue("LowCut Slope"))
        settings.lowCutSlope = param->load();
    if (auto* param = apvts.getRawParameterValue("HighCut Slope"))
        settings.highCutSlope = param->load();

    /*
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("PeakCut Gain")->load();
    settings.peakQuailty = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = apvts.getRawParameterValue("LowCut Slope")->load();
    settings.highCutSlope = apvts.getRawParameterValue("HighCut Slope")->load();
    */
    

    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout
SimpleEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>
                    ("LowCut Freq",
                    "LowCut Freq",
                    juce::NormalisableRange<float>(20.f, 2000.f, 1.f, 0.25f),
                    20.f
                    )
              );

    layout.add(std::make_unique<juce::AudioParameterFloat>
                    ("HighCut Freq",
                    "HighCut Freq",
                    juce::NormalisableRange<float>(20.f, 2000.f, 1.f, 0.25f) ,
                    2000.f
                    )
              );

    layout.add(std::make_unique<juce::AudioParameterFloat>
                    ("PeakCut Freq",
                    "PeakCut Freq",
                    juce::NormalisableRange<float>(20.f, 2000.f, 1.f, 0.25f),
                    750.f
                    )
              );

    layout.add(std::make_unique<juce::AudioParameterFloat>
                    ("PeakCut Gain",
                    "PeakCut Gain",
                    juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                    0.0f
                    )
              );

    layout.add(std::make_unique<juce::AudioParameterFloat>
                    ("Peak Quality",
                    "Peak Quality",
                    juce::NormalisableRange<float>(0.1, 10.f, 0.05f, 1.f),
                    1.0f
                    )
              );


    juce::StringArray stringArray;
    for (int i = 0; i < 4; i++)
    {
        juce::String str;
        str << (12 + i * 12);
        str << "db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    return layout;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
