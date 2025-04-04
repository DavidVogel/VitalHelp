#pragma once

#include "value.h"
#include <string>

/**
 * @namespace vital
 * @brief The main namespace for Vital synthesizer-related classes, functions, and constants.
 */
namespace vital {

    /// Number of LFO sources available in the Vital synthesizer.
    constexpr int kNumLfos = 8;

    /// Number of oscillators available in Vital.
    constexpr int kNumOscillators = 3;

    /// Number of wave frames in each oscillator’s wavetable.
    constexpr int kNumOscillatorWaveFrames = 257;

    /// Number of envelope generators in Vital.
    constexpr int kNumEnvelopes = 6;

    /// Number of random LFO sources (random modulation generators).
    constexpr int kNumRandomLfos = 4;

    /// Number of macro controls available.
    constexpr int kNumMacros = 4;

    /// Number of filter slots in Vital.
    constexpr int kNumFilters = 2;

    /// Number of formant filters available.
    constexpr int kNumFormants = 4;

    /// Number of output channels (stereo = 2).
    constexpr int kNumChannels = 2;

    /// The maximum number of voices allocated for polyphony (includes an extra for handling transitions).
    constexpr int kMaxPolyphony = 33;

    /// The maximum number of active voices Vital uses simultaneously.
    constexpr int kMaxActivePolyphony = 32;

    /// Resolution used for generating LFO data tables.
    constexpr int kLfoDataResolution = 2048;

    /// Maximum number of modulation connections allowed.
    constexpr int kMaxModulationConnections = 64;

    /// Sample rate (in Hz) at which the oscilloscope memory is sampled.
    constexpr int kOscilloscopeMemorySampleRate = 22000;

    /// Resolution (number of samples) in the oscilloscope memory buffer.
    constexpr int kOscilloscopeMemoryResolution = 512;

    /// Size of the stereo audio memory buffer used for visualization.
    constexpr int kAudioMemorySamples = 1 << 15;

    /// Default width of the Vital window (in pixels).
    constexpr int kDefaultWindowWidth = 1400;

    /// Default height of the Vital window (in pixels).
    constexpr int kDefaultWindowHeight = 820;

    /// Minimum allowable window width.
    constexpr int kMinWindowWidth = 350;

    /// Minimum allowable window height.
    constexpr int kMinWindowHeight = 205;

    /// Default starting octave offset for the computer keyboard layout.
    constexpr int kDefaultKeyboardOffset = 48;

    /// Default key for octave-up action in the computer keyboard layout.
    constexpr wchar_t kDefaultKeyboardOctaveUp = 'x';

    /// Default key for octave-down action in the computer keyboard layout.
    constexpr wchar_t kDefaultKeyboardOctaveDown = 'z';

    /// The default keyboard layout (QWERTY-based) mapping keys to notes.
    const std::wstring kDefaultKeyboard = L"awsedftgyhujkolp;'";

    /// File extension for Vital preset files.
    const std::string kPresetExtension = "vital";

    /// File extension for Vital wavetable files.
    const std::string kWavetableExtension = "vitaltable";

    /// A semicolon-separated list of supported wavetable file extensions, including external formats like .wav and .flac.
    const std::string kWavetableExtensionsList = "*." + vital::kWavetableExtension + ";*.wav;*.flac";

    /// A semicolon-separated list of supported sample file formats (e.g., wav and flac).
    const std::string kSampleExtensionsList = "*.wav;*.flac";

    /// File extension for Vital skin/theme files.
    const std::string kSkinExtension = "vitalskin";

    /// File extension for Vital LFO shape files.
    const std::string kLfoExtension = "vitallfo";

    /// File extension for Vital bank files, which group multiple presets.
    const std::string kBankExtension = "vitalbank";

    /**
     * @namespace vital::constants
     * @brief Contains enumerations, additional constants, and helper functions related to routing and effects within Vital.
     */
    namespace constants {

        /**
         * @enum SourceDestination
         * @brief Represents routing destinations for oscillators and other sound sources through the signal chain.
         */
        enum SourceDestination {
            kFilter1,       ///< Route through Filter 1.
            kFilter2,       ///< Route through Filter 2.
            kDualFilters,   ///< Route through both filters.
            kEffects,       ///< Route directly to the effects chain.
            kDirectOut,     ///< Route directly to the output (bypass filters and effects).
            kNumSourceDestinations
        };

        /**
         * @brief Toggles Filter 1 in the current routing destination.
         *
         * If Filter 1 is turned on, it may combine with Filter 2 to form Dual Filters, or become Filter 1 if Filter 2 isn't active.
         * If Filter 1 is turned off, routing adjusts accordingly.
         *
         * @param current_destination The current routing destination.
         * @param on True if Filter 1 should be turned on, false if turning off.
         * @return The updated routing destination after toggling Filter 1.
         */
        static SourceDestination toggleFilter1(SourceDestination current_destination, bool on) {
            if (on) {
                if (current_destination == vital::constants::kFilter2)
                    return vital::constants::kDualFilters;
                else
                    return vital::constants::kFilter1;
            }
            else if (current_destination == vital::constants::kDualFilters)
                return vital::constants::kFilter2;
            else if (current_destination == vital::constants::kFilter1)
                return vital::constants::kEffects;

            return current_destination;
        }

        /**
         * @brief Toggles Filter 2 in the current routing destination.
         *
         * Similar logic as toggleFilter1(), but for Filter 2. Adjusts routing based on whether Filter 2 is added or removed.
         *
         * @param current_destination The current routing destination.
         * @param on True if Filter 2 should be turned on, false if turning off.
         * @return The updated routing destination after toggling Filter 2.
         */
        static SourceDestination toggleFilter2(SourceDestination current_destination, bool on) {
            if (on) {
                if (current_destination == vital::constants::kFilter1)
                    return vital::constants::kDualFilters;
                else
                    return vital::constants::kFilter2;
            }
            else if (current_destination == vital::constants::kDualFilters)
                return vital::constants::kFilter1;
            else if (current_destination == vital::constants::kFilter2)
                return vital::constants::kEffects;

            return current_destination;
        }

        /**
         * @enum Effect
         * @brief Identifiers for the various audio effects available in Vital.
         */
        enum Effect {
            kChorus,
            kCompressor,
            kDelay,
            kDistortion,
            kEq,
            kFilterFx,
            kFlanger,
            kPhaser,
            kReverb,
            kNumEffects
        };

        /**
         * @enum FilterModel
         * @brief Identifiers for different filter models available in Vital’s filters.
         */
        enum FilterModel {
            kAnalog,
            kDirty,
            kLadder,
            kDigital,
            kDiode,
            kFormant,
            kComb,
            kPhase,
            kNumFilterModels
        };

        /**
         * @enum RetriggerStyle
         * @brief Styles for how modulators (such as LFOs) are retriggered.
         */
        enum RetriggerStyle {
            kFree,          ///< LFO runs free without retriggering.
            kRetrigger,     ///< LFO restarts phase on note-on.
            kSyncToPlayHead,///< LFO syncs to the DAW’s timeline.
            kNumRetriggerStyles,
        };

        /// Number of frequency ratios used when syncing parameters (e.g., LFO speed) to tempo.
        constexpr int kNumSyncedFrequencyRatios = 13;

        /// Predefined list of frequency ratios for synced parameters (from 1/128th to 16x speed).
        constexpr vital::mono_float kSyncedFrequencyRatios[kNumSyncedFrequencyRatios] = {
        0.0f,
        1.0f / 128.0f,
        1.0f / 64.0f,
        1.0f / 32.0f,
        1.0f / 16.0f,
        1.0f / 8.0f,
        1.0f / 4.0f,
        1.0f / 2.0f,
        1.0f,
        2.0f,
        4.0f,
        8.0f,
        16.0f
    };

    // Predefined poly_float and poly_mask values for internal DSP operations.
    /// A poly_float representing a vector [1.0f, 0.0f] used for channel manipulations.
    const poly_float kLeftOne(1.0f, 0.0f);

    /// A poly_float representing a vector [0.0f, 1.0f] commonly used for stereo channel operations.
    const poly_float kRightOne(0.0f, 1.0f);

    /// A poly_float indicating the first voice in a polyphonic group.
    const poly_float kFirstVoiceOne(1.0f, 1.0f, 0.0f, 0.0f);

    /// A poly_float indicating the second voice in a polyphonic group.
    const poly_float kSecondVoiceOne(0.0f, 0.0f, 1.0f, 1.0f);

    /// Splits stereo channels into left and right components.
    const poly_float kStereoSplit = kLeftOne - kRightOne;

    /// A poly_float representing sqrt(2), used in various DSP calculations.
    const poly_float kPolySqrt2 = kSqrt2;

    /// A mask covering all lanes of a poly_float vector.
    const poly_mask kFullMask = poly_float::equal(0.0f, 0.0f);

    /// A mask identifying the left channel when comparing to kLeftOne.
    const poly_mask kLeftMask = poly_float::equal(kLeftOne, 1.0f);

    /// A mask identifying the right channel when comparing to kRightOne.
    const poly_mask kRightMask = poly_float::equal(kRightOne, 1.0f);

    /// A mask identifying the first voice slots in a polyphonic vector.
    const poly_mask kFirstMask = poly_float::equal(kFirstVoiceOne, 1.0f);

    /// A mask identifying the second voice slots in a polyphonic vector.
    const poly_mask kSecondMask = poly_float::equal(kSecondVoiceOne, 1.0f);

    // Predefined constant values stored as cr::Value for quick access.
    const cr::Value kValueZero(0.0f);
    const cr::Value kValueOne(1.0f);
    const cr::Value kValueTwo(2.0f);
    const cr::Value kValueHalf(0.5f);
    const cr::Value kValueFifth(0.2f);
    const cr::Value kValueTenth(0.1f);
    const cr::Value kValuePi(kPi);
    const cr::Value kValue2Pi(2.0f * kPi);
    const cr::Value kValueSqrt2(kSqrt2);
    const cr::Value kValueNegOne(-1.0f);

} // namespace constants
} // namespace vital
