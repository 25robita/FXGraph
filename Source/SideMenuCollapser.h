/*
  ==============================================================================

    SideMenuCollapser.h
    Created: 23 Jul 2024 10:26:09am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SideMenuCollapser  : public juce::Component
{
public:
    SideMenuCollapser();
    ~SideMenuCollapser() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    
    std::function<void()> onUpdate;
    
    bool isRight;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideMenuCollapser)
};
