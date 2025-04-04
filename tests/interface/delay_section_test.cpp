/**
 * @file delay_section_test.cpp
 * @brief Implements the DelaySectionTest class, performing stress tests on the DelaySection UI component.
 */

#include "delay_section_test.h"
#include "delay_section.h"

void DelaySectionTest::runTest() {
    // Create and configure the synth engine required by the DelaySection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the DelaySection component, passing in mono modulations from the engine.
    DelaySection delay_section("Delay", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the delay_section to ensure proper behavior.
    runStressRandomTest(&delay_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static DelaySectionTest delay_section_test;
