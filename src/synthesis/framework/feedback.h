#pragma once

#include "processor.h"
#include "utils.h"

namespace vital {

    /**
     * @class Feedback
     * @brief A processor that buffers and replays audio, providing a feedback loop mechanism.
     *
     * The Feedback class stores a block of samples from its input and then makes them available
     * on its output at a later time, effectively creating a delay-based feedback mechanism. It can
     * operate at either audio rate or control rate depending on the constructor parameter.
     *
     * This processor is often used in feedback loops within a larger audio graph, allowing processed
     * audio signals to be fed back into earlier stages for effects such as echoes, resonances, or
     * other time-based feedback phenomena.
     *
     * Inputs:
     * - Single input (audio or control).
     *
     * Output:
     * - Single output that provides buffered samples from a previous block.
     */
    class Feedback : public Processor {
    public:
        /**
         * @brief Constructs a Feedback processor.
         *
         * @param control_rate If true, operates at control rate rather than audio rate.
         */
        Feedback(bool control_rate = false) : Processor(1, 1, control_rate), buffer_index_(0) {
            utils::zeroBuffer(buffer_, kMaxBufferSize);
        }

        /**
         * @brief Virtual destructor.
         */
        virtual ~Feedback() { }

        Processor* clone() const override { return new Feedback(*this); }

        /**
         * @brief Processes a block of samples, storing them into an internal buffer.
         *
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Refreshes the output with previously stored samples.
         *
         * This method can be used to read out samples that were stored in a previous block,
         * effectively creating a feedback loop or simple delay line.
         *
         * @param num_samples The number of samples to refresh.
         */
        virtual void refreshOutput(int num_samples);

        /**
         * @brief Processes a single sample, storing it in the internal buffer.
         *
         * @param i The index of the sample being processed.
         */
        force_inline void tick(int i) {
            buffer_[i] = input(0)->source->buffer[i];
        }

    protected:
        poly_float buffer_[kMaxBufferSize]; ///< Internal buffer to store samples for feedback.
        int buffer_index_;                  ///< Current write index in the buffer.

        JUCE_LEAK_DETECTOR(Feedback)
    };

    namespace cr {
        /**
         * @class Feedback
         * @brief A control-rate variant of the Feedback processor.
         *
         * This version of Feedback operates at control rate (non-audio rate), keeping track of
         * a single last value rather than a full buffer of samples. It can be used for control signals
         * that need to maintain a last known value for feedback-type behaviors.
         */
        class Feedback : public ::vital::Feedback {
        public:
            Feedback() : ::vital::Feedback(true), last_value_(0.0f) { }

            /**
             * @brief Processes the control input for a given number of samples (usually 1).
             *
             * Stores the last input value for future retrieval.
             *
             * @param num_samples Number of samples to process (typically 1 for control signals).
             */
            void process(int num_samples) override {
                last_value_ = input()->at(0);
            }

            Processor* clone() const override { return new cr::Feedback(*this); }

            /**
             * @brief Updates the output with the last stored value.
             *
             * @param num_samples Number of samples to refresh (usually 1).
             */
            void refreshOutput(int num_samples) override {
                output()->buffer[0] = last_value_;
            }

            /**
             * @brief Resets the internal state of the feedback processor.
             *
             * @param reset_mask Mask specifying which voices to reset.
             */
            void reset(poly_mask reset_mask) override {
                last_value_ = 0.0f;
                output()->buffer[0] = last_value_;
            }

        protected:
            poly_float last_value_; ///< The last value stored, used as feedback for control signals.
        };
    } // namespace cr
} // namespace vital
