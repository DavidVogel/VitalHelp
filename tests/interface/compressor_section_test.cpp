/**
 * @file compressor_section_test.cpp
 * @brief Implements the CompressorSectionTest class, executing stress tests on the CompressorSection UI component.
 */

#include "compressor_section_test.h"
#include "compressor_section.h"

void CompressorSectionTest::runTest() {
    // Set up a synth engine environment for the test.
    createSynthEngine();

    // Acquire a MessageManagerLock for thread-safe UI creation and manipulation.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the compressor section component for testing.
    CompressorSection compressor_section("Compressor");

    // Release the lock so the UI thread can process messages during the test.
    lock.reset();

    // Run stress tests providing random input to ensure the compressor section behaves correctly.
    runStressRandomTest(&compressor_section);

    // Clean up and tear down the synth engine after tests complete.
    deleteSynthEngine();
}

// Create a static instance of the test to be automatically registered and run.
static CompressorSectionTest compressor_section_test;
