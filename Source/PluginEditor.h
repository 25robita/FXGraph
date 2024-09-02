/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SideMenu.h"
#include "GraphNode.h"
#include "GraphAreaStreams.h"
#include "Common.h"
#include "GraphAreaNodeContainer.h"

//==============================================================================
/**
*/
class FXGraphAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FXGraphAudioProcessorEditor (FXGraphAudioProcessor&, std::shared_ptr<DataManager>);
    ~FXGraphAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void resetNodes();

private:
    
    void setSelection(ParameterType type, int streamId); // sets selection to a specific stream
    void setSelection(int nodeId); // sets selection to a specific node
    void setSelection(); // clear selection
    
    void addNode(Data::Node*& node, int nodeId);
    
    bool streamSelected;
    ParameterType selectedStreamType;
    int selectedStreamId;
    
    bool nodeSelected;
    int selectedNodeId;
    
    struct KeepInAreaConstrainer;
    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    FXGraphAudioProcessor& audioProcessor;
    
    juce::Rectangle<float> draggableArea;
    
    juce::OwnedArray<Common::Node> graphNodes;
    
    SideMenu m_sideMenu;
    GraphAreaStreams m_graphAreaStreams;
    GraphAreaNodeContainer m_graphAreaNodeContainer;
    
    std::shared_ptr<DataManager> dataManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FXGraphAudioProcessorEditor)
};
