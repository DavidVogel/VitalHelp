/**
 * @file synthesis_interface_test.cpp
 * @brief Implements the SynthesisInterfaceTest class, performing stress tests on the SynthesisInterface UI component.
 */

#include "synthesis_interface_test.h"
#include "synthesis_interface.h"

void SynthesisInterfaceTest::runTest() {
    // Create and configure the synth engine required by the SynthesisInterface.
    vital::SoundEngine* engine = createSynthEngine();

    // Acquire a MessageManagerLock for safe UI operations.
    MessageManager::getInstance();
    std::unique_ptr<MessageManagerLock> lock = std::make_unique<MessageManagerLock>();

    // Create the SynthesisInterface component, passing in mono and poly modulations from the engine.
    SynthesisInterface synthesis_interface(nullptr, engine->getMonoModulations(), engine->getPolyModulations());

    // Release the lock before running stress tests to allow UI processing.
    lock.reset();

    // Perform stress tests with random input on the synthesis_interface
    // to ensure it behaves correctly and robustly.
    runStressRandomTest(&synthesis_interface);

    // Clean up the synth engine once tests are complete.
    deleteSynthEngine();
}

// Instantiate a static test instance that registers and runs this test.
static SynthesisInterfaceTest synthesis_interface_test;
