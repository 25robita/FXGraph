/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SideMenuHeader.h"
#include "DataManager.h"
#include "SideMenu.h"

//==============================================================================
FXGraphAudioProcessorEditor::FXGraphAudioProcessorEditor (FXGraphAudioProcessor& p, std::shared_ptr<DataManager> d)
: AudioProcessorEditor (&p), audioProcessor (p), m_sideMenu(d), m_graphAreaStreams(graphNodes, d)
{
    dataManager = d;
    
    addAndMakeVisible(m_graphAreaNodeContainer);
    for (int nodeId = 0; nodeId < NUM_NODES; nodeId++)
    {
        auto& node = dataManager->activeInstance->nodes[nodeId];
        
        if (!node.isActive) break;
        
        auto* n = new Common::Node();
        
        n->component.reset(new GraphNode(node, draggableArea));
        
        m_graphAreaNodeContainer.addAndMakeVisible(n->component.get());
        n->component->setBounds(node.position.x, node.position.y, node.hasInputSide && node.hasOutputSide ? 300 : 150, n->component->getIdealHeight());
        
        graphNodes.add(n);
        
        
        n->component->onMove = [this] () {
            m_graphAreaStreams.repaint();
        };
        
        n->component->onDataUpdate = [this] () {
//            dataManager->startEditing();
            
            // do the stuff
            
//            dataManager->finishEditing();
        };
        
        n->component->onRemove = [this, nodeId, n] () {
            dataManager->startEditing();
            dataManager->removeNode(nodeId);
            dataManager->finishEditing();
            
            graphNodes.removeObject(n);
            
            m_graphAreaStreams.repaint();
        };
        
        n->component->getCollidingRects = [this] (GraphNode& graphNode, float padding) -> juce::Array<juce::Rectangle<float>>
        {
            auto thisBounds = graphNode.getBounds().toFloat();
            
            juce::Array<juce::Rectangle<float>> out;
            
            for (auto node : graphNodes)
            {
                if (node->component.get() == &graphNode) continue;
                
                auto b = node->component->getBounds().toFloat().expanded(padding);
                
                if (thisBounds.intersects(b))
                    out.add(b);
            }
            
            return out;
        };
    }
    
    addAndMakeVisible(m_graphAreaStreams);
    m_graphAreaStreams.handleSelectStream = [this] (ParameterType type, int streamId) {
        setSelection(type, streamId);
    };
    
    m_graphAreaStreams.handleRemoveStream = [this] () {
        setSelection(); // clear selection
    };
    
    
    addAndMakeVisible(m_sideMenu);
    
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (1000, 600);
    
    // set look and feel
    
    getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff2E2F38));
    getLookAndFeel().setColour(juce::SidePanel::backgroundColour, juce::Colour(0xff42444E));
}

FXGraphAudioProcessorEditor::~FXGraphAudioProcessorEditor()
{
    
}

//==============================================================================
void FXGraphAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background for Graph Area
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    juce::Rectangle<float> backgroundRect = getLocalBounds().toFloat();
    
    g.setColour (juce::Colour (0xff60636C));
    
    const float startX = 2;
    const float intX = 10;
    const float offsetX = intX / 2.0;
    
    const float startY = 2;
    const float intY = 10;
    const float diameter = 2;
    
    bool offset = false;
    
    for (float y = startY; y < backgroundRect.getHeight(); y += intY) {
        offset = !offset;
        for (float x = startX + (offset ? offsetX : 0); x < backgroundRect.getWidth(); x += intX) {
            g.fillEllipse(x, y, diameter, diameter);
        }
    }
    

}

void FXGraphAudioProcessorEditor::resized()
{
    auto globalBounds = getBounds();
    
    draggableArea.setPosition(globalBounds.getX(), globalBounds.getY());
    draggableArea.setSize(globalBounds.getWidth(), globalBounds.getHeight());
    
    if (m_sideMenu.getShown()) {
        draggableArea.setLeft(300); // hardcoded side-panel width
    }
    
    
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto menuBounds = getLocalBounds();
    
    menuBounds.setRight(300 + 50);
    
    m_sideMenu.setBounds(menuBounds);
    
    m_graphAreaStreams.setBounds(getLocalBounds());
    
    m_graphAreaNodeContainer.setBounds(getLocalBounds());
}

void FXGraphAudioProcessorEditor::setSelection(ParameterType type, int streamId)
{
    streamSelected = true;
    nodeSelected = false;
    
    selectedStreamId = streamId;
    selectedStreamType = type;
    
    m_graphAreaStreams.selectStream(type, streamId);
    
    m_sideMenu.getInspector()->setSelection(type, streamId);
    
    /* TODO: at this point:
     [x] the graph area streams must be notified that this stream is selected
     [ ] all nodes, or just the previously selected node, must be notified that they are not selected
     [ ] the inspector must be notified that a stream has been selected for it to display appropriate data
    */
}

void FXGraphAudioProcessorEditor::setSelection(int nodeId)
{
    streamSelected = false;
    nodeSelected = true;
    
    selectedNodeId = nodeId;
    
    m_graphAreaStreams.selectStream();
    
    /* TODO: at this point:
     [x] the graph area streams must be notified that there is no stream selected
     [ ] the selected node must be notified that it must change its appearance
     [ ] all other nodes, or just the previously selected node, must be notified that they are not selected
     [ ] the inspector must be notified that a node has been selected for it to display appropriate data
    */
}


void FXGraphAudioProcessorEditor::setSelection()
{
    streamSelected = false;
    nodeSelected = false;
    
    m_graphAreaStreams.selectStream();
    
    //TODO: tell node that not selected
}
