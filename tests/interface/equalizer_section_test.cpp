/**
 * @file equalizer_section_test.cpp
 * @brief Implements the EqualizerSectionTest class, performing stress tests on the EqualizerSection UI component.
 */

#include "equalizer_section_test.h"
#include "equalizer_section.h"

void EqualizerSectionTest::runTest() {
    // Create and configure the synth engine required by the EqualizerSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the EqualizerSection component, passing in mono modulations from the engine.
    EqualizerSection equalizer_section("Equalizer", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the equalizer_section
    // to ensure it functions correctly under various conditions.
    runStressRandomTest(&equalizer_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static EqualizerSectionTest equalizer_section_test;
