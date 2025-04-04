#pragma once

#include "JuceHeader.h"

#include <map>

class SynthBase;

namespace vital {
    class StringLayout;
}

class MidiManager;

/**
 * @brief A utility class that handles initialization tasks and compatibility checks upon Vital’s startup.
 *
 * The Startup class provides methods to perform essential checks and setup when Vital is launched. This includes:
 * - Verifying installation and upgrade status.
 * - Loading configuration data such as MIDI mappings and user preferences.
 * - Ensuring the computer supports the required CPU features.
 */
class Startup {
public:
    /**
     * @brief Performs initial startup checks and configuration loading.
     *
     * This method checks if the software has been installed or upgraded since the last run,
     * updates configuration if necessary, and loads vital preferences and MIDI mappings.
     *
     * @param midi_manager A pointer to the MidiManager, used to restore MIDI mappings and other related settings.
     * @param layout An optional pointer to a vital::StringLayout instance, allowing the restoration of custom keyboard layouts.
     */
    static void doStartupChecks(MidiManager* midi_manager, vital::StringLayout* layout = nullptr);

    /**
     * @brief Checks if the current computer hardware is compatible with Vital’s CPU instruction requirements.
     *
     * This typically checks for SSE2 or AVX2 support on non-ARM platforms, and assumes compatibility on ARM platforms.
     *
     * @return True if the computer meets the CPU requirements; false otherwise.
     */
    static bool isComputerCompatible();

private:
    /**
     * @brief Deleted constructor to prevent instantiation of this utility class.
     */
    Startup() = delete;
};
