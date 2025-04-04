/**
 * @file oscillator_section_test.cpp
 * @brief Implements the OscillatorSectionTest class, performing stress tests on the OscillatorSection UI component.
 */

#include "oscillator_section_test.h"
#include "oscillator_section.h"

void OscillatorSectionTest::runTest() {
    // Create and configure the synth engine required by the OscillatorSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the OscillatorSection component, providing no parent and given parameters.
    OscillatorSection oscillator_section(nullptr, 1, engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the oscillator_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&oscillator_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static OscillatorSectionTest oscillator_section_test;
