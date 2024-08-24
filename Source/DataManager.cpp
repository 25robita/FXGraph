/*
  ==============================================================================

    DataManager.cpp
    Created: 30 Jul 2024 10:25:17am
    Author:  School

  ==============================================================================
*/
#include <JuceHeader.h>
#include "DataManager.h"


void Data::DataInstance::prepareStreams()
{
    // TODO: this is being called more than necessary
    
    int paramId = 0;
    int nodeId = 0;
    int maxValueStreamId = -1;
    int maxAudioStreamId = -1;
    
    for (int iterNodeId = 0; iterNodeId < NUM_NODES; iterNodeId++)
    {
        Data::Node* node = nodes[iterNodeId];
        if (node == nullptr || !node->isActive) break;
        
        paramId = 0;
        
        for (int iterParamId = 0; iterParamId < NUM_PARAMS; iterParamId ++)
        {
            if (!node->inputParams[iterParamId].isActive || node->inputParams[iterParamId].isConst) break;
            
            if (node->inputParams[iterParamId].streamId == -1) {
                paramId ++;
                continue;
            }
            
            int streamId = node->inputParams[iterParamId].streamId;
                        
            if (node->inputParams[iterParamId].type == ParameterType::Audio)
            {
                audioStreams[streamId].outputNodeId = nodeId;
                audioStreams[streamId].outputParamId = paramId;
                
                if (streamId > maxAudioStreamId) maxAudioStreamId = streamId;
            } else if (node->inputParams[iterParamId].type == ParameterType::Value)
            {
                valueStreams[streamId].outputNodeId = nodeId;
                valueStreams[streamId].outputParamId = paramId;
                
                if (streamId > maxValueStreamId) maxValueStreamId = streamId;
            }
            
            paramId++;
        }
        
        paramId = 0;
        
        for (int iterParamId = 0; iterParamId < NUM_PARAMS; iterParamId ++)
        {
            if (!node->outputParams[iterParamId].isActive) break;
            
            for (int streamId : node->outputParams[iterParamId].streamIds)
            {
                if (streamId == -1) break;
                
                if (node->outputParams[iterParamId].type == ParameterType::Audio)
                {
                    audioStreams[streamId].inputNodeId = nodeId;
                    audioStreams[streamId].inputParamId = paramId;
                } else
                {
                    valueStreams[streamId].inputNodeId = nodeId;
                    valueStreams[streamId].inputParamId = paramId;
                }
            }
            
            paramId++;
        }
        nodeId++;
    }
    
    // clear rest of streams
    
    for (int streamId = maxAudioStreamId + 1; streamId < NUM_AUDIO_STREAMS; streamId++)
    {
        if (audioStreams[streamId].inputNodeId == -1 || audioStreams[streamId].outputNodeId == -1) break;
        
        audioStreams[streamId].inputNodeId = -1;
        audioStreams[streamId].inputParamId = -1;
        audioStreams[streamId].outputNodeId = -1;
        audioStreams[streamId].outputParamId = -1;
    }
    
    for (int streamId = maxValueStreamId + 1; streamId < NUM_VALUE_STREAMS; streamId++)
    {
        if (valueStreams[streamId].inputNodeId == -1 || valueStreams[streamId].outputNodeId == -1) break;
        
        valueStreams[streamId].inputNodeId = -1;
        valueStreams[streamId].inputParamId = -1;
        valueStreams[streamId].outputNodeId = -1;
        valueStreams[streamId].outputParamId = -1;
    }
}

void Data::DataInstance::prepare()
{
    prepareStreams(); // for now just this, but later also include any pre-calc for evaluation of nodes
    
}

void Data::DataInstance::evaluate(int streamId, ParameterType type)
{
    if (streamId == -1) return;
    
    if (type == ParameterType::Audio)
    {
        evaluate(audioStreams[streamId].inputNodeId);
    } else if (type == ParameterType::Value)
    {
        evaluate(valueStreams[streamId].inputNodeId);
    }
    
}

void Data::DataInstance::evaluate(int nodeId)
{
    if (nodeId == -1) return;
    
    Data::Node* node = nodes[nodeId];
    
    for (int i = 0; i < NUM_PARAMS; i++)
    {
        if (node->getType() == NodeType::MainInput) break;
        
        if (!node->inputParams[i].isActive) break;
        
        if (!node->inputParams[i].isConst && node->inputParams[i].streamId != -1)
        {
            evaluate(node->inputParams[i].streamId, node->inputParams[i].type);
        }
    }
    
    // then evaluate internal node logic
    
    switch (node->getType()) {
        case NodeType::MainInput:
        {
            MainInputNode* mainInputNode = static_cast<MainInputNode*>(node);
            
            for (int outputIdId = 0; outputIdId < 8; outputIdId++)
            {
                int outputId = mainInputNode->outputParams[0].streamIds[outputIdId];
                
                if (outputId == -1) break;
                auto* output = &audioStreams[outputId].buffer;
                
                for (int channel = 0; channel < mainInputNode->mainInput->getNumChannels(); channel++)
                {
                    output->copyFrom(channel, 0, *mainInputNode->mainInput, channel, 0, mainInputNode->mainInput->getNumSamples());
                }
            }
            break;
        }
        case NodeType::MainOutput:
            break;
        case NodeType::Gain:
        {
            int inputStreamId = node->inputParams[0].streamId;
            int gainStreamId = node->inputParams[1].streamId;
            
            if (inputStreamId == -1) break; // no point doing anything
            
            float gain;
            
            float prevGain;
            
            if (node->inputParams[1].isConst)
            {
                gain = node->inputParams[1].constValue;
                prevGain = gain; // not much i can think of to do about this really
            } else if (gainStreamId != -1) {
                gain = valueStreams[gainStreamId].value;
                prevGain = valueStreams[gainStreamId].prevValue;
            } else {
                gain = 0;
                prevGain = 0;
            }
            
            gain = juce::Decibels::decibelsToGain(gain);
            prevGain = juce::Decibels::decibelsToGain(prevGain);
            
            auto input = audioStreams[node->inputParams[0].streamId].buffer;
            
            for (int outputId : node->outputParams[0].streamIds)
            {
                if (outputId == -1) break;
                auto* output = &audioStreams[outputId].buffer;
            
                for (int channel = 0; channel < input.getNumChannels(); channel++)
                {   
                    output->copyFrom(channel, 0, input, channel, 0, input.getNumSamples());
                    output->applyGainRamp(channel, 0, output->getNumSamples(), prevGain, gain);
                }
            }
            
            break;
        }
        case NodeType::Level:
        {
            int inputStreamId = node->inputParams[0].streamId;
            
            if (inputStreamId == -1) {
                for (int streamId : node->outputParams[0].streamIds) // lin
                {
                    if (streamId == -1) break;
                    valueStreams[streamId].setValue(0);
                }
                
                for (int streamId : node->outputParams[1].streamIds) // gain
                {
                    if (streamId == -1) break;
                    valueStreams[streamId].setValue(-INFINITY);
                }
                break;
            }
            
            auto input = audioStreams[inputStreamId].buffer;
            
            float total = 0;
            
            for (int channel = 0; channel < input.getNumChannels(); channel++)
            {
                total += input.getRMSLevel(channel, 0, input.getNumSamples());
            }
            
            total /= input.getNumChannels();

            
            for (int streamId : node->outputParams[0].streamIds)
            {
                if (streamId == -1) break;
                valueStreams[streamId].setValue(total);
            }
            
            total = juce::Decibels::gainToDecibels(total);
            
            for (int streamId : node->outputParams[1].streamIds)
            {
                if (streamId == -1) break;
                valueStreams[streamId].setValue(total);
            }
            
            break;
        }
        case NodeType::Correlation:
        {
            int inputStreamId = node->inputParams[0].streamId;
            
            if (inputStreamId == -1) {
                for (int streamId : node->outputParams[0].streamIds)
                {
                    if (streamId == -1) break;
                    valueStreams[streamId].setValue(0);
                }
                break;
            }
            
            auto input = audioStreams[inputStreamId].buffer;
            
            if (input.getNumChannels() != 2)
            {
                for (int streamId : node->outputParams[0].streamIds)
                {
                    if (streamId == -1) break;
                    valueStreams[streamId].setValue(0);
                }
                break;
            }
            
            // then there are deffo 2 channels for stereo correlation
            
            float sumOfProduct = 0.0f;
            float sumOfSquaresLeft = 0.0f;
            float sumOfSquaresRight = 0.0f;
            
            for (int sample = 0; sample < input.getNumSamples(); ++sample)
            {
                float leftChannel = input.getSample(0, sample);
                float rightChannel = input.getSample(1, sample);

                sumOfProduct += leftChannel * rightChannel;
                sumOfSquaresLeft += leftChannel * leftChannel;
                sumOfSquaresRight += rightChannel * rightChannel;
            }
            
            float sumsOfSquares = sumOfSquaresLeft * sumOfSquaresRight;

            float correlation = sumOfProduct / sqrtf(sumsOfSquares);

            
            for (int streamId : node->outputParams[0].streamIds)
            {
                if (streamId == -1) break;
                valueStreams[streamId].setValue(correlation);
            }
            break;
        }
    }
}

void Data::DataInstance::evaluate()
{
    int finalStreamId = nodes[1]->inputParams[0].streamId;
    
    if (finalStreamId != -1)
        evaluate(finalStreamId, ParameterType::Audio);
}

int Data::DataInstance::getNextNodeId()
{
    for (int i = 0; i < NUM_NODES; i++)
    {
        if (nodes[i] == nullptr || !nodes[i]->isActive) return i;
    }
    return -1;
}

int Data::DataInstance::getNextStreamId(ParameterType type)
{
    prepareStreams();
    
    if (type == ParameterType::Audio) {
        for (int i = 0; i < NUM_AUDIO_STREAMS; i++) if (audioStreams[i].inputNodeId == -1 || audioStreams[i].outputNodeId == -1) return i;
    } else if (type == ParameterType::Value) {
        for (int i = 0; i < NUM_VALUE_STREAMS; i++) if (valueStreams[i].inputNodeId == -1 || valueStreams[i].outputNodeId == -1) return i;
    }
    
    return -1;
}


DataManager::DataManager()
{
//    a = new Data::DataInstance;
//    b = new Data::DataInstance;
    a.i = 0;
    b.i = 1;
    activeInstance = &a;
    inactiveInstance = &b;
    
    for (auto instance : {activeInstance, inactiveInstance})
    { // Add global locked nodes for each instance
        addNode(instance, 0, NodeType::MainInput, "Input Node", {300, 300});
        addNode(instance, 1, NodeType::MainOutput, "Output Node", {600, 300});
    }
}

DataManager::~DataManager()
{
    
}

/** Editing methods */

void DataManager::addNode(int index, NodeType type, juce::String name, juce::Point<float> position)
{
    addNode(inactiveInstance, index, type, name, position);
}

void DataManager::addNode(Data::DataInstance* instance, int index, NodeType type, juce::String name, juce::Point<float> position)
{
    
    //TODO: change whole thing
//    auto node = instance->nodes[index];

    
    Data::Node* node;
    
    switch (type) // pre-init streams
    {
        case NodeType::MainInput:
            node = new Data::MainInputNode();
            
            node->isGlobalLockedNode = true;
            node->hasInputSide = false;
            
            node->outputParams[0].isActive = true;
            node->outputParams[0].friendlyName = "In";
            node->outputParams[0].type = ParameterType::Audio;
            break;
        case NodeType::MainOutput:
            node = new Data::MainOutputNode();
            break;
        case NodeType::Gain:
            node = new Data::GainNode();
            break;
        case NodeType::Level:
            node = new Data::LevelNode();
            break;
        case NodeType::Correlation:
            node  = new Data::CorrelationNode();
            break;
    }
    
    if (node != nullptr)
    {
        node->isActive = true;
        node->position = position;
        node->friendlyName = name;
        
        instance->nodes[index] = node;
    }
}

void DataManager::removeStream(ParameterType type, int streamId)
{
    removeStream(inactiveInstance, type, streamId);
}

void DataManager::removeStream(Data::DataInstance* instance, ParameterType type, int streamId)
{
    // TODO: list
    
    // go through every node and remove references to the stream
    // go through every node and bump down the streamId of streams of the same type with a streamId greater than the streamid passed.
    // copy data on each stream object in order to bump down the numbers
    // clear the data on the last stream to ensure it is back to normal, particularly envelope
//    
    for (int nodeId = 0; nodeId < NUM_NODES; nodeId++)
    {
        if (instance->nodes[nodeId] == nullptr) break;
        for (int inputParamId = 0; inputParamId < NUM_PARAMS; inputParamId++)
        {
            if (!instance->nodes[nodeId]->inputParams[inputParamId].isActive) break;
            if (instance->nodes[nodeId]->inputParams[inputParamId].type != type) continue;
            
            if (instance->nodes[nodeId]->inputParams[inputParamId].streamId == streamId)
                instance->nodes[nodeId]->inputParams[inputParamId].streamId = -1;
            else if (instance->nodes[nodeId]->inputParams[inputParamId].streamId > streamId)
                instance->nodes[nodeId]->inputParams[inputParamId].streamId -= 1;
        }
        
        for (int outputParamId = 0; outputParamId < NUM_PARAMS; outputParamId++)
        {
            if (!instance->nodes[nodeId]->outputParams[outputParamId].isActive) break;
            if (instance->nodes[nodeId]->outputParams[outputParamId].type != type) continue;
            
            for (int readPtr = 0, writePtr = 0; writePtr < 8; readPtr++)
            {
                if (readPtr >= 8)
                {
                    instance->nodes[nodeId]->outputParams[outputParamId].streamIds[writePtr] = -1;
                    writePtr++;
                    continue;
                }
                
                if (instance->nodes[nodeId]->outputParams[outputParamId].streamIds[readPtr] == streamId)
                    continue;
                
                if (instance->nodes[nodeId]->outputParams[outputParamId].streamIds[readPtr] > streamId)
                    instance->nodes[nodeId]->outputParams[outputParamId].streamIds[writePtr] = instance->nodes[nodeId]->outputParams[outputParamId].streamIds[readPtr] - 1;
                else
                    instance->nodes[nodeId]->outputParams[outputParamId].streamIds[writePtr] = instance->nodes[nodeId]->outputParams[outputParamId].streamIds[readPtr];
                
                writePtr++;
            }
        }
    }
    
    
    if (type == ParameterType::Audio)
    {
        for (int i = streamId; i < NUM_AUDIO_STREAMS - 1; i++)
        {
            
            for (int channel = 0; channel < instance->audioStreams[i].buffer.getNumChannels(); channel++)
            {
                instance->audioStreams[i].buffer.copyFrom(channel, 0, instance->audioStreams[i + 1].buffer, channel, 0, instance->audioStreams[i].buffer.getNumSamples());
            }
        }
        
        instance->audioStreams[NUM_AUDIO_STREAMS - 1].buffer.clear();
        
    } else if (type == ParameterType::Value)
    {
        for (int i = streamId; i < NUM_VALUE_STREAMS - 1; i++)
        {
            instance->valueStreams[i].copyFrom(&instance->valueStreams[i + 1]);
        }
        
        instance->valueStreams[NUM_VALUE_STREAMS - 1].unset();
        instance->valueStreams[NUM_VALUE_STREAMS - 1].envelope.setMsAttack(35);
        instance->valueStreams[NUM_VALUE_STREAMS - 1].envelope.setMsRelease(35);
    }
    
    instance->prepareStreams();
}


void DataManager::removeNode(int nodeId)
{
    removeNode(inactiveInstance, nodeId);
}

void DataManager::removeNode(Data::DataInstance *instance, int nodeId)
{
    // remove all linked streams
    
    for (int paramId = 0; paramId < NUM_PARAMS; paramId++)
    {
        if (!instance->nodes[nodeId]->inputParams[paramId].isActive) break;
        
        int streamId = instance->nodes[nodeId]->inputParams[paramId].streamId;
        if (streamId == -1) continue;
        
        removeStream(instance->nodes[nodeId]->inputParams[paramId].type, streamId);
    }
    
    for (int paramId = 0; paramId < NUM_PARAMS; paramId++)
    {
        if (!instance->nodes[nodeId]->outputParams[paramId].isActive) break;
        
        for (int streamIdId = 0; streamIdId < 8; streamIdId++)
        {
            int streamId = instance->nodes[nodeId]->outputParams[paramId].streamIds[streamIdId];
            if (streamId == -1) break;
            
            removeStream(instance->nodes[nodeId]->outputParams[paramId].type, streamId);
        }
    }
    
    // shift nodes
    for (int iterNodeId = nodeId; iterNodeId < NUM_NODES - 1; iterNodeId++)
    {
        instance->nodes[iterNodeId] = instance->nodes[iterNodeId + 1]; // should maybe be fine with nullptrs? who knows tbh
//        instance->nodes[iterNodeId]->copyFrom(instance->nodes[iterNodeId + 1]);
    }
    
    // TODO: clear final node
    
    
    
    instance->prepare();
}

/** Reading methods*/

Data::MainOutputNode* DataManager::getOutputNode(Data::DataInstance* instance)
{
    return static_cast<Data::MainOutputNode*>(instance->nodes[1]);
}

Data::MainOutputNode* DataManager::getOutputNode()
{
    return getOutputNode(activeInstance);
}


Data::MainInputNode* DataManager::getInputNode(Data::DataInstance* instance)
{
    return static_cast<Data::MainInputNode*>(instance->nodes[0]);
}

Data::MainInputNode* DataManager::getInputNode()
{
    return getInputNode(activeInstance);
}



/** Management methods */

void DataManager::startEditing()
{
    if (editing) return;
    
    editing = true;
    
    if (changeQueued) {
        changeQueued = false; // no need to copy, just edit current inactive instance
        return;
    }
    
    // copy data from activeInstance to inactiveInstance
    
    for (int i = 0; i < NUM_NODES; i++)
    {
        if (activeInstance->nodes[i] == nullptr)
        {
            inactiveInstance->nodes[i] = nullptr;
            continue;
        }
        inactiveInstance->nodes[i] = activeInstance->nodes[i]->getCopy();
        
//        ->copyFrom();
    }
    
    for (int i = 0; i < NUM_AUDIO_STREAMS; i++)
    {
        inactiveInstance->audioStreams[i].inputNodeId = activeInstance->audioStreams[i].inputNodeId;
        inactiveInstance->audioStreams[i].inputParamId = activeInstance->audioStreams[i].inputParamId;
        inactiveInstance->audioStreams[i].outputNodeId = activeInstance->audioStreams[i].outputNodeId;
        inactiveInstance->audioStreams[i].outputParamId = activeInstance->audioStreams[i].outputParamId;
        
        inactiveInstance->audioStreams[i].buffer.clear();
    }
    
    for (int i = 0; i < NUM_VALUE_STREAMS; i++)
    {
        inactiveInstance->valueStreams[i].inputNodeId = activeInstance->valueStreams[i].inputNodeId;
        inactiveInstance->valueStreams[i].inputParamId = activeInstance->valueStreams[i].inputParamId;
        inactiveInstance->valueStreams[i].outputNodeId = activeInstance->valueStreams[i].outputNodeId;
        inactiveInstance->valueStreams[i].outputParamId = activeInstance->valueStreams[i].outputParamId;
        
        inactiveInstance->valueStreams[i].unset();
        
        inactiveInstance->valueStreams[i].envelope = Envelope(activeInstance->valueStreams[i].envelope);
    }
    
}

void DataManager::finishEditing()
{
    editing = false;
    
    changeQueued = true;
}

void DataManager::realise()
{
    if (!changeQueued) return;
    
    changeQueued = false;
    
    if (activeInstance->i == a.i)
    {
        activeInstance = &b;
        inactiveInstance = &a;
    } else {
        activeInstance = &a;
        inactiveInstance = &b;
    }
}

