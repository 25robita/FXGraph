/*
  ==============================================================================

    NodeLibraryNode.h
    Created: 5 Sep 2024 3:01:43pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GraphNode.h"
#include "DataManager.h"

//==============================================================================
/*
*/
class NodeLibraryNode  : public juce::Component
{
public:
    NodeLibraryNode(juce::String _name, bool _hasInputSide, bool _hasOutputSide);
    ~NodeLibraryNode() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseUp(const MouseEvent &event) override;
    
    std::function<void(juce::Point<float> position)> onDragEnd;

    void addParameter(InputOrOutput side, ParameterType type, juce::String name)
    {
        int nextIndex = getNextParameterIndex(side);
        
        if (nextIndex == -1) return;
        
        if (side == InputOrOutput::Input)
        {
            inputParameters[nextIndex].isActive = true;
            inputParameters[nextIndex].type = type;
            inputParameters[nextIndex].name = name;
            return;
        }
        
        outputParameters[nextIndex].isActive = true;
        outputParameters[nextIndex].type = type;
        outputParameters[nextIndex].name = name;
    }
    
    void drawParameter(juce::Graphics& g, InputOrOutput side, int index, juce::Rectangle<int>&);
    
    struct Parameter
    {
        bool isActive = false;
        
        ParameterType type;
        juce::String name;
    };
    
    float getIdealHeight();
    
    
    
private:
    const int paramHeight = 20;
    const int paramPadding = 5;
    
    int getNextParameterIndex(InputOrOutput side)
    {
        for (int i = 0; i < 16; i++)
        {
            if (side == InputOrOutput::Output && outputParameters[i].isActive == false) return i;
            if (side == InputOrOutput::Input && inputParameters[i].isActive == false) return i;
        }
        
        return -1;
    }
    
    bool hasInputSide = true;
    bool hasOutputSide = true;
    
    const float headerHeight = GraphNode::headerHeight;
    const float cornerRadius = GraphNode::cornerRadius;
    
    juce::String name;
    
    Parameter inputParameters[NUM_PARAMS];
    Parameter outputParameters[NUM_PARAMS];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeLibraryNode)
};
