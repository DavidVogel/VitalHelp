#pragma once

#include "JuceHeader.h"

#include "synth_base.h"
#include "synth_gui_interface.h"

/** Forward declaration of SynthComputerKeyboard class. */
class SynthComputerKeyboard;

/**
 * @class SynthEditor
 * @brief A standalone JUCE AudioAppComponent for hosting the Synth interface and audio engine.
 *
 * SynthEditor combines the GUI (if enabled) with real-time audio handling,
 * MIDI management, and the Vital synthesizer engine. It inherits from various
 * classes to integrate JUCE’s audio processing, the Vital engine, and the GUI interface.
 */
class SynthEditor : public AudioAppComponent,
                    public SynthBase,
                    public SynthGuiInterface,
                    public Timer {
public:
    /**
     * @brief Constructs a SynthEditor.
     *
     * @param use_gui Determines whether to create and display the full GUI interface.
     */
    SynthEditor(bool use_gui = true);

    /**
     * @brief Destructor. Cleans up resources, shutting down audio and dismissing menus.
     */
    ~SynthEditor();

    /**
     * @brief Prepares the audio engine to begin playing.
     *
     * Called by the audio subsystem. Sets up sample rate and other parameters in the engine.
     *
     * @param buffer_size The size of the audio buffer in samples.
     * @param sample_rate The sampling rate.
     */
    void prepareToPlay(int buffer_size, double sample_rate) override;

    /**
     * @brief Callback for the audio device. Processes incoming audio and MIDI.
     *
     * @param buffer The channel info containing buffer data.
     */
    void getNextAudioBlock(const AudioSourceChannelInfo& buffer) override;

    /**
     * @brief Releases resources allocated for audio playback.
     */
    void releaseResources() override;

    /**
     * @brief Called to repaint any custom graphics, which are not currently used here.
     *
     * @param g The JUCE Graphics context.
     */
    void paint(Graphics& g) override { }

    /**
     * @brief Called when the component is resized. Resizes internal GUI components if present.
     */
    void resized() override;

    /**
     * @brief Returns the critical section used for thread-safe operations.
     *
     * @return Reference to the CriticalSection.
     */
    const CriticalSection& getCriticalSection() override { return critical_section_; }

    /**
     * @brief Pauses or resumes audio processing by locking or unlocking the critical section.
     *
     * @param pause If true, locks the section; otherwise, unlocks it.
     */
    void pauseProcessing(bool pause) override {
      if (pause)
        critical_section_.enter();
      else
        critical_section_.exit();
    }

    /**
     * @brief Returns a pointer to the GUI interface of this synth.
     *
     * @return This SynthEditor as a SynthGuiInterface.
     */
    SynthGuiInterface* getGuiInterface() override { return this; }

    /**
     * @brief Returns a pointer to the AudioDeviceManager for this standalone app.
     *
     * @return Pointer to the AudioDeviceManager.
     */
    AudioDeviceManager* getAudioDeviceManager() override { return &deviceManager; }

    /**
     * @brief Timer callback used to periodically scan for new MIDI devices.
     */
    void timerCallback() override;

    /**
     * @brief Enables or disables animation in the GUI (e.g., for meters or visualizations).
     *
     * @param animate Whether to animate the GUI elements.
     */
    void animate(bool animate);

private:
    /**
     * @brief A keyboard object that enables handling of computer key events as MIDI.
     */
    std::unique_ptr<SynthComputerKeyboard> computer_keyboard_;

    /**
     * @brief CriticalSection to ensure thread-safety for audio and GUI interactions.
     */
    CriticalSection critical_section_;

    /**
     * @brief Currently active MIDI input device names.
     */
    StringArray current_midi_ins_;

    /**
     * @brief Tracks the current playback time in seconds.
     */
    double current_time_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthEditor)
};
