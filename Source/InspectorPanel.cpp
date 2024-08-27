/*
  ==============================================================================

    InspectorPanel.cpp
    Created: 24 Jul 2024 8:44:48pm
    Author:  School

  ==============================================================================
*/

#include <string>
#include <JuceHeader.h>
#include "InspectorPanel.h"
#include "DataManager.h"
#include "AnalysisGraphContent.h"

//==============================================================================
InspectorPanel::InspectorPanel(std::shared_ptr<DataManager> d) : valueStreamGraph(d)
{
    dataManager = d;
    
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    addAndMakeVisible(header);
    addChildComponent(valueStreamGraph);
    
    header.setText("Value Stream #1");
    
}

InspectorPanel::~InspectorPanel()
{
}

void InspectorPanel::paint (juce::Graphics& g)
{
    
}

void InspectorPanel::reset()
{
    groups.clear();
    individualParams.clear();
    header.setText("");
    valueStreamGraph.setVisible(false);
}

void InspectorPanel::resized()
{
    
    const float padding = 10;
    // This method is where you should set the bounds of any child
    // components that your component contains..
    
    auto b = getLocalBounds().reduced(10, 15);

    float currHeight = b.getY();
    
    header.setBounds(b.withY(currHeight).withHeight(header.textHeight + padding));
    currHeight += header.textHeight + padding;
    currHeight += padding;
    
    for (auto group : groups)
    {
        float idealHeight = group->getIdealHeight();
        
        group->setBounds(b.withY(currHeight).withHeight(idealHeight));
        
        currHeight += idealHeight + padding;
    }
    
    for (auto param : individualParams)
    {
        float idealHeight = param->getIdealHeight();
        
        param->setBounds(b.withY(currHeight).withHeight(idealHeight));
        
        currHeight += idealHeight + padding;
    }
    
    valueStreamGraph.setBounds(b.withY(currHeight).withHeight(100));
    valueStreamGraph.setSelection(selectedStreamType, selectedStreamId);
    
}

void InspectorPanel::pubtest()
{
    reset();
}

void InspectorPanel::setSelection()
{
    reset();
}

void InspectorPanel::setSelection(ParameterType type, int streamId)
{
    reset();
    
    nodeSelected = false;
    streamSelected = true;
    selectedStreamType = type;
    selectedStreamId = streamId;
    
    std::string s = type == ParameterType::Audio ? "Audio stream #" : "Value stream #";
    s.append(std::to_string(streamId));
    header.setText(s);
    
    if (type == ParameterType::Value)
    {
        auto envelope = new InspectorPanel__Group();
        envelope->setName("Envelope");
        
        auto attack = new InspectorPanel__Param();
        attack->setName("Attack time");
        attack->setValue(juce::String(dataManager->activeInstance->valueStreams[streamId].envelope.getMsAttack()));
        attack->setSuffix("ms");
        attack->handleInput = [this, streamId] (const juce::String& newVal) {
            dataManager->startEditing();
            dataManager->inactiveInstance->valueStreams[streamId].envelope.setMsAttack(newVal.getFloatValue());
            dataManager->finishEditing();
        };
        
        auto release = new InspectorPanel__Param();
        release->setName("Release time");
        release->setSuffix("ms");
        release->setValue(juce::String(dataManager->activeInstance->valueStreams[streamId].envelope.getMsRelease()));
        release->handleInput = [this, streamId] (const juce::String& newVal) {
            dataManager->startEditing();
            
            dataManager->inactiveInstance->valueStreams[streamId].envelope.setMsRelease(newVal.getFloatValue());
            dataManager->finishEditing();
        };
        
        envelope->addParam(attack);
        envelope->addParam(release);
        
        addGroup(envelope);
        
        valueStreamGraph.setVisible(true);
        valueStreamGraph.setSelection(type, streamId);
    }
    
    resized();
}

void InspectorPanel::addGroup(InspectorPanel__Group *group)
{
    addAndMakeVisible(group);
    groups.add(group);
    
    // TODO: redraw maybe?
}

void InspectorPanel::addParam(InspectorPanel__Param *param)
{
    addAndMakeVisible(param);
    individualParams.add(param);
    
    // TODO: redraw maybe?
}





InspectorPanel__Param::InspectorPanel__Param() : font(juce::FontOptions(textHeight))
{
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(fieldLabel);
    addAndMakeVisible(suffixLabel);
    
    nameLabel.setFont(font);
    fieldLabel.setFont(font);
    suffixLabel.setFont(font);
    
    suffixLabel.setJustificationType(juce::Justification::centred);
    fieldLabel.setJustificationType(juce::Justification::centredRight);
    
    fieldLabel.setEditable(true);
    
    fieldLabel.onTextChange = [this] ()
    {
        handleInput(fieldLabel.getText()); // TODO: validate input as a float
    };

}

InspectorPanel__Param::~InspectorPanel__Param()
{
}

void InspectorPanel__Param::paint (juce::Graphics& g)
{
}

void InspectorPanel__Param::resized()
{
    auto b = getLocalBounds();
    
    float right = 0;
    
    float suffixWidth = font.getStringWidthFloat(suffix);
    
    right += suffixWidth;
    
    if (suffixWidth > 0)
    {
        right += horizontalPadding * 2;
    }
    
    suffixLabel.setBounds(b.withLeft(b.getWidth() - right));
    
    const float fieldWidth = 50;
    
    fieldLabel.setBounds(b.removeFromRight(right + fieldWidth).withTrimmedRight(right));
    
    nameLabel.setBounds(b.withTrimmedRight(horizontalPadding));

}

float InspectorPanel__Param::getIdealHeight()
{
    return 20;
}

void InspectorPanel__Param::setName(juce::String name_)
{
    name = name_;
    
    nameLabel.setText(name, juce::dontSendNotification);
}

void InspectorPanel__Param::setSuffix()
{
    setSuffix("");
}

void InspectorPanel__Param::setSuffix(juce::String suffix_)
{
    suffix = suffix_;
    
    suffixLabel.setText(suffix, juce::dontSendNotification);
}

void InspectorPanel__Param::setValue(juce::String value)
{
    fieldLabel.setText(value, juce::dontSendNotification);
}





InspectorPanel__Header::InspectorPanel__Header()
{
    addAndMakeVisible(label);
    
    label.setFont(juce::Font(juce::FontOptions(textHeight, juce::Font::FontStyleFlags::italic)));
}

InspectorPanel__Header::~InspectorPanel__Header()
{
}

void InspectorPanel__Header::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    if (text.length() == 0) return;
    
    auto b = getLocalBounds();

    g.setColour(juce::Colour(0x99ffffff));
    g.drawLine(b.getX(), b.getBottom(), b.getRight(), b.getBottom(), 1);
}

void InspectorPanel__Header::resized()
{
    label.setBounds(getLocalBounds().withHeight(textHeight));
}

void InspectorPanel__Header::setText(juce::String text_)
{
    text = text_;
    
    label.setText(text, juce::NotificationType::dontSendNotification);
    
    // TODO: redraw something probably?
}







InspectorPanel__Group::InspectorPanel__Group()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    addAndMakeVisible(label);
    
    label.setFont(juce::Font(juce::FontOptions(textHeight, juce::Font::FontStyleFlags::bold)));
}

InspectorPanel__Group::~InspectorPanel__Group()
{
}

void InspectorPanel__Group::paint (juce::Graphics& g)
{
    
}

void InspectorPanel__Group::resized()
{
    auto b = getLocalBounds();
    
    label.setBounds(b.withHeight(textHeight));
    
    float currHeight = textHeight;
    
    b.setLeft(paramIndent);
    
    for (auto param : params)
    {
        float h = param->getIdealHeight();
        
        currHeight += padding;
        param->setBounds(b.withY(currHeight).withHeight(h));
        currHeight += h;
    }
}

float InspectorPanel__Group::getIdealHeight()
{
    float h = 0;
    
    h += textHeight;
    
    for (auto param : params)
    {
        h += param->getIdealHeight();
        h += padding;
    }
    
    return h;
    
}

void InspectorPanel__Group::setName(juce::String name_)
{
    name = name_;
    
    label.setText(name << ":", juce::dontSendNotification);
}

void InspectorPanel__Group::addParam(InspectorPanel__Param *param)
{
    addAndMakeVisible(param);
    params.add(param);
}
