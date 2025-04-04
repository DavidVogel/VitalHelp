#pragma once

#include "synth_module.h"
#include "synth_constants.h"

namespace vital {

    class StereoMemory;

    /**
     * @brief A module that manages a chain of audio effects whose order can be dynamically changed.
     *
     * The ReorderableEffectChain allows for a set of effects to be rearranged in any order,
     * enabling flexible routing and experimentation with different processing chains. It
     * also handles enabling or disabling individual effects based on parameter controls.
     */
    class ReorderableEffectChain : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Defines the input indices for this chain.
         */
        enum {
            kAudio, /**< Input audio signal to be processed by the effects chain. */
            kOrder, /**< A control for specifying the order of the effects. */
            kNumInputs /**< Total number of inputs. */
        };

        /**
         * @brief Constructs a ReorderableEffectChain.
         *
         * @param beats_per_second An Output providing the current tempo in beats per second.
         * @param keytrack An Output providing a key-tracking signal.
         */
        ReorderableEffectChain(const Output* beats_per_second, const Output* keytrack);

        /**
         * @brief Processes a block of audio samples using the configured effect chain order.
         *
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Performs a hard reset on all effects in the chain.
         *
         * This ensures that each effect returns to its initial state.
         */
        virtual void hardReset() override;

        /**
         * @brief Processes a block of samples from a given input buffer.
         *
         * @param audio_in A pointer to the input audio buffer.
         * @param num_samples The number of samples to process.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Creates a clone of this ReorderableEffectChain.
         *
         * @return A new ReorderableEffectChain instance identical to this one.
         */
        virtual Processor* clone() const override { return new ReorderableEffectChain(*this); }

        /**
         * @brief Corrects the timing of all effects to a given playback time.
         *
         * Ensures that time-synced parameters and buffers are updated to the specified time.
         *
         * @param seconds The playback time in seconds.
         */
        virtual void correctToTime(double seconds) override;

        /**
         * @brief Retrieves a pointer to a specific effect in the chain by its enum type.
         *
         * @param effect The type of effect to retrieve (e.g., constants::kChorus).
         * @return A pointer to the requested SynthModule effect.
         */
        SynthModule* getEffect(constants::Effect effect) { return effects_[effect]; }

        /**
         * @brief Retrieves the memory object used by the equalizer effect.
         *
         * This can be useful for analyzing or monitoring the EQ state.
         *
         * @return A pointer to the StereoMemory of the equalizer, or nullptr if not available.
         */
        const StereoMemory* getEqualizerMemory() { return equalizer_memory_; }

    protected:
        /**
         * @brief Creates an effect module based on a given index.
         *
         * Uses a switch statement to instantiate the correct type of effect module.
         *
         * @param index The index corresponding to an effect type (e.g., constants::kChorus).
         * @return A pointer to the newly created SynthModule for the effect.
         */
        SynthModule* createEffectModule(int index);

        const StereoMemory* equalizer_memory_; /**< A reference to the EQ's stereo memory for analysis and inspection. */
        const Output* beats_per_second_;       /**< Tempo reference output in beats per second. */
        const Output* keytrack_;               /**< Keytrack output for pitch-dependent effects. */
        SynthModule* effects_[constants::kNumEffects]; /**< Array of pointers to all effect modules. */
        Value* effects_on_[constants::kNumEffects];    /**< Array of Values determining if each effect is enabled. */
        int effect_order_[constants::kNumEffects];      /**< The current order of effects in the chain. */
        float last_order_;                              /**< The last known order value to detect changes. */

        JUCE_LEAK_DETECTOR(ReorderableEffectChain)
    };
} // namespace vital
