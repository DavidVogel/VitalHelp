/**
 * @file processor_test.cpp
 * @brief Implements the ProcessorTest class, providing utility methods to run input bounds tests on Processors.
 */

#include "processor_test.h"
#include "processor.h"
#include "value.h"

#include <cmath>

#define PROCESS_AMOUNT 600
#define RANDOMIZE_AMOUNT 50

void ProcessorTest::processAndCheckFinite(vital::Processor* processor, const std::set<int>& ignore_outputs) {
    // Reassert the sample rate to ensure consistency in internal processing.
    processor->setSampleRate(processor->getSampleRate());

    int num_outputs = processor->numOutputs();

    // Process multiple times to stress-test the processor under given conditions.
    for (int i = 0; i < PROCESS_AMOUNT; ++i)
        processor->process(vital::kMaxBufferSize);

    // Check that all non-ignored outputs are finite after processing.
    for (int i = 0; i < num_outputs; ++i) {
        if (ignore_outputs.count(i) == 0) {
            vital::Output* output = processor->output(i);
            expect(vital::utils::isContained(output->buffer, output->buffer_size),
                   "Output buffer contains non-finite values.");
        }
    }
}

void ProcessorTest::runInputBoundsTest(vital::Processor* processor) {
    runInputBoundsTest(processor, std::set<int>(), std::set<int>());
}

void ProcessorTest::runInputBoundsTest(vital::Processor* processor, std::set<int> leave_inputs,
                                       std::set<int> ignore_outputs) {
    int num_inputs = processor->numInputs();

    std::vector<vital::Value> inputs;
    inputs.resize(num_inputs);

    // Create an audio input buffer filled with random noise in [-1, 1].
    vital::Output audio;
    audio.ensureBufferSize(vital::kMaxBufferSize);
    for (int i = 0; i < vital::kMaxBufferSize; ++i)
        audio.buffer[i] = (rand() * 2.0f) / RAND_MAX - 1.0f;

    // Plug the audio buffer into the first input and Values into others (unless left unchanged).
    processor->plug(&audio, 0);
    for (int i = 1; i < num_inputs; ++i) {
        if (!leave_inputs.count(i))
            processor->plug(&inputs[i], i);
    }

    beginTest("Inputs Zeroed Test");
    // Inputs remain zero except the audio buffer. Check stability.
    processAndCheckFinite(processor, ignore_outputs);

    beginTest("Inputs High");
    for (int i = 1; i < num_inputs; ++i)
        inputs[i].set(100000.0f);
    processAndCheckFinite(processor, ignore_outputs);

    beginTest("Inputs Negative");
    for (int i = 1; i < num_inputs; ++i)
        inputs[i].set(-100000.0f);
    processAndCheckFinite(processor, ignore_outputs);

    beginTest("Inputs Random");
    for (int r = 0; r < RANDOMIZE_AMOUNT; ++r) {
        for (int i = 1; i < num_inputs; ++i)
            inputs[i].set(100000.0f * ((rand() % 3) - 1.0f)); // Values in {-100000, 0, 100000}
        processAndCheckFinite(processor, ignore_outputs);
    }

    // Final check after randomization tests.
    processAndCheckFinite(processor, ignore_outputs);
}
