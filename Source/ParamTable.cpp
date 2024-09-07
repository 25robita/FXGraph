/*
  ==============================================================================

    ParamTable.cpp
    Created: 7 Sep 2024 3:10:45pm
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ParamTable.h"

ParamTable_Model::ParamTable_Model(std::shared_ptr<DataManager> d, int nodeId, InputOrOutput side_)
{
    selectedNodeId = nodeId;
    side = side_;
    
    dataManager = d;
    
    addAndMakeVisible(table);
    
    table.getHeader().addColumn("#", ColumnId::Index, 25);
    table.getHeader().addColumn("Display Name", ColumnId::DisplayName, 100);
    table.getHeader().addColumn("Name", ColumnId::Name, 50);
    table.getHeader().addColumn("x", ColumnId::Remove, 25);
}

ParamTable_Model::~ParamTable_Model()
{
    table.setModel(nullptr);
}

int ParamTable_Model::getNumRows()
{
    for (int i = 0; i < NUM_PARAMS; i++)
    {
        if (!getParameter(i).isActive) return i;
    }
    
    return NUM_PARAMS;
}

Data::Parameter& ParamTable_Model::getParameter(int index)
{
    if (side == InputOrOutput::Input)
        return dataManager->activeInstance->nodes[selectedNodeId]->inputParams[index];
    
    return dataManager->activeInstance->nodes[selectedNodeId]->outputParams[index];
}

float ParamTable_Model::getIdealHeight()
{
    return table.getRowHeight() * (std::min(getNumRows(), 5) + 1) + 10; // tack some onto the end for funsies (and in case there's like borders or innacuracies, but mostly for funsies)
}

void ParamTable_Model::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    auto alternateColour = getLookAndFeel().findColour (juce::ListBox::backgroundColourId).interpolatedWith (getLookAndFeel().findColour (juce::ListBox::textColourId), 0.03f);
    
    if (rowNumber % 2) g.fillAll(alternateColour);
}

void ParamTable_Model::paintCell(juce::Graphics &, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    //TODO: this
}

void ParamTable_Model::resized()
{
    table.setBounds(getLocalBounds());
}

juce::Component* ParamTable_Model::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component *existingComponentToUpdate)
{
    switch (columnId)
    {
        case ColumnId::Index:
        {
            auto* component = static_cast<EditableCell*> (existingComponentToUpdate);
            
            if (component == nullptr)
            {
                component = new EditableCell(*this, false);
                component->setText(juce::String(rowNumber), juce::dontSendNotification);
            }
            
            component->setPosition(columnId, rowNumber);
            return component;
        }
        case ColumnId::DisplayName:
        {
            auto* component = static_cast<EditableCell*> (existingComponentToUpdate);

            if (component == nullptr)
            {
                auto& param = getParameter(rowNumber);
                
                component = new EditableCell(*this, param.canEditFriendlyName);
                component->setText(param.friendlyName, juce::dontSendNotification);
            }
            
            component->setPosition(columnId, rowNumber);
            return component;
        }
        case ColumnId::Name:
        {
            auto* component = static_cast<EditableCell*> (existingComponentToUpdate);

            if (component == nullptr)
            {
                auto& param = getParameter(rowNumber);

                component = new EditableCell(*this, param.canEditName);
                component->setText(param.name, juce::dontSendNotification);
            }
            
            component->setPosition(columnId, rowNumber);
            return component;
        }
        case ColumnId::Remove:
            auto* component = static_cast<RemoveButton*> (existingComponentToUpdate);

            if (component == nullptr)
            {
                auto& param = getParameter(rowNumber);

                component = new RemoveButton(*this); // TODO: editability check?
            }
            
            component->setPosition(columnId, rowNumber);
            return component;
    }
    
    return nullptr;
}

void ParamTable_Model::handleInput(int column, int row, const juce::String &newVal)
{
    switch (column) {
        case ColumnId::DisplayName:
            dataManager->startEditing();
            
            if (side == InputOrOutput::Input)
                dataManager->inactiveInstance->nodes[selectedNodeId]->inputParams[row].friendlyName = newVal;
            else
                dataManager->inactiveInstance->nodes[selectedNodeId]->outputParams[row].friendlyName = newVal;
            
            dataManager->finishEditing();
            break;
        case ColumnId::Name:
            dataManager->startEditing();
            
            if (side == InputOrOutput::Input)
                dataManager->inactiveInstance->nodes[selectedNodeId]->inputParams[row].name = newVal;
            else
                dataManager->inactiveInstance->nodes[selectedNodeId]->outputParams[row].name = newVal;
            
            dataManager->finishEditing();
            break;
        default:
            DBG(column);
            jassertfalse; // this shouldn't be called for the other rows
            break;
    }
    
    dataManager->registerOneTimeRealisationListener([this] () { // FIXME: this doesn't actually update the table cells
        table.updateContent();
        table.repaint();
        
        getParentComponent()->getParentComponent()->resized();
    });
}

void ParamTable_Model::handleInput(int row)
{
    dataManager->startEditing();
    
    dataManager->inactiveInstance->nodes[selectedNodeId]->removeParameter(row, side);
    
    dataManager->finishEditing();
    
    dataManager->registerOneTimeRealisationListener([this] () { // FIXME: this doesn't actually update the table cells
        table.updateContent();
        table.repaint();
        
        getParentComponent()->getParentComponent()->resized();
    });
}
