#pragma once

#include "processor.h"
#include "synth_filter.h"

namespace vital {

  /**
   * @class DigitalSvf
   * @brief A state-variable filter (SVF) implementation, supporting multiple filter types (12/24 dB, shelving, dual modes).
   *
   * The DigitalSvf class provides a flexible filter design that can morph between low-pass,
   * high-pass, band-pass, notch, peak, and specialized dual filter modes. It optionally
   * supports a basic or advanced processing path, drive compensation, and user-defined
   * resonance bounds.
   */
  class DigitalSvf : public Processor, public SynthFilter {
    public:
      /**
       * @brief Default minimum resonance used when filtering (if not overridden).
       */
      static constexpr mono_float kDefaultMinResonance = 0.5f;

      /**
       * @brief Default maximum resonance used when filtering (if not overridden).
       */
      static constexpr mono_float kDefaultMaxResonance = 16.0f;

      /**
       * @brief Minimum allowed cutoff frequency in Hz for the filter.
       */
      static constexpr mono_float kMinCutoff = 1.0f;

      /**
       * @brief Maximum gain in dB for shelf or gain-based operations.
       */
      static constexpr mono_float kMaxGain = 15.0f;

      /**
       * @brief Minimum gain in dB for shelf or gain-based operations.
       */
      static constexpr mono_float kMinGain = -15.0f;

      /**
       * @brief Computes a one-pole SVF coefficient from a normalized frequency ratio.
       *
       * Ensures the ratio is clamped so that tan() doesn’t blow up near Nyquist.
       *
       * @param frequency_ratio The normalized frequency in [0..0.5).
       * @return The computed one-pole filter coefficient.
       */
      static force_inline mono_float computeSvfOnePoleFilterCoefficient(mono_float frequency_ratio) {
        static constexpr float kMaxRatio = 0.499f;
        return std::tan(std::min(kMaxRatio, frequency_ratio) * vital::kPi);
      }

      /**
       * @typedef SvfCoefficientLookup
       * @brief A lookup table type for quickly converting frequency ratios into filter coefficients.
       */
      typedef OneDimLookup<computeSvfOnePoleFilterCoefficient, 2048> SvfCoefficientLookup;

      /**
       * @brief A static global lookup table instance for SVF coefficients.
       */
      static const SvfCoefficientLookup svf_coefficient_lookup_;

      /**
       * @brief Retrieves a pointer to the global SVF coefficient lookup table.
       *
       * @return Pointer to the @c svf_coefficient_lookup_.
       */
      static const SvfCoefficientLookup* getSvfCoefficientLookup() { return &svf_coefficient_lookup_; }

      /**
       * @struct FilterValues
       * @brief Stores three filter state variables (v0, v1, v2) used for multi-mode mixing.
       */
      struct FilterValues {
        poly_float v0; ///< Additional mixing or amplitude value
        poly_float v1; ///< Typically the band or mid portion
        poly_float v2; ///< Typically the low or high portion

        /**
         * @brief Resets all filter values to zero (for all voices).
         */
        void hardReset() {
          v0 = 0.0f;
          v1 = 0.0f;
          v2 = 0.0f;
        }

        /**
         * @brief Selectively resets values for voices specified by @p reset_mask,
         *        otherwise keeps the current values.
         *
         * @param reset_mask A mask determining which voices should be reset.
         * @param other      The FilterValues to copy from.
         */
        void reset(poly_mask reset_mask, const FilterValues& other) {
          v0 = utils::maskLoad(v0, other.v0, reset_mask);
          v1 = utils::maskLoad(v1, other.v1, reset_mask);
          v2 = utils::maskLoad(v2, other.v2, reset_mask);
        }

        /**
         * @brief Computes the per-sample increments needed to move from this FilterValues
         *        state to @p target over a certain fraction of a block (increment).
         *
         * @param target   The desired end-state of filter values.
         * @param increment The fraction or ratio for interpolation per sample.
         * @return A FilterValues struct representing the increments per sample.
         */
        FilterValues getDelta(const FilterValues& target, mono_float increment) {
          FilterValues result;
          result.v0 = (target.v0 - v0) * increment;
          result.v1 = (target.v1 - v1) * increment;
          result.v2 = (target.v2 - v2) * increment;
          return result;
        }

        /**
         * @brief Increments the filter values by the amounts specified in @p delta.
         *
         * @param delta The increments to apply per sample.
         */
        force_inline void increment(const FilterValues& delta) {
          v0 += delta.v0;
          v1 += delta.v1;
          v2 += delta.v2;
        }
      };

      /**
       * @brief Constructor that initializes the filter’s internal states.
       */
      DigitalSvf();

      /** @brief Default destructor. */
      virtual ~DigitalSvf() { }

      /**
       * @brief Creates a clone of this filter by invoking the copy constructor.
       *
       * @return A pointer to a newly allocated DigitalSvf object.
       */
      virtual Processor* clone() const override { return new DigitalSvf(*this); }

      /**
       * @brief Processes a block of samples by pulling from the primary audio input
       *        and computing the SVF output. Delegates to @c processWithInput().
       *
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes a block of samples using a provided input buffer.
       *
       * Applies configured filter settings to compute the final output.
       *
       * @param audio_in    Pointer to the input audio buffer.
       * @param num_samples Number of samples to process.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Resets internal filter states for voices specified by the @p reset_masks.
       *
       * @param reset_masks Mask specifying which voices to reset.
       */
      void reset(poly_mask reset_masks) override;

      /**
       * @brief Performs a complete reset of the filter states for all voices.
       */
      void hardReset() override;

      /**
       * @brief Configures this SVF based on a FilterState (cutoff, resonance, style, etc.).
       *
       * @param filter_state The FilterState containing all relevant parameters.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Sets the minimum and maximum resonance for the filter (used in resonance interpolation).
       *
       * @param min The new minimum resonance value.
       * @param max The new maximum resonance value.
       */
      void setResonanceBounds(mono_float min, mono_float max);

      /**
       * @brief Processes a 12 dB filter style, iterating through the block.
       *
       * @param audio_in             Input audio data.
       * @param num_samples          Number of samples in the buffer.
       * @param current_resonance    Current resonance value (possibly interpolated).
       * @param current_drive        Current drive value.
       * @param current_post_multiply Additional multiplier after the filter.
       * @param blends               Filter mix gains (low/band/high).
       */
      void process12(const poly_float* audio_in, int num_samples,
                     poly_float current_resonance, poly_float current_drive,
                     poly_float current_post_multiply, FilterValues& blends);

      /**
       * @brief Processes a simpler 12 dB filter style, skipping extra color or overshoot logic.
       *
       * @param audio_in             Input audio data.
       * @param num_samples          Number of samples in the buffer.
       * @param current_resonance    Current resonance value.
       * @param current_drive        Current drive value.
       * @param current_post_multiply Additional multiplier after the filter.
       * @param blends               Filter mix gains (low/band/high).
       */
      void processBasic12(const poly_float* audio_in, int num_samples,
                          poly_float current_resonance, poly_float current_drive,
                          poly_float current_post_multiply, FilterValues& blends);

      /**
       * @brief Processes a 24 dB filter style, adding additional stages.
       *
       * @param audio_in             Input audio data.
       * @param num_samples          Number of samples in the buffer.
       * @param current_resonance    Current resonance value.
       * @param current_drive        Current drive value.
       * @param current_post_multiply Additional multiplier after the filter.
       * @param blends               Filter mix gains (low/band/high).
       */
      void process24(const poly_float* audio_in, int num_samples,
                     poly_float current_resonance, poly_float current_drive,
                     poly_float current_post_multiply, FilterValues& blends);

      /**
       * @brief Processes a simpler 24 dB filter style, skipping advanced processing.
       *
       * @param audio_in             Input audio data.
       * @param num_samples          Number of samples in the buffer.
       * @param current_resonance    Current resonance value.
       * @param current_drive        Current drive value.
       * @param current_post_multiply Additional multiplier after the filter.
       * @param blends               Filter mix gains (low/band/high).
       */
      void processBasic24(const poly_float* audio_in, int num_samples,
                          poly_float current_resonance, poly_float current_drive,
                          poly_float current_post_multiply, FilterValues& blends);

      /**
       * @brief Processes a dual filter mode, e.g., dual notch + band pass.
       *
       * Splits filter processing into two sets of FilterValues (blends1 and blends2).
       *
       * @param audio_in             Input audio data.
       * @param num_samples          Number of samples to process.
       * @param current_resonance    Current resonance value.
       * @param current_drive        Current drive value.
       * @param current_post_multiply Additional multiplier.
       * @param blends1              The first set of filter mixes (low/band/high).
       * @param blends2              The second set of filter mixes (low/band/high).
       */
      void processDual(const poly_float* audio_in, int num_samples,
                       poly_float current_resonance, poly_float current_drive,
                       poly_float current_post_multiply,
                       FilterValues& blends1, FilterValues& blends2);

      /**
       * @brief Applies advanced distortion to the input while performing a single SVF tick (12 dB).
       *
       * @param audio_in  The current input sample.
       * @param drive     Amount of drive (pre-gain).
       * @param resonance The filter resonance.
       * @param coefficient The filter coefficient derived from cutoff.
       * @param blends    The low/band/high mix values.
       * @return The final filter output for the sample.
       */
      force_inline poly_float tick(poly_float audio_in, poly_float coefficient,
                                   poly_float resonance, poly_float drive, FilterValues& blends);

      /**
       * @brief A basic (non-distorting) single SVF tick for a 12 dB filter style.
       *
       * @param audio_in   The current input sample.
       * @param coefficient The filter coefficient derived from cutoff.
       * @param resonance  The filter resonance.
       * @param drive      The input drive multiplier.
       * @param blends     The filter’s mix values for low/band/high output.
       * @return The filter output for the sample.
       */
      force_inline poly_float tickBasic(poly_float audio_in, poly_float coefficient,
                                        poly_float resonance, poly_float drive, FilterValues& blends);

      /**
       * @brief Tick function for a 24 dB multi-stage filter, adding an additional pre-stage.
       *
       * @param audio_in   The current input sample.
       * @param coefficient The filter coefficient.
       * @param resonance  The filter resonance.
       * @param drive      The input drive multiplier.
       * @param blends     The filter’s mix values (low/band/high).
       * @return The filter output for the sample.
       */
      force_inline poly_float tick24(poly_float audio_in, poly_float coefficient,
                                     poly_float resonance, poly_float drive, FilterValues& blends);

      /**
       * @brief A simpler 24 dB tick function without advanced distortion or color.
       *
       * @param audio_in   The current input sample.
       * @param coefficient The filter coefficient.
       * @param resonance  The filter resonance.
       * @param drive      The input drive multiplier.
       * @param blends     The filter’s mix values (low/band/high).
       * @return Filtered sample.
       */
      force_inline poly_float tickBasic24(poly_float audio_in, poly_float coefficient,
                                          poly_float resonance, poly_float drive, FilterValues& blends);

      /**
       * @brief Tick function for a dual filter approach, e.g. notch + band, etc.
       *
       * @param audio_in   The current input sample.
       * @param coefficient The filter coefficient (cutoff).
       * @param resonance  The filter resonance.
       * @param drive      The input drive multiplier.
       * @param blends1    Low/band/high mixing for the first filter pass.
       * @param blends2    Low/band/high mixing for the second filter pass.
       * @return Filtered sample after both filter passes.
       */
      force_inline poly_float tickDual(poly_float audio_in, poly_float coefficient,
                                       poly_float resonance, poly_float drive,
                                       FilterValues& blends1, FilterValues& blends2);

      /**
       * @brief Retrieves the final drive (post drive compensation) used in the filter.
       * @return The computed drive multiplied by post_multiply.
       */
      poly_float getDrive() const { return drive_ * post_multiply_; }

      /**
       * @brief Retrieves the current MIDI-based cutoff frequency.
       * @return The stored MIDI cutoff value.
       */
      poly_float getMidiCutoff() const { return midi_cutoff_; }

      /**
       * @brief Retrieves the current resonance value (inverted if needed).
       * @return The stored resonance used by the filter.
       */
      poly_float getResonance() const { return resonance_; }

      /**
       * @brief Retrieves the current low-frequency mix portion.
       * @return The low portion mix amount.
       */
      poly_float getLowAmount() const { return low_amount_; }

      /**
       * @brief Retrieves the current band-frequency mix portion.
       * @return The band portion mix amount.
       */
      poly_float getBandAmount() const { return band_amount_; }

      /**
       * @brief Retrieves the current high-frequency mix portion.
       * @return The high portion mix amount.
       */
      poly_float getHighAmount() const { return high_amount_; }

      /**
       * @brief Helper for a 24 dB filter style that may swap low/high in a dual notch band.
       *
       * @param style An integer enumerating the filter style.
       * @return The appropriate low amount for a 24 dB style.
       */
      poly_float getLowAmount24(int style) const {
        if (style == kDualNotchBand)
          return high_amount_;
        return low_amount_;
      }

      /**
       * @brief Helper for a 24 dB filter style that may swap low/high in a dual notch band.
       *
       * @param style An integer enumerating the filter style.
       * @return The appropriate high amount for a 24 dB style.
       */
      poly_float getHighAmount24(int style) const {
        if (style == kDualNotchBand)
          return low_amount_;
        return high_amount_;
      }

      /**
       * @brief Sets whether this filter should use a simpler, “basic” processing path.
       *
       * @param basic If true, uses the simplified process path.
       */
      void setBasic(bool basic) { basic_ = basic; }

      /**
       * @brief Enables or disables drive compensation (reducing drive as resonance increases).
       *
       * @param drive_compensation If true, drive is reduced as resonance goes up.
       */
      void setDriveCompensation(bool drive_compensation) { drive_compensation_ = drive_compensation; }

    private:
      /**
       * @brief MIDI-based cutoff (in Hz) that dictates the filter coefficient lookups.
       */
      poly_float midi_cutoff_;

      /**
       * @brief Internal resonance value used in the filter computations.
       */
      poly_float resonance_;

      /**
       * @brief First set of filter mixing values (low, band, high).
       */
      FilterValues blends1_;

      /**
       * @brief Second set of filter mixing values, used in dual filter modes.
       */
      FilterValues blends2_;

      /**
       * @brief Pre-gain factor for overdriving or scaling input samples.
       */
      poly_float drive_;

      /**
       * @brief Post-multiplier factor, adjusting output gain or applying additional compensation.
       */
      poly_float post_multiply_;

      /**
       * @brief Low-frequency portion of the filter output mix in certain styles.
       */
      poly_float low_amount_;

      /**
       * @brief Band-frequency portion of the filter output mix in certain styles.
       */
      poly_float band_amount_;

      /**
       * @brief High-frequency portion of the filter output mix in certain styles.
       */
      poly_float high_amount_;

      /**
       * @brief Intermediate filter states for a 24 dB approach’s first stage.
       */
      poly_float ic1eq_pre_, ic2eq_pre_;

      /**
       * @brief State variables for the main filter (one or multiple stages).
       */
      poly_float ic1eq_, ic2eq_;

      /**
       * @brief Defines the minimum allowed resonance for this filter.
       */
      mono_float min_resonance_;

      /**
       * @brief Defines the maximum allowed resonance for this filter.
       */
      mono_float max_resonance_;

      /**
       * @brief Whether to use a simplified filter path (skip advanced coloration).
       */
      bool basic_;

      /**
       * @brief Whether to apply drive compensation, reducing drive as resonance increases.
       */
      bool drive_compensation_;

      JUCE_LEAK_DETECTOR(DigitalSvf)
  };
} // namespace vital
