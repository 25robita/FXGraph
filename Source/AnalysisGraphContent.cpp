/*
  ==============================================================================

    AnalysisGraphContent.cpp
    Created: 20 Aug 2024 9:54:27am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalysisGraphContent.h"
#include "DataManager.h"

//==============================================================================
AnalysisGraphContent::AnalysisGraphContent(std::shared_ptr<DataManager> d) : timer([this] () {timerCallback();}) , textTimer([this] () {textTimerCallback();})
{
    dataManager = d;
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
}

AnalysisGraphContent::~AnalysisGraphContent()
{
}

void AnalysisGraphContent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xff222222));   // clear the background

    g.setColour (juce::Colour(0xffAAAAAA));
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    if (analysingRn)
    {
        g.setColour(juce::Colour(0xffEADEED));
        
        if (selectedId != -1 && !isnan(dataManager->activeInstance->valueStreams[selectedId].value))
        {
            float x2 = xVal + 1;
            float y2 = -dataManager->activeInstance->valueStreams[selectedId].value;
            
            if (xVal == 0) {
                graphPath.addEllipse(xVal, isnan(prevVal) ? -dataManager->activeInstance->valueStreams[selectedId].value : prevVal, 1e-5, 1e-5);
            }
            
            graphPath.lineTo({x2, y2});
            
            prevVal = -dataManager->activeInstance->valueStreams[selectedId].value;
            
            xVal++;
            
            juce::AffineTransform pathTransform = graphPath.getTransformToScaleToFit(getLocalBounds().toFloat(), false);
            
            pathTransform.mat00 = 1;
            
            if (xVal >= getWidth())
                pathTransform.mat02 = getWidth() - xVal;
            else
                pathTransform.mat02 = 0;
            
            if (xVal >= getWidth() * 2)
            {
                // TODO: periodically clear path
            }
            
            auto p = juce::Path(graphPath);
            p.applyTransform(pathTransform);
            g.strokePath(p, juce::PathStrokeType(1.0f));
            
            // TODO: add + sign for positive values
            auto topText = juce::String::toDecimalStringWithSignificantFigures(-graphPath.getBounds().getY(), 3);
            auto bottomText = juce::String::toDecimalStringWithSignificantFigures(-graphPath.getBounds().getBottom(), 3);
            
            g.setFont(juce::FontOptions(14.0f));
            
            g.setColour(juce::Colour(0xa0000000));
            g.fillRect(2, 2, g.getCurrentFont().getStringWidth(topText) + 4, 16);
            g.fillRect(2, 100 - 16 - 2, g.getCurrentFont().getStringWidth(bottomText) + 4, 16);
            g.fillRect(getWidth() - g.getCurrentFont().getStringWidth(currText) - 4 - 2, 2, g.getCurrentFont().getStringWidth(currText) + 4, 16);
            
            g.setColour(juce::Colour(0xffFA1012));
            
            
            g.drawText(topText, getLocalBounds().reduced(5), juce::Justification::topLeft, true);
            g.drawText(bottomText, getLocalBounds().reduced(5), juce::Justification::bottomLeft, true);
            g.drawText(currText, getLocalBounds().reduced(5), juce::Justification::topRight, true);
        }
        

    } else
    {
        g.setColour(juce::Colour(0xffEADEED));
        g.setFont(juce::FontOptions(14.0f));
        g.drawText("Click to start metering", getLocalBounds(), juce::Justification::centred, true);
    }
    
//    g.setColour (juce::Colours::white);
//    g.setFont (juce::FontOptions (14.0f));
//    g.drawText ("AnalysisGraphContent", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
}

void AnalysisGraphContent::resized() { }

void AnalysisGraphContent::mouseDown(const juce::MouseEvent &)
{
    analysingRn = !analysingRn;
    updateMetering();
}

void AnalysisGraphContent::updateMetering()
{
    repaint();
    if (analysingRn) {
        timer.startTimer(timerInterval);
        textTimer.startTimer(textTimerInterval);
        textTimerCallback();
        
        graphPath.clear();
        xVal = 0;
    } else {
        timer.stopTimer();
        textTimer.stopTimer();
    }
}

void AnalysisGraphContent::timerCallback()
{
    repaint();
}

void AnalysisGraphContent::textTimerCallback()
{
    // update the current text if analysing
    if (!analysingRn) return;
    currText = juce::String::toDecimalStringWithSignificantFigures(-prevVal, 3);
}

void AnalysisGraphContent::setSelection()
{
    selectedId = -1;
}

void AnalysisGraphContent::setSelection(ParameterType type, int streamId)
{
    selectedId = streamId;
    selectedType = type;
}
