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
    GraphNode__Parameter(ParameterType, int, const juce::String&, InputOrOutput, GraphNode&);
    ~GraphNode__Parameter() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseUp(const juce::MouseEvent &event) override;
    
    bool hitTest(int x, int y) override;
    
    std::function<void(float)> onConstValueChanged;
    std::function<void(bool)> onSetIsConst;
    
    std::function<void()> onDragStart;
    std::function<void(juce::Point<float>)> onDrag;
    std::function<void(juce::Point<float>)> onDragEnd;
    
    juce::Rectangle<float> getPillRect();
    
    void setIsConst(bool v) {isConst = v; constLabel.setVisible(isConst);}
    bool getIsConst() {return isConst;}
    
    void setConstValue(float v) {constValue = v; constLabel.setText(juce::String(constValue), juce::dontSendNotification);} // update ui
    float getConstValue() {return constValue;}
    
    int getParamId() {return paramId;}
    
    juce::String paramName;
    ParameterType paramType;
    
private:
    GraphNode& owner;
    InputOrOutput inputOrOutput;
    
    juce::Label constLabel;
    
    bool isConst = false;
    float constValue;
    
    const float padSide = 7;
    const float padVertical  = 3;
    
    int paramId;
    
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


class GraphNode : public juce::Component
{
public:
    struct Parameter {
        std::unique_ptr<GraphNode__Parameter> component;
        InputOrOutput inputOrOutput;
    };
    
    GraphNode(std::shared_ptr<DataManager>, int nodeId, juce::Rectangle<float>&); // TODO: bump nodeId when nodes are removed
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
    
    std::function<void(InputOrOutput, int)> onDragStreamStart;
    std::function<void(InputOrOutput, int, juce::Point<float>)> onDragStream;
    std::function<void(InputOrOutput, int, juce::Point<float>)> onDragStreamEnd;
    
    std::function<void(int nodeId)> handleSelectNode;

    float getIdealHeight();
    void setNodeId(int n) {nodeId = n;}
    int getNodeId() {return nodeId;}
    
    void addParameter(ParameterType, int, const juce::String&, InputOrOutput);
    juce::Point<float> getParameterPosition(InputOrOutput side, int index);
    ParameterType getParameterType(InputOrOutput side, int index);
    
    juce::OwnedArray<Parameter>& getInputParams() {return inputParameters;}
    juce::OwnedArray<Parameter>& getOutputParams() {return outputParameters;}
    
    void setSelected(bool v) {isSelected = v;}
    
    bool hasInputSide = true;
    bool hasOutputSide = true;
    
    static const float cornerRadius;
    static const float headerHeight;
    static const float shadowSize;
private:
    
    const int paramHeight = 20;
    const int paramPadding = 5;
    
    juce::ComponentDragger componentDragger;
    juce::Rectangle<int> headerBounds;
    juce::Rectangle<float>& dragArea;
    
    juce::String name;
    
    bool allowDrag;
    bool isBeingDragged;
    
    bool isSelected = false;
    
    juce::OwnedArray<Parameter> inputParameters;
    juce::OwnedArray<Parameter> outputParameters;
    
    GraphNode__RemoveButton removeButton;
    
    int nodeId;
    std::shared_ptr<DataManager> dataManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphNode)
};
