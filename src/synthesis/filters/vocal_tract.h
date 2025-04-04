#pragma once

#include "processor_router.h"
#include "circular_queue.h"
#include "synth_constants.h"

namespace vital {

    /**
     * @class VocalTract
     * @brief A model of a vocal tract for generating vowel-like formants and vocal articulations.
     *
     * The VocalTract processor simulates the resonances and dynamic shaping of a human vocal tract.
     * By adjusting parameters such as tongue position and height, as well as blend factors, the
     * processor can produce sounds reminiscent of various vowels and vocalizations. This is
     * experimental and may serve as a building block for synthetic vocal timbres.
     *
     * Inputs:
     * - kAudio: Input audio signal to be shaped.
     * - kReset: Resets internal states if triggered.
     * - kBlend: Controls the mix or interpolation of different articulations.
     * - kTonguePosition: Adjusts the position of the tongue within the vocal tract model, affecting formant frequencies.
     * - kTongueHeight: Adjusts the height of the tongue, influencing the resonant characteristics and vowel formation.
     *
     * Outputs:
     * - A single output audio signal transformed by the vocal tract model.
     *
     * This class does not currently have a full implementation within Vital (the .cpp is empty),
     * indicating it's either incomplete or serves as a placeholder for future development.
     */
    class VocalTract : public ProcessorRouter {
    public:
        /// Input indices for the VocalTract parameters.
        enum {
            kAudio,          ///< The input audio signal.
            kReset,          ///< Reset signal to clear internal states.
            kBlend,          ///< Blend control for articulations.
            kTonguePosition, ///< Controls the virtual tongue's forward/backward position.
            kTongueHeight,   ///< Controls the vertical (height) position of the virtual tongue.
            kNumInputs
        };

        /**
         * @brief Constructs a VocalTract processor.
         */
        VocalTract();

        /**
         * @brief Destructor.
         */
        virtual ~VocalTract();

        Processor* clone() const override { return new VocalTract(*this); }

        /**
         * @brief Resets internal states for specific voices indicated by the reset mask.
         *
         * @param reset_mask A mask specifying which voices should be reset.
         */
        void reset(poly_mask reset_mask) override;

        /**
         * @brief Performs a hard reset of all internal states, returning to default values.
         */
        void hardReset() override;

        /**
         * @brief Processes audio with the given number of samples using the already connected input.
         *
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Processes a given input buffer through the vocal tract model.
         *
         * Currently unimplemented, serving as a placeholder for future vocal tract modeling.
         *
         * @param audio_in Pointer to the input audio buffer.
         * @param num_samples The number of samples to process.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

    private:
        JUCE_LEAK_DETECTOR(VocalTract)
    };
} // namespace vital
