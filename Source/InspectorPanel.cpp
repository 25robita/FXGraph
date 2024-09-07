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
    addChildComponent(mathsNodeTextBox);
    
    mathsNodeTextBox.setName("Expression (ExprTK):");
    
    header.setText("Nothing Selected");
    
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
    mathsNodeTextBox.setVisible(false);
    
    inputParamsList.reset();
    outputParamsList.reset();
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
    
    if (streamSelected && selectedStreamType == ParameterType::Value)
    {
        valueStreamGraph.setBounds(b.withY(currHeight).withHeight(100));
        valueStreamGraph.setSelection(selectedStreamType, selectedStreamId);
        
        currHeight += 100 + padding;
    }
    
    if (nodeSelected && dataManager->activeInstance->nodes[selectedNodeId]->getType() == NodeType::Maths)
    {
        float h = mathsNodeTextBox.getIdealHeight();
        
        mathsNodeTextBox.setBounds(b.withY(currHeight).withHeight(h));
        
        currHeight += h + padding;
    }
    
    if (inputParamsList != nullptr) {
        float idealHeight =inputParamsList->getIdealHeight();
        
        inputParamsList->setBounds(b.withY(currHeight).withHeight(idealHeight));
        
        currHeight += idealHeight + padding;
    }
    
    if (outputParamsList != nullptr) {
        float idealHeight =outputParamsList->getIdealHeight();
        
        outputParamsList->setBounds(b.withY(currHeight).withHeight(idealHeight));
        
        currHeight += idealHeight + padding;
    }
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

void InspectorPanel::setSelection(int nodeId)
{
    reset();
    
    nodeSelected = true;
    streamSelected = false;
    selectedNodeId = nodeId;
    
    auto node = dataManager->activeInstance->nodes[nodeId];
    
    if (node == nullptr) return;
    
    header.setText(node->friendlyName);
    
    auto position = new InspectorPanel__Group(); // TODO: doesn't yet update shown values with dataManager, idk how to do this though and i cant be bothered rn
    position->setName("Position");
    
    auto xPos = new InspectorPanel__Param();
    xPos->setName("x");
    xPos->setValue(juce::String(node->position.getX()));
    xPos->handleInput = [this, nodeId] (const juce::String& newVal) {
        dataManager->startEditing();
        dataManager->inactiveInstance->nodes[nodeId]->position.setX(newVal.getFloatValue());
        dataManager->finishEditing();
    };
    
    auto yPos = new InspectorPanel__Param();
    yPos->setName("y");
    yPos->setValue(juce::String(node->position.getY()));
    yPos->handleInput = [this, nodeId] (const juce::String& newVal) {
        dataManager->startEditing();
        dataManager->inactiveInstance->nodes[nodeId]->position.setY(newVal.getFloatValue());
        dataManager->finishEditing();
    };
    
    position->addParam(xPos);
    position->addParam(yPos);
    
    addGroup(position);
    
    if (node->getType() == NodeType::Maths)
    {
        mathsNodeTextBox.setVisible(true);
        mathsNodeTextBox.setValue(((Data::MathsNode*)node)->expression_string);
        
        mathsNodeTextBox.handleInput = [this, nodeId] (const juce::String& newVal) {
            dataManager->startEditing();
            
            auto node = dataManager->inactiveInstance->nodes[nodeId];
            
            if (node == nullptr || node->getType() != NodeType::Maths) {
                dataManager->finishEditing();
                return;
            }
            
            auto mathsNode = (Data::MathsNode*) node;
            
            mathsNode->updateExpressionString(newVal);
            
            dataManager->finishEditing();
        };
    }
    
    inputParamsList.reset(new InspectorPanel__ParamsList(dataManager, selectedNodeId, InputOrOutput::Input));
    outputParamsList.reset(new InspectorPanel__ParamsList(dataManager, selectedNodeId, InputOrOutput::Output));
    
    addAndMakeVisible(*inputParamsList);
    addAndMakeVisible(*outputParamsList);
    
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



InspectorPanel__TextBox::InspectorPanel__TextBox() : font(juce::FontOptions(textHeight))
{
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(textEditor);
    
    nameLabel.setFont(juce::Font(juce::FontOptions(textHeight, juce::Font::FontStyleFlags::bold)));
    textEditor.setFont(font);
    
    textEditor.setMultiLine(true);
    textEditor.setReturnKeyStartsNewLine(true);
    
    textEditor.onFocusLost = [this] ()
    {
        handleInput(textEditor.getText());
    };
}

InspectorPanel__TextBox::~InspectorPanel__TextBox()
{
}

void InspectorPanel__TextBox::paint (juce::Graphics& g)
{
}

void InspectorPanel__TextBox::resized()
{
    auto b = getLocalBounds();
    
    nameLabel.setBounds(b.removeFromTop(textHeight + 3));
    
    textEditor.setBounds(b);
}

float InspectorPanel__TextBox::getIdealHeight()
{
    return (textHeight + 3) * (numLines + 1);
}

void InspectorPanel__TextBox::setName(juce::String name_)
{
    name = name_;
    
    nameLabel.setText(name, juce::dontSendNotification);
}

void InspectorPanel__TextBox::setValue(juce::String value)
{
    textEditor.setText(value);
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




InspectorPanel__ParamsList::InspectorPanel__ParamsList(std::shared_ptr<DataManager> d, int nodeId_, InputOrOutput side) : paramTable(d, nodeId_, side), font(juce::FontOptions(textHeight))
{
    dataManager = d;
    
    setName(side == InputOrOutput::Input ? "Input Parameters" : "Output Parameters");
    addAndMakeVisible(nameLabel);
    
    nodeId = nodeId_;
    
    addAndMakeVisible(paramTable);
    
    hasAddButton = (side == InputOrOutput::Input && dataManager->activeInstance->nodes[nodeId]->canAddInputParam())
                    || (side == InputOrOutput::Output && dataManager->activeInstance->nodes[nodeId]->canAddOutputParam());
    
    if (hasAddButton)
    {
        addAndMakeVisible(addParamButton);
        addParamButton.setButtonText("Add Parameter");
        
        addParamButton.onClick = [this, side] () {
            // assume its a value param, because that's all that makes sense really
            
            dataManager->startEditing();
            
            int id = dataManager->inactiveInstance->nodes[nodeId]->nextAvailableParamId(side);
            
            if (id == -1)
            {
                //TODO: don't fail silently please
                dataManager->finishEditing();
                return;
            }
            
            Data::Parameter& param = side == InputOrOutput::Input ? (Data::Parameter&)dataManager->inactiveInstance->nodes[nodeId]->inputParams[id] : (Data::Parameter&)dataManager->inactiveInstance->nodes[nodeId]->outputParams[id];
            
            param.type = ParameterType::Value;
            param.name = "input" + juce::String(id + 1);
            param.friendlyName = "Input " + juce::String(id + 1);
            param.isActive = true;
            
            dataManager->finishEditing();
            
        };
    }
    
    nameLabel.setFont(juce::Font(juce::FontOptions(textHeight, juce::Font::FontStyleFlags::bold)));
}

InspectorPanel__ParamsList::~InspectorPanel__ParamsList()
{
}

void InspectorPanel__ParamsList::paint (juce::Graphics& g)
{
}

void InspectorPanel__ParamsList::resized()
{
    auto b = getLocalBounds();
    
    nameLabel.setBounds(b.removeFromTop(textHeight + 5));
    
    paramTable.setBounds(b.removeFromTop(paramTable.getIdealHeight()));
    
    b.removeFromTop(5);
    
    addParamButton.setBounds(b);
}

float InspectorPanel__ParamsList::getIdealHeight()
{
    return (textHeight + 5) + paramTable.getIdealHeight() + (hasAddButton ? 5 + buttonHeight : 0); // TODO: add button
}

void InspectorPanel__ParamsList::setName(juce::String name_)
{
    name = name_;
    
    nameLabel.setText(name, juce::dontSendNotification);
}
