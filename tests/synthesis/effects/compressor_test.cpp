/**
 * @file compressor_test.cpp
 * @brief Implements the CompressorTest class, running input bounds tests against the Compressor processor.
 */

#include "compressor_test.h"
#include "compressor.h"

void CompressorTest::runTest() {
    // Create a Compressor instance with arbitrary parameters.
    vital::Compressor compressor(1.0f, 10.0f, 1.0f, 10.0f);

    // Run a standardized input bounds test to ensure the compressor behaves correctly
    // with extreme and typical input values.
    runInputBoundsTest(&compressor);
}

// Registers the test instance so it will be automatically discovered and run.
static CompressorTest compressor_test;
