/**
 * @file envelope_test.h
 * @brief Declares the EnvelopeTest class, which tests the Envelope processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class EnvelopeTest
 * @brief A test class that verifies the stability and correctness of the Envelope processor.
 *
 * This test checks whether the Envelope processor handles extreme input conditions without producing
 * non-finite outputs. By leveraging the ProcessorTest framework, it applies standardized input bounds tests
 * to ensure the envelope behaves predictably.
 */
class EnvelopeTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new EnvelopeTest with the specified test name.
     */
    EnvelopeTest() : ProcessorTest("Envelope") { }

    /**
     * @brief Runs the Envelope test by performing input bounds checks, verifying that the envelope
     *        output remains finite and stable under extreme input conditions.
     */
    void runTest() override;
};
