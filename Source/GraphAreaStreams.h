/*
  ==============================================================================

    GraphAreaStreams.h
    Created: 3 Aug 2024 10:21:30am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GraphNode.h"
#include "DataManager.h"
#include "Common.h"

//==============================================================================
/*
*/
class GraphAreaStreams  : public juce::Component
{
public:
    
    
    GraphAreaStreams(juce::OwnedArray<Common::Node>& nodes, std::shared_ptr<DataManager> d);
    ~GraphAreaStreams() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool hitTest(int x, int y) override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
   
    void selectStream(); // clears selection
    void selectStream(ParameterType type, int streamId);
    
    void handleStartDragStream(int nodeId, InputOrOutput inputOrOutuput, int paramId);
    void handleDragStreamMove(juce::Point<float> position);
    void handleDragStreamEnd(juce::Point<float> position);
    
    std::function<void(ParameterType type, int streamId)> handleSelectStream;
    std::function<void()> handleRemoveStream;
    
private:
    enum Direction
    {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3
    };
    
    struct Stream
    {
        std::unique_ptr<juce::Path> path;
        int streamId;
        ParameterType type;
    };
    
    juce::Path* createStream(juce::Array<juce::Point<float>> anchorPoints);
    void paintStream(juce::Graphics& g, Data::Stream stream);
    Stream* paintStreamInternal(juce::Graphics& g, juce::Point<float> startPosition, juce::Point<float> endPosition, ParameterType type);
    juce::Rectangle<float> getStreamRectangle(Direction direction, juce::Point<float> start, juce::Point<float> end, float thickness);
    
    juce::OwnedArray<Common::Node>& graphNodes;
    std::shared_ptr<DataManager> dataManager;
    
    juce::OwnedArray<Stream> streams;
    
    ParameterType selectedStreamType;
    int selectedStreamId;
    bool streamSelected;
    
    int dragStreamNodeId = -1;
    int dragStreamParamId;
    InputOrOutput dragStreamInputOrOutput;
    ParameterType dragStreamType;
    juce::Point<float> dragStreamOrigin;
    juce::Point<float> dragStreamEndpoint;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphAreaStreams)
};
