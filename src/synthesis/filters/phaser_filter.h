#pragma once

#include "processor.h"
#include "synth_filter.h"

#include "one_pole_filter.h"

namespace vital {

  /**
   * @class PhaserFilter
   * @brief A multi-stage phaser filter for the Vital synthesizer.
   *
   * The PhaserFilter class applies up to three sets of all-pass filter stages, each creating
   * notches/peaks in the frequency response. Adjustable parameters include resonance, drive,
   * and peak distribution. The filter can operate in either "clean" mode or a more distorted
   * mode, changing how saturation is applied to the filter feedback and input signals.
   */
  class PhaserFilter : public Processor, public SynthFilter {
    public:
      /**
       * @brief Minimum resonance value.
       */
      static constexpr mono_float kMinResonance = 0.0f;

      /**
       * @brief Maximum resonance value.
       */
      static constexpr mono_float kMaxResonance = 1.0f;

      /**
       * @brief Minimum cutoff frequency in Hz (used internally).
       */
      static constexpr mono_float kMinCutoff = 1.0f;

      /**
       * @brief A ratio used to remove/clear low frequencies or high frequencies in the phaser path.
       */
      static constexpr mono_float kClearRatio = 20.0f;

      /**
       * @brief Number of all-pass stages per peak cluster. (4 stages per cluster)
       */
      static constexpr int kPeakStage = 4;

      /**
       * @brief Maximum number of stages (3 clusters of 4 stages = 12 total).
       */
      static constexpr int kMaxStages = 3 * kPeakStage;

      /**
       * @brief Constructs a PhaserFilter object.
       * @param clean If true, uses a cleaner saturation mode on feedback; if false, uses a more distorted mode.
       */
      PhaserFilter(bool clean);

      /**
       * @brief Virtual destructor.
       */
      virtual ~PhaserFilter() { }

      /**
       * @brief Creates a clone (deep copy) of the PhaserFilter.
       * @return A pointer to the cloned PhaserFilter.
       */
      virtual Processor* clone() const override { return new PhaserFilter(*this); }

      /**
       * @brief Processes the audio buffer through the phaser effect.
       * @param num_samples The number of samples to process.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes a given input buffer through the phaser effect.
       * @param audio_in Pointer to the buffer containing input samples.
       * @param num_samples The number of samples to process.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Sets up the filter parameters (resonance, drive, peaks) based on the FilterState.
       * @param filter_state A reference to the FilterState containing relevant parameters.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Resets internal filter states for the specified voices.
       * @param reset_mask A bitmask indicating which voices need resetting.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Performs a full reset of the filter states (for all voices).
       */
      void hardReset() override;

      /**
       * @brief Toggles between a clean or distorted phaser mode.
       * @param clean If true, uses the cleaner mode; otherwise, uses the more distorted mode.
       */
      void setClean(bool clean) { clean_ = clean; }

      /**
       * @brief Gets the current resonance value.
       * @return The current resonance setting.
       */
      poly_float getResonance() { return resonance_; }

      /**
       * @brief Gets the current drive setting.
       * @return The current drive amount.
       */
      poly_float getDrive() { return drive_; }

      /**
       * @brief Gets the current peak1 (lowest peak cluster) mix amount.
       * @return The mix amount for the first peak cluster.
       */
      poly_float getPeak1Amount() { return peak1_amount_; }

      /**
       * @brief Gets the current peak3 (second peak cluster) mix amount.
       * @return The mix amount for the third-order peak cluster.
       */
      poly_float getPeak3Amount() { return peak3_amount_; }

      /**
       * @brief Gets the current peak5 (highest peak cluster) mix amount.
       * @return The mix amount for the fifth-order peak cluster.
       */
      poly_float getPeak5Amount() { return peak5_amount_; }

    private:
      /**
       * @brief Template method handling phaser processing based on saturation functions for resonance and input.
       *
       * The choice between a "clean" or "distorted" mode depends on which saturation functions are passed.
       *
       * @tparam saturateResonance A function pointer used for resonance feedback saturation.
       * @tparam saturateInput A function pointer used for input saturation.
       * @param audio_in Pointer to the input audio buffer.
       * @param num_samples The number of samples to process.
       */
      template <poly_float(*saturateResonance)(poly_float), poly_float(*saturateInput)(poly_float)>
      void process(const poly_float* audio_in, int num_samples) {
        // Cache current parameters
        poly_float current_resonance = resonance_;
        poly_float current_drive = drive_;
        poly_float current_peak1 = peak1_amount_;
        poly_float current_peak3 = peak3_amount_;
        poly_float current_peak5 = peak5_amount_;

        // Reload FilterState settings (in case they changed)
        filter_state_.loadSettings(this);
        setupFilter(filter_state_);

        // Check for resets
        poly_mask reset_mask = getResetMask(kReset);
        if (reset_mask.anyMask()) {
          reset(reset_mask);

          current_resonance = utils::maskLoad(current_resonance, resonance_, reset_mask);
          current_drive = utils::maskLoad(current_drive, drive_, reset_mask);
          current_peak1 = utils::maskLoad(current_peak1, peak1_amount_, reset_mask);
          current_peak3 = utils::maskLoad(current_peak3, peak3_amount_, reset_mask);
          current_peak5 = utils::maskLoad(current_peak5, peak5_amount_, reset_mask);
        }

        // Calculate per-sample increments for parameter smoothing
        mono_float tick_increment = 1.0f / num_samples;
        poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
        poly_float delta_drive = (drive_ - current_drive) * tick_increment;
        poly_float delta_peak1 = (peak1_amount_ - current_peak1) * tick_increment;
        poly_float delta_peak3 = (peak3_amount_ - current_peak3) * tick_increment;
        poly_float delta_peak5 = (peak5_amount_ - current_peak5) * tick_increment;

        poly_float* audio_out = output()->buffer;
        const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
        const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
        poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
        poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

        // Process each sample
        for (int i = 0; i < num_samples; ++i) {
          poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
          poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
          poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

          // Smoothly update parameters
          current_resonance += delta_resonance;
          current_drive += delta_drive;
          current_peak1 += delta_peak1;
          current_peak3 += delta_peak3;
          current_peak5 += delta_peak5;

          // Process a single sample through phaser stages
          tick<saturateResonance, saturateInput>(audio_in[i], coefficient,
                                                 current_resonance, current_drive,
                                                 current_peak1, current_peak3, current_peak5);

          // The phaser output is a blend of the allpass output and the original signal
          audio_out[i] = (audio_in[i] + invert_mult_ * allpass_output_) * 0.5f;
        }
      }

      /**
       * @brief Template function to process a single sample through the phaser’s stages.
       *
       * @tparam saturateResonance A function pointer used for resonance saturation.
       * @tparam saturateInput A function pointer used for input sample saturation.
       * @param audio_in The current input audio sample.
       * @param coefficient Main filter coefficient derived from the cutoff frequency.
       * @param resonance Current resonance value.
       * @param drive Current drive value.
       * @param peak1 Mix amount for the first set of 4 stages (peak1).
       * @param peak3 Mix amount for the second set of 4 stages (peak3).
       * @param peak5 Mix amount for the third set of 4 stages (peak5).
       */
      template <poly_float(*saturateResonance)(poly_float), poly_float(*saturateInput)(poly_float)>
      force_inline void tick(poly_float audio_in, poly_float coefficient,
                             poly_float resonance, poly_float drive,
                             poly_float peak1, poly_float peak3, poly_float peak5) {
        // Remove some lows, remove some highs
        poly_float filter_state_lows = remove_lows_stage_.tickBasic(allpass_output_,
                                                                    utils::min(coefficient * kClearRatio, 0.9f));
        poly_float filter_state_highs = remove_highs_stage_.tickBasic(filter_state_lows,
                                                                      coefficient * (1.0f / kClearRatio));

        // Saturate resonance feedback
        poly_float filter_state = saturateResonance(resonance * (filter_state_lows - filter_state_highs));

        // Combine input with negative feedback
        poly_float filter_input = utils::mulAdd(drive * audio_in, invert_mult_, filter_state);
        poly_float all_pass_input = saturateInput(filter_input);

        // Pass through kPeakStage (4) all-pass stages for peak 1
        poly_float stage_out;
        for (int i = 0; i < kPeakStage; ++i) {
          stage_out = stages_[i].tickBasic(all_pass_input, coefficient);
          all_pass_input = utils::mulAdd(all_pass_input, stage_out, -2.0f);
        }
        poly_float peak1_out = all_pass_input;

        // Next kPeakStage all-pass stages for peak 3
        for (int i = kPeakStage; i < 2 * kPeakStage; ++i) {
          stage_out = stages_[i].tickBasic(all_pass_input, coefficient);
          all_pass_input = utils::mulAdd(all_pass_input, stage_out, -2.0f);
        }
        poly_float peak3_out = all_pass_input;

        // Final kPeakStage all-pass stages for peak 5
        for (int i = 2 * kPeakStage; i < 3 * kPeakStage; ++i) {
          stage_out = stages_[i].tickBasic(all_pass_input, coefficient);
          all_pass_input = utils::mulAdd(all_pass_input, stage_out, -2.0f);
        }
        poly_float peak5_out = all_pass_input;

        // Combine the outputs of each peak cluster
        poly_float all_pass_output_1_3 = utils::mulAdd(peak1 * peak1_out, peak3, peak3_out);
        allpass_output_ = utils::mulAdd(all_pass_output_1_3, peak5, peak5_out);
      }

      /**
       * @brief True if operating in a "clean" saturation mode; false if using a more distorted mode.
       */
      bool clean_;

      /**
       * @brief Current resonance value.
       */
      poly_float resonance_;

      /**
       * @brief Current drive amount.
       */
      poly_float drive_;

      /**
       * @brief Mix amount for the first set of 4 stages (peak 1).
       */
      poly_float peak1_amount_;

      /**
       * @brief Mix amount for the second set of 4 stages (peak 3).
       */
      poly_float peak3_amount_;

      /**
       * @brief Mix amount for the third set of 4 stages (peak 5).
       */
      poly_float peak5_amount_;

      /**
       * @brief Used to invert the phaser output for certain styles (-1.0f) or leave it unaltered (1.0f).
       */
      poly_float invert_mult_;

      /**
       * @brief The array of OnePoleFilters representing each all-pass stage.
       */
      OnePoleFilter<> stages_[kMaxStages];

      /**
       * @brief Filters used to remove extreme lows (remove_lows_stage_) and extremes highs (remove_highs_stage_).
       */
      OnePoleFilter<> remove_lows_stage_;
      OnePoleFilter<> remove_highs_stage_;

      /**
       * @brief The accumulated result of all all-pass stages.
       */
      poly_float allpass_output_;

      JUCE_LEAK_DETECTOR(PhaserFilter)
  };
} // namespace vital
