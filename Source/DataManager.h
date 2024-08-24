/*
  ==============================================================================

    DataManager.h
    Created: 30 Jul 2024 10:25:17am
    Author:  School

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "Envelope.h"

enum ParameterType
{
    Audio,
    Value
};

enum NodeType
{
    MainInput = 0,
    MainOutput = 1,
    Gain = 2,
    Level = 3,
    Correlation = 4,
};

const int NUM_NODES = 64;
const int NUM_AUDIO_STREAMS = 128;
const int NUM_VALUE_STREAMS = 128;
const int NUM_PARAMS = 16;
const int NUM_OUTPUT_STREAMS = 8;

namespace Data
{
struct Stream;
struct Node;

struct Parameter
{
    bool isActive = false;
    
    ParameterType type;
    
    juce::String friendlyName;
    juce::String name;
    
    Parameter() : friendlyName(), name()
    {
        
    }
    
    ~Parameter()
    {
    }
};

struct InputParameter : Parameter {
    bool isConst = false;
    float constValue;
    int streamId = -1;
    
    void copyFrom(InputParameter other)
    {
        isActive = other.isActive;
        type = other.type;
        friendlyName = other.friendlyName;
        name = other.name;
        
        isConst = other.isConst;
        constValue = other.constValue;
        streamId = other.streamId;
    }
};

struct OutputParameter : Parameter {
    int streamIds[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    
    void copyFrom(OutputParameter other)
    {
        isActive = other.isActive;
        type = other.type;
//        friendlyName = other->friendlyName;
//        name = other->name;
        
        for (int i = 0; i < 8; i ++)
        {
            streamIds[i] = other.streamIds[i];
        }
    }
};

struct Stream {
    int inputNodeId = -1;
    int inputParamId = -1;
    int outputNodeId = -1;
    int outputParamId = -1;
    
    int selfId;
    
//    Node* inputNode;
//    Node* outputNode;
    
    const ParameterType type;
    
    bool computed; // not used (yet?)
    
    void clear()
    {
        computed = false;
    }
    
    Stream(ParameterType t) : type(t) {};
};

class Node
{
public:
    Node() {};
    Node(juce::String& name) { friendlyName = name; };
    virtual ~Node() {};
    
    bool isActive = false; // TODO: this doesn't seem necessary now that inactive nodes are nullptr?
    InputParameter inputParams[NUM_PARAMS];
    OutputParameter outputParams[NUM_PARAMS];
    juce::String friendlyName;
    juce::Point<float> position;
    
    virtual NodeType getType() = 0;
    virtual Node* getCopy() = 0;
    
    bool isGlobalLockedNode = false;
    bool hasInputSide = true;
    bool hasOutputSide = true;
};

class MainInputNode : public Node
{
public:
    MainInputNode() 
    {
        isGlobalLockedNode = true;
        hasInputSide = false;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "In";
        outputParams[0].type = ParameterType::Audio;
    }
    
    NodeType getType() {return NodeType::MainInput;}
    Node* getCopy() {return new MainInputNode(*this);}
};

class MainOutputNode : public Node
{
public:
    MainOutputNode()
    {
        isGlobalLockedNode = true;
        hasOutputSide = false;
        
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "Out";
        inputParams[0].type = ParameterType::Audio;
    }
    
    NodeType getType() {return NodeType::MainOutput;}
    Node* getCopy() {return new MainOutputNode(*this);}
};

class GainNode : public Node
{
public:
    GainNode()
    {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        inputParams[1].isActive = true;
        inputParams[1].friendlyName = "Gain";
        inputParams[1].type = ParameterType::Value;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Out";
        outputParams[0].type = ParameterType::Audio;
    }
    
    NodeType getType() {return NodeType::Gain;}
    Node* getCopy() {return new GainNode(*this);}
};

class LevelNode : public Node
{
public:
    LevelNode()
    {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Float";
        outputParams[0].type = ParameterType::Value;
        
        outputParams[1].isActive = true;
        outputParams[1].friendlyName = "dbFS";
        outputParams[1].type = ParameterType::Value;
    }
    
    NodeType getType() {return NodeType::Level;}
    Node* getCopy() {return new LevelNode(*this);}
};

class CorrelationNode : public Node
{
public:
    CorrelationNode()
    {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Correlation";
        outputParams[0].type = ParameterType::Value;
    }
    
    NodeType getType() {return NodeType::Correlation;}
    Node* getCopy() {return new CorrelationNode(*this);}
};

//struct Node
//{
//    bool isActive = false;
//    InputParameter inputParams[NUM_PARAMS];
//    OutputParameter outputParams[NUM_PARAMS];
//    juce::String friendlyName;
//    juce::Point<float> position;
//    NodeType type;
//    // TODO: add thingo for when external VST, like path? or VST id, idk how this stuff works yet
//    
//    bool isGlobalLockedNode = false; // used for global input and output
//    bool hasInputSide = true;
//    bool hasOutputSide = true;
//    
//    Node() { };
//    
//    Node(juce::String& name)
//    {
//        friendlyName = name;
//    };
//    
////    void evaluate();
//    
//    void whattype() {DBG(type);};
//    
//    void copyFrom(Node other)
//    {
//        isActive = other.isActive;
//        friendlyName = other.friendlyName;
//        position = other.position;
//        type = other.type;
//        
//        isGlobalLockedNode = other.isGlobalLockedNode;
//        hasInputSide = other.hasInputSide;
//        hasOutputSide = other.hasOutputSide;
//        
//        for (int i = 0; i < NUM_PARAMS; i++)
//        {
//            inputParams[i].copyFrom(other.inputParams[i]);
//            outputParams[i].copyFrom(other.outputParams[i]);
//        }
//    }
//};



struct AudioStream : Stream {
    juce::AudioBuffer<float> buffer;
    
    AudioStream() : Stream(ParameterType::Audio) {};
    
};

struct ValueStream : Stream {
    float value;
    float prevValue;
    
    Envelope envelope;
    
    bool hasBeenSet = false;
    
    void setValue(float v)
    {
        if (isnan(v))
        {
            unset();
            return;
        }
        if (hasBeenSet)
        {
            prevValue = value;
            envelope.run(v, value);
        } else {
            hasBeenSet = true;
            value = prevValue = v;
        }
    }
    
    void unset()
    {
        hasBeenSet = false;
    }
    
    void copyFrom(ValueStream* v)
    {
        value = v->value;
        prevValue = v->prevValue;
        hasBeenSet = v->hasBeenSet;
        
        envelope.setMsAttack(v->envelope.getMsAttack());
        envelope.setMsRelease(v->envelope.getMsRelease());
    }
    
    ValueStream() : Stream(ParameterType::Value) {
        envelope = Envelope(35, 35, 44100.0);
    };
};

struct DataInstance
{
    int i;
    Node* nodes[NUM_NODES]; // could be vector, but probably not for the best
    AudioStream audioStreams[NUM_AUDIO_STREAMS];
    ValueStream valueStreams[NUM_VALUE_STREAMS];
    
    DataInstance()
    {
        for (int i = 0; i < NUM_AUDIO_STREAMS; i++)
            audioStreams[i].selfId = i;
        
        for (int i = 0; i < NUM_VALUE_STREAMS; i++)
            valueStreams[i].selfId = i;
        
        for (int i = 0; i < NUM_NODES; i++)
            nodes[i] = nullptr; // maybe?
    }
    
    void prepareStreams();
    void prepare();
    
    void evaluate();
    void evaluate(int streamId, ParameterType type);
    void evaluate(int nodeId);
    
    int getNextNodeId();
    int getNextStreamId(ParameterType type);
    
    juce::AudioBuffer<float>* tempInpt;
};
}


/**
 The DataManager must be owned by the PluginProcessor (so that it can handle saving and loading plugin state) and passed as a reference to the PluginEditor constructor.
 The DataManager must contain two Data Instances, switching between them atomically with a pointer. The DataManager should probably also manage the state changes such that they only occur at safe intervals, such as at the very start or end of an audio chunk in the processor. This can be done by having a DataManager::realise() method which realises all changes that have been cued. This would be called on the PluginProcessor. The PluginEditor would call a DataManager::finishEditing() method to cue the changes.
 There should be no way to edit the currently active Data Instance, and it should instead (if it receives two edits before a realise) re-edit the inactive Data Instance and un-cue the previously selected changes. Thus, the Data Instances should be privatised to the DataManager.
 There should be a DataManager::startEditing() method to 'lock' the inactive Data Instance and ensure that all cued modifications are cancelled.
 There should be methods to edit the current inactive Data Instance, perhaps just by using a pointer for the inactive Data Instance as well as the active one, or perhaps through specific methods for each possible edit. This is more effort, but likely safer. Careful programming mitigates this as well.
 */
class DataManager
{
public:
    DataManager();
    ~DataManager();
    
    void addNode(int index, NodeType type, juce::String name, juce::Point<float> position);
    void addNode(Data::DataInstance* instance, int index, NodeType type, juce::String name, juce::Point<float> position);
    
    void removeStream(ParameterType type, int streamid);
    void removeStream(Data::DataInstance* instance, ParameterType type, int streamid);
    
    void removeNode(int nodeId);
    void removeNode(Data::DataInstance* instance, int nodeId);
    
    Data::Node* getOutputNode(Data::DataInstance* instance);
    Data::Node* getOutputNode();
    
    Data::Node* getInputNode(Data::DataInstance* instance);
    Data::Node* getInputNode();
    
    Data::DataInstance* activeInstance;
    Data::DataInstance* inactiveInstance;
    
    void startEditing();
    void finishEditing();
    void realise();
    
    bool isEditing() {return editing;};
    
private:
    Data::DataInstance a;
    Data::DataInstance b;
    
    bool editing;
    bool changeQueued;
};
