/*
  ==============================================================================

    NodeLibraryPanel.h
    Created: 24 Jul 2024 8:45:28pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "NodeLibraryNode.h"

//==============================================================================
/*
*/
class NodeLibraryPanel  : public juce::Component
{
public:
    NodeLibraryPanel(std::shared_ptr<DataManager> d);
    ~NodeLibraryPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    std::function<void()> onNodeAdded;
private:
    juce::OwnedArray<NodeLibraryNode> nodes;
    std::shared_ptr<DataManager> dataManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NodeLibraryPanel)
};
