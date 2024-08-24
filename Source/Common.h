/*
  ==============================================================================

    Common.h
    Created: 3 Aug 2024 10:37:37am
    Author:  School

  ==============================================================================
*/

#pragma once

#include "GraphNode.h"

namespace Common {

struct Node {
    std::unique_ptr<GraphNode> component;
};

}
