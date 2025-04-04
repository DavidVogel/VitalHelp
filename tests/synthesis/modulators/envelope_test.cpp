/**
 * @file envelope_test.cpp
 * @brief Implements the EnvelopeTest class, performing input bounds tests on the Envelope processor.
 */

#include "envelope_test.h"
#include "envelope.h"

void EnvelopeTest::runTest() {
    // Create an Envelope processor instance.
    vital::Envelope envelope;

    // Run a standardized input bounds test to ensure the envelope handles extreme input values gracefully.
    runInputBoundsTest(&envelope);
}

// Registers the test instance so it will be automatically discovered and run.
static EnvelopeTest envelope_test;
