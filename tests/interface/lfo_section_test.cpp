/**
 * @file lfo_section_test.cpp
 * @brief Implements the LfoSectionTest class, performing stress tests on the LfoSection UI component.
 */

#include "lfo_section_test.h"
#include "lfo_section.h"

void LfoSectionTest::runTest() {
    // Create and configure the synth engine required by the LfoSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Create a LineGenerator for the LFO source.
    LineGenerator line_source;

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the LfoSection component with a name, internal ID, and references to the LFO source and modulations.
    LfoSection lfo_section("LFO 3", "lfo_3", &line_source, engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the lfo_section
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&lfo_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static LfoSectionTest lfo_section_test;
