#include "startup.h"
#include "load_save.h"
#include "JuceHeader.h"
#include "synth_base.h"

void Startup::doStartupChecks(MidiManager* midi_manager, vital::StringLayout* layout) {
  // If Vital is not installed or properly configured, no action is taken.
  if (!LoadSave::isInstalled())
    return;

  // If Vital was upgraded since the last run, update the stored version information.
  if (LoadSave::wasUpgraded())
    LoadSave::saveVersionConfig();

  // Load configuration data, which may include MIDI mappings, GUI layout preferences, and other settings.
  LoadSave::loadConfig(midi_manager, layout);
}

bool Startup::isComputerCompatible() {
  #if defined(__ARM_NEON__)
  // On ARM platforms with NEON, assume compatibility.
  return true;
  #else
  // On x86 platforms, check for SSE2 or AVX2 support.
  return SystemStats::hasSSE2() || SystemStats::hasAVX2();
  #endif
}
