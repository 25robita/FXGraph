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
        if (stream->path->contains(x, y)) return true; // TODO: exclude param pill rects
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

GraphAreaStreams::Stream* GraphAreaStreams::paintStreamInternal(juce::Graphics& g, juce::Point<float> startPosition, juce::Point<float> endPosition, ParameterType type, bool addStream)
{
    auto streamEl = new Stream;
    
    streamEl->type = type;
//    streamEl->streamId = stream.selfId;
    
    if (type == ParameterType::Audio)
        g.setColour(juce::Colour(0xff4CA2C5));
    else
        g.setColour(juce::Colour(0xffFAA21B));
    
//    auto inputNode = graphNodes[stream.inputNodeId];
//    auto outputNode = graphNodes[stream.outputNodeId];
//    
//    // InputOrOutput is swapped because the stream inputs from the output side of the input node
//    auto start = inputNode->component->getParameterPosition(InputOrOutput::Output, stream.inputParamId);
//    
//    auto end = outputNode->component->getParameterPosition(InputOrOutput::Input, stream.outputParamId);
    
    streamEl->path.reset(createStream({
        startPosition,
        {(startPosition.getX() + endPosition.getX()) / 2.0f, startPosition.getY()},
        {(startPosition.getX() + endPosition.getX()) / 2.0f, endPosition.getY()},
        endPosition
    }));
    
    g.fillPath(*streamEl->path.get());
    
    if (addStream)
        streams.add(streamEl);
    
    return streamEl;
}

void GraphAreaStreams::paintStream(juce::Graphics& g, Data::Stream stream)
{
    auto inputNode = graphNodes[stream.inputNodeId];
    auto outputNode = graphNodes[stream.outputNodeId];
    
    if (inputNode == nullptr || outputNode == nullptr) return;
    
    // InputOrOutput is swapped because the stream inputs from the output side of the input node
    auto start = inputNode->component->getParameterPosition(InputOrOutput::Output, stream.inputParamId);
    
    auto end = outputNode->component->getParameterPosition(InputOrOutput::Input, stream.outputParamId);
    
    auto* streamEl = paintStreamInternal(g, start, end, stream.type);
    
    streamEl->streamId = stream.selfId;
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
    
    if (dragStreamNodeId != -1)
    {
        auto s = paintStreamInternal(g, dragStreamOrigin, dragStreamEndpoint, dragStreamType, false);
        
        delete s;
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
        if (s->streamId != -1 && s->path->contains(event.getPosition().toFloat()))
        {
            stream = s;
            break;
        }
    }
    
    if (stream == nullptr) return;
    
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

void GraphAreaStreams::handleStartDragStream(int nodeId, InputOrOutput inputOrOutput, int paramId)
{
    dragStreamNodeId = nodeId;
    dragStreamInputOrOutput = inputOrOutput;
    dragStreamParamId = paramId;
    dragStreamType = graphNodes[nodeId]->component->getParameterType(dragStreamInputOrOutput, paramId);
    
    // don't need to repaint yet, because the stream won't be going anywhere
    
    dragStreamOrigin = graphNodes[nodeId]->component->getParameterPosition(inputOrOutput, paramId);
}

void GraphAreaStreams::handleDragStreamEnd(juce::Point<float> position)
{
    // check if mouse is over another parameter; if yes, connect the two, overriding and deleting any existing stream connected to the input side of the output node of the new stream if yk what i mean
    // if not, just nodeId = -1 and repaint so it clears the thingo. consider whether if side is input (as in dragging left to right) should it clear the stream? maybe? maybe not? who knows, i mean they can already do it anyway.

    for (auto node : graphNodes)
    {
        if (!node->component->getBounds().contains(position.toInt())) continue;
        
        if (node->component->getBounds().getCentreX() > position.getX()) // check input params
        {
            if (dragStreamInputOrOutput == InputOrOutput::Input)
            { // wait nvm this can't work so we have to do the default which is clear the stream
                goto endloop;
            }
            
            for (auto param : node->component->getInputParams())
            {
                if (!param->component->getPillRect().contains(param->component->getLocalPoint(this, position.toInt()).toFloat())) continue;
                
                auto& paramData = dataManager->activeInstance->nodes[node->component->getNodeId()]->inputParams[param->component->getParamId()];
                
                // ensure they are of the same type
                if (paramData.type != dragStreamType) goto endloop;
                
                dataManager->startEditing();
                
                // first, remove any stream with the remove stream method that connects to the other one's input side
                
                auto currentStreamId = paramData.streamId;
                
                if (currentStreamId != -1)
                {
                    dataManager->removeStream(dragStreamType, currentStreamId);
                }
                
                // then, find the next available streamid
                int id = dataManager->inactiveInstance->getNextStreamId(dragStreamType);
                
                // then, just add to the output and set the input
                if (!dataManager->inactiveInstance->nodes[dragStreamNodeId]->outputParams[dragStreamParamId].addStreamId(id)) 
                {
                    //TODO: handle error!!!!!!
                    DBG("failed to add stream - too many streams on output parameter");
                }
                
                dataManager->inactiveInstance->nodes[node->component->getNodeId()]->inputParams[param->component->getParamId()].streamId = id;
                
                dataManager->finishEditing();
                
//                DBG("we've finished");
                                
                repaint();
            }
        } else
        { // check output params
            if (dragStreamInputOrOutput == InputOrOutput::Output)
            { // wait nvm this can't work so we have to do the default which is clear the stream
                goto endloop;
            }
            
            for (auto param : node->component->getOutputParams())
            {
                if (!param->component->getPillRect().contains(param->component->getLocalPoint(this, position.toInt()).toFloat())) continue;
                
                auto& paramData = dataManager->activeInstance->nodes[node->component->getNodeId()]->outputParams[param->component->getParamId()];
                
                // ensure they are of the same type
                if (paramData.type != dragStreamType) goto endloop;
                
                dataManager->startEditing();
                
                // first, remove any stream with the remove stream method that connects to the other one's input side
                
                auto currentStreamId = dataManager->inactiveInstance->nodes[dragStreamNodeId]->inputParams[dragStreamParamId].streamId;
                
                if (currentStreamId != -1)
                {
                    dataManager->removeStream(dragStreamType, currentStreamId);
                }
                
                // then, find the next available streamid
                int id = dataManager->inactiveInstance->getNextStreamId(dragStreamType);
                
                // then, just add to the output and set the input
                if (!dataManager->inactiveInstance->nodes[node->component->getNodeId()]->outputParams[param->component->getParamId()].addStreamId(id))
                {
                    //TODO: handle error!!!!!!
                    DBG("failed to add stream - too many streams on output parameter");
                }
                
                dataManager->inactiveInstance->nodes[dragStreamNodeId]->inputParams[dragStreamParamId].streamId = id;
                
                dataManager->finishEditing();
                
//                DBG("we've finished");
                                
                repaint();
            }
        }
    }
    
    endloop: // omg its a label and goto statement
    
    // here we clear the stream; hopefully one returns once one has created a stream in the previous loop. one may be an idiot, but that would be their problem.
    
    dragStreamNodeId = -1;
    repaint();
}

void GraphAreaStreams::handleDragStreamMove(juce::Point<float> position)
{
    dragStreamEndpoint = position;
    repaint();
}
