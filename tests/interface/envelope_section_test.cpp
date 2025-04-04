/**
 * @file envelope_section_test.cpp
 * @brief Implements the EnvelopeSectionTest class, running stress tests on the EnvelopeSection UI component.
 */

#include "envelope_section_test.h"
#include "envelope_section.h"

void EnvelopeSectionTest::runTest() {
    // Create and configure the synth engine required by the EnvelopeSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the EnvelopeSection component with specified names and modulation sources.
    EnvelopeSection envelope_section("ENV 2", "env_2", engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the envelope_section to ensure it behaves correctly.
    runStressRandomTest(&envelope_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static EnvelopeSectionTest envelope_section_test;
