#pragma once

#include "synth_module.h"

namespace vital {

    class DigitalSvf;

    /**
     * @brief A multiband equalizer module that provides low, mid, and high frequency shaping.
     *
     * The EqualizerModule uses multiple DigitalSvf filter instances to implement flexible EQ curves.
     * It allows switching between different filter modes (e.g., low shelf vs. high-pass, band shelf vs. notch, etc.),
     * and can store recent audio samples in a memory buffer for analysis or other uses.
     */
    class EqualizerModule : public SynthModule {
    public:
        /**
         * @brief Constructs an EqualizerModule.
         */
        EqualizerModule();
        virtual ~EqualizerModule() { }

        /**
         * @brief Initializes the EqualizerModule by creating parameter controls and linking them to internal filters.
         */
        void init() override;

        /**
         * @brief Resets the equalizer filters to their initial states, clearing any buffer or filter history.
         */
        void hardReset() override;

        /**
         * @brief Enables or disables the equalizer module.
         *
         * When enabled, it resets all filters to ensure a clean state. When disabled, the module
         * stops processing audio but retains its current parameter settings.
         *
         * @param enable True to enable, false to disable.
         */
        void enable(bool enable) override;

        /**
         * @brief Sets the sample rate of the equalizer and propagates it to all internal filters.
         *
         * @param sample_rate The new sample rate in Hz.
         */
        void setSampleRate(int sample_rate) override;

        /**
         * @brief Processes an input audio buffer through the equalizer.
         *
         * The module chooses between different filter topologies (low shelf vs. high-pass, etc.)
         * based on the mode parameters and processes the audio in a chain: low section, then band section,
         * then high section. The result is stored in the output buffer and also pushed into internal memory.
         *
         * @param audio_in Pointer to the input audio samples.
         * @param num_samples The number of samples to process.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Clones this EqualizerModule, creating a new instance with identical configuration.
         *
         * @return A pointer to the newly cloned EqualizerModule.
         */
        Processor* clone() const override { return new EqualizerModule(*this); }

        /**
         * @brief Retrieves the internal stereo memory used for storing audio samples.
         *
         * @return A pointer to the StereoMemory object containing recent audio samples.
         */
        const StereoMemory* getAudioMemory() { return audio_memory_.get(); }

    protected:
        Value* low_mode_;   ///< Determines if the low band uses a shelf or a high-pass filter.
        Value* band_mode_;  ///< Determines if the mid band uses a shelf or a notch filter.
        Value* high_mode_;  ///< Determines if the high band uses a shelf or a low-pass filter.

        DigitalSvf* high_pass_;   ///< High-pass filter for low band mode.
        DigitalSvf* low_shelf_;   ///< Low shelf filter for low band mode.
        DigitalSvf* notch_;       ///< Notch filter for mid band mode.
        DigitalSvf* band_shelf_;  ///< Band shelf filter for mid band mode.
        DigitalSvf* low_pass_;    ///< Low-pass filter for high band mode.
        DigitalSvf* high_shelf_;  ///< High shelf filter for high band mode.

        std::shared_ptr<StereoMemory> audio_memory_; ///< Memory buffer for storing processed audio samples.

        JUCE_LEAK_DETECTOR(EqualizerModule)
    };
} // namespace vital
