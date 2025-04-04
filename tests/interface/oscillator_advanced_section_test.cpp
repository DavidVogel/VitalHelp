/**
 * @file oscillator_advanced_section_test.cpp
 * @brief Implements the OscillatorAdvancedSectionTest class, performing stress tests on the OscillatorAdvancedSection UI component.
 */

#include "oscillator_advanced_section_test.h"
#include "oscillator_advanced_section.h"

void OscillatorAdvancedSectionTest::runTest() {
    // Create and configure the synth engine required by the OscillatorAdvancedSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the OscillatorAdvancedSection component with given parameters and modulation sources.
    OscillatorAdvancedSection oscillator_section(1, engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the oscillator_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&oscillator_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static OscillatorAdvancedSectionTest oscillator_advanced_section_test;
