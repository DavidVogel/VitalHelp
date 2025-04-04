/**
 * @file flanger_section_test.cpp
 * @brief Implements the FlangerSectionTest class, performing stress tests on the FlangerSection UI component.
 */

#include "flanger_section_test.h"
#include "flanger_section.h"

void FlangerSectionTest::runTest() {
    // Create and configure the synth engine required by the FlangerSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the FlangerSection component, passing in mono modulations from the engine.
    FlangerSection flanger_section("Flanger", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the flanger_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&flanger_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static FlangerSectionTest flanger_section_test;
