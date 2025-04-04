/**
 * @file portamento_slope.h
 * @brief Declares the PortamentoSlope class, which applies a portamento transition between a source and target pitch or value.
 */

#pragma once

#include "processor.h"

namespace vital {

    /**
     * @class PortamentoSlope
     * @brief A processor that smoothly transitions (portamento) from a source value to a target value over a specified time.
     *
     * This class implements a portamento slope, allowing you to smoothly glide between a source pitch/value
     * and a target pitch/value over a given time duration. It supports scaling the transition time based on
     * pitch intervals and applying a slope power curve to the transition.
     *
     * Inputs:
     * - kTarget: The target value to reach (e.g., a new note's pitch).
     * - kSource: The source value from which to start the transition.
     * - kPortamentoForce: Forces portamento if non-zero, even if there's only one note.
     * - kPortamentoScale: If non-zero, scales the portamento time based on interval distance.
     * - kRunSeconds: The duration over which the portamento should run, in seconds.
     * - kSlopePower: A power value that adjusts the shape of the glide curve.
     * - kReset: A trigger that resets the portamento position to the start.
     * - kNumNotesPressed: The number of notes currently pressed (for conditional portamento).
     *
     * Outputs:
     * - The smoothly transitioning output value that moves from source to target over time.
     */
    class PortamentoSlope : public Processor {
    public:
        /// Minimum portamento time in seconds.
        static constexpr float kMinPortamentoTime = 0.001f;

        /**
         * @enum InputIndices
         * @brief The input indices for the PortamentoSlope processor.
         */
        enum {
            kTarget,           ///< Target value input index.
            kSource,           ///< Source value input index.
            kPortamentoForce,  ///< Force portamento on/off input index.
            kPortamentoScale,  ///< Scale portamento by interval input index.
            kRunSeconds,       ///< Duration of portamento in seconds input index.
            kSlopePower,       ///< Power/curve of the slope input index.
            kReset,            ///< Reset trigger input index.
            kNumNotesPressed,  ///< Number of notes currently pressed input index.
            kNumInputs         ///< Total number of inputs.
        };

        /**
         * @brief Constructs a new PortamentoSlope processor.
         */
        PortamentoSlope();

        /**
         * @brief Destructor.
         */
        virtual ~PortamentoSlope() { }

        /**
         * @brief Clones this PortamentoSlope processor.
         * @return A pointer to a newly allocated PortamentoSlope object that is a copy of this one.
         */
        virtual Processor* clone() const override {
            return new PortamentoSlope(*this);
        }

        /**
         * @brief Processes a block when the portamento is effectively bypassed.
         *
         * @param start The starting sample index for the bypass process. Typically set to 0.
         */
        void processBypass(int start);

        /**
         * @brief Processes the portamento over the given number of samples.
         *
         * @param num_samples The number of audio samples to process.
         */
        virtual void process(int num_samples) override;

    private:
        /// The current position in the portamento [0.0, 1.0].
        poly_float position_;

        JUCE_LEAK_DETECTOR(PortamentoSlope)
    };
} // namespace vital
