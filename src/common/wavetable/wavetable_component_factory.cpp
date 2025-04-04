/*
Summary:
WavetableComponentFactory provides a centralized means to instantiate various WavetableComponents, both sources and modifiers, and to map between enumerated types, string names, and constructed objects. This makes it easier to integrate new components, handle component-specific UI or preset logic, and maintain a clean separation of concerns for creation and naming of WavetableComponents.
 */

#include "wavetable_component_factory.h"
#include "file_source.h"
#include "frequency_filter_modifier.h"
#include "phase_modifier.h"
#include "shepard_tone_source.h"
#include "slew_limit_modifier.h"
#include "wave_frame.h"
#include "wave_fold_modifier.h"
#include "wave_line_source.h"
#include "wave_source.h"
#include "wave_warp_modifier.h"
#include "wave_window_modifier.h"

WavetableComponent* WavetableComponentFactory::createComponent(ComponentType type) {
  // Creates a WavetableComponent instance based on the given enumerated type.
  switch (type) {
    case kWaveSource:
      return new WaveSource();
    case kLineSource:
      return new WaveLineSource();
    case kFileSource:
      return new FileSource();
    case kShepardToneSource:
      return new ShepardToneSource();
    case kPhaseModifier:
      return new PhaseModifier();
    case kWaveWindow:
      return new WaveWindowModifier();
    case kFrequencyFilter:
      return new FrequencyFilterModifier();
    case kSlewLimiter:
      return new SlewLimitModifier();
    case kWaveFolder:
      return new WaveFoldModifier();
    case kWaveWarp:
      return new WaveWarpModifier();
    default:
      VITAL_ASSERT(false);
      return nullptr;
  }
}

WavetableComponent* WavetableComponentFactory::createComponent(const std::string& type) {
  // Creates a WavetableComponent instance from a string name.
  if (type == "Wave Source")
    return new WaveSource();
  if (type == "Line Source")
    return new WaveLineSource();
  if (type == "Audio File Source")
    return new FileSource();
  if (type == "Shepard Tone Source")
    return new ShepardToneSource();
  if (type == "Phase Shift")
    return new PhaseModifier();
  if (type == "Wave Window")
    return new WaveWindowModifier();
  if (type == "Frequency Filter")
    return new FrequencyFilterModifier();
  if (type == "Slew Limiter")
    return new SlewLimitModifier();
  if (type == "Wave Folder")
    return new WaveFoldModifier();
  if (type == "Wave Warp")
    return new WaveWarpModifier();

  VITAL_ASSERT(false);
  return nullptr;
}

std::string WavetableComponentFactory::getComponentName(ComponentType type) {
  // Maps enumerated types to their human-readable names.
  switch (type) {
    case kWaveSource:
      return "Wave Source";
    case kLineSource:
      return "Line Source";
    case kFileSource:
      return "Audio File Source";
    case kShepardToneSource:
      return "Shepard Tone Source";
    case kPhaseModifier:
      return "Phase Shift";
    case kWaveWindow:
      return "Wave Window";
    case kFrequencyFilter:
      return "Frequency Filter";
    case kSlewLimiter:
      return "Slew Limiter";
    case kWaveFolder:
      return "Wave Folder";
    case kWaveWarp:
      return "Wave Warp";
    default:
      return "";
  }
}
