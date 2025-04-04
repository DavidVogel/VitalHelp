/**
 * @file distortion_section_test.cpp
 * @brief Implements the DistortionSectionTest class, running stress tests on the DistortionSection UI component.
 */

#include "distortion_section_test.h"
#include "distortion_section.h"

void DistortionSectionTest::runTest() {
    // Create and configure the synth engine required by the DistortionSection.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the DistortionSection component, passing in mono modulations from the engine.
    DistortionSection distortion_section("Distortion", engine->getMonoModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the distortion_section
    // to ensure correct and robust behavior.
    runStressRandomTest(&distortion_section);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static DistortionSectionTest distortion_section_test;
