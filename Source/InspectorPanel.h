/*
  ==============================================================================

    InspectorPanel.h
    Created: 24 Jul 2024 8:44:48pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DataManager.h"
#include "AnalysisGraphContent.h"

class InspectorPanel__Param  : public juce::Component
{
public:
    InspectorPanel__Param();
    ~InspectorPanel__Param() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    std::function<void(const juce::String& newValue)> handleInput;
    
    void setName(juce::String name_);
    juce::String getName() {return name;};
    
    void setSuffix();
    void setSuffix(juce::String suffix_);
    juce::String getSuffix() {return suffix;};
    
    void setValue(juce::String value);
    
    float getIdealHeight();
    
    const float textHeight = 15;

private:
    juce::Label nameLabel;
    juce::Label fieldLabel;
    juce::Label suffixLabel;
    
    juce::String name;
    juce::String suffix;
    
    juce::Font font;
    
    const float horizontalPadding = 3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel__Param)
};

class InspectorPanel__TextBox  : public juce::Component
{
public:
    InspectorPanel__TextBox();
    ~InspectorPanel__TextBox() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    std::function<void(const juce::String& newValue)> handleInput;
    
    void setName(juce::String name_);
    juce::String getName() {return name;};
    
    void setValue(juce::String value);
    
    void setNumLines(int v);
    int getNumLines() {return numLines;};
    
    float getIdealHeight();
    
    const float textHeight = 15;

private:
    juce::Label nameLabel;
    juce::TextEditor textEditor;
    
    juce::String name;
    
    juce::Font font;
    
    const float horizontalPadding = 3;
    
    int numLines = 3;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel__TextBox)
};

class InspectorPanel__Group  : public juce::Component
{
public:
    InspectorPanel__Group();
    ~InspectorPanel__Group() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void setName(juce::String name_);
    juce::String getName() {return name;};
    
    float getIdealHeight();
    
    void addParam(InspectorPanel__Param* param);

    const float textHeight = 15;
    const float paramIndent = 20;
    const float padding = 5;
private:
    juce::OwnedArray<InspectorPanel__Param> params;
    juce::Label label;
    juce::String name;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel__Group)
};


class InspectorPanel__Header  : public juce::Component
{
public:
    InspectorPanel__Header();
    ~InspectorPanel__Header() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void setText(juce::String text_);
    juce::String getText() {return text;};

    const int textHeight = 23;
private:
    juce::Label label;
    juce::String text;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel__Header)
};


class InspectorPanel  : public juce::Component
{
public:
    InspectorPanel(std::shared_ptr<DataManager>);
    ~InspectorPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void pubtest();
    
    void setSelection();
    void setSelection(ParameterType type, int streamId);
    void setSelection(int nodeId);
    
private:
    bool streamSelected;
    ParameterType selectedStreamType;
    int selectedStreamId;
    
    bool nodeSelected;
    int selectedNodeId;
    
    InspectorPanel__Header header;
    juce::OwnedArray<InspectorPanel__Group> groups;
    juce::OwnedArray<InspectorPanel__Param> individualParams;
    AnalysisGraphContent valueStreamGraph;
    InspectorPanel__TextBox mathsNodeTextBox;
    
    void addGroup(InspectorPanel__Group* group);
    void addParam(InspectorPanel__Param* param);
    
    void reset();
    
    std::shared_ptr<DataManager> dataManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel)
};
