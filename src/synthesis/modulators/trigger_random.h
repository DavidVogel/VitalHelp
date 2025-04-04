#pragma once

#include "processor.h"
#include "utils.h"

namespace vital {

    /**
     * @brief A processor that outputs a random value on a trigger event.
     *
     * The TriggerRandom class generates a new random value whenever it receives a trigger signal on its input.
     * It holds the value constant until the next trigger occurs. This can be used for modulations where a random value
     * is needed at note-on or other trigger events.
     */
    class TriggerRandom : public Processor {
    public:
        /**
         * @brief Input parameter indices for TriggerRandom.
         *
         * - kReset: When triggered, a new random value is generated and output.
         */
        enum {
            kReset,
            kNumInputs
        };

        /**
         * @brief Constructs a TriggerRandom processor, initializing the output value and random generator.
         */
        TriggerRandom();

        /**
         * @brief Destroys the TriggerRandom processor.
         */
        virtual ~TriggerRandom() { }

        /**
         * @brief Clones the TriggerRandom processor, creating a new instance with identical configuration.
         *
         * @return A pointer to the newly cloned TriggerRandom instance.
         */
        virtual Processor* clone() const override { return new TriggerRandom(*this); }

        /**
         * @brief Processes a block of samples, updating the random output if a trigger event occurs.
         *
         * @param num_samples The number of samples in the current block.
         */
        virtual void process(int num_samples) override;

    private:
        poly_float value_;                ///< The current random value output.
        utils::RandomGenerator random_generator_; ///< The random number generator used to produce new values.

        JUCE_LEAK_DETECTOR(TriggerRandom)
    };
} // namespace vital
