/**
 * @file sample_section_test.cpp
 * @brief Implements the SampleSectionTest class, performing stress tests on the SampleSection UI component.
 */

#include "sample_section_test.h"
#include "sample_section.h"

void SampleSectionTest::runTest() {
    // Create and configure the synth engine required by the SampleSection.
    createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the SampleSection component for testing.
    SampleSection sample_section("Sample");

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the sample_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&sample_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static SampleSectionTest sample_section_test;
