#pragma once

#include "common.h"
#include "lookup_table.h"
#include "synth_constants.h"

namespace vital {

  class Processor;

  /**
   * @class SynthFilter
   * @brief Abstract base class for Vital’s synthesizer filters.
   *
   * SynthFilter defines the interface and shared utilities for filters in the
   * Vital synthesizer. It manages the internal FilterState and offers a
   * static method for creating specific filter models (analog, ladder, comb, etc.).
   */
  class SynthFilter {
    public:
      /**
       * @brief Computes a one-pole filter coefficient from a frequency ratio.
       * @param frequency_ratio The ratio of the desired cutoff frequency to the sampling rate’s Nyquist.
       * @return A pre-warped filter coefficient (scaled by tan).
       */
      static force_inline mono_float computeOnePoleFilterCoefficient(mono_float frequency_ratio) {
        // Limit the warp value to avoid extreme or unstable coefficients
        static constexpr float kMaxRads = 0.499f * kPi;
        mono_float scaled = frequency_ratio * vital::kPi;
        return std::tan(std::min(kMaxRads, scaled / (scaled + 1.0f)));
      }

      /**
       * @typedef CoefficientLookup
       * @brief A lookup table for quick computation of one-pole filter coefficients.
       */
      typedef OneDimLookup<computeOnePoleFilterCoefficient, 2048> CoefficientLookup;

      /**
       * @brief Static instance of the coefficient lookup table, generated at compile time.
       */
      static const CoefficientLookup coefficient_lookup_;

      /**
       * @brief Retrieves a pointer to the static coefficient lookup table.
       * @return Pointer to the static CoefficientLookup.
       */
      static const CoefficientLookup* getCoefficientLookup() { return &coefficient_lookup_; }

      /**
       * @enum FilterInputs
       * @brief Enumerates indices for filter inputs in Vital’s processing system.
       */
      enum {
        kAudio,       ///< Audio input index
        kReset,       ///< Reset signal
        kMidiCutoff,  ///< MIDI-based cutoff parameter
        kResonance,   ///< Resonance parameter
        kDriveGain,   ///< Drive amount in dB
        kGain,        ///< Additional gain
        kStyle,       ///< Filter style (12 dB, 24 dB, etc.)
        kPassBlend,   ///< Blending parameter for low-pass, high-pass, band-pass
        kInterpolateX,///< For formant or XY interpolation
        kInterpolateY,///< For formant or XY interpolation
        kTranspose,   ///< MIDI transpose in semitones
        kSpread,      ///< Additional parameter for e.g. formant spread
        kNumInputs    ///< Number of total inputs
      };

      /**
       * @enum Style
       * @brief Different filter styles used in various derived classes.
       */
      enum Style {
        k12Db,
        k24Db,
        kNotchPassSwap,
        kDualNotchBand,
        kBandPeakNotch,
        kShelving,
        kNumStyles
      };

      /**
       * @class FilterState
       * @brief Holds the parameters necessary to configure a SynthFilter at runtime.
       *
       * This struct stores values like MIDI cutoff, resonance, drive, gain, style,
       * pass blend, interpolation, and transpose. It can be loaded from the Processor
       * inputs via loadSettings().
       */
      class FilterState {
        public:
          /**
           * @brief Default constructor, initializes with standard default values.
           */
          FilterState()
            : midi_cutoff(1.0f),
              midi_cutoff_buffer(nullptr),
              resonance_percent(0.0f),
              drive(1.0f),
              drive_percent(0.0f),
              gain(0.0f),
              style(0),
              pass_blend(0.0f),
              interpolate_x(0.5f),
              interpolate_y(0.5f),
              transpose(0.0f) { }

          poly_float midi_cutoff;           ///< MIDI note-based cutoff value
          const poly_float* midi_cutoff_buffer; ///< Pointer to the buffer storing per-sample MIDI cutoff
          poly_float resonance_percent;     ///< Resonance parameter in [0..1]
          poly_float drive;                 ///< Drive in linear magnitude
          poly_float drive_percent;         ///< Normalized drive parameter in [0..1]
          poly_float gain;                  ///< Additional gain parameter
          int style;                        ///< Filter style enum (e.g., k12Db, k24Db)
          poly_float pass_blend;           ///< Blend parameter in [0..2], controlling pass type
          poly_float interpolate_x;         ///< Interpolation X coordinate (e.g., for formant filters)
          poly_float interpolate_y;         ///< Interpolation Y coordinate (e.g., for formant filters)
          poly_float transpose;             ///< Transpose in semitones (applied to midi_cutoff)

          /**
           * @brief Loads state from a Processor’s input signals (MIDI cutoff, drive, style, etc.).
           * @param processor The Processor to query for input data.
           */
          void loadSettings(Processor* processor);
      };

      /**
       * @brief Virtual destructor for the SynthFilter base class.
       */
      virtual ~SynthFilter() { }

      /**
       * @brief Configures the filter’s parameters from the given FilterState.
       * @param filter_state The FilterState object with new settings.
       */
      virtual void setupFilter(const FilterState& filter_state) = 0;

      /**
       * @brief Factory method for creating a specialized filter based on a model enum.
       * @param model The selected filter model (analog, ladder, comb, etc.).
       * @return A pointer to the newly allocated SynthFilter instance, or nullptr if invalid.
       */
      static SynthFilter* createFilter(constants::FilterModel model);

    protected:
      /**
       * @brief Internal storage of the most recent FilterState, used by derived filters.
       */
      FilterState filter_state_;

      JUCE_LEAK_DETECTOR(SynthFilter)
  };
} // namespace vital
