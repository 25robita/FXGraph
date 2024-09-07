/*
  ==============================================================================

    NodeLibraryPanel.cpp
    Created: 24 Jul 2024 8:45:28pm
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NodeLibraryPanel.h"
#include "DataManager.h"

//==============================================================================
NodeLibraryPanel::NodeLibraryPanel(std::shared_ptr<DataManager> d) : dataManager(d)
{
    
    // initialise members of nodes array
    
    for (auto type : NodeTypes)
    {
        NodeLibraryNode* node;
        
        
        switch (type) {
            case NodeType::MainInput:
            case NodeType::MainOutput:
                goto continueloop;
                break;
                
            case NodeType::Gain:
                node = new NodeLibraryNode(Data::GainNode::defaults.name, Data::GainNode::defaults.hasInputSide, Data::GainNode::defaults.hasOutputSide);
                
                // initialise parameters
                // TODO: make this better, maybe modularise or add a thingy on the Data::GainNode to store this stuff (across all of these enum vals)
                node->addParameter(InputOrOutput::Input, ParameterType::Audio, "In");
                node->addParameter(InputOrOutput::Input, ParameterType::Value, "Gain");
                node->addParameter(InputOrOutput::Output, ParameterType::Audio, "Out");
                
                nodes.add(node);
                break;
            case NodeType::Level:
                node = new NodeLibraryNode(Data::LevelNode::defaults.name, Data::LevelNode::defaults.hasInputSide, Data::LevelNode::defaults.hasOutputSide);
                
                // initialise parameters
                node->addParameter(InputOrOutput::Input, ParameterType::Audio, "In");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "Float");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "dbFS");
                
                nodes.add(node);
                break;
            case NodeType::Correlation:
                node = new NodeLibraryNode(Data::CorrelationNode::defaults.name, Data::CorrelationNode::defaults.hasInputSide, Data::CorrelationNode::defaults.hasOutputSide);
                
                // initialise parameters
                node->addParameter(InputOrOutput::Input, ParameterType::Audio, "In");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "Correlation");
                
                nodes.add(node);
                break;
            case NodeType::Loudness:
                node = new NodeLibraryNode(Data::LoudnessNode::defaults.name, Data::LoudnessNode::defaults.hasInputSide, Data::LoudnessNode::defaults.hasOutputSide);
                
                // initialise parameters
                node->addParameter(InputOrOutput::Input, ParameterType::Audio, "In");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "LUFS-S");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "LUFS-M");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "LUFS-I");
                
                nodes.add(node);
                break;
            case NodeType::Maths:
                node = new NodeLibraryNode(Data::MathsNode::defaults.name, Data::MathsNode::defaults.hasInputSide, Data::MathsNode::defaults.hasOutputSide);
                
                // initialise parameters
                node->addParameter(InputOrOutput::Input, ParameterType::Value, "Input 1");
                node->addParameter(InputOrOutput::Output, ParameterType::Value, "Out");
                
                nodes.add(node);
                break;
        }
        
        node->onDragEnd = [this, type] (juce::Point<float> p) {
            // TODO: finish this
            
            if (getParentComponent()->getParentComponent()->getBounds().contains(p.toInt())) return;
            
            dataManager->startEditing();
            
            int id = dataManager->inactiveInstance->getNextNodeId();
            dataManager->addNode(id, type, p);
            
            dataManager->finishEditing();
            
//            dataManager->registerOneTimeRealisationListener(onNodeAdded);
            
//            onNodeAdded();
        };
        continueloop:
    }
    
    for (auto node : nodes)
    {
        addAndMakeVisible(node);
    }
    
}

NodeLibraryPanel::~NodeLibraryPanel()
{
}

void NodeLibraryPanel::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

//    g.fillAll (getLookAndFeel().findColour (juce::SidePanel::backgroundColour));   // clear the background
//
//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (juce::FontOptions (14.0f));
//    g.drawText ("NodeLibraryPanel", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void NodeLibraryPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    const int padding = 10;
    
    auto b = getLocalBounds().reduced(padding);
    
    int y = padding;
    
    for (auto node : nodes)
    {
        float height = node->getIdealHeight();
        
        node->setBounds(b.withY(y).withHeight(height));
        
        y += height + padding;
    }
}
