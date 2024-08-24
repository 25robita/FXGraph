/*
  ==============================================================================

    GraphNode.cpp
    Created: 25 Jul 2024 9:48:09pm
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GraphNode.h"


//==============================================================================
GraphNode::GraphNode(Data::Node* d, juce::Rectangle<float>& dragRect) : dragArea(dragRect), removeButton(*this)
{
    data = d;
    name = data->friendlyName;
    
    removeButton.onClick = [this] () {
        onRemove();
    };
    
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    hasInputSide = data->hasInputSide;
    hasOutputSide = data->hasOutputSide;
    
    for (int paramId = 0; paramId < NUM_PARAMS; paramId++)
    {
        if (!data->inputParams[paramId].isActive) break;
        
        addParameter(data->inputParams[paramId].type, data->inputParams[paramId].friendlyName, InputOrOutput::Input);
    }
    
    for (int paramId = 0; paramId < NUM_PARAMS; paramId++)
    {
        if (!data->outputParams[paramId].isActive) break;
        addParameter(data->outputParams[paramId].type, data->outputParams[paramId].friendlyName, InputOrOutput::Output);
    }
    
    if (!data->isGlobalLockedNode)
        addAndMakeVisible(removeButton);
}

GraphNode::~GraphNode()
{
}

void GraphNode::addParameter(ParameterType paramType, const juce::String& name, InputOrOutput inputOrOutput)
{
    auto* p = new Parameter();
    
    p->component.reset(new GraphNode__Parameter(paramType, name, inputOrOutput, *this));
    addAndMakeVisible(p->component.get());
    
    p->inputOrOutput = inputOrOutput;
    
    
    if (inputOrOutput == InputOrOutput::Input)
    {
        inputParameters.add(p);
    } else
    {
        outputParameters.add(p);
    }
}

juce::Point<float> GraphNode::getParameterPosition(InputOrOutput side, int index)
{
    std::unique_ptr<GraphNode__Parameter> component;
    
    if (side == InputOrOutput::Input) {
        return inputParameters[index]->component->getLocalPoint(getParentComponent(), inputParameters[index]->component->getPillRect().getCentre() * -1) * -1;
    }
    else {
        return outputParameters[index]->component->getLocalPoint(getParentComponent(), outputParameters[index]->component->getPillRect().getCentre() * -1) * -1;
    }
    
//    return component->getLocalPoint(getParentComponent(), component->getPillRect().getCentre() * -1) * -1;
}

void GraphNode::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    auto bounds = getLocalBounds().reduced(shadowSize);

    juce::Path roundedBackground;
    
    roundedBackground.addRoundedRectangle(bounds, cornerRadius);
    
    auto shadow = juce::DropShadow(juce::Colour(0x40aaaaaa), shadowSize, {0, 0});

    shadow.drawForPath(g, roundedBackground);
    
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
    g.drawText (name, headerBounds.reduced(10.0f, 5.0f),
                juce::Justification::centredLeft, true);
}

void GraphNode::resized()
{
    // Header

    headerBounds = getLocalBounds().reduced(shadowSize);
    headerBounds.setBottom(headerBounds.getY() + headerHeight);
    
    // Remove Button
    removeButton.setBounds(headerBounds.withTrimmedLeft(headerBounds.getWidth() - headerBounds.getHeight()));
    
    // Input Params
    if (hasInputSide)
    {
        auto inputParamArea = getLocalBounds().reduced(shadowSize);
        
        inputParamArea.removeFromRight(inputParamArea.getWidth() / 2);
        inputParamArea.removeFromTop(headerHeight + paramPadding);
        inputParamArea.setHeight(paramHeight);
        
        for (auto param : inputParameters) {
            param->component->setBounds(inputParamArea);
            inputParamArea.translate(0, paramHeight + paramPadding);
        }
    }
    
    // Output Params
    if (hasOutputSide)
    {
        auto outputParamArea = getLocalBounds().reduced(shadowSize);
        
        outputParamArea.removeFromLeft(outputParamArea.getWidth() / 2);
        outputParamArea.removeFromTop(headerHeight + paramPadding);
        outputParamArea.setHeight(paramHeight);
        
        for (auto param : outputParameters) {
            param->component->setBounds(outputParamArea);
            outputParamArea.translate(0, paramHeight + paramPadding);
        }
    }
}

float GraphNode::getIdealHeight() 
{
    // TODO: add anything else needed on node
    return shadowSize * 2 + headerHeight + std::max(inputParameters.size(), outputParameters.size()) * (paramHeight + paramPadding) + paramPadding;
}

void GraphNode::mouseDown(const juce::MouseEvent &event) {
    if (headerBounds.contains(event.getPosition())) {
        allowDrag = true;
        
        componentDragger.startDraggingComponent(this, event);
    } else {
        allowDrag = false;
    }
}

void GraphNode::mouseDrag(const juce::MouseEvent &event) {
//    auto screenPos = event.getPosition();
    
    if (allowDrag) {
        isBeingDragged = true;

        componentDragger.dragComponent(this, event, nullptr);
        
        auto bounds = getBounds();

        if (!dragArea.contains(bounds.getX(), bounds.getY()) || !dragArea.contains(bounds.getRight(), bounds.getBottom()))
        {
            
            if (bounds.getRight() > dragArea.getRight())
                bounds.translate(dragArea.getRight() - bounds.getRight(), 0);
            
            if (bounds.getX() < dragArea.getX())
                bounds.translate(dragArea.getX() - bounds.getX(), 0);
            
            if (bounds.getBottom() > dragArea.getBottom())
                bounds.translate(0, dragArea.getBottom() - bounds.getBottom());
            
            if (bounds.getY() < dragArea.getY())
                bounds.translate(0, dragArea.getY() - bounds.getY());
            
            setBounds(bounds);
        }

//        juce::Rectangle<float> col;
//        auto ; // TODO: account for when this node is larger than the colliding rect

        for (auto col : getCollidingRects(*this, 5))
        {
            bool topRight = col.contains(bounds.getTopRight().toFloat());
            bool topLeft = col.contains(bounds.getTopLeft().toFloat());
            bool bottomRight = col.contains(bounds.getBottomRight().toFloat());
            bool bottomLeft = col.contains(bounds.getBottomLeft().toFloat());
            
            bool invTopRight = bounds.toFloat().contains(col.getTopRight());
            bool invTopLeft = bounds.toFloat().contains(col.getTopLeft());
            bool invBottomRight = bounds.toFloat().contains(col.getBottomRight());
            bool invBottomLeft = bounds.toFloat().contains(col.getBottomLeft());
            
            enum Direction {
                None = -1,
                Right = 0,
                Left = 1,
                Up = 2,
                Down = 3
            };
            
            Direction direction = Direction::None;
            float shortestDistance = INFINITY;
            
            if ((topLeft || topRight || invBottomLeft || invBottomRight) && col.getBottom() - bounds.getY() < shortestDistance) {
                shortestDistance = col.getBottom() - bounds.getY();
                direction = Direction::Down;
            }
            
            if ((topRight || bottomRight || invTopLeft || invBottomLeft) && bounds.getRight() - col.getX() < shortestDistance)
            {
                shortestDistance = bounds.getRight() - col.getX();
                direction = Direction::Left;
            }
            
            if ((bottomLeft || bottomRight || invTopLeft || invTopRight) && bounds.getBottom() - col.getY() < shortestDistance)
            {
                shortestDistance = bounds.getBottom() - col.getY();
                direction = Direction::Up;
            }
            
            if ((topLeft || bottomLeft || invTopRight || invBottomRight) && col.getRight() - bounds.getX() < shortestDistance)
            {
                shortestDistance = col.getRight() - bounds.getX();
                direction = Direction::Right;
            }
            
            if (direction == Direction::Right)
            {
                bounds.translate(shortestDistance, 0);
            } else if (direction == Direction::Left)
            {
                bounds.translate(-shortestDistance, 0);
            } else if (direction == Direction::Up)
            {
                bounds.translate(0, -shortestDistance);
            } else if (direction == Direction::Down)
            {
                bounds.translate(0, shortestDistance);
            }
            
            setBounds(bounds);
        }
        
        onMove();
    }
}

void GraphNode::mouseUp(const juce::MouseEvent &event)
{
    if (!isBeingDragged) return;
    
    isBeingDragged = false;
    
    data->position.setX(getX());
    data->position.setY(getY());
    
    onDataUpdate();
}

GraphNode__Parameter::GraphNode__Parameter(ParameterType type, const juce::String& text, InputOrOutput side, GraphNode& graphNode) : paramName(text), paramType(type), owner(graphNode)
{
    inputOrOutput = side;
}

GraphNode__Parameter::~GraphNode__Parameter()
{
    
}

void GraphNode__Parameter::paint (juce::Graphics& g)
{
    // draw the pill
    
    if (paramType == ParameterType::Audio) {
        g.setColour(juce::Colour(0xff4CA2C5));
    } else {
        g.setColour(juce::Colour(0xffFAA21B));
    }
    
    juce::Path pill;
    
    
    
    auto bounds = getLocalBounds();
    
    auto pillRect = getPillRect();
    
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
    
    if (inputOrOutput == InputOrOutput::Input) 
    {
        textRect.removeFromLeft(2 * padSide + pillRect.getHeight() * 1.5f);
    } else
    {
        textRect.removeFromRight(2 * padSide + pillRect.getHeight() * 1.5f);
    }
    
    g.drawText(paramName, textRect, inputOrOutput == InputOrOutput::Input ? juce::Justification::centredLeft : juce::Justification::centredRight);
}

juce::Rectangle<float> GraphNode__Parameter::getPillRect()
{
    auto bounds = getLocalBounds();
    
    float pillHeight = bounds.getHeight() - 2 * padVertical;
    float pillWidth = pillHeight * 1.5f;
    
    return juce::Rectangle<float>(
        inputOrOutput == InputOrOutput::Input ? padSide : bounds.getWidth() - padSide - pillWidth,
        padVertical,
        pillWidth,
        pillHeight
    );
}

void GraphNode__Parameter::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void GraphNode__Parameter::mouseDown(const juce::MouseEvent &event)
{
    
}

void GraphNode__Parameter::mouseDrag(const juce::MouseEvent &event)
{
    
}

GraphNode__RemoveButton::GraphNode__RemoveButton(GraphNode& parent) : owner(parent)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

GraphNode__RemoveButton::~GraphNode__RemoveButton()
{
}

void GraphNode__RemoveButton::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colour(0xbfFFFFFF));
    
    const float lineThicknesss = 1.3f;
    const float padding = 9.0f;
    
    auto crossRect = getLocalBounds().reduced(padding);
    
//    juce::Line<float> line;
//    
//    line.setStart(crossRect.getTopLeft().toFloat());
//    line.setEnd(crossRect.getBottomRight().toFloat());
//
//    g.drawLine(line, lineThicknesss);
//    
//    line.setStart(crossRect.getTopRight().toFloat());
//    line.setEnd(crossRect.getBottomLeft().toFloat());
//    
//    g.drawLine(line, lineThicknesss);
    
    g.drawLine(crossRect.getX(), crossRect.getY(), crossRect.getRight(), crossRect.getBottom(), lineThicknesss);
    g.drawLine(crossRect.getRight(), crossRect.getY(), crossRect.getX(), crossRect.getBottom(), lineThicknesss);
    
    
//    g.setFont (juce::FontOptions (14.0f));
//    g.drawText ("x", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void GraphNode__RemoveButton::resized()
{

}

void GraphNode__RemoveButton::mouseDown(const juce::MouseEvent&)
{
    onClick(); // pass to parent
}
