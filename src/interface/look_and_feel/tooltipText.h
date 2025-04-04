#pragma once

#include "JuceHeader.h"

#include <string>
#include <unordered_map>

// Example: A global function returning a static map, or a static global variable:
const std::unordered_map<std::string, std::string>& getTooltips();
