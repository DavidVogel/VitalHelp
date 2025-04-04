/**
 * @file reverb_section_test.cpp
 * @brief Implements the ReverbSectionTest class, performing stress tests on the ReverbSection UI component.
 */

#include "reverb_section_test.h"
#include "reverb_section.h"

void ReverbSectionTest::runTest() {
    // Create and configure the synth engine required by the ReverbSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the ReverbSection component, passing in mono modulations from the engine.
    ReverbSection reverb_section("Reverb", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the reverb_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&reverb_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static ReverbSectionTest reverb_section_test;
