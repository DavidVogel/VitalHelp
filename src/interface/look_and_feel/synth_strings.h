#pragma once

#include <string>
#include "synth_constants.h"

/**
 * @namespace strings
 * @brief Contains a variety of string arrays used to label UI elements, parameters, modes, and options throughout the synthesizer.
 *
 * The arrays defined in this namespace serve as human-readable names for various parameters and modes within the synthesizer's UI.
 * For instance, they are used to populate combo boxes, describe parameter states, and display text in the interface.
 */
namespace strings {
    /**
     * @var kOffOnNames
     * @brief Off/On state names.
     */
    const std::string kOffOnNames[] = {
            "Off",
            "On"
    };

    /**
     * @var kWavetableDimensionNames
     * @brief Names for wavetable dimension display modes.
     */
    const std::string kWavetableDimensionNames[] = {
            "3D",
            "2D",
            "SP"
    };

    /**
     * @var kOversamplingNames
     * @brief Names for oversampling rates.
     */
    const std::string kOversamplingNames[] = {
            "1x",
            "2x",
            "4x",
            "8x"
    };

    /**
     * @var kDelayStyleNames
     * @brief Names for delay line styles (mono, stereo, ping-pong, etc.).
     */
    const std::string kDelayStyleNames[] = {
            "Mono",
            "Stereo",
            "Ping Pong",
            "Mid Ping Pong",
    };

    /**
     * @var kCompressorBandNames
     * @brief Full names for compressor band modes.
     */
    const std::string kCompressorBandNames[] = {
            "Multiband",
            "Low Band",
            "High Band",
            "Single Band"
    };

    /**
     * @var kCompressorBandShortNames
     * @brief Shorter alternative names for compressor band modes.
     */
    const std::string kCompressorBandShortNames[] = {
            "Multiband",
            "Low Band",
            "High Band",
            "Single"
    };

    /**
     * @var kPresetStyleNames
     * @brief Names for preset categories or styles.
     */
    const std::string kPresetStyleNames[] = {
            "Bass",
            "Lead",
            "Keys",
            "Pad",
            "Percussion",
            "Sequence",
            "Experiment",
            "SFX",
            "Template",
    };

    /**
     * @var kUnisonStackNames
     * @brief Names for different unison voice stacking modes.
     */
    const std::string kUnisonStackNames[] = {
            "Unison",
            "Center Drop 12",
            "Center Drop 24",
            "Octave",
            "2x Octave",
            "Power Chord",
            "2x Power Chord",
            "Major Chord",
            "Minor Chord",
            "Harmonics",
            "Odd Harmonics",
    };

    /**
     * @var kFilterStyleNames
     * @brief Filter style names for visualizing different filter slopes and blends.
     */
    const std::string kFilterStyleNames[] = {
            "12dB",
            "24dB",
            "Notch Blend",
            "Notch Spread",
            "B/P/N"
    };

    /**
     * @var kDiodeStyleNames
     * @brief Special diode filter style names.
     */
    const std::string kDiodeStyleNames[] = {
            "Low Shelf",
            "Low Cut"
    };

    /**
     * @var kCombStyleNames
     * @brief Names for comb and flanger filter styles.
     */
    const std::string kCombStyleNames[] = {
            "Low High Comb",
            "Low High Flange+",
            "Low High Flange-",
            "Band Spread Comb",
            "Band Spread Flange+",
            "Band Spread Flange-"
    };

    /**
     * @var kFrequencySyncNames
     * @brief Names for frequency synchronization modes (time, tempo, keytrack, etc.).
     */
    const std::string kFrequencySyncNames[] = {
            "Seconds",
            "Tempo",
            "Tempo Dotted",
            "Tempo Triplets",
            "Keytrack"
    };

    /**
     * @var kDistortionTypeShortNames
     * @brief Short names for various distortion types.
     */
    const std::string kDistortionTypeShortNames[] = {
            "Soft Clip",
            "Hard Clip",
            "Lin Fold",
            "Sine Fold",
            "Bit Crush",
            "Down Samp",
    };

    /**
     * @var kDistortionTypeNames
     * @brief More descriptive names for various distortion types.
     */
    const std::string kDistortionTypeNames[] = {
            "Soft Clip",
            "Hard Clip",
            "Linear Fold",
            "Sine Fold",
            "Bit Crush",
            "Down Sample",
    };

    /**
     * @var kDistortionFilterOrderNames
     * @brief Filter order names for distortion section (pre, post, none).
     */
    const std::string kDistortionFilterOrderNames[] = {
            "None",
            "Pre",
            "Post",
    };

    /**
     * @var kFilterModelNames
     * @brief Names of different filter models (analog, dirty, ladder, etc.).
     */
    const std::string kFilterModelNames[] = {
            "Analog",
            "Dirty",
            "Ladder",
            "Digital",
            "Diode",
            "Formant",
            "Comb",
            "Phaser",
    };

    /**
     * @var kPredefinedWaveformNames
     * @brief Common predefined oscillator waveforms.
     */
    const std::string kPredefinedWaveformNames[] = {
            "Sin",
            "Saturated Sin",
            "Triangle",
            "Square",
            "Pulse",
            "Saw",
    };

    /**
     * @var kEffectOrder
     * @brief Defines the processing order of various effects.
     */
    const std::string kEffectOrder[] = {
            "chorus",
            "compressor",
            "delay",
            "distortion",
            "eq",
            "filter_fx",
            "flanger",
            "phaser",
            "reverb"
    };

    /**
     * @var kSyncedFrequencyNames
     * @brief Names for synced LFO frequencies (Freeze, 32/1, 16/1, etc.).
     */
    const std::string kSyncedFrequencyNames[] = {
            "Freeze",
            "32/1",
            "16/1",
            "8/1",
            "4/1",
            "2/1",
            "1/1",
            "1/2",
            "1/4",
            "1/8",
            "1/16",
            "1/32",
            "1/64",
    };

    /**
     * @var kStereoModeNames
     * @brief Names for stereo routing modes.
     */
    const std::string kStereoModeNames[vital::StereoEncoder::kNumStereoModes] = {
            "SPREAD",
            "ROTATE"
    };

    /**
     * @var kSmoothModeNames
     * @brief Names for smoothing modes in parameters or LFO shapes.
     */
    const std::string kSmoothModeNames[] = {
            "FADE IN",
            "SMOOTH"
    };

    /**
     * @var kSyncShortNames
     * @brief Short names for different sync/re-triggering modes of modulators.
     */
    const std::string kSyncShortNames[] = {
            "Trigger",
            "Sync",
            "Envelope",
            "Sustain Env",
            "Loop Point",
            "Loop Hold",
    };

    /**
     * @var kSyncNames
     * @brief Full names for different sync/re-triggering modes of modulators.
     */
    const std::string kSyncNames[] = {
            "Trigger",
            "Sync",
            "Envelope",
            "Sustain Envelope",
            "Loop Point",
            "Loop Hold",
    };

    /**
     * @var kRandomShortNames
     * @brief Short names for different random generator modes.
     */
    const std::string kRandomShortNames[] = {
            "Perlin",
            "S & H",
            "Sine",
            "Lorenz",
    };

    /**
     * @var kRandomNames
     * @brief Full names for different random generator modes.
     */
    const std::string kRandomNames[] = {
            "Perlin",
            "Sample & Hold",
            "Sine Interpolate",
            "Lorenz Attractor",
    };

    /**
     * @var kPaintPatternNames
     * @brief Names for painting patterns in wavetable or LFO editing.
     */
    const std::string kPaintPatternNames[] = {
            "Step",
            "Half",
            "Down",
            "Up",
            "Tri"
    };

    /**
     * @var kVoicePriorityNames
     * @brief Voice allocation priority modes (e.g., newest, oldest note).
     */
    const std::string kVoicePriorityNames[] = {
            "Newest",
            "Oldest",
            "Highest",
            "Lowest",
            "Round Robin"
    };

    /**
     * @var kVoiceOverrideNames
     * @brief Behavior of what happens when voice limit is exceeded (kill or steal).
     */
    const std::string kVoiceOverrideNames[] = {
            "Kill",
            "Steal"
    };

    /**
     * @var kEqHighModeNames
     * @brief Names for EQ high band modes.
     */
    const std::string kEqHighModeNames[] = {
            "Shelf",
            "Low Pass"
    };

    /**
     * @var kEqBandModeNames
     * @brief Names for EQ band modes (Shelf or Notch).
     */
    const std::string kEqBandModeNames[] = {
            "Shelf",
            "Notch"
    };

    /**
     * @var kEqLowModeNames
     * @brief Names for EQ low band modes.
     */
    const std::string kEqLowModeNames[] = {
            "Shelf",
            "High Pass"
    };

    /**
     * @var kDestinationNames
     * @brief Names of possible audio routing destinations (filters, effects).
     */
    const std::string kDestinationNames[vital::constants::kNumSourceDestinations + vital::constants::kNumEffects] = {
            "FILTER 1",
            "FILTER 2",
            "FILTER 1+2",
            "EFFECTS",
            "DIRECT OUT",
            "CHORUS",
            "COMPRESSOR",
            "DELAY",
            "DISTORTION",
            "EQ",
            "FX FILTER",
            "FLANGER",
            "PHASER",
            "REVERB",
    };

    /**
     * @var kDestinationMenuNames
     * @brief Names for destinations as displayed in menus.
     */
    const std::string kDestinationMenuNames[vital::constants::kNumSourceDestinations + vital::constants::kNumEffects] = {
            "Filter 1",
            "Filter 2",
            "Filter 1+2",
            "Effects",
            "Direct Out",
            "Chours",
            "Compressor",
            "Delay",
            "Distortion",
            "Equalizer",
            "Effects Filter",
            "Flanger",
            "Phaser",
            "Reverb",
    };

    /**
     * @var kPhaseDistortionNames
     * @brief Names of different phase distortion modes.
     */
    const std::string kPhaseDistortionNames[] = {
            "None",
            "Sync",
            "Formant",
            "Quantize",
            "Bend",
            "Squeeze",
            "Pulse",
            "FM <- Osc",
            "FM <- Osc",
            "FM <- Sample",
            "RM <- Osc",
            "RM <- Osc",
            "RM <- Sample"
    };

    /**
     * @var kSpectralMorphNames
     * @brief Names for spectral morphing modes in the wavetable.
     */
    const std::string kSpectralMorphNames[] = {
            "None",
            "Vocode",
            "Formant Scale",
            "Harmonic Stretch",
            "Inharmonic Stretch",
            "Smear",
            "Random Amplitudes",
            "Low Pass",
            "High Pass",
            "Phase Disperse",
            "Shepard Tone",
            "Spectral Time Skew",
    };
} // namespace strings
