/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FXGraphAudioProcessor::FXGraphAudioProcessor()
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
    
    // handmake some data for now
    
    dataManager.reset(new DataManager());
    
    
    dataManager->startEditing();
    
    dataManager->inactiveInstance->nodes[0]->outputParams[0].streamIds[0] = 0;
    dataManager->inactiveInstance->nodes[0]->outputParams[0].streamIds[1] = 1;
    
    dataManager->addNode(2, NodeType::Gain, "Gain", {400, 100});
    
    dataManager->inactiveInstance->nodes[2]->inputParams[0].streamId = 0;
    
    dataManager->addNode(3, NodeType::Correlation, "Correlation", {100, 300});
    
    dataManager->inactiveInstance->nodes[3]->inputParams[0].streamId = 1;
    dataManager->inactiveInstance->nodes[3]->outputParams[0].streamIds[0] = 0;
    
    dataManager->inactiveInstance->nodes[2]->inputParams[1].streamId = 0;
    
    dataManager->inactiveInstance->nodes[2]->outputParams[0].streamIds[0] = 2;
    
    dataManager->inactiveInstance->nodes[1]->inputParams[0].streamId = 2;
    
    dataManager->inactiveInstance->prepare();
    
    dataManager->finishEditing();
    dataManager->realise();
    
}

FXGraphAudioProcessor::~FXGraphAudioProcessor()
{
}

//==============================================================================
const juce::String FXGraphAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FXGraphAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FXGraphAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FXGraphAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FXGraphAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FXGraphAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FXGraphAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FXGraphAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FXGraphAudioProcessor::getProgramName (int index)
{
    return {};
}

void FXGraphAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FXGraphAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    
    
    // Setting the size of all audio stream buffers
    // TODO: see if it is possible to only set the size of used audio stream buffers. this may not be the case, but hopefully for RAM reasons it is
    
    for (int i = 0; i < NUM_AUDIO_STREAMS; i++)
    {
        dataManager->activeInstance->audioStreams[i].buffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
        dataManager->activeInstance->audioStreams[i].buffer.clear(); // in case there is garbage
        
        dataManager->inactiveInstance->audioStreams[i].buffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
        dataManager->inactiveInstance->audioStreams[i].buffer.clear(); // in case there is garbage

    }
    
    // prepare the envelopes of the value streams:
    
    for (int i = 0; i < NUM_VALUE_STREAMS; i++)
    {
        dataManager->activeInstance->valueStreams[i].envelope.setBlockRate(sampleRate / samplesPerBlock);
        dataManager->inactiveInstance->valueStreams[i].envelope.setBlockRate(sampleRate / samplesPerBlock);
    }
}

void FXGraphAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FXGraphAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FXGraphAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    dataManager->realise();
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // processor for the output node
    dataManager->getInputNode()->mainInput = &buffer; // ewwwwww
    
    dataManager->activeInstance->evaluate();
    
    auto outputStreamId = dataManager->getOutputNode()->inputParams[0].streamId;
    
    if (outputStreamId == -1)
    {
        buffer.clear();
        return;
    }
    
    auto b = dataManager->activeInstance->audioStreams[outputStreamId].buffer;
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        buffer.copyFrom(channel, 0, b, channel, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool FXGraphAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FXGraphAudioProcessor::createEditor()
{
    return new FXGraphAudioProcessorEditor (*this, dataManager);
}

//==============================================================================
void FXGraphAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FXGraphAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FXGraphAudioProcessor();
}
