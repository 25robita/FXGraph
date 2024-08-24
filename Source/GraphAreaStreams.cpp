/*
  ==============================================================================

    GraphAreaStreams.cpp
    Created: 3 Aug 2024 10:21:30am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GraphAreaStreams.h"

//==============================================================================
GraphAreaStreams::GraphAreaStreams(juce::OwnedArray<Common::Node>& nodes, std::shared_ptr<DataManager> d)
 : graphNodes(nodes)
{
    dataManager = d;
    
}

GraphAreaStreams::~GraphAreaStreams()
{
}

bool GraphAreaStreams::hitTest(int x, int y)
{
    for (auto stream : streams)
    {
        if (stream->path->contains(x, y)) return true;
    }
    return false;
}

juce::Rectangle<float> GraphAreaStreams::getStreamRectangle(Direction direction, juce::Point<float> start, juce::Point<float> end, float thickness)
{
    
    float x, y, width, height;
    switch (direction) {
        case Direction::Right:
            x = start.getX();
            y = start.getY() - thickness / 2.0f;
            width = end.getX() - start.getX() + thickness / 2.0f;
            height = thickness;
            break;
            
        case Direction::Left:
            x = end.getX() - thickness / 2.0f;
            y = end.getY() - thickness / 2.0f;
            width = start.getX() - end.getX() + thickness / 2.0f;
            height = thickness;
            break;
            
        case Direction::Up:
            x = end.getX() - thickness / 2.0f;
            y = end.getY() - thickness / 2.0f;
            width = thickness;
            height = start.getY() - end.getY() + thickness / 2.0f;
            break;
            
        case Direction::Down:
            x = start.getX() - thickness / 2.0f;
            y = start.getY();
            width = thickness;
            height = end.getY() - start.getY() + thickness / 2.0f;
            break;
    }
    
    return {x, y, width, height};
}

juce::Path* GraphAreaStreams::createStream(juce::Array<juce::Point<float>> anchorPoints)
{
    const float pillHeight = 14;
    const float cornerSize = pillHeight / 2.0f;
    
    auto out = new juce::Path();
    
    for (int a = 0, b = 1, c = 2; c < anchorPoints.size(); a = (b = c++) - 1)
    {
        auto start = anchorPoints[a];
        auto end = anchorPoints[b];
        auto future = anchorPoints[c];
        
        Direction lineDir = start.getX() < end.getX() ? Direction::Right : start.getX() > end.getX() ? Direction::Left : start.getY() > end.getY() ? Direction::Up : Direction::Down;
        
        Direction nextDir = end.getX() < future.getX() ? Direction::Right : end.getX() > future.getX() ? Direction::Left : end.getY() > future.getY() ? Direction::Up : Direction::Down;
        
        auto r = getStreamRectangle(lineDir, start, end, pillHeight);
        
        out->addRoundedRectangle(
            r.getX(),
            r.getY(),
            r.getWidth(),
            r.getHeight(),
            cornerSize,
            cornerSize,
            (lineDir == Direction::Left && nextDir == Direction::Down) || (lineDir == Direction::Up && nextDir == Direction::Right),
            (lineDir == Direction::Right && nextDir == Direction::Down) || (lineDir == Direction::Up && nextDir == Direction::Left),
            (lineDir == Direction::Left && nextDir == Direction::Up) || (lineDir == Direction::Down && nextDir == Direction::Right),
            (lineDir == Direction::Right && nextDir == Direction::Up) || (lineDir == Direction::Down && nextDir == Direction::Left)
        );
    }
    
    // then draw final connection
    
    auto start = anchorPoints[anchorPoints.size()-2];
    auto end = anchorPoints[anchorPoints.size()-1];
    
    Direction lineDir = start.getX() > end.getX() ? Direction::Right : start.getX() < end.getX() ? Direction::Left : start.getY() > end.getY() ? Direction::Up : Direction::Down;
    
    auto r2 = getStreamRectangle(lineDir, start, end, pillHeight);
    
    out->addRoundedRectangle(r2.translated((lineDir == Direction::Right ? -1 : 1) * pillHeight / 2.0f, 0), 0);
    
    return out;
}

void GraphAreaStreams::paintStream(juce::Graphics& g, Data::Stream stream)
{
    auto streamEl = new Stream;
    
    streamEl->type = stream.type;
    streamEl->streamId = stream.selfId;
    
    if (stream.type == ParameterType::Audio)
        g.setColour(juce::Colour(0xff4CA2C5));
    else
        g.setColour(juce::Colour(0xffFAA21B));
    
    auto inputNode = graphNodes[stream.inputNodeId];
    auto outputNode = graphNodes[stream.outputNodeId];
    
    // InputOrOutput is swapped because the stream inputs from the output side of the input node
    auto start = inputNode->component->getParameterPosition(InputOrOutput::Output, stream.inputParamId);
    
    auto end = outputNode->component->getParameterPosition(InputOrOutput::Input, stream.outputParamId);
    
    streamEl->path.reset(createStream({
        start,
        {(start.getX() + end.getX()) / 2.0f, start.getY()},
        {(start.getX() + end.getX()) / 2.0f, end.getY()},
        end
    }));
    
    g.fillPath(*streamEl->path.get());
    
    streams.add(streamEl);
}

void GraphAreaStreams::paint (juce::Graphics& g)
{
    dataManager->activeInstance->prepareStreams();
    streams.clear();

    for (auto stream : dataManager->activeInstance->audioStreams)
    {
        if (stream.inputNodeId == -1 || stream.outputNodeId == -1) break;
        
        paintStream(g, stream);
    }
    
    for (auto stream : dataManager->activeInstance->valueStreams)
    {
        if (stream.inputNodeId == -1 || stream.outputNodeId == -1) break;
        
        paintStream(g, stream);
    }
    
    
    if (streamSelected)
    {
        auto shadow = juce::DropShadow(juce::Colour(0xa0cccccc), 40, {0, 0});

        for (auto stream : streams)
        {
            if (selectedStreamId == stream->streamId && selectedStreamType == stream->type)
            {
                shadow.drawForPath(g, *stream->path);
                break;
            }
        }
    }
}

void GraphAreaStreams::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void GraphAreaStreams::mouseDown(const juce::MouseEvent &event)
{
    // get clicked stream
    Stream* stream;
    for (auto s : streams) {
        if (s->path->contains(event.getPosition().toFloat()))
        {
            stream = s;
            break;
        }
    }
    
    // give to parent to handle
    handleSelectStream(stream->type, stream->streamId);
}

void GraphAreaStreams::mouseDoubleClick(const juce::MouseEvent &event)
{
    Stream* stream;
    for (auto s : streams) {
        if (s->path->contains(event.getPosition().toFloat()))
        {
            stream = s;
            break;
        }
    }
    
    dataManager->startEditing();
    dataManager->removeStream(stream->type, stream->streamId);
    dataManager->finishEditing();
    
    handleRemoveStream();
    
    repaint();
}


void GraphAreaStreams::selectStream()
{
    streamSelected = false;
    
    repaint();
}

void GraphAreaStreams::selectStream(ParameterType type, int streamId)
{
    streamSelected = true;
    selectedStreamId = streamId;
    selectedStreamType = type;
    
    repaint();
}
