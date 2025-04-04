#include "tooltipText.h"

// We define our dictionary of parameter->tooltip
static std::unordered_map<std::string, std::string> makeTooltipMap() {
  std::unordered_map<std::string, std::string> tooltips;
  tooltips["distortion_amount"] = R"(
This parameter controls the "Wave Morph" amount. 
Higher values produce more aggressive distortion or bending of the waveform.)";

  tooltips["spectral_morph_amount"] = R"(

This parameter changes the spectral content of the waveform.
Useful for adding unusual harmonic behaviors.

)";

  // ... add more as needed

  return tooltips;
}

const std::unordered_map<std::string, std::string>& getTooltips() {
  static const auto tooltips = makeTooltipMap();
  return tooltips;
}
