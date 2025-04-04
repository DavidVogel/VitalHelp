#pragma once

#include "JuceHeader.h"
#include "memory.h"
#include "oscillator_advanced_section.h"
#include "synth_constants.h"
#include "synth_section.h"

class TextSelector;
class DisplaySettings;
class OversampleSettings;
class VoiceSettings;
class OutputDisplays;
class OscillatorSection;

/**
 * @class TuningSelector
 * @brief A specialized TextSelector for selecting tuning systems or loading custom tuning files.
 *
 * TuningSelector provides a selection of predefined tuning scales (e.g., Just Intonation,
 * Pythagorean) as well as the ability to load a custom .scl tuning file. It integrates
 * closely with the synthesizer model to apply the selected tuning system.
 */
class TuningSelector : public TextSelector {
public:
    /**
     * @enum TuningStyle
     * @brief Enumerates the available tuning styles.
     */
    enum TuningStyle {
        kDefault,       ///< Default equal-tempered tuning.
        k7Limit,        ///< Just intonation with a 7-limit scale.
        k5Limit,        ///< Just intonation with a 5-limit scale.
        kPythagorean,   ///< Pythagorean tuning.
        kNumTunings     ///< Number of available tuning styles.
    };

    /**
     * @brief Constructs a TuningSelector with a given name.
     *
     * @param name The name of this tuning selector component.
     */
    TuningSelector(String name);

    /**
     * @brief Destructor.
     */
    virtual ~TuningSelector();

    /**
     * @brief Handles mouse down events, including showing a popup menu for selecting tunings.
     *
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Called when the value of the selector changes.
     *
     * Updates the synthesizer tuning if attached to a SynthGuiInterface.
     */
    void valueChanged() override;

    /**
     * @brief Called when the component's parent hierarchy changes.
     *
     * Ensures the displayed custom string matches the current tuning's name.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Ignores mouse wheel moves for this component.
     */
    void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel) override { }

    /**
     * @brief Sets the current tuning style.
     *
     * @param tuning The tuning index to set.
     */
    void setTuning(int tuning);

private:
    /**
     * @brief Loads a predefined tuning style and applies it to the synth.
     *
     * @param tuning The tuning style to load.
     */
    void loadTuning(TuningStyle tuning);

    /**
     * @brief Presents a file chooser for selecting a custom tuning file.
     */
    void loadTuningFile();

    /**
     * @brief Loads a custom tuning from a given file.
     *
     * @param file The tuning file to load.
     */
    void loadTuningFile(const File& file);

    /**
     * @brief Retrieves the current tuning name from the synthesizer.
     *
     * @return The current tuning name as a String.
     */
    String getTuningName();

    /**
     * @brief Updates the custom tuning name displayed in the selector.
     *
     * @param custom_string The new custom string to display.
     */
    void setCustomString(std::string custom_string) {
        strings_[kNumTunings] = custom_string;
        repaint();
    }

    std::string strings_[kNumTunings + 1]; ///< Array of tuning names, including "Custom".
};

/**
 * @class MasterControlsInterface
 * @brief A top-level UI component that contains various sections for configuring the synthesizer.
 *
 * The MasterControlsInterface aggregates several sections that control global aspects
 * of the synth engine and interface:
 * - Advanced oscillator sections (if in a synth mode)
 * - Voice settings (MPE, tuning, voice priority)
 * - Oversampling settings
 * - Display settings (frequency units, skins)
 * - Output displays (oscilloscope, spectrogram)
 *
 * It arranges these sections, provides layout management, and integrates with the
 * synthesizer's model to update and reflect global parameters.
 */
class MasterControlsInterface : public SynthSection {
public:
    /**
     * @brief Constructs a MasterControlsInterface.
     *
     * @param mono_modulations A map of available monophonic modulations.
     * @param poly_modulations A map of available polyphonic modulations.
     * @param synth If true, includes advanced oscillator sections.
     */
    MasterControlsInterface(const vital::output_map& mono_modulations,
                            const vital::output_map& poly_modulations, bool synth);

    /**
     * @brief Destructor.
     */
    virtual ~MasterControlsInterface();

    /**
     * @brief Paints the background of the interface.
     *
     * @param g The JUCE Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Resizes and lays out all child components (oscillators, voice settings, etc.).
     */
    void resized() override;

    /**
     * @brief Sets the bounds of a specific oscillator section.
     *
     * @param index The oscillator index.
     * @param bounds The new rectangle bounds for that oscillator.
     */
    void setOscillatorBounds(int index, Rectangle<int> bounds) { oscillator_advanceds_[index]->setBounds(bounds); }

    /**
     * @brief Passes an oscillator section model to the corresponding advanced section UI.
     *
     * @param index The oscillator index.
     * @param oscillator The OscillatorSection to display/edit.
     */
    void passOscillatorSection(int index, const OscillatorSection* oscillator);

    /**
     * @brief Sets the oscilloscope memory for audio visualization.
     *
     * @param memory Pointer to the poly_float buffer used by the oscilloscope.
     */
    void setOscilloscopeMemory(const vital::poly_float* memory);

    /**
     * @brief Sets the audio memory for spectrogram analysis.
     *
     * @param memory Pointer to the StereoMemory buffer used by the spectrogram.
     */
    void setAudioMemory(const vital::StereoMemory* memory);

private:
    std::unique_ptr<OscillatorAdvancedSection> oscillator_advanceds_[vital::kNumOscillators]; ///< Advanced oscillator controls.
    std::unique_ptr<DisplaySettings> display_settings_;                                       ///< Display settings section.
    std::unique_ptr<OversampleSettings> oversample_settings_;                                 ///< Oversampling settings section.
    std::unique_ptr<VoiceSettings> voice_settings_;                                           ///< Voice settings section.
    std::unique_ptr<OutputDisplays> output_displays_;                                         ///< Output displays (oscilloscope, spectrogram).

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterControlsInterface)
};
