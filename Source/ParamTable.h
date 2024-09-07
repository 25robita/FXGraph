/*
  ==============================================================================

    ParamTable.h
    Created: 7 Sep 2024 3:10:45pm
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DataManager.h"
#include "Common.h"

class ParamTable_Model  : public juce::Component, public juce::TableListBoxModel
{
public:
    ParamTable_Model(std::shared_ptr<DataManager> d, int, InputOrOutput);
    ~ParamTable_Model() override;
    
    int getNumRows() override;
    void paintRowBackground(juce::Graphics &, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics &, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    
    void resized() override;
    
    void handleInput(int column, int row, const juce::String& newVal);
    void handleInput(int row); // for remove
    
    float getIdealHeight();
    Data::Parameter& getParameter(int index);
    
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate) override;

private:
    std::shared_ptr<DataManager> dataManager;
    juce::TableListBox table {{}, this};
    
    int selectedNodeId;
    InputOrOutput side;
    
    enum ColumnId
    {
        Index = 1,
        DisplayName = 2,
        Name = 3,
        Remove = 4,
    };
    
    class EditableCell : public juce::Label
    {
    public:
        EditableCell (ParamTable_Model& pt_m, bool editable_)
                    : owner (pt_m), editable(editable_)
        {
            setEditable (false, editable, false);
        }
        
        void setIsEditable(bool v)
        {
            editable = v;
            setEditable(false, editable, false);
        }
        
        void textWasEdited() override
        {
            owner.handleInput (column, row, getText());
        }
        
        void setPosition(int column_, int row_)
        {
            column = column_;
            row = row_;
        }
    private:
        ParamTable_Model& owner;
        int column, row;
        bool editable;
    };
    
    class RemoveButton  : public juce::Component
    {
    public:
        RemoveButton(ParamTable_Model& parent) : owner(parent) {};
        ~RemoveButton() override {};

        void paint (juce::Graphics& g) override {
            g.setColour (juce::Colour(0xbfFFFFFF));
            
            const float lineThicknesss = 1.3f;
            const float padding = 9.0f;
            
            auto crossRect = getLocalBounds().reduced(padding);

            g.drawLine(crossRect.getX(), crossRect.getY(), crossRect.getRight(), crossRect.getBottom(), lineThicknesss);
            g.drawLine(crossRect.getRight(), crossRect.getY(), crossRect.getX(), crossRect.getBottom(), lineThicknesss);
        };
        void mouseDown(const juce::MouseEvent&) override {
            owner.handleInput(row);
        };
        
        void setPosition(int column_, int row_)
        {
            column = column_;
            row = row_;
        }

    private:
        ParamTable_Model& owner;
        int column, row;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RemoveButton)
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParamTable_Model)
};
