#pragma once

#include "synth_constants.h"
#include "synth_module.h"

namespace vital {

    class Distortion;
    class DigitalSvf;

    /**
     * @brief A module that applies distortion and optional filtering to an audio signal.
     *
     * The DistortionModule provides various distortion types, adjustable drive, and a post-distortion
     * filter with configurable order, cutoff, resonance, and blend. The module also supports mixing
     * between the dry and distorted signals.
     */
    class DistortionModule : public SynthModule {
    public:
        /**
         * @brief Constructs a new DistortionModule.
         */
        DistortionModule();

        /**
         * @brief Destroys the DistortionModule, releasing its internal resources.
         */
        virtual ~DistortionModule();

        /**
         * @brief Initializes the DistortionModule, setting up internal controls and connecting processors.
         *
         * This involves creating the Distortion and DigitalSvf (filter) processors,
         * registering controls for drive, mix, filter cutoff, resonance, etc., and linking them.
         */
        virtual void init() override;

        /**
         * @brief Sets the sample rate for the module, ensuring time-dependent parameters scale properly.
         * @param sample_rate The new sample rate in Hz.
         */
        virtual void setSampleRate(int sample_rate) override;

        /**
         * @brief Processes a block of samples with given input audio, applying distortion and filtering.
         *
         * Depending on the filter order parameter, the signal may be distorted first and then filtered,
         * filtered then distorted, or just distorted. The wet/dry mix is also applied to blend the
         * processed output with the original input.
         *
         * @param audio_in Pointer to the input audio samples.
         * @param num_samples The number of samples to process.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Clones this DistortionModule instance, creating a new identical module.
         * @return A pointer to the newly created DistortionModule clone.
         */
        virtual Processor* clone() const override { return new DistortionModule(*this); }

    protected:
        Distortion* distortion_;     ///< The internal Distortion processor.
        Value* filter_order_;        ///< Determines the order of filtering relative to distortion.
        DigitalSvf* filter_;         ///< The internal digital state-variable filter processor.
        Output* distortion_mix_;     ///< Control for blending between dry and distorted signal.
        poly_float mix_;             ///< The current effective mix value for wet/dry blending.

        JUCE_LEAK_DETECTOR(DistortionModule)
    };
} // namespace vital
