/*
  ==============================================================================

    GraphNode.h
    Created: 25 Jul 2024 9:48:09pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DataManager.h"

//==============================================================================
/*
*/
class GraphNode;

enum InputOrOutput
{
    Input,
    Output
};

class GraphNode__Parameter  : public juce::Component
{
public:
    GraphNode__Parameter(ParameterType, const juce::String&, InputOrOutput, GraphNode&);
    ~GraphNode__Parameter() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    
    juce::Rectangle<float> getPillRect();
    
    juce::String paramName;
    ParameterType paramType;
    
private:
    GraphNode& owner;
    InputOrOutput inputOrOutput;
    
    const float padSide = 7;
    const float padVertical  = 3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphNode__Parameter)
};

class GraphNode__RemoveButton  : public juce::Component
{
public:
    GraphNode__RemoveButton(GraphNode& parent);
    ~GraphNode__RemoveButton() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent&) override;
    std::function<void()> onClick;

private:
    GraphNode& owner;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphNode__RemoveButton)
};


class GraphNode  : public juce::Component
{
public:
    GraphNode(Data::Node*, juce::Rectangle<float>&);
    ~GraphNode() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    
    std::function<void()> onMove;
    std::function<void()> onDataUpdate;
    std::function<void()> onRemove;
    std::function<juce::Array<juce::Rectangle<float>>(GraphNode&, float padding)> getCollidingRects;

    float getIdealHeight();
    
    void addParameter(ParameterType, const juce::String&, InputOrOutput);
    juce::Point<float> getParameterPosition(InputOrOutput side, int index);
    
    bool hasInputSide = true;
    bool hasOutputSide = true;
    
private:
    const float cornerRadius = 10.0f;
    const float headerHeight = 25.0f;
    const float shadowSize = 10.0f;
    
    const int paramHeight = 20;
    const int paramPadding = 5;
    
    juce::ComponentDragger componentDragger;
    juce::Rectangle<int> headerBounds;
    juce::Rectangle<float>& dragArea;
    
    juce::String name;
    
    bool allowDrag;
    bool isBeingDragged;
    
    struct Parameter {
        std::unique_ptr<GraphNode__Parameter> component;
        InputOrOutput inputOrOutput;
    };
    
    juce::OwnedArray<Parameter> inputParameters;
    juce::OwnedArray<Parameter> outputParameters;
    
    GraphNode__RemoveButton removeButton;
    
    
    Data::Node* data;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphNode)
};
