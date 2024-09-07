/*
  ==============================================================================

    SideMenu.h
    Created: 23 Jul 2024 10:34:34am
    Author:  School

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SideMenuCollapser.h"
#include "InspectorPanel.h"
#include "NodeLibraryPanel.h"
#include "DataManager.h"

//==============================================================================
/*
*/
class SideMenu;

class SideMenuContent  : public juce::Component
{
public:
    SideMenuContent(SideMenu& parent, std::shared_ptr<DataManager> d);
    ~SideMenuContent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void handleTabChanged(int);
    
    void addTab(Component* contentComponent);
    
    InspectorPanel* getInspector() {return &m_inspectorPanel;};
    NodeLibraryPanel* getNodeLibrary() {return &m_nodeLibraryPanel;};
    
    std::function<void()> onNodeAdded;
    
protected:
    friend class SideMenu;
    SideMenu& owner;
    
private:
    InspectorPanel m_inspectorPanel;
    NodeLibraryPanel m_nodeLibraryPanel;
    
    juce::Array<juce::WeakReference<juce::Component>> contentComponents;
    juce::WeakReference<juce::Component> panelComponent;
    
//    juce::TabbedComponent m_tabbedComponent;
    
    int tab;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideMenuContent)
};

class SideMenu  : public juce::Component
{
public:
    SideMenu(std::shared_ptr<DataManager> d);
    ~SideMenu() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool getShown();
    void handleTabChanged(int newTabIndex);
    
    InspectorPanel* getInspector() {return inspector;};
    NodeLibraryPanel* getNodeLibrary() {return nodeLibrary;};
    
    std::function<void()> onNodeAdded;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SideMenu)
    
    juce::SidePanel m_sidePanel;
    SideMenuCollapser m_sideMenuCollapser;
    
    InspectorPanel* inspector;
    NodeLibraryPanel* nodeLibrary;
    
    void handlePanelShowHide(bool);
    
    struct Header;
    
    int tab;
    
    bool shown;
};


