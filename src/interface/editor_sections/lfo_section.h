#pragma once

#include "JuceHeader.h"

#include "synth_section.h"
#include "lfo_editor.h"
#include "preset_selector.h"

class RetriggerSelector;
class SynthSlider;
class TempoSelector;
class TextSelector;
class LineGenerator;
class PaintPatternSelector;

/**
 * @class LfoSection
 * @brief A user interface section that displays and controls an LFO (Low Frequency Oscillator) in the synthesizer.
 *
 * The LfoSection provides controls for editing LFO shapes, timing, smoothing,
 * paint patterns, and preset management. It integrates an LfoEditor for graphical
 * editing and supports loading and saving LFO configurations (presets).
 *
 * In addition to providing UI controls like sliders, text selectors, and toggle buttons,
 * the LfoSection supports painting LFO shapes directly in the editor, synchronizing
 * frequencies with tempo, and browsing through different LFO presets.
 *
 * It inherits from SynthSection, and implements PresetSelector::Listener and
 * LineEditor::Listener to react to changes in presets and manual line editing.
 */
class LfoSection : public SynthSection,
                   public PresetSelector::Listener,
                   public LineEditor::Listener {
public:
    /**
     * @enum PaintPattern
     * @brief Enumerates the paint patterns used for LFO shape painting.
     */
    enum PaintPattern {
        kStep,   ///< A step pattern (horizontal line at the top).
        kHalf,   ///< A pattern rising half-way and then dropping to zero.
        kDown,   ///< A downward linear ramp.
        kUp,     ///< An upward linear ramp.
        kTri,    ///< A triangular shape.
        kNumPaintPatterns ///< The number of available paint patterns.
    };

    /**
     * @brief Retrieves a paint pattern as a vector of (x, y) pairs.
     *
     * @param pattern The pattern identifier.
     * @return A vector of (x, y) pairs defining the selected paint pattern.
     */
    static std::vector<std::pair<float, float>> getPaintPattern(int pattern) {
        if (pattern == LfoSection::kHalf) {
            return {
                    { 0.0f, 1.0f },
                    { 0.5f, 1.0f },
                    { 0.5f, 0.0f },
                    { 1.0f, 0.0f }
            };
        }
        if (pattern == LfoSection::kDown) {
            return {
                    { 0.0f, 1.0f },
                    { 1.0f, 0.0f }
            };
        }
        if (pattern == LfoSection::kUp) {
            return {
                    { 0.0f, 0.0f },
                    { 1.0f, 1.0f }
            };
        }
        if (pattern == LfoSection::kTri) {
            return {
                    { 0.0f, 0.0f },
                    { 0.5f, 1.0f },
                    { 1.0f, 0.0f }
            };
        }
        return {
                { 0.0f, 1.0f },
                { 1.0f, 1.0f }
        };
    }

    /**
     * @brief Constructs a new LfoSection.
     *
     * @param name The display name of the LFO section.
     * @param value_prepend A prefix for parameter names, used to differentiate parameters of multiple LFOs.
     * @param lfo_source A pointer to a LineGenerator that provides the LFO data model.
     * @param mono_modulations A map of available monophonic modulations.
     * @param poly_modulations A map of available polyphonic modulations.
     */
    LfoSection(String name, std::string value_prepend,
               LineGenerator* lfo_source,
               const vital::output_map& mono_modulations,
               const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    ~LfoSection();

    /**
     * @brief Paints the background of the LFO section.
     *
     * @param g The JUCE Graphics context to use for drawing.
     *
     * This method draws the UI elements behind the LFO editor and control widgets.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * Recomputes the layout of all sliders, buttons, and the LfoEditor area.
     */
    void resized() override;

    /**
     * @brief Resets all LFO settings to their default states.
     */
    void reset() override;

    /**
     * @brief Updates all control values in this section from the given map of controls.
     *
     * @param controls A map of parameter names to their current values.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Responds to a slider value change event.
     *
     * @param changed_slider A pointer to the slider that changed value.
     */
    void sliderValueChanged(Slider* changed_slider) override;

    /**
     * @brief Responds to a button click event.
     *
     * @param clicked_button A pointer to the clicked button.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Sets the LFO phase.
     *
     * @param phase A float representing the new LFO phase.
     */
    void setPhase(float phase) override { phase_->setValue(phase); }

    /**
     * @brief Responds to scrolling events in the line editor component.
     *
     * @param e The mouse event.
     * @param wheel The mouse wheel details.
     */
    void lineEditorScrolled(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Toggles paint mode for the LFO editor.
     *
     * @param enabled Whether to enable or disable the paint mode.
     * @param temporary_switch If true, temporarily switches the mode rather than toggling permanently.
     */
    void togglePaintMode(bool enabled, bool temporary_switch) override;

    /**
     * @brief Imports an LFO from a user-selected file.
     */
    void importLfo() override;

    /**
     * @brief Exports the current LFO configuration to a user-selected file.
     */
    void exportLfo() override;

    /**
     * @brief Called when an LFO file has been successfully loaded.
     */
    void fileLoaded() override;

    /**
     * @brief Loads the LFO configuration from the specified file.
     *
     * @param file The file to load from.
     */
    void loadFile(const File& file) override;

    /**
     * @brief Retrieves the current LFO file.
     *
     * @return The currently loaded LFO File object.
     */
    File getCurrentFile() override { return current_file_; }

    /**
     * @brief Loads the previous LFO preset.
     */
    void prevClicked() override;

    /**
     * @brief Loads the next LFO preset.
     */
    void nextClicked() override;

    /**
     * @brief Handles mouse-down events on the text component, usually to open a file browser.
     *
     * @param e The mouse event.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Sets the selected smooth mode.
     *
     * @param result The selected smooth mode index.
     */
    void setSmoothModeSelected(int result);

private:
    /**
     * @brief The currently loaded LFO file.
     */
    File current_file_;

    /**
     * @brief The graphical LFO editor component.
     */
    std::unique_ptr<LfoEditor> editor_;

    /**
     * @brief A preset selector for managing and browsing LFO presets.
     */
    std::unique_ptr<PresetSelector> preset_selector_;

    /**
     * @brief A slider controlling LFO phase.
     */
    std::unique_ptr<SynthSlider> phase_;

    /**
     * @brief A slider controlling LFO frequency.
     */
    std::unique_ptr<SynthSlider> frequency_;

    /**
     * @brief A slider controlling LFO tempo.
     */
    std::unique_ptr<SynthSlider> tempo_;

    /**
     * @brief A slider controlling key tracking (transpose).
     */
    std::unique_ptr<SynthSlider> keytrack_transpose_;

    /**
     * @brief A slider controlling key tracking (tune).
     */
    std::unique_ptr<SynthSlider> keytrack_tune_;

    /**
     * @brief A slider controlling the fade time of the LFO.
     */
    std::unique_ptr<SynthSlider> fade_;

    /**
     * @brief A slider controlling the smoothing time of the LFO.
     */
    std::unique_ptr<SynthSlider> smooth_;

    /**
     * @brief The internal control name for the smooth mode.
     */
    std::string smooth_mode_control_name_;

    /**
     * @brief A text component displaying the current smooth mode.
     */
    std::unique_ptr<PlainTextComponent> smooth_mode_text_;

    /**
     * @brief A button to select the smooth mode type.
     */
    std::unique_ptr<ShapeButton> smooth_mode_type_selector_;

    /**
     * @brief A slider controlling the delay time of the LFO.
     */
    std::unique_ptr<SynthSlider> delay_;

    /**
     * @brief A slider controlling the stereo width of the LFO.
     */
    std::unique_ptr<SynthSlider> stereo_;

    /**
     * @brief A selector that manages tempo synchronization for the LFO.
     */
    std::unique_ptr<TempoSelector> sync_;

    /**
     * @brief A text selector controlling the synchronization type.
     */
    std::unique_ptr<TextSelector> sync_type_;

    /**
     * @brief A selector controlling the pattern to paint in the LFO editor.
     */
    std::unique_ptr<PaintPatternSelector> paint_pattern_;

    /**
     * @brief A component used as a divider between transpose and tune controls.
     */
    std::unique_ptr<OpenGlQuad> transpose_tune_divider_;

    /**
     * @brief A slider controlling the X-axis grid size in the LFO editor.
     */
    std::unique_ptr<SynthSlider> grid_size_x_;

    /**
     * @brief A slider controlling the Y-axis grid size in the LFO editor.
     */
    std::unique_ptr<SynthSlider> grid_size_y_;

    /**
     * @brief A button enabling paint mode in the LFO editor.
     */
    std::unique_ptr<OpenGlShapeButton> paint_;

    /**
     * @brief A button enabling/disabling smoothing in the LFO editor.
     */
    std::unique_ptr<OpenGlShapeButton> lfo_smooth_;

    /**
     * @brief The index of the current preset.
     */
    int current_preset_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfoSection)
};
