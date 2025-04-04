#pragma once

#include "synth_module.h"
#include "synth_constants.h"
#include "line_generator.h"

namespace vital {

    /**
     * @brief A processor that applies a modulation signal to a parameter, performing mapping, scaling, and morphing.
     *
     * The ModulationConnectionProcessor takes a modulation input (e.g., from an LFO or envelope),
     * applies transformations such as bipolar shifting, remapping via a LineGenerator, applying power curves,
     * and scaling by a destination amount. It can handle both control-rate and audio-rate modulation,
     * and supports polyphonic and stereo modulation. The output is a transformed modulation signal
     * ready to be applied to a parameter in the synth.
     */
    class ModulationConnectionProcessor : public SynthModule {
    public:
        /**
         * @brief Input parameter indices.
         *
         * - kModulationInput:  The raw modulation source signal.
         * - kModulationAmount: The scaling amount applied to the modulation signal.
         * - kModulationPower:  The morphing power used for nonlinear shaping of the modulation.
         * - kReset:            A reset trigger to reinitialize states for new voice events.
         */
        enum {
            kModulationInput,
            kModulationAmount,
            kModulationPower,
            kReset,
            kNumInputs
        };

        /**
         * @brief Output parameter indices.
         *
         * - kModulationOutput:  The final, transformed modulation signal after all processing.
         * - kModulationPreScale: The modulation signal before final scaling (for debug or further usage).
         * - kModulationSource:  The source modulation trigger value (for analysis or debug).
         */
        enum {
            kModulationOutput,
            kModulationPreScale,
            kModulationSource,
            kNumOutputs
        };

        /**
         * @brief Constructs a ModulationConnectionProcessor with a given index.
         *
         * @param index An identifier for this modulation connection, for parameter naming and tracking.
         */
        ModulationConnectionProcessor(int index);
        virtual ~ModulationConnectionProcessor() { }

        /**
         * @brief Initializes the processor, creating and connecting parameter controls (e.g., bipolar, stereo, bypass).
         */
        void init() override;

        /**
         * @brief Processes a block of samples, handling both control-rate and audio-rate modulation.
         *
         * If the modulation or input source is control-rate, it processes once per block. Otherwise, it processes every sample.
         *
         * @param num_samples Number of samples in the block.
         */
        void process(int num_samples) override;

        /**
         * @brief Processes the modulation at audio-rate using the provided source output.
         *
         * This method determines if power morphing, remapping, or linear processing is needed and calls the appropriate function.
         *
         * @param num_samples Number of samples to process.
         * @param source      The modulation source output to read from.
         */
        void processAudioRate(int num_samples, const Output* source);

        /**
         * @brief Audio-rate processing with a linear transform (no remap, no morph).
         */
        void processAudioRateLinear(int num_samples, const Output* source);

        /**
         * @brief Audio-rate processing with remapping via LineGenerator, but no morphing power.
         */
        void processAudioRateRemapped(int num_samples, const Output* source);

        /**
         * @brief Audio-rate processing with morphing power applied (no remapping).
         */
        void processAudioRateMorphed(int num_samples, const Output* source, poly_float power);

        /**
         * @brief Audio-rate processing with both remapping and morphing power applied.
         */
        void processAudioRateRemappedAndMorphed(int num_samples, const Output* source, poly_float power);

        /**
         * @brief Processes the modulation at control-rate (once per block), when modulation or source is not audio-rate.
         *
         * @param source The modulation source output at control-rate.
         */
        void processControlRate(const Output* source);

        /**
         * @brief Creates a clone of this processor.
         *
         * @return A new ModulationConnectionProcessor instance with identical settings.
         */
        virtual Processor* clone() const override { return new ModulationConnectionProcessor(*this); }

        /**
         * @brief Initializes a base value for this modulation connection, used as a starting point.
         *
         * @param base_value A pointer to a Value that holds the base or default modulation value.
         */
        void initializeBaseValue(Value* base_value) { current_value_ = base_value; }

        /**
         * @brief Sets the mapping function to a linear mapping in the internal LineGenerator.
         */
        void initializeMapping() { map_generator_->initLinear(); }

        /**
         * @brief Retrieves the current base value of the modulation connection.
         *
         * @return The current base value as a mono_float.
         */
        mono_float currentBaseValue() const { return current_value_->value(); }

        /**
         * @brief Sets the base value of the modulation connection.
         *
         * @param value The new base value.
         */
        void setBaseValue(mono_float value) { current_value_->set(value); }

        /**
         * @brief Checks if the modulation is polyphonic (per-voice).
         *
         * @return True if polyphonic, false if monophonic.
         */
        bool isPolyphonicModulation() const { return polyphonic_; }

        /**
         * @brief Sets whether the modulation should be treated as polyphonic.
         *
         * @param polyphonic True for polyphonic modulation, false for monophonic.
         */
        void setPolyphonicModulation(bool polyphonic) { polyphonic_ = polyphonic; }

        /**
         * @brief Checks if the modulation is bipolar (range -1 to 1).
         *
         * @return True if bipolar, false if unipolar (0 to 1).
         */
        bool isBipolar() const { return bipolar_->value() != 0.0f; }

        /**
         * @brief Sets whether the modulation should be bipolar.
         *
         * @param bipolar True if bipolar, false otherwise.
         */
        void setBipolar(bool bipolar) { bipolar_->set(bipolar ? 1.0f : 0.0f); }

        /**
         * @brief Checks if the modulation is stereo, applying a different scale factor to the right channel.
         *
         * @return True if stereo, false if mono.
         */
        bool isStereo() const { return stereo_->value() != 0.0f; }

        /**
         * @brief Sets whether the modulation should be stereo.
         *
         * @param stereo True if stereo, false otherwise.
         */
        void setStereo(bool stereo) { stereo_->set(stereo ? 1.0f : 0.0f); }

        /**
         * @brief Checks if the modulation connection is bypassed.
         *
         * @return True if bypassed, false if active.
         */
        bool isBypassed() const { return bypass_->value() != 0.0f; }

        /**
         * @brief Sets the scaling factor for the destination parameter.
         *
         * @param scale A mono_float scaling factor applied to the final modulation.
         */
        force_inline void setDestinationScale(mono_float scale) { *destination_scale_ = scale; }

        /**
         * @brief Gets the index identifier of this modulation connection.
         *
         * @return The index of the modulation connection.
         */
        force_inline int index() const { return index_; }

        /**
         * @brief Retrieves the LineGenerator (map_generator_) used for remapping the modulation.
         *
         * @return A pointer to the LineGenerator instance.
         */
        LineGenerator* lineMapGenerator() { return map_generator_.get(); }

    protected:
        int index_;                        ///< Unique identifier for this modulation connection.
        bool polyphonic_;                  ///< True if modulation is polyphonic.
        Value* current_value_;             ///< The base or initial value for the modulation.
        Value* bipolar_;                   ///< Controls if the modulation is bipolar.
        Value* stereo_;                    ///< Controls if the modulation is stereo.
        Value* bypass_;                    ///< Controls if the modulation connection is bypassed.

        poly_float power_;                 ///< Stored current power for morphing modulation.
        poly_float modulation_amount_;     ///< The current modulation amount scaled by the destination scale.

        std::shared_ptr<mono_float> destination_scale_; ///< The scale factor for final modulation output.
        mono_float last_destination_scale_;             ///< The last known destination scale to detect changes.

        std::shared_ptr<LineGenerator> map_generator_;  ///< The line mapping function for remapping modulation.

        JUCE_LEAK_DETECTOR(ModulationConnectionProcessor)
    };
} // namespace vital
