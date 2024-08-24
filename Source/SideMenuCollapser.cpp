/*
  ==============================================================================

    SideMenuCollapser.cpp
    Created: 23 Jul 2024 10:26:09am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SideMenuCollapser.h"

//==============================================================================
SideMenuCollapser::SideMenuCollapser()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

SideMenuCollapser::~SideMenuCollapser()
{
}

void SideMenuCollapser::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    const float padding = 7.0f;
    const float thickness = 6.0f;
    
    const float widthFactor = 0.9f;

    auto bounds = getLocalBounds();
    
    juce::Path buttonBackground;
    
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    
    g.setColour(getLookAndFeel().findColour(juce::SidePanel::backgroundColour));
    
    buttonBackground.addRoundedRectangle(0, 0, width, height, 10, 10, false, true, false, true);
    
    g.fillPath(buttonBackground);
    
    g.setColour(juce::Colours::white);
    
    juce::Path arrow;
    
    arrow.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    if (!isRight) arrow.applyTransform (juce::AffineTransform(-1, 0, 1, 0, 1, 0));
    
    arrow.applyTransform (arrow.getTransformToScaleToFit ((isRight ? 0 : -2.0f) + padding + width * (1-widthFactor) / 2, padding, width * widthFactor - 2 * padding, height - 2 * padding, false));
    
    g.fillPath(arrow);
    
    g.setColour(getLookAndFeel().findColour(juce::SidePanel::backgroundColour));
    
    arrow.applyTransform (arrow.getTransformToScaleToFit ((isRight ? 0 : -2.0f) + padding + width * (1-widthFactor) / 2 + (isRight ? -1 : 1) * thickness, padding, width * widthFactor - 2 * padding, height - 2 * padding, false));
    
    g.fillPath(arrow);
    
    
}

void SideMenuCollapser::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void SideMenuCollapser::mouseDown(const juce::MouseEvent& event)
{
    onUpdate();
}
