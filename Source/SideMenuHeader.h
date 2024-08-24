/*
  ==============================================================================

    SideMenuHeader.h
    Created: 23 Jul 2024 10:16:13am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

class SideMenuHeader;

class SideMenuHeaderButton  : public juce::Component
{
public:
    SideMenuHeaderButton(const juce::String& buttonName, int index, SideMenuHeader& parent);
    ~SideMenuHeaderButton() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    
    float getIdealWidth();
    
    juce::String name;
    
    int index;
    
    bool pressed = false;
    
protected:
    friend class SideMenuHeader;
    SideMenuHeader& owner;
    
private:
    const float fontSize = 14.0f;
    const float paddingX = 20.0f;
    const float cornerRounding = 5.0f;
    
    const juce::Colour pressedBackground = juce::Colour(0xff3E3F48);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideMenuHeaderButton)
};


class SideMenuHeader  : public juce::Component
{
public:
    SideMenuHeader();
    ~SideMenuHeader() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    virtual void onTabChanged(int tabIndex);
    
    void handleTabChanged(int tabIndex);

private:
    SideMenuHeaderButton m_nodeLibraryButton;
    SideMenuHeaderButton m_inspectorButton;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideMenuHeader)
};
