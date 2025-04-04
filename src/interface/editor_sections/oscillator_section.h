#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "open_gl_image_component.h"
#include "preset_selector.h"
#include "transpose_quantize.h"
#include "wavetable_3d.h"

class IncrementerButtons;
class OpenGlShapeButton;
class PresetSelector;
class SaveSection;
class TextSelector;
class UnisonViewer;
class WavetableCreator;

/**
 * @class OscillatorSection
 * @brief A UI section representing an oscillator in the synthesizer.
 *
 * This class manages the visual and interactive components of a single oscillator,
 * including wavetable display, parameter controls (spectral morph, distortion,
 * destination routing, unison parameters), pitch quantization, and preset selection.
 * It integrates with the Vital backend via modulations and can load and manipulate wavetables.
 */
class OscillatorSection : public SynthSection,
                          public PresetSelector::Listener,
                          public TextEditor::Listener,
                          public Wavetable3d::Listener,
                          public TransposeQuantizeButton::Listener,
                          public Timer {
public:
    /** @brief Relative width ratio of the oscillator section. */
    static constexpr float kSectionWidthRatio = 0.19f;

    /**
     * @class Listener
     * @brief Listener interface for receiving oscillator section changes.
     */
    class Listener {
    public:
        virtual ~Listener() = default;
        /**
         * @brief Callback triggered when the distortion type changes.
         * @param section Pointer to this oscillator section.
         * @param type The new distortion type.
         */
        virtual void distortionTypeChanged(OscillatorSection* section, int type) = 0;

        /**
         * @brief Callback triggered when the oscillator destination changes.
         * @param section Pointer to this oscillator section.
         * @param destination The new destination setting.
         */
        virtual void oscillatorDestinationChanged(OscillatorSection* section, int destination) = 0;
    };

    /**
     * @brief Constructs a new OscillatorSection.
     * @param auth Pointer to the Authentication object.
     * @param index Index of the oscillator (0-based).
     * @param mono_modulations Map of mono modulation outputs.
     * @param poly_modulations Map of polyphonic modulation outputs.
     */
    OscillatorSection(Authentication* auth,
                      int index,
                      const vital::output_map& mono_modulations,
                      const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~OscillatorSection();

    /**
     * @brief Sets visual skin values.
     * @param skin The Skin object with UI styling parameters.
     * @param top_level Whether this is the top-level call.
     */
    void setSkinValues(const Skin& skin, bool top_level) override;

    /**
     * @brief Paints the background of this section.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the background shadow if active.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Called when the component is resized. Handles layout.
     */
    void resized() override;

    /**
     * @brief Resets the oscillator section and marks the wavetable as dirty.
     */
    void reset() override {
        SynthSection::reset();
        wavetable_->setDirty();
    }

    /**
     * @brief Handles button clicks.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Sets all control values from a given control map.
     * @param controls The control map with parameter values.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Handles the return key press in the text editor.
     * @param text_editor Reference to the TextEditor.
     */
    void textEditorReturnKeyPressed(TextEditor& text_editor) override;

    /**
     * @brief Handles focus loss in the text editor.
     * @param text_editor Reference to the TextEditor.
     */
    void textEditorFocusLost(TextEditor& text_editor) override;

    /**
     * @brief Timer callback for handling transient states (e.g. error messages).
     */
    void timerCallback() override;

    /**
     * @brief Sets the active state of the oscillator section.
     * @param active True if active, false otherwise.
     */
    void setActive(bool active) override;

    /**
     * @brief Sets the display name of the oscillator section.
     * @param name The name to display (e.g. a preset name).
     */
    void setName(String name);

    /**
     * @brief Resets the oscillator modulation distortion type to default.
     */
    void resetOscillatorModulationDistortionType();

    /**
     * @brief Adds a listener for oscillator section events.
     * @param listener Pointer to the listener object.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Loads a wavetable from text input (TTWT - text to wavetable).
     * @param text The user input text.
     * @return Error message if any, otherwise an empty string.
     */
    std::string loadWavetableFromText(const String& text);

    /**
     * @brief Gets the wave frame slider.
     * @return Pointer to the wave frame slider.
     */
    Slider* getWaveFrameSlider();

    /**
     * @brief Sets the currently selected distortion type.
     * @param selection The index of the distortion type.
     */
    void setDistortionSelected(int selection);

    /**
     * @brief Gets the current distortion type.
     * @return The current distortion type index.
     */
    int getDistortion() const { return current_distortion_type_; }

    /**
     * @brief Sets the currently selected spectral morph type.
     * @param selection The index of the spectral morph type.
     */
    void setSpectralMorphSelected(int selection);

    /**
     * @brief Sets the currently selected destination routing.
     * @param selection The index of the destination option.
     */
    void setDestinationSelected(int selection);

    /**
     * @brief Toggles filter input on or off.
     * @param filter_index Index of the filter to toggle.
     * @param on True to turn on, false to turn off.
     */
    void toggleFilterInput(int filter_index, bool on);

    /**
     * @brief Loads the browser state for wavetable preset selection.
     */
    void loadBrowserState();

    /**
     * @brief Sets the language index for the text-to-wavetable functionality.
     */
    void setIndexSelected();

    /**
     * @brief Sets the language for text-to-wavetable processing.
     * @param index Index of the selected language.
     */
    void setLanguage(int index);

    /**
     * @brief Cancels the language selection for TTWT.
     */
    void languageSelectCancelled();

    /**
     * @brief Loads the previous wavetable in the browser.
     */
    void prevClicked() override;

    /**
     * @brief Loads the next wavetable in the browser.
     */
    void nextClicked() override;

    /**
     * @brief Handles mouse-down events on textual components.
     * @param e The MouseEvent.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Called when transpose quantization is updated.
     */
    void quantizeUpdated() override;

    /**
     * @brief Loads audio data as a wavetable.
     * @param name The name of the wavetable.
     * @param audio_stream Input audio data stream.
     * @param style The load style (e.g. for TTWT, splice).
     * @return True if successful, false otherwise.
     */
    bool loadAudioAsWavetable(String name, InputStream* audio_stream,
                              WavetableCreator::AudioFileLoadStyle style) override;

    /**
     * @brief Loads a wavetable from JSON data.
     * @param wavetable_data JSON object with wavetable data.
     */
    void loadWavetable(json& wavetable_data) override;

    /**
     * @brief Loads the default (init) wavetable.
     */
    void loadDefaultWavetable() override;

    /**
     * @brief Resynthesizes the current source to a wavetable.
     */
    void resynthesizeToWavetable() override;

    /**
     * @brief Converts entered text to a wavetable (TTWT).
     */
    void textToWavetable() override;

    /**
     * @brief Saves the current wavetable to disk.
     */
    void saveWavetable() override;

    /**
     * @brief Loads a wavetable file from disk.
     * @param wavetable_file The file to load.
     */
    void loadFile(const File& wavetable_file) override;

    /**
     * @brief Gets the current wavetable file.
     * @return The current File object.
     */
    File getCurrentFile() override { return current_file_; }

    /**
     * @brief Gets the name of the loaded wavetable file.
     * @return The file name as a string.
     */
    std::string getFileName() override { return wavetable_->getWavetable()->getName(); }

    /**
     * @brief Gets the author of the loaded wavetable file.
     * @return The author's name as a string.
     */
    std::string getFileAuthor() override { return wavetable_->getWavetable()->getAuthor(); }

    /**
     * @brief Gets the oscillator index.
     * @return The 0-based oscillator index.
     */
    int index() const { return index_; }

    /**
     * @brief Gets the unison voices slider.
     * @return Pointer to the unison voices slider.
     */
    SynthSlider* getVoicesSlider() const { return unison_voices_.get(); }

    /**
     * @brief Gets the wave frame slider.
     * @return Pointer to the wave frame slider (const version).
     */
    const SynthSlider* getWaveFrameSlider() const { return wave_frame_.get(); }

    /**
     * @brief Gets the spectral morph slider.
     * @return Pointer to the spectral morph slider.
     */
    const SynthSlider* getSpectralMorphSlider() const { return spectral_morph_amount_.get(); }

    /**
     * @brief Gets the distortion slider.
     * @return Pointer to the distortion amount slider.
     */
    const SynthSlider* getDistortionSlider() const { return distortion_amount_.get(); }

    /**
     * @brief Gets the relative bounds of the wavetable display area.
     * @return A rectangle defining the wavetable display in relative coordinates.
     */
    Rectangle<float> getWavetableRelativeBounds();

private:
    /**
     * @brief Shows the text-to-wavetable settings popup.
     */
    void showTtwtSettings();

    /**
     * @brief Configures the spectral morph parameters after a change.
     */
    void setupSpectralMorph();

    /**
     * @brief Configures the distortion parameters after a change.
     */
    void setupDistortion();

    /**
     * @brief Configures the destination parameters after a change.
     */
    void setupDestination();

    /**
     * @brief Shows or hides the distortion phase slider.
     * @param visible True to show, false to hide.
     */
    void setDistortionPhaseVisible(bool visible);

    /**
     * @brief Notifies listeners that the spectral morph type changed.
     */
    void notifySpectralMorphTypeChange();

    /**
     * @brief Notifies listeners that the distortion type changed.
     */
    void notifyDistortionTypeChange();

    /**
     * @brief Notifies listeners that the destination changed.
     */
    void notifyDestinationChange();

    Authentication* auth_;                   ///< Pointer to the authentication object.
    std::vector<Listener*> listeners_;       ///< Registered listeners.
    int index_;                              ///< Oscillator index.
    File current_file_;                      ///< Current wavetable file.

    std::string distortion_control_name_;    ///< Distortion control parameter name.
    std::string spectral_morph_control_name_;///< Spectral morph control parameter name.
    std::string destination_control_name_;   ///< Destination control parameter name.
    std::string quantize_control_name_;      ///< Quantize control parameter name.
    int current_distortion_type_;            ///< Current distortion type.
    int current_spectral_morph_type_;        ///< Current spectral morph type.
    int current_destination_;                ///< Current destination routing.
    bool show_ttwt_error_;                   ///< Flag indicating whether to show TTWT error.
    bool showing_language_menu_;             ///< Flag indicating whether language menu is shown.
    int ttwt_language_;                      ///< Current TTWT language index.

    std::unique_ptr<SynthButton> oscillator_on_;       ///< On/Off button for the oscillator.
    std::unique_ptr<SynthButton> dimension_button_;    ///< Button to toggle wavetable dimension view.
    std::unique_ptr<SynthSlider> dimension_value_;     ///< Dimension value slider.
    std::unique_ptr<PresetSelector> preset_selector_;  ///< Wavetable preset selector.
    std::unique_ptr<Wavetable3d> wavetable_;           ///< 3D Wavetable viewer.
    std::unique_ptr<UnisonViewer> unison_viewer_;      ///< Unison visualization component.

    std::unique_ptr<TransposeQuantizeButton> transpose_quantize_button_; ///< Quantize button.
    std::unique_ptr<SynthSlider> transpose_;                              ///< Transpose slider.
    std::unique_ptr<SynthSlider> tune_;                                   ///< Fine-tune slider.

    std::unique_ptr<PlainTextComponent> distortion_type_text_;    ///< Distortion type text display.
    std::unique_ptr<ShapeButton> distortion_type_selector_;       ///< Distortion type selector button.
    std::unique_ptr<SynthSlider> distortion_amount_;              ///< Distortion amount slider.
    std::unique_ptr<SynthSlider> distortion_phase_;               ///< Distortion phase slider.
    std::unique_ptr<SynthSlider> phase_;                          ///< Phase slider.
    std::unique_ptr<SynthSlider> random_phase_;                   ///< Random phase slider.

    std::unique_ptr<PlainTextComponent> spectral_morph_type_text_;///< Spectral morph type text display.
    std::unique_ptr<ShapeButton> spectral_morph_type_selector_;   ///< Spectral morph type selector.
    std::unique_ptr<SynthSlider> spectral_morph_amount_;          ///< Spectral morph amount slider.

    std::unique_ptr<PlainTextComponent> destination_text_;        ///< Destination text display.
    std::unique_ptr<ShapeButton> destination_selector_;           ///< Destination selector button.

    std::unique_ptr<SynthSlider> level_;                          ///< Level (amplitude) slider.
    std::unique_ptr<SynthSlider> pan_;                            ///< Pan slider.
    std::unique_ptr<SynthSlider> wave_frame_;                     ///< Wavetable frame slider.

    std::unique_ptr<SynthSlider> unison_voices_;                  ///< Unison voices slider.
    std::unique_ptr<SynthSlider> unison_detune_;                  ///< Unison detune slider.
    std::unique_ptr<SynthSlider> unison_detune_power_;            ///< Unison detune power slider.
    std::unique_ptr<OpenGlShapeButton> edit_button_;              ///< Edit wavetable button.

    OpenGlQuad ttwt_overlay_;                                     ///< Overlay for TTWT input.
    std::unique_ptr<OpenGlTextEditor> ttwt_;                      ///< TTWT input text editor.
    std::unique_ptr<SynthButton> ttwt_settings_;                  ///< TTWT settings button.
    std::unique_ptr<PlainTextComponent> ttwt_error_text_;         ///< TTWT error message display.

    std::unique_ptr<OpenGlShapeButton> prev_destination_;          ///< Previous destination button.
    std::unique_ptr<OpenGlShapeButton> next_destination_;          ///< Next destination button.
    std::unique_ptr<OpenGlShapeButton> prev_spectral_;             ///< Previous spectral morph type button.
    std::unique_ptr<OpenGlShapeButton> next_spectral_;             ///< Next spectral morph type button.
    std::unique_ptr<OpenGlShapeButton> prev_distortion_;           ///< Previous distortion type button.
    std::unique_ptr<OpenGlShapeButton> next_distortion_;           ///< Next distortion type button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorSection)
};
