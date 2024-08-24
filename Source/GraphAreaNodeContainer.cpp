/*
  ==============================================================================

    GraphAreaNodeContainer.cpp
    Created: 6 Aug 2024 9:41:45am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GraphAreaNodeContainer.h"

//==============================================================================

/**
 Handles the transformations for the graph area for zooming and panning. All logic should ideally be implemented on the PluginEditor
 */
GraphAreaNodeContainer::GraphAreaNodeContainer()
{
//    setTransform(juce::AffineTransform().scale(0.5f));
}

GraphAreaNodeContainer::~GraphAreaNodeContainer()
{
}

void GraphAreaNodeContainer::paint (juce::Graphics& g)
{
}

void GraphAreaNodeContainer::resized()
{
}
