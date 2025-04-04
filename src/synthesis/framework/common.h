#pragma once

#include "JuceHeader.h"

// Debugging assertions.
// For debug builds, VITAL_ASSERT(x) behaves like assert(x), otherwise it's a no-op.
#if DEBUG
#include <cassert>
#define VITAL_ASSERT(x) assert(x)
#else
#define VITAL_ASSERT(x) ((void)0)
#endif // DEBUG

// Marks a variable or parameter as unused to avoid compiler warnings.
#define UNUSED(x) ((void)x)

// Force inline macros for compilers.
#if !defined(force_inline)
#if defined (_MSC_VER)
#define force_inline __forceinline
  #define vector_call __vectorcall
#else
#define force_inline inline __attribute__((always_inline))
#define vector_call
#endif
#endif

#include "poly_values.h"

namespace vital {

    // The base floating-point type for monophonic calculations.
    typedef float mono_float;

    // Mathematical and audio-related constants used throughout the synthesis framework.
    constexpr mono_float kPi = 3.1415926535897932384626433832795f;       ///< Pi constant.
    constexpr mono_float kSqrt2 = 1.414213562373095048801688724209698f; ///< Square root of 2.
    constexpr mono_float kEpsilon = 1e-16f;                             ///< A small epsilon for floating comparisons.
    constexpr int kMaxBufferSize = 128;                                 ///< Maximum buffer size for processing.
    constexpr int kMaxOversample = 8;                                   ///< Maximum allowed oversampling factor.
    constexpr int kDefaultSampleRate = 44100;                           ///< Default sample rate in Hz.
    constexpr mono_float kMinNyquistMult = 0.45351473923f;              ///< Minimum ratio relative to Nyquist frequency.
    constexpr int kMaxSampleRate = 192000;                              ///< Maximum expected sample rate in Hz.
    constexpr int kMidiSize = 128;                                      ///< MIDI note count (0-127).
    constexpr int kMidiTrackCenter = 60;                                ///< MIDI note considered as center (Middle C).

    constexpr mono_float kMidi0Frequency = 8.1757989156f;               ///< Frequency of MIDI note 0 (C-1).
    constexpr mono_float kDbfsIncrease = 6.0f;                          ///< A gain increase of 6 dB.
    constexpr int kDegreesPerCycle = 360;                               ///< Degrees in a full rotation (for LFO phases).
    constexpr int kMsPerSec = 1000;                                     ///< Milliseconds per second.
    constexpr int kNotesPerOctave = 12;                                 ///< Number of semitones per octave.
    constexpr int kCentsPerNote = 100;                                  ///< Number of cents per semitone.
    constexpr int kCentsPerOctave = kNotesPerOctave * kCentsPerNote;    ///< Cents per octave (1200).

    constexpr int kPpq = 960;                                           ///< Pulses per quarter note used internally.
    constexpr mono_float kVoiceKillTime = 0.05f;                        ///< Time in seconds after which a silent voice is considered dead.
    constexpr int kNumMidiChannels = 16;                                ///< MIDI channels available per device.
    constexpr int kFirstMidiChannel = 0;                                ///< The first MIDI channel index.
    constexpr int kLastMidiChannel = kNumMidiChannels - 1;              ///< The last MIDI channel index.

    /**
     * @enum VoiceEvent
     * @brief Enumerates different states or events of a synth voice's lifecycle.
     *
     * kInvalid: Invalid state.
     * kVoiceIdle: Voice is idle and not producing sound.
     * kVoiceOn: Voice is triggered on (note-on event).
     * kVoiceHold: Voice is holding a steady state (sustained note).
     * kVoiceDecay: Voice is in the release/decay phase after note-off but still audible.
     * kVoiceOff: Voice has received a note-off event and is fading out.
     * kVoiceKill: Voice is to be terminated and reused for another note.
     * kNumVoiceEvents: Number of possible voice events.
     */
    enum VoiceEvent {
        kInvalid,
        kVoiceIdle,
        kVoiceOn,
        kVoiceHold,
        kVoiceDecay,
        kVoiceOff,
        kVoiceKill,
        kNumVoiceEvents
    };
} // namespace vital

