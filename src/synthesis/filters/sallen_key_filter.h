#pragma once

#include "one_pole_filter.h"
#include "processor.h"
#include "synth_filter.h"

namespace vital {

    /**
     * @class SallenKeyFilter
     * @brief A Sallen-Key style filter capable of multiple modes (12dB, 24dB, dual modes) with nonlinear drive and resonance.
     *
     * The SallenKeyFilter implements a classic Sallen-Key topology using one-pole filters combined
     * in various configurations to produce low-pass, high-pass, and band-pass responses, as well as dual
     * and notch-band configurations. It applies nonlinear saturations and allows tuning of drive, resonance,
     * and relative blends of filter outputs to create a wide variety of timbres.
     *
     * Inputs:
     * - kAudio: The input audio signal.
     *
     * Outputs:
     * - A single filtered audio output.
     *
     * Users can set the cutoff frequency (via MIDI note or frequency), resonance, and drive. The filter
     * dynamically adjusts its internal states and blends low, band, and high outputs based on the mode
     * selected. This results in flexible filtering behavior suitable for various synthesis tasks.
     */
    class SallenKeyFilter : public Processor, public SynthFilter {
    public:
        /// Minimum and maximum resonance factors.
        static constexpr mono_float kMinResonance = 0.0f;
        static constexpr mono_float kMaxResonance = 2.15f;
        static constexpr mono_float kDriveResonanceBoost = 1.1f;
        static constexpr mono_float kMaxVisibleResonance = 2.0f;
        static constexpr mono_float kMinCutoff = 1.0f;

        /**
         * @brief Tunes the resonance based on the coefficient to maintain stability and smooth response.
         *
         * @param resonance The raw resonance value.
         * @param coefficient The filter coefficient derived from the cutoff frequency.
         * @return Adjusted resonance value.
         */
        static poly_float tuneResonance(poly_float resonance, poly_float coefficient) {
            return resonance / utils::max(1.0f, coefficient * 0.09f + 0.97f);
        }

        /**
         * @brief Constructs a SallenKeyFilter with default parameters.
         */
        SallenKeyFilter();
        virtual ~SallenKeyFilter() { }

        Processor* clone() const override { return new SallenKeyFilter(*this); }

        /**
         * @brief Processes a block of samples using the already set input and outputs the filtered result.
         *
         * @param num_samples Number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Processes a given audio input buffer through the Sallen-Key filter.
         *
         * @param audio_in Pointer to the input audio buffer.
         * @param num_samples The number of samples to process.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Sets up the filter parameters (cutoff, resonance, drive) and mode from a given FilterState.
         *
         * @param filter_state The desired filter parameters and style.
         */
        void setupFilter(const FilterState& filter_state) override;

        /**
         * @brief Processes the filter in 12 dB mode.
         *
         * @param audio_in Input audio buffer.
         * @param num_samples Number of samples.
         * @param current_resonance Current resonance value.
         * @param current_drive Current drive factor.
         * @param current_post_multiply Current post-multiplier for output gain.
         * @param current_low Low-pass blend amount.
         * @param current_band Band-pass blend amount.
         * @param current_high High-pass blend amount.
         */
        void process12(const poly_float* audio_in, int num_samples,
                       poly_float current_resonance,
                       poly_float current_drive, poly_float current_post_multiply,
                       poly_float current_low, poly_float current_band, poly_float current_high);

        /**
         * @brief Processes the filter in 24 dB mode.
         */
        void process24(const poly_float* audio_in, int num_samples,
                       poly_float current_resonance,
                       poly_float current_drive, poly_float current_post_multiply,
                       poly_float current_low, poly_float current_band, poly_float current_high);

        /**
         * @brief Processes the filter in dual mode (e.g., dual notch band).
         */
        void processDual(const poly_float* audio_in, int num_samples,
                         poly_float current_resonance,
                         poly_float current_drive, poly_float current_post_multiply,
                         poly_float current_low, poly_float current_high);

        /**
         * @brief A single sample tick for the filter in its 12 dB configuration or as a building block.
         *
         * @param audio_in The input sample.
         * @param coefficient Filter coefficient.
         * @param resonance Adjusted resonance factor.
         * @param stage1_feedback_mult Feedback multiplier for stage 1.
         * @param drive Input drive factor.
         * @param normalizer Normalization factor to maintain level.
         */
        force_inline void tick(poly_float audio_in, poly_float coefficient, poly_float resonance,
                               poly_float stage1_feedback_mult, poly_float drive, poly_float normalizer);

        /**
         * @brief A single sample tick for the filter in its 24 dB mode with nonlinear calculations.
         */
        force_inline void tick24(poly_float audio_in, poly_float coefficient, poly_float resonance,
                                 poly_float stage1_feedback_mult, poly_float drive,
                                 poly_float pre_normalizer, poly_float normalizer,
                                 poly_float low, poly_float band, poly_float high);

        /**
         * @brief Resets internal states for specific voices.
         *
         * @param reset_mask A mask specifying which voices to reset.
         */
        void reset(poly_mask reset_mask) override;

        /**
         * @brief Performs a hard reset of all internal states.
         */
        void hardReset() override;

        /**
         * @brief Gets the current resonance value.
         */
        poly_float getResonance() { return resonance_; }

        /**
         * @brief Gets the current drive value.
         */
        poly_float getDrive() { return drive_; }

        /**
         * @brief Gets the low-pass blend amount.
         */
        poly_float getLowAmount() { return low_pass_amount_; }

        /**
         * @brief Gets the band-pass blend amount.
         */
        poly_float getBandAmount() { return band_pass_amount_; }

        /**
         * @brief Gets the high-pass blend amount.
         */
        poly_float getHighAmount() { return high_pass_amount_; }

        /**
         * @brief For 24 dB modes, gets the low amount depending on the style.
         */
        poly_float getLowAmount24(int style) {
            if (style == kDualNotchBand)
                return high_pass_amount_;
            return low_pass_amount_;
        }

        /**
         * @brief For 24 dB modes, gets the high amount depending on the style.
         */
        poly_float getHighAmount24(int style) {
            if (style == kDualNotchBand)
                return low_pass_amount_;
            return high_pass_amount_;
        }

    private:
        poly_float cutoff_;           ///< Current cutoff frequency.
        poly_float resonance_;        ///< Current resonance factor.
        poly_float drive_;            ///< Drive (input gain) factor.
        poly_float post_multiply_;    ///< Post-multiplier for output gain.
        poly_float low_pass_amount_;  ///< Low-pass blend factor.
        poly_float band_pass_amount_; ///< Band-pass blend factor.
        poly_float high_pass_amount_; ///< High-pass blend factor.

        poly_float stage1_input_;     ///< Input to stage 1 filter in the chain.

        OnePoleFilter<> pre_stage1_;  ///< Pre-filter stage 1.
        OnePoleFilter<> pre_stage2_;  ///< Pre-filter stage 2.
        OnePoleFilter<> stage1_;      ///< First main stage filter.
        OnePoleFilter<> stage2_;      ///< Second main stage filter.

        JUCE_LEAK_DETECTOR(SallenKeyFilter)
    };
} // namespace vital
