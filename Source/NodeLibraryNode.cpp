/*
  ==============================================================================

    NodeLibraryNode.cpp
    Created: 5 Sep 2024 3:01:43pm
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NodeLibraryNode.h"

//==============================================================================
NodeLibraryNode::NodeLibraryNode(juce::String _name, bool _hasInputSide, bool _hasOutputSide) : hasInputSide(_hasInputSide), hasOutputSide(_hasOutputSide), name(_name)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

NodeLibraryNode::~NodeLibraryNode()
{
}

void NodeLibraryNode::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    juce::Path roundedBackground;
    
    roundedBackground.addRoundedRectangle(bounds, cornerRadius);
    
    g.setColour(juce::Colour(0xff36353A)); //(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillPath(roundedBackground);
    
    if (hasOutputSide)
    {
        juce::Path outputSide;
        
        outputSide.addRoundedRectangle(bounds.getX() + ((hasInputSide ? 1 : 0) * bounds.getWidth() / 2), bounds.getY() + headerHeight, bounds.getWidth() / (hasInputSide ? 2 : 1), bounds.getHeight() - headerHeight, cornerRadius, cornerRadius, false, false, !hasInputSide, true);
        
        g.setColour(juce::Colour(0xff2E2F38));
        
        g.fillPath(outputSide);
    }
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (headerHeight * 0.5f));
    g.drawText (name, bounds.withHeight(headerHeight).reduced(10.0f, 5.0f),
                juce::Justification::centredLeft, true);
    
    if (hasInputSide)
    {
        auto inputParamArea = getLocalBounds();
        
        inputParamArea.removeFromRight(inputParamArea.getWidth() / 2);
        inputParamArea.removeFromTop(headerHeight + paramPadding);
        inputParamArea.setHeight(paramHeight);
        
        for (int i = 0; i < NUM_PARAMS; i++) {
            if (!inputParameters[i].isActive) break;
            
            drawParameter(g, InputOrOutput::Input, i, inputParamArea);
            inputParamArea.translate(0, paramHeight + paramPadding);
        }
    }
    
    if (hasOutputSide)
    {
        auto outputParamArea = getLocalBounds();
        
        outputParamArea.removeFromLeft(outputParamArea.getWidth() / 2);
        outputParamArea.removeFromTop(headerHeight + paramPadding);
        outputParamArea.setHeight(paramHeight);
        
        for (int i = 0; i < NUM_PARAMS; i++) {
            if (!outputParameters[i].isActive) break;
            
            drawParameter(g, InputOrOutput::Output, i, outputParamArea);
            outputParamArea.translate(0, paramHeight + paramPadding);
        }
    }
}

float NodeLibraryNode::getIdealHeight()
{
    int maxNumParams;
    for (maxNumParams = 0; maxNumParams < NUM_PARAMS; maxNumParams++)
    {
        if (!inputParameters[maxNumParams].isActive && !outputParameters[maxNumParams].isActive) break;
    }
    
    return headerHeight + maxNumParams * (paramHeight + paramPadding) + paramPadding;
}

void NodeLibraryNode::drawParameter(juce::Graphics &g, InputOrOutput side, int index, juce::Rectangle<int>& bounds)
{
    ParameterType paramType;
    juce::String paramName;
    
    const float padSide = 7;
    const float padVertical  = 3;
    
    if (side == InputOrOutput::Output)
    {
        paramType = outputParameters[index].type;
        paramName = outputParameters[index].name;
    } else {
        paramType = inputParameters[index].type;
        paramName = inputParameters[index].name;
    }
    
    if (paramType == ParameterType::Audio) {
        g.setColour(juce::Colour(0xff4CA2C5));
    } else {
        g.setColour(juce::Colour(0xffFAA21B));
    }
    
    juce::Path pill;
    
    float pillHeight = bounds.getHeight() - 2 * padVertical;
    float pillWidth = pillHeight * 1.5f;
    
    auto pillRect = juce::Rectangle<float>(
        side == InputOrOutput::Input ? padSide : bounds.getWidth() - padSide - pillWidth,
        padVertical,
        pillWidth,
        pillHeight
    );
    
    pillRect.translate(bounds.getX(), bounds.getY());
    
    pill.addRoundedRectangle(
        pillRect.getX(),
        pillRect.getY(),
        pillRect.getWidth(),
        pillRect.getHeight(),
        pillRect.getHeight() / 2.0f,
        pillRect.getHeight() / 2.0f,
        true, true, true, true);
    
    g.fillPath(pill);
    
    g.setColour(juce::Colour(0xFFABABAB));
    
    g.setFont(juce::FontOptions(pillRect.getHeight(), juce::Font::FontStyleFlags::italic));
    
    auto textRect = bounds.reduced(0, padVertical * 2);
    
    if (side == InputOrOutput::Input)
    {
        textRect.removeFromLeft(2 * padSide + pillRect.getHeight() * 1.5f);
    } else
    {
        textRect.removeFromRight(2 * padSide + pillRect.getHeight() * 1.5f);
    }
    
    textRect.translate(padSide / 2, 0);
    
    g.drawText(paramName, textRect, side == InputOrOutput::Input ? juce::Justification::centredLeft : juce::Justification::centredRight);
}

void NodeLibraryNode::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void NodeLibraryNode::mouseUp(const MouseEvent &event)
{
    onDragEnd(getLocalPoint(getTopLevelComponent(), event.getPosition().toFloat() * -1) * -1);
}
