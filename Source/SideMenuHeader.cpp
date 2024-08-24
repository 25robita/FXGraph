/*
  ==============================================================================

    SideMenuHeader.cpp
    Created: 23 Jul 2024 10:16:13am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SideMenuHeader.h"

SideMenuHeaderButton::SideMenuHeaderButton(const juce::String& buttonName, int buttonIndex, SideMenuHeader& parent) : name(buttonName), owner(parent)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    index = buttonIndex;
}

SideMenuHeaderButton::~SideMenuHeaderButton()
{
}

void SideMenuHeaderButton::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    if (pressed)
        g.setColour(getLookAndFeel().findColour(juce::SidePanel::backgroundColour));
    else
        g.setColour(pressedBackground);
    
    juce::Path path;
    
    auto bounds = getLocalBounds();
    
    if (!pressed)
        bounds.removeFromTop(bounds.getHeight() * 0.2f);
    
    path.addRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), cornerRounding, cornerRounding, index > 0, true, false, false);
    
    g.fillPath(path);
    
    g.setColour(juce::Colours::white);
    
    g.setFont (juce::FontOptions (fontSize));
    g.drawText (name, bounds, juce::Justification::centred, true);   // draw some placeholder text
}

void SideMenuHeaderButton::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

float SideMenuHeaderButton::getIdealWidth() {
    auto font = juce::Font(juce::FontOptions(fontSize));
    
    return font.getStringWidthFloat(name) + 2 * paddingX;
}

void SideMenuHeaderButton::mouseDown(const juce::MouseEvent &event)
{
    owner.handleTabChanged(index);
    
}

//==============================================================================
SideMenuHeader::SideMenuHeader() : m_nodeLibraryButton("Node Library", 0, *this), m_inspectorButton("Inspector", 1, *this)
    
{
    addAndMakeVisible(m_nodeLibraryButton);
    m_nodeLibraryButton.pressed = true;
    addAndMakeVisible(m_inspectorButton);
    m_inspectorButton.pressed = false;
}

SideMenuHeader::~SideMenuHeader()
{
}

void SideMenuHeader::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (juce::Colour(0xff35363E));   // clear the background

//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (juce::FontOptions (14.0f));
//    g.drawText ("SideMenuHeader", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void SideMenuHeader::resized()
{
    auto w = m_nodeLibraryButton.getIdealWidth();
    
    m_nodeLibraryButton.setBounds (0, 0, w, getHeight());
    
    m_inspectorButton.setBounds(w, 0, m_inspectorButton.getIdealWidth(), getHeight());
}


void SideMenuHeader::handleTabChanged(int tabIndex)
{
    m_nodeLibraryButton.pressed = tabIndex == 0;
    m_inspectorButton.pressed = tabIndex == 1;
    
    repaint();
    
    onTabChanged(tabIndex);
}

void SideMenuHeader::onTabChanged(int tabIndex) {}
