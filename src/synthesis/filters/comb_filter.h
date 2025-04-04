#pragma once

#include "processor.h"
#include "synth_filter.h"

#include "memory.h"
#include "one_pole_filter.h"

namespace vital {

  /**
   * @class CombFilter
   * @brief A Processor implementing a comb-based filter with multiple feedback styles.
   *
   * The CombFilter supports comb, positive flange, and negative flange feedback variations,
   * with options to blend low/high filter responses or spread band filters.
   */
  class CombFilter : public Processor, public SynthFilter {
    public:
      /**
       * @enum FeedbackStyle
       * @brief Types of feedback for the comb filter (comb, positive/negative flange).
       */
      enum FeedbackStyle {
        kComb,            ///< Standard comb filtering
        kPositiveFlange,  ///< Positive flanging effect
        kNegativeFlange,  ///< Negative flanging effect
        kNumFeedbackStyles
      };

      /**
       * @enum FilterStyle
       * @brief Types of filter styles (blend of low/high, band spread).
       */
      enum FilterStyle {
        kLowHighBlend, ///< Blend low and high output from filter
        kBandSpread,   ///< Spread the band around center frequency
        kNumFilterStyles
      };

      /**
       * @brief Converts an integer to a valid FeedbackStyle, wrapping around kNumFeedbackStyles.
       *
       * @param style Integer style index.
       * @return Corresponding FeedbackStyle.
       */
      static FeedbackStyle getFeedbackStyle(int style) {
        return static_cast<FeedbackStyle>(style % kNumFeedbackStyles);
      }

      /**
       * @brief Converts an integer to a valid FilterStyle, taking advantage of the style integer layout.
       *
       * @param style Integer style index.
       * @return Corresponding FilterStyle.
       */
      static FilterStyle getFilterStyle(int style) {
        return static_cast<FilterStyle>(style / kNumFeedbackStyles);
      }

      /**
       * @brief Total number of distinct filter types (FeedbackStyle x FilterStyle).
       */
      static constexpr int kNumFilterTypes = kNumFilterStyles * kNumFeedbackStyles;

      /**
       * @brief Range of band spread in octaves.
       */
      static constexpr mono_float kBandOctaveRange = 8.0f;

      /**
       * @brief Minimum band spread in octaves.
       */
      static constexpr mono_float kBandOctaveMin = 0.0f;

      /**
       * @brief Minimum period for the comb filter delay line.
       */
      static constexpr int kMinPeriod = 2;

      /**
       * @brief Scaling factor for the comb filter input signal.
       */
      static constexpr mono_float kInputScale = 0.5f;

      /**
       * @brief Maximum allowable feedback amount.
       */
      static constexpr mono_float kMaxFeedback = 1.0f;

      /**
       * @brief Constructs a CombFilter with a given memory buffer size.
       *
       * @param size The initial size of the delay memory buffer.
       */
      CombFilter(int size = kMinPeriod);

      /**
       * @brief Copy constructor for CombFilter.
       *
       * Creates a deep copy of the memory buffer and resets relevant states.
       *
       * @param other The CombFilter instance to copy.
       */
      CombFilter(const CombFilter& other);

      /**
       * @brief Destructor. Cleans up allocated memory.
       */
      virtual ~CombFilter();

      /**
       * @brief Creates a clone of this filter by invoking the copy constructor.
       *
       * @return A new CombFilter that is a copy of this one.
       */
      virtual Processor* clone() const override {
        return new CombFilter(*this);
      }

      /**
       * @brief Sets up the CombFilter state based on a FilterState struct.
       *
       * Populates feedback style, filter style, and corresponding coefficients.
       *
       * @param filter_state The FilterState struct with style, resonance, cutoff, etc.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Processes a block of samples, choosing the appropriate feedback style to apply.
       *
       * @param num_samples Number of samples in the audio buffer.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief A templated function to handle processing for each feedback style implementation.
       *
       * @tparam tick Pointer to the function that does the per-sample comb/flange processing.
       * @param num_samples Number of samples in the buffer.
       */
      template<poly_float(*tick)(poly_float, Memory*, OnePoleFilter<>&, OnePoleFilter<>&,
                                 poly_float, poly_float, poly_float, poly_float,
                                 poly_float, poly_float, poly_float)>
      void processFilter(int num_samples);

      /**
       * @brief Resets the filter state, clearing memory and reinitializing variables.
       *
       * @param reset_mask A poly_mask indicating which voices should be reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Resets the filter completely for all voices.
       */
      void hardReset() override;

      /**
       * @brief Getter for the drive (scale) parameter controlling input amplitude scaling.
       *
       * @return The current scale value.
       */
      poly_float getDrive() { return scale_; }

      /**
       * @brief Getter for the feedback parameter controlling comb/flange feedback amount.
       *
       * @return The current feedback value.
       */
      poly_float getResonance() { return feedback_; }

      /**
       * @brief Getter for the low-frequency gain used in filter blending.
       *
       * @return The current low gain value.
       */
      poly_float getLowAmount() { return low_gain_; }

      /**
       * @brief Getter for the high-frequency gain used in filter blending.
       *
       * @return The current high gain value.
       */
      poly_float getHighAmount() { return high_gain_; }

      /**
       * @brief Getter for the primary filter MIDI cutoff.
       *
       * @return The current primary MIDI cutoff.
       */
      poly_float getFilterMidiCutoff() { return filter_midi_cutoff_; }

      /**
       * @brief Getter for the secondary filter MIDI cutoff (used in band-spread style).
       *
       * @return The current secondary MIDI cutoff.
       */
      poly_float getFilter2MidiCutoff() { return filter2_midi_cutoff_; }

    protected:
      /**
       * @brief Pointer to the Memory buffer used for the comb delay line.
       */
      std::unique_ptr<Memory> memory_;

      /**
       * @brief Current feedback style (comb, positive flange, negative flange).
       */
      FeedbackStyle feedback_style_;

      /**
       * @brief The computed maximum delay period based on input frequency and sample rate.
       */
      poly_float max_period_;

      /**
       * @brief Current feedback amount for the comb/flange filter.
       */
      poly_float feedback_;

      /**
       * @brief Coefficient for the one-pole feedback filter (low pass).
       */
      poly_float filter_coefficient_;

      /**
       * @brief Secondary coefficient for the band-spread or second filter stage.
       */
      poly_float filter2_coefficient_;

      /**
       * @brief Gain applied to the low output in the low/high blend mode.
       */
      poly_float low_gain_;

      /**
       * @brief Gain applied to the high output in the low/high blend mode.
       */
      poly_float high_gain_;

      /**
       * @brief Scaling multiplier applied to the incoming audio or feedback path.
       */
      poly_float scale_;

      /**
       * @brief MIDI note value controlling the main filter’s cutoff frequency.
       */
      poly_float filter_midi_cutoff_;

      /**
       * @brief MIDI note value controlling the secondary filter’s cutoff frequency (in band-spread mode).
       */
      poly_float filter2_midi_cutoff_;

      /**
       * @brief One-pole filter for the feedback path (first stage).
       */
      OnePoleFilter<> feedback_filter_;

      /**
       * @brief One-pole filter for the feedback path (second stage in certain styles).
       */
      OnePoleFilter<> feedback_filter2_;

      JUCE_LEAK_DETECTOR(CombFilter)
  };
} // namespace vital
