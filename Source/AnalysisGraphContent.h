/*
  ==============================================================================

    AnalysisGraphContent.h
    Created: 20 Aug 2024 9:54:27am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DataManager.h"

//==============================================================================
/*
*/
class AnalysisGraphContent  : public juce::Component
{
public:
    AnalysisGraphContent(std::shared_ptr<DataManager>);
    ~AnalysisGraphContent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    
    void setSelection();
    void setSelection(ParameterType type, int streamId);

private:
    bool analysingRn;
    void updateMetering();
    void timerCallback();
    void textTimerCallback();
    juce::TimedCallback timer;
    juce::TimedCallback textTimer;
    
    juce::String currText;
    
    ParameterType selectedType;
    int selectedId;
    
    juce::Path graphPath;
    float xVal = 0;
    
    float prevVal = NAN;
    
    const int timerInterval = 10;
    const int textTimerInterval = 100;
    
    std::shared_ptr<DataManager> dataManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalysisGraphContent)
};
