#pragma once

#include "processor.h"
#include "synth_filter.h"

#include "futils.h"
#include "one_pole_filter.h"

namespace vital {

    /**
     * @class DirtyFilter
     * @brief A nonlinear filter that produces a "dirty" and saturated sound, ideal for adding character to the signal.
     *
     * The DirtyFilter implements a filter with a nonlinear, saturating signal path. It supports multiple filter styles
     * including 12 dB/oct and 24 dB/oct slopes, as well as a dual mode for more complex responses (e.g., dual notch bands).
     * Resonance, drive, and blend between low, band, and high responses can all be controlled. Internal saturations and
     * nonlinearities produce a gritty, warm sound that can be more aggressive than traditional state-variable filters.
     *
     * Inputs:
     * - kAudio: Input audio signal.
     *
     * Output:
     * - The processed, dirtied, and filtered audio signal.
     *
     * The DirtyFilter uses a series of one-pole filters combined with nonlinear saturations. It dynamically adjusts parameters
     * (coefficient, resonance, drive, and blend amounts) based on the provided FilterState, and can also respond to MIDI note-based
     * cutoff frequencies. Through careful setting of drive and resonance, it can produce strong character and even distortion.
     */
    class DirtyFilter : public Processor, public SynthFilter {
    public:
        /// Minimum resonance factor.
        static constexpr mono_float kMinResonance = 0.1f;
        /// Maximum resonance factor.
        static constexpr mono_float kMaxResonance = 2.15f;
        /// Scaling factor applied during saturation.
        static constexpr mono_float kSaturationBoost = 1.4f;
        /// Maximum visible resonance value.
        static constexpr mono_float kMaxVisibleResonance = 2.0f;
        /// Additional resonance boost when drive is applied.
        static constexpr mono_float kDriveResonanceBoost = 0.05f;

        /// Minimum cutoff frequency in Hz.
        static constexpr mono_float kMinCutoff = 1.0f;
        /// Minimum drive value.
        static constexpr mono_float kMinDrive = 0.1f;

        /// Flat resonance factor used internally.
        static constexpr mono_float kFlatResonance = 1.0f;

        /**
         * @brief Tunes the resonance based on the filter coefficient.
         *
         * @param resonance The raw resonance input.
         * @param coefficient The filter coefficient (depends on cutoff).
         * @return The tuned resonance value suitable for processing.
         */
        force_inline poly_float tuneResonance(poly_float resonance, poly_float coefficient) {
            return resonance / utils::max(1.0f, coefficient * 0.25f + 0.97f);
        }

        /**
         * @brief Constructs a DirtyFilter with default parameters.
         */
        DirtyFilter();
        virtual ~DirtyFilter() { }

        Processor* clone() const override { return new DirtyFilter(*this); }

        /**
         * @brief Processes a block of audio samples.
         *
         * Automatically retrieves the input from connected sources and writes the processed audio to the output.
         *
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Sets up the filter's internal state (cutoff, resonance, drive, blend) from the given FilterState.
         *
         * @param filter_state The filter parameters and style to use.
         */
        void setupFilter(const FilterState& filter_state) override;

        /**
         * @brief Processes the filter in 12 dB/oct mode.
         *
         * @param num_samples Number of samples to process.
         * @param current_resonance Current resonance value.
         * @param current_drive Current drive value.
         * @param current_drive_boost Additional drive boost.
         * @param current_drive_blend Drive blending factor.
         * @param current_low Low-pass blend amount.
         * @param current_band Band-pass blend amount.
         * @param current_high High-pass blend amount.
         */
        void process12(int num_samples, poly_float current_resonance,
                       poly_float current_drive, poly_float current_drive_boost, poly_float current_drive_blend,
                       poly_float current_low, poly_float current_band, poly_float current_high);

        /**
         * @brief Processes the filter in 24 dB/oct mode.
         */
        void process24(int num_samples, poly_float current_resonance,
                       poly_float current_drive, poly_float current_drive_boost, poly_float current_drive_blend,
                       poly_float current_low, poly_float current_band, poly_float current_high);

        /**
         * @brief Processes the filter in dual (e.g., dual-notch band) mode.
         */
        void processDual(int num_samples, poly_float current_resonance,
                         poly_float current_drive, poly_float current_drive_boost,
                         poly_float current_drive_blend, poly_float current_drive_mult,
                         poly_float current_low, poly_float current_high);

        /**
         * @brief Processes a single sample in 24 dB mode with nonlinearities and drive.
         */
        force_inline poly_float tick24(poly_float audio_in,
                                       poly_float coefficient, poly_float resonance,
                                       poly_float drive, poly_float feed_mult, poly_float normalizer,
                                       poly_float pre_feedback_mult, poly_float pre_normalizer,
                                       poly_float low, poly_float band, poly_float high);

        /**
         * @brief Processes a single sample in dual mode.
         */
        force_inline poly_float tickDual(poly_float audio_in,
                                         poly_float coefficient, poly_float resonance,
                                         poly_float drive, poly_float feed_mult, poly_float normalizer,
                                         poly_float pre_feedback_mult, poly_float pre_normalizer,
                                         poly_float low, poly_float high);

        /**
         * @brief Processes a single sample in 12 dB mode or as a part of other modes' chains.
         */
        force_inline poly_float tick(poly_float audio_in,
                                     poly_float coefficient, poly_float resonance,
                                     poly_float drive, poly_float feed_mult, poly_float normalizer,
                                     poly_float low, poly_float band, poly_float high);

        /**
         * @brief Resets the filter state for specific voices.
         *
         * @param reset_mask The mask of voices to reset.
         */
        void reset(poly_mask reset_mask) override;

        /**
         * @brief Hard resets the filter, clearing all internal states.
         */
        void hardReset() override;

        /**
         * @brief Gets the current resonance value adjusted by coefficient and drive.
         */
        force_inline poly_float getResonance() {
            poly_float resonance_in = utils::clamp(tuneResonance(resonance_, coefficient_ * 2.0f), 0.0f, 1.0f);
            return utils::interpolate(kMinResonance, kMaxResonance, resonance_in) + drive_boost_;
        }

        /**
         * @brief Gets the current drive value adjusted based on resonance.
         */
        force_inline poly_float getDrive() {
            poly_float resonance = getResonance();
            poly_float scaled_drive = utils::max(poly_float(kMinDrive), drive_) / (resonance * resonance * 0.5f + 1.0f);
            return utils::interpolate(drive_, scaled_drive, drive_blend_);
        }

        /**
         * @brief Gets the low-pass blend amount.
         */
        force_inline poly_float getLowAmount() { return low_pass_amount_; }

        /**
         * @brief Gets the band-pass blend amount.
         */
        force_inline poly_float getBandAmount() { return band_pass_amount_; }

        /**
         * @brief Gets the high-pass blend amount.
         */
        force_inline poly_float getHighAmount() { return high_pass_amount_; }

        /**
         * @brief For 24 dB mode, gets the low amount depending on filter style.
         */
        force_inline poly_float getLowAmount24(int style) {
            if (style == kDualNotchBand)
                return high_pass_amount_;
            return low_pass_amount_;
        }

        /**
         * @brief For 24 dB mode, gets the high amount depending on filter style.
         */
        force_inline poly_float getHighAmount24(int style) {
            if (style == kDualNotchBand)
                return low_pass_amount_;
            return high_pass_amount_;
        }

    private:
        poly_float coefficient_, resonance_;
        poly_float drive_, drive_boost_, drive_blend_, drive_mult_;

        poly_float low_pass_amount_;
        poly_float band_pass_amount_;
        poly_float high_pass_amount_;

        OnePoleFilter<> pre_stage1_;
        OnePoleFilter<> pre_stage2_;
        OnePoleFilter<> stage1_;
        OnePoleFilter<> stage2_;
        OnePoleFilter<futils::quickTanh> stage3_;
        OnePoleFilter<futils::quickTanh> stage4_;

        JUCE_LEAK_DETECTOR(DirtyFilter)
    };
} // namespace vital
