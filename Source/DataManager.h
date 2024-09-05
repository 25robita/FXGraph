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
#include "LUFSMeter/Ebu128LoudnessMeter.h"
#include "exprtk/exprtk.hpp"

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
    Loudness = 5,
    Maths = 6,
};

/**
 
 TODO: add new node type, Maths
 - using ExprTK
 - each MathsNode will have one ExprTK expression<float>, and one value output
 - through the inspector, the user will be able to add and name inputs how they wish (up to 16 ofc), as well as altering the expression text, which is stored as a string
 - syntax highlighting??
 - this expression stuff will all only be evaluated on change to the number of params or their names; else they will stay compiled (or however that works) for high efficiency
 
 */

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
    
    virtual ~Parameter() { }
    
    virtual juce::XmlElement* serialise(int i) = 0;
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
    
    juce::XmlElement* serialise(int i) override
    {
        auto output = new juce::XmlElement("inputParam");
        
        auto friendlyNameElement = new juce::XmlElement("friendlyName");
        friendlyNameElement->addTextElement(juce::String(friendlyName));

        auto nameElement = new juce::XmlElement("name");
        nameElement->addTextElement(juce::String(name));
        
        auto typeElement = new juce::XmlElement("type");
        typeElement->addTextElement(type == ParameterType::Audio ? "audio" : "value");
        
        if (streamId != -1)
        {
            auto streamIdElement = new juce::XmlElement("streamId");
            streamIdElement->addTextElement(juce::String(streamId));
            
            output->addChildElement(streamIdElement);
        }
        
        
        if (isConst)
        {
            auto constValueElement = new juce::XmlElement("constValue");
            constValueElement->addTextElement(juce::String(constValue));
            
            output->addChildElement(constValueElement); // it knows if its const if it has a constValue thingo
        }
        
        
        auto paramIdElement = new juce::XmlElement("paramId");
        paramIdElement->addTextElement(juce::String(i));
        
        output->addChildElement(friendlyNameElement);
        output->addChildElement(nameElement);
        output->addChildElement(typeElement);
        output->addChildElement(paramIdElement);
        
        return output;
    }
    
    void deserialise(juce::XmlElement* elem)
    {
        auto friendlyNameElement = elem->getChildByName("friendlyName");
        friendlyName = friendlyNameElement->getAllSubText();
    
        auto nameElement = elem->getChildByName("name");
        name = nameElement->getAllSubText();
        
        auto typeElement = elem->getChildByName("type");
        type = typeElement->getAllSubText() == "audio" ? ParameterType::Audio : ParameterType::Value;
        
        juce::XmlElement* relevantChild = nullptr;
        
        if ((relevantChild = elem->getChildByName("streamId")) != nullptr)
        {
            isConst = false;
            
            streamId = relevantChild->getAllSubText().getIntValue();
        } else if ((relevantChild = elem->getChildByName("constValue")) != nullptr) {
            isConst = true;
            streamId = -1;
            constValue = relevantChild->getAllSubText().getFloatValue();
        }
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
    
    bool addStreamId(int streamId) // returns success
    {
        for (int i = 0; i < 8; i++)
        {
            if (streamIds[i] != -1) continue;
            
            streamIds[i] = streamId;
            return true;
        }
        
        return false;
    }
    
    juce::XmlElement* serialise(int i) override
    {
        auto output = new juce::XmlElement("outputParam");
        
        auto friendlyNameElement = new juce::XmlElement("friendlyName");
        friendlyNameElement->addTextElement(juce::String(friendlyName));

        auto nameElement = new juce::XmlElement("name");
        nameElement->addTextElement(juce::String(name));
        
        auto typeElement = new juce::XmlElement("type");
        typeElement->addTextElement(type == ParameterType::Audio ? "audio" : "value");
        
        auto streamIdsElement = new juce::XmlElement("streamIds");
        
        for (int i = 0; i < 8; i++)
        {
            if (streamIds[i] == -1) continue;
            
            auto elem = new juce::XmlElement("streamId");
            elem->addTextElement(juce::String(streamIds[i]));
            
            streamIdsElement->addChildElement(elem);
        }
        
        auto paramIdElement = new juce::XmlElement("paramId");
        paramIdElement->addTextElement(juce::String(i));
        
        output->addChildElement(friendlyNameElement);
        output->addChildElement(nameElement);
        output->addChildElement(typeElement);
        output->addChildElement(streamIdsElement);
        output->addChildElement(paramIdElement);
        
        return output;
    }
    
    void deserialise(juce::XmlElement* elem)
    {
        auto friendlyNameElement = elem->getChildByName("friendlyName");
        friendlyName = friendlyNameElement->getAllSubText();
    
        auto nameElement = elem->getChildByName("name");
        name = nameElement->getAllSubText();
        
        auto typeElement = elem->getChildByName("type");
        type = typeElement->getAllSubText() == "audio" ? ParameterType::Audio : ParameterType::Value;
        
        
        int streamIdId = 0;
        auto streamIdsElement = elem->getChildByName("streamIds");
        for (auto child : streamIdsElement->getChildIterator())
        {
            streamIds[streamIdId++] = child->getAllSubText().getIntValue();
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
    Node(juce::XmlElement* elem)
    {
        if (elem == nullptr) return;
        
        isActive = true;
//
//        for (int i = 0; i < NUM_PARAMS; i++)
//        {
//            if (!inputParams[i].isActive) break;
//            output->addChildElement(inputParams[i].serialise(i));
//        }
//        
//        
//        for (int i = 0; i < NUM_PARAMS; i++)
//        {
//            if (!outputParams[i].isActive) break;
//            output->addChildElement(outputParams[i].serialise(i));
//        }
        
        auto isGlobalLockedNodeElement = elem->getChildByName("isGlobalLockedNode");
        isGlobalLockedNode = isGlobalLockedNodeElement->getAllSubText() == "true";
        
        auto hasInputSideElement = elem->getChildByName("hasInputSide");
        hasInputSide = hasInputSideElement->getAllSubText() == "true";
        
        auto hasOutputSideElement = elem->getChildByName("hasOutputSide");
        hasOutputSide = hasOutputSideElement->getAllSubText() == "true";
        
        auto friendlyNameElement = elem->getChildByName("friendlyName");
        friendlyName = friendlyNameElement->getAllSubText();
        
        auto positionElement = elem->getChildByName("position");
        auto xElement = positionElement->getChildByName("x");
        auto yElement = positionElement->getChildByName("y");
        
        position.setX(xElement->getAllSubText().getFloatValue());
        position.setY(yElement->getAllSubText().getFloatValue());
        
        for (auto inputParamElement : elem->getChildWithTagNameIterator("inputParam"))
        {
            inputParams[inputParamElement->getChildByName("paramId")->getAllSubText().getIntValue()].deserialise(inputParamElement);
        }
        
        for (auto outputParamElement : elem->getChildWithTagNameIterator("outputParam"))
        {
            outputParams[outputParamElement->getChildByName("paramId")->getAllSubText().getIntValue()].deserialise(outputParamElement);
        }
    }
    virtual ~Node() {};
    
    bool isActive = false; // TODO: this doesn't seem necessary now that inactive nodes are nullptr?
    InputParameter inputParams[NUM_PARAMS];
    OutputParameter outputParams[NUM_PARAMS];
    juce::String friendlyName;
    juce::Point<float> position;
    
    bool isGlobalLockedNode = false;
    bool hasInputSide = true;
    bool hasOutputSide = true;
    
    juce::XmlElement* serialise()
    {
        auto output = new juce::XmlElement("node");
        
        auto typeElement = new juce::XmlElement("type");
        typeElement->addTextElement(juce::String(getType()));
        
        auto isGlobalLockedNodeElement = new juce::XmlElement("isGlobalLockedNode");
        isGlobalLockedNodeElement->addTextElement(isGlobalLockedNode ? "true" : "false");
        
        auto hasInputSideElement = new juce::XmlElement("hasInputSide");
        hasInputSideElement->addTextElement(hasInputSide ? "true" : "false");
        
        auto hasOutputSideElement = new juce::XmlElement("hasOutputSide");
        hasOutputSideElement->addTextElement(hasOutputSide ? "true" : "false");
        
        auto friendlyNameElement = new juce::XmlElement("friendlyName");
        friendlyNameElement->addTextElement(friendlyName);
        
        auto positionElement = new juce::XmlElement("position");
        
        auto positionXElement = new juce::XmlElement("x");
        auto positionYElement = new juce::XmlElement("y");
        positionXElement->addTextElement(juce::String(position.getX()));
        positionYElement->addTextElement(juce::String(position.getY()));
        
        positionElement->addChildElement(positionXElement);
        positionElement->addChildElement(positionYElement);
        
        output->addChildElement(typeElement);
        output->addChildElement(isGlobalLockedNodeElement);
        output->addChildElement(hasInputSideElement);
        output->addChildElement(hasOutputSideElement);
        output->addChildElement(friendlyNameElement);
        output->addChildElement(positionElement);
        
        for (int i = 0; i < NUM_PARAMS; i++)
        {
            if (!inputParams[i].isActive) break;
            output->addChildElement(inputParams[i].serialise(i));
        }
        
        
        for (int i = 0; i < NUM_PARAMS; i++)
        {
            if (!outputParams[i].isActive) break;
            output->addChildElement(outputParams[i].serialise(i));
        }
        
        return output;
    }
    
    virtual NodeType getType() = 0;
    virtual Node* getCopy() = 0;
};

class MainInputNode : public Node
{
public:
    MainInputNode() : MainInputNode(nullptr) { };
    
    MainInputNode(juce::XmlElement* elem) : Node(elem) {
        isGlobalLockedNode = true;
        hasInputSide = false;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "In";
        outputParams[0].type = ParameterType::Audio;
    };
    
    juce::AudioBuffer<float>* mainInput;
    
    NodeType getType() {return NodeType::MainInput;}
    Node* getCopy() {return new MainInputNode(*this);}
};

class MainOutputNode : public Node
{
public:
    MainOutputNode() : MainOutputNode(nullptr) { };
    
    MainOutputNode(juce::XmlElement* elem) : Node(elem) {
        isGlobalLockedNode = true;
        hasOutputSide = false;
        
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "Out";
        inputParams[0].type = ParameterType::Audio;
    };
    
    NodeType getType() {return NodeType::MainOutput;}
    Node* getCopy() {return new MainOutputNode(*this);}
};

class GainNode : public Node
{
public:
    GainNode() : GainNode(nullptr) { };
    
    GainNode(juce::XmlElement* elem) : Node(elem) {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        inputParams[1].isActive = true;
        inputParams[1].friendlyName = "Gain";
        inputParams[1].type = ParameterType::Value;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Out";
        outputParams[0].type = ParameterType::Audio;
    };
    
    NodeType getType() {return NodeType::Gain;}
    Node* getCopy() {return new GainNode(*this);}
};

class LevelNode : public Node
{
public:
    LevelNode() : LevelNode(nullptr) { };
    
    LevelNode(juce::XmlElement* elem) : Node(elem) {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Float";
        outputParams[0].type = ParameterType::Value;
        
        outputParams[1].isActive = true;
        outputParams[1].friendlyName = "dbFS";
        outputParams[1].type = ParameterType::Value;
    };
    
    NodeType getType() {return NodeType::Level;}
    Node* getCopy() {return new LevelNode(*this);}
};

class CorrelationNode : public Node
{
public:
    CorrelationNode() : CorrelationNode(nullptr) { };
    
    CorrelationNode(juce::XmlElement* elem) : Node(elem) {
        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Correlation";
        outputParams[0].type = ParameterType::Value;
    };
    
    NodeType getType() {return NodeType::Correlation;}
    Node* getCopy() {return new CorrelationNode(*this);}
};

class LoudnessNode : public Node
{
public:
    LoudnessNode() : LoudnessNode(nullptr) { };
    
    LoudnessNode(const LoudnessNode& ln) : Node(ln), meter(new Ebu128LoudnessMeter()) { };
    
    LoudnessNode(juce::XmlElement* elem) : Node(elem) {
        meter.reset(new Ebu128LoudnessMeter());

        inputParams[0].isActive = true;
        inputParams[0].friendlyName = "In";
        inputParams[0].type = ParameterType::Audio;
        
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "LUFS-S";
        outputParams[0].type = ParameterType::Value;
        
        outputParams[1].isActive = true;
        outputParams[1].friendlyName = "LUFS-M";
        outputParams[1].type = ParameterType::Value;
        
        outputParams[2].isActive = true;
        outputParams[2].friendlyName = "LUFS-I";
        outputParams[2].type = ParameterType::Value;
    };
    
    NodeType getType() {return NodeType::Loudness;}
    Node* getCopy() {return new LoudnessNode(*this);}
    
    std::unique_ptr<Ebu128LoudnessMeter> meter;
};

class MathsNode : public Node
{
public:
    MathsNode() : MathsNode(nullptr) { };
    
    MathsNode(const MathsNode& ln) : Node(ln) { };
    
    MathsNode(juce::XmlElement* elem) : Node(elem) {
        if (elem == nullptr) // else the things will be preset by Node constructor
        {
            inputParams[0].isActive = true;
            inputParams[0].friendlyName = "Input 1";
            inputParams[0].name = "input1";
            inputParams[0].type = ParameterType::Value;
        }
                
        outputParams[0].isActive = true;
        outputParams[0].friendlyName = "Output";
        outputParams[0].type = ParameterType::Value;
        
        registerSymbols();
        
        expr_str = "input1 * 2";
        
        parser.compile(expr_str, expression);
    };
    
    void registerSymbols()
    {
//        symbols.clear();
        
//        for (int i = 0; i < NUM_PARAMS; i++)
//        {
//            if (!inputParams[i].isActive) break;
////            DBG("p" << i << " " << inputParams[i].name << " = " << inputs[i]);
//            
//            DBG("\"" << inputParams[i].name.toStdString() << "\"");
//            symbols.add_variable(inputParams[i].name.toStdString(), getInputValue(i));
//        }
        
        symbols.add_variable("input1", input0);
        DBG("done this so it should preferably work");
        
        if (expression.num_symbol_tables() == 0)
        {
            expression.register_symbol_table(symbols);
        }
    }
    
    void setInputValue(int index, float value)
    {
        switch (index)
        {
            case 0:
                input0 = value;
                return;
            case 1:
                input1 = value;
                return;
            case 2:
                input2 = value;
                return;
            case 3:
                input3 = value;
                return;
            case 4:
                input4 = value;
                return;
            case 5:
                input5 = value;
                return;
            case 6:
                input6 = value;
                return;
            case 7:
                input7 = value;
                return;
            case 8:
                input8 = value;
                return;
            case 9:
                input9 = value;
                return;
            case 10:
                input10 = value;
                return;
            case 11:
                input11 = value;
                return;
            case 12:
                input12 = value;
                return;
            case 13:
                input13 = value;
                return;
            case 14:
                input14 = value;
                return;
            case 15:
                input15 = value;
                return;
        }
    }
    
    float& getInputValue(int index)
    {
        switch (index)
        {
            case 0:
                return input0;
            case 1:
                return input1;
            case 2:
                return input2;
            case 3:
                return input3;
            case 4:
                return input4;
            case 5:
                return input5;
            case 6:
                return input6;
            case 7:
                return input7;
            case 8:
                return input8;
            case 9:
                return input9;
            case 10:
                return input10;
            case 11:
                return input11;
            case 12:
                return input12;
            case 13:
                return input13;
            case 14:
                return input14;
            case 15:
                return input15;
        }
    }
    
    NodeType getType() {return NodeType::Maths;}
    Node* getCopy() {return new MathsNode(*this);}
    
    exprtk::symbol_table<float> symbols;
    exprtk::expression<float> expression;
    exprtk::parser<float> parser;
    
    std::string expr_str;
    
    
    // dont @ me it wasnt working as an array so i fixed it :) im very tired :)
    float input0;
    float input1;
    float input2;
    float input3;
    float input4;
    float input5;
    float input6;
    float input7;
    float input8;
    float input9;
    float input10;
    float input11;
    float input12;
    float input13;
    float input14;
    float input15;
};

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
    
    juce::XmlElement* serialise(int i)
    {
        auto output = new juce::XmlElement("valueStream");
        
        auto streamIdElement = new juce::XmlElement("streamId");
        streamIdElement->addTextElement(juce::String(i));
        
        auto envelopeElement = new juce::XmlElement("envelope");
        
        auto attackElement = new juce::XmlElement("attack");
        attackElement->addTextElement(juce::String(envelope.getMsAttack()));
        
        auto releaseElement = new juce::XmlElement("release");
        releaseElement->addTextElement(juce::String(envelope.getMsRelease()));
        
        envelopeElement->addChildElement(attackElement);
        envelopeElement->addChildElement(releaseElement);
        
        output->addChildElement(envelopeElement);
        output->addChildElement(streamIdElement);
        
        return output;
    }
    
    void deserialise(juce::XmlElement* elem)
    {
        auto envelopeElement = elem->getChildByName("envelope");
        
        envelope.setMsAttack(envelopeElement->getChildByName("attack")->getAllSubText().getFloatValue());
        envelope.setMsRelease(envelopeElement->getChildByName("release")->getAllSubText().getFloatValue());
    }
};

struct DataInstance
{
    int i;
    Node* nodes[NUM_NODES]; // could be vector, but probably not for the best TODO: maybe change to std::unique_ptr or even owned array?? im giving up rn tbh
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
    
    void deserialise(juce::XmlElement* element)
    {
        DBG("deserialise");
        int nodeId = 0;
        
//        auto root = element->getChildByName("dataInstance");
        
        DBG(element->getTagName());
        
        for (auto child : element->getChildIterator())
        {
            if (child->getTagName() == "node")
            {
                DBG(child->toString());
                switch ((NodeType) child->getChildByName("type")->getAllSubText().getIntValue())
                {
                    case NodeType::MainInput:
                        nodes[nodeId++] = new Data::MainInputNode(child);
                        break;
                    case NodeType::MainOutput:
                        nodes[nodeId++] = new Data::MainOutputNode(child);
                        break;
                    case NodeType::Gain:
                        nodes[nodeId++] = new Data::GainNode(child);
                        break;
                    case NodeType::Level:
                        nodes[nodeId++] = new Data::LevelNode(child);
                        break;
                    case NodeType::Correlation:
                        nodes[nodeId++] = new Data::CorrelationNode(child);
                        break;
                    case NodeType::Loudness:
                        nodes[nodeId++] = new Data::LoudnessNode(child);
                        break;
                    case NodeType::Maths:
                        nodes[nodeId++] = new Data::MathsNode(child);
                        break;
                }
                
            } else if (child->getTagName() == "valueStream")
            {
                valueStreams[child->getChildByName("streamId")->getAllSubText().getIntValue()].deserialise(child);
            } else
            {
                DBG("DataInstance deserialisation – Unknown tagName" << child->getTagName());
            }
        }
        
        for (int i = nodeId; i < NUM_NODES; i++)
        {
            nodes[i] = nullptr;
        }
        
        prepareStreams();
    }
    
    ~DataInstance()
    {
        for (int i = 0; i < NUM_NODES; i++) // manual construction because im being naughty and using raw pointers
        {
            delete nodes[i];
        }
    }
    
    juce::XmlElement* serialise()
    {
        prepareStreams();
        
        auto output = new juce::XmlElement("dataInstance");
        
        for (int i = 0; i < NUM_NODES; i++)
        {
            if (nodes[i] == nullptr || !nodes[i]->isActive) break;
            output->addChildElement(nodes[i]->serialise());
        }
        
        for (int i = 0; i < NUM_VALUE_STREAMS; i++)
        {
            if (valueStreams[i].inputNodeId == -1 || valueStreams[i].outputNodeId == -1) break;
            
            output->addChildElement(valueStreams[i].serialise(i));
        }
        
        return output;
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
    
    Data::MainOutputNode* getOutputNode(Data::DataInstance* instance);
    Data::MainOutputNode* getOutputNode();
    
    Data::MainInputNode* getInputNode(Data::DataInstance* instance);
    Data::MainInputNode* getInputNode();
    
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
