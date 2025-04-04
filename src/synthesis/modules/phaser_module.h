#pragma once

#include "synth_constants.h"
#include "synth_module.h"

namespace vital {

    class Phaser;

    /**
     * @brief A module that provides a phaser effect for audio signals.
     *
     * The PhaserModule takes an audio input and applies a phaser effect, producing
     * an output with modulated notches in its frequency response. The module handles
     * modulation parameters such as rate, feedback, mix, center frequency, modulation depth,
     * phase offset, and blend.
     */
    class PhaserModule : public SynthModule {
    public:
        /**
         * @enum OutputIndices
         * @brief Specifies the output indices for this module.
         */
        enum {
            kAudioOutput,  /**< The processed audio output after applying phaser effect. */
            kCutoffOutput, /**< An output representing the phaser's cutoff parameter. */
            kNumOutputs    /**< Total number of outputs. */
        };

        /**
         * @brief Constructs a PhaserModule.
         *
         * @param beats_per_second An Output that provides the current beats-per-second value, used for tempo sync.
         */
        PhaserModule(const Output* beats_per_second);

        /**
         * @brief Destructor.
         */
        virtual ~PhaserModule();

        /**
         * @brief Initializes the phaser module by creating and connecting parameters to the Phaser processor.
         *
         * Called after construction to set up internal controls and signal paths.
         */
        void init() override;

        /**
         * @brief Performs a hard reset of the phaser's internal state.
         *
         * Resets modulation phases and internal buffers, ensuring that it starts cleanly from its initial state.
         */
        void hardReset() override;

        /**
         * @brief Enables or disables the phaser module.
         *
         * When enabled, the module processes audio. When disabled, it may produce silence.
         *
         * @param enable A boolean indicating whether to enable (true) or disable (false) the module.
         */
        void enable(bool enable) override;

        /**
         * @brief Adjusts the phaser to a given time position.
         *
         * This corrects the phaser's internal LFO or modulation to match a given playback time,
         * ensuring time-synced parameters remain consistent.
         *
         * @param seconds The playback time in seconds.
         */
        void correctToTime(double seconds) override;

        /**
         * @brief Sets the sample rate for the module.
         *
         * Adjusts any internal parameters that depend on the sample rate.
         *
         * @param sample_rate The new sample rate in Hz.
         */
        void setSampleRate(int sample_rate) override;

        /**
         * @brief Processes the input audio buffer through the phaser effect.
         *
         * @param audio_in A pointer to the input audio buffer (poly_float) to process.
         * @param num_samples The number of samples in the input and output buffers.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Creates a clone of this PhaserModule.
         *
         * @return A new PhaserModule instance that is a copy of this one.
         */
        Processor* clone() const override { return new PhaserModule(*this); }

    protected:
        const Output* beats_per_second_; /**< Provides a tempo-based reference (beats per second) for time-synced parameters. */
        Phaser* phaser_;                 /**< The underlying Phaser processor that applies the phaser effect. */

        JUCE_LEAK_DETECTOR(PhaserModule)
    };
} // namespace vital
