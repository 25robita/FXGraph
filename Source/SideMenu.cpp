/*
  ==============================================================================

    SideMenu.cpp
    Created: 23 Jul 2024 10:34:34am
    Author:  School

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SideMenu.h"
#include "SideMenuHeader.h"
#include "DataManager.h"

struct SideMenu::Header final : public SideMenuHeader {
    Header(SideMenu& parent) : SideMenuHeader(), owner(parent)
    {
    }
    
    void onTabChanged(int tabIndex) override {
        owner.handleTabChanged(tabIndex);
    }
    
    SideMenu& owner;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Header);
};

//==============================================================================
SideMenu::SideMenu(std::shared_ptr<DataManager> d) : m_sidePanel("Side panel", 300, true, nullptr)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    auto sideMenuHeader = new Header(*this);
    auto m_sideMenuContent = new SideMenuContent(*this, d);
    
    m_sideMenuContent->onNodeAdded = [this] () {
        onNodeAdded();
    };
    
    inspector = m_sideMenuContent->getInspector();
    nodeLibrary = m_sideMenuContent->getNodeLibrary();
    
    shown = true;
    
    m_sidePanel.setTitleBarHeight(30);
    m_sidePanel.setTitleBarComponent(sideMenuHeader, false);
    
    m_sidePanel.onPanelShowHide = [this] (bool willShown) {
        if (shown == willShown) return;
        
        shown = willShown;
        handlePanelShowHide(willShown);
    };
    
    m_sidePanel.setContent(m_sideMenuContent);
    
    addAndMakeVisible(m_sideMenuCollapser, 1);
    addAndMakeVisible(m_sidePanel, 0);
    
    m_sideMenuCollapser.onUpdate = [this] () {
        shown = !shown;
        
        handlePanelShowHide(shown);
    };
}

SideMenu::~SideMenu()
{
    
}

void SideMenu::handlePanelShowHide(bool willShown)
{
    m_sideMenuCollapser.isRight = !shown;
    m_sidePanel.showOrHide(shown);
    
    auto oldBounds = m_sideMenuCollapser.getBounds();
    
    juce::Rectangle<int> newBounds;
    
    if (shown)
    {
        newBounds = juce::Rectangle<int>(300 - m_sidePanel.getShadowWidth(), 30, oldBounds.getWidth(), oldBounds.getHeight());
        
        setInterceptsMouseClicks(true, true);
    } else
    {
        newBounds = juce::Rectangle<int>(0, oldBounds.getY(), oldBounds.getWidth(), oldBounds.getHeight());
        
        setInterceptsMouseClicks(false, true);
    }
    
    float milliseconds = 250 * (300 - m_sidePanel.getShadowWidth()) / 300.0f;
    
    juce::Desktop::getInstance().getAnimator().animateComponent(&m_sideMenuCollapser, newBounds, 1.0f, (int) round(milliseconds), true, 1.0, 0.0);
    
    getParentComponent()->resized();
}

void SideMenu::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
}

void SideMenu::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    m_sidePanel.showOrHide(shown);
    
    m_sideMenuCollapser.setBounds(shown ? 300 - m_sidePanel.getShadowWidth() : 0, 30, 35, 40);
}

bool SideMenu::getShown()
{
    return shown;
}


void SideMenu::handleTabChanged(int newTabIndex) {
    // soo unsafe i feel dirty and disgusting for having written it but this program
    // has not executed in hours i don't know what else to do
    ((SideMenuContent*) m_sidePanel.getContent())->handleTabChanged(newTabIndex);
}

SideMenuContent::SideMenuContent(SideMenu& parent, std::shared_ptr<DataManager> d) : owner(parent), m_inspectorPanel(d), m_nodeLibraryPanel(d)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    addTab(&m_nodeLibraryPanel);
    addTab(&m_inspectorPanel);
    
    m_nodeLibraryPanel.onNodeAdded = [this] () {
        onNodeAdded();
    };
    
    panelComponent = contentComponents[0].get();
    addChildComponent(panelComponent);
    panelComponent->setVisible(true);
}

SideMenuContent::~SideMenuContent()
{
    
}

void SideMenuContent::paint (juce::Graphics& g)
{
}

void SideMenuContent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    
    if (panelComponent != nullptr) {
        panelComponent->setBounds(getLocalBounds());
    }
    
}

void SideMenuContent::handleTabChanged(int newTabIndex) {
    
    tab = newTabIndex;
    
    if (panelComponent != nullptr) {
        panelComponent->setVisible(false);
        removeChildComponent(panelComponent);
    }
    
    panelComponent = contentComponents[tab].get();
    
    addChildComponent(panelComponent);
    panelComponent->setVisible(true);
    panelComponent->toFront(false);
    
    repaint();
    resized();
}

void SideMenuContent::addTab (Component* contentComponent)
{
    contentComponents.add(juce::WeakReference<Component> (contentComponent));
    
    resized();
}
