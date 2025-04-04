/**
 * @file interface_test.h
 * @brief Declares the InterfaceTest class, which provides a testing framework for Vital's UI components.
 */

#pragma once

#include "JuceHeader.h"
#include "sound_engine.h"
#include "synth_base.h"

class FullInterface;
class SynthSection;
class TestSynthBase;

/**
 * @class TestSynthBase
 * @brief A TestSynthBase derived from SynthBase that facilitates testing by implementing required virtual methods.
 *
 * TestSynthBase provides a basic environment where the UI can interact with the underlying synth engine.
 * It allows pausing of processing to ensure thread-safe operations on the engine during tests.
 */
class TestSynthBase : public SynthBase {
public:
    /**
     * @brief Constructs a TestSynthBase with no GUI interface initially.
     */
    TestSynthBase() : gui_interface_(nullptr) { }

    /**
     * @brief Sets the GUI interface for this synth base.
     * @param gui_interface A pointer to a SynthGuiInterface.
     */
    void setGuiInterface(SynthGuiInterface* gui_interface) { gui_interface_ = gui_interface; }

    /**
     * @brief Returns a reference to the internal CriticalSection for thread-safe operations.
     * @return A reference to the CriticalSection.
     */
    const CriticalSection& getCriticalSection() override { return critical_section_; }

    /**
     * @brief Pauses or resumes processing by entering or exiting the critical section.
     * @param pause True to pause (enter critical section), false to resume (exit critical section).
     */
    void pauseProcessing(bool pause) override {
        if (pause)
            critical_section_.enter();
        else
            critical_section_.exit();
    }

    /**
     * @brief Gets the GUI interface associated with this synth base.
     * @return A pointer to the SynthGuiInterface, or nullptr if not set.
     */
    SynthGuiInterface* getGuiInterface() override { return gui_interface_; }

    /**
     * @brief Processes a block of audio with the synth engine.
     * @param buffer The audio buffer to process.
     * @param channels The number of channels in the buffer.
     * @param samples The number of samples to process.
     * @param offset The offset in the buffer to start processing.
     */
    void process(AudioSampleBuffer* buffer, int channels, int samples, int offset) {
        ScopedLock lock(getCriticalSection());
        processAudio(buffer, channels, samples, offset);
    }

private:
    CriticalSection critical_section_;  ///< Synchronization lock for safe concurrent operations.
    SynthGuiInterface* gui_interface_;  ///< Associated GUI interface (if any).
};

/**
 * @class InterfaceTest
 * @brief A base test class for testing Vital's interface components.
 *
 * InterfaceTest provides functionality to set up a test environment with a synth engine,
 * and offers methods to run stress tests on UI components. Derived classes can use these utilities
 * to test various UI sections of the synthesizer.
 */
class InterfaceTest : public UnitTest {
public:
    /**
     * @brief Constructs an InterfaceTest with a given name.
     * @param name The name of the test.
     */
    InterfaceTest(std::string name) : UnitTest(name, "Interface") { }

    /**
     * @brief Runs a stress test by randomly modifying controls of the given SynthSection component
     *        (and optionally a FullInterface) over time.
     *
     * @param component The SynthSection to test.
     * @param full_interface Optional pointer to a FullInterface if the test involves the full UI.
     */
    void runStressRandomTest(SynthSection* component, FullInterface* full_interface = nullptr);

    /**
     * @brief Creates a SynthEngine by instantiating a TestSynthBase.
     * @return A pointer to the created SoundEngine.
     */
    vital::SoundEngine* createSynthEngine() {
        synth_base_ = std::make_unique<TestSynthBase>();
        return synth_base_->getEngine();
    }

    /**
     * @brief Gets the current SynthBase instance.
     * @return A pointer to the current SynthBase.
     */
    SynthBase* getSynthBase() {
        return synth_base_.get();
    }

    /**
     * @brief Gets the current SoundEngine instance from the SynthBase.
     * @return A pointer to the SoundEngine.
     */
    vital::SoundEngine* getSynthEngine() {
        return synth_base_->getEngine();
    }

    /**
     * @brief Deletes the currently held SynthEngine (and associated SynthBase).
     */
    void deleteSynthEngine() {
        synth_base_ = nullptr;
    }

private:
    std::unique_ptr<TestSynthBase> synth_base_; ///< The SynthBase used for testing.
};
