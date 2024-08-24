/*
  ==============================================================================

    NodeLibraryPanel.h
    Created: 24 Jul 2024 8:45:28pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class NodeLibraryPanel  : public juce::Component
{
public:
    NodeLibraryPanel();
    ~NodeLibraryPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeLibraryPanel)
};
