/*
  ==============================================================================

    GraphAreaNodeContainer.h
    Created: 6 Aug 2024 9:41:45am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class GraphAreaNodeContainer  : public juce::Component
{
public:
    GraphAreaNodeContainer();
    ~GraphAreaNodeContainer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphAreaNodeContainer)
};
