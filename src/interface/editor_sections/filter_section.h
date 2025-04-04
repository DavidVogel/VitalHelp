#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "preset_selector.h"

class FilterResponse;
class OpenGlToggleButton;
class SynthButton;
class SynthSlider;
class TextSelector;
class TextSlider;

/**
 * @class FilterSection
 * @brief A graphical user interface component representing a filter section in the synthesizer.
 *
 * The FilterSection displays and manages a variety of filter parameters, including filter models, styles,
 * cutoff, resonance, and other filter-related controls. It can show different filter types (analog, ladder, digital,
 * formant, comb, etc.) and dynamically adapts its UI to the selected filter model and style.
 *
 * This class also manages input routing controls (oscillator inputs, sample input, serial filtering input)
 * when necessary, and integrates with the preset selector for choosing different filter models and styles.
 */
class FilterSection : public SynthSection, public PresetSelector::Listener {
public:
    /**
     * @brief Vertical padding used for label placement relative to the blend slider.
     */
    static constexpr int kBlendLabelPaddingY = 4;

    /**
     * @class Listener
     * @brief An interface for objects that need to respond to changes in the FilterSection.
     *
     * Implement this interface if you need to be notified when certain filter events occur,
     * such as switching to a serial filter topology or toggling oscillator/sample inputs.
     */
    class Listener {
    public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~Listener() { }

        /**
         * @brief Called when the filter input button toggles to a serial filter topology.
         * @param section The FilterSection instance that triggered the change.
         */
        virtual void filterSerialSelected(FilterSection* section) = 0;

        /**
         * @brief Called when an oscillator input toggle button changes state.
         * @param section The FilterSection instance that triggered the change.
         * @param index The oscillator index (0, 1, or 2).
         * @param on True if the oscillator input is now active, false otherwise.
         */
        virtual void oscInputToggled(FilterSection* section, int index, bool on) = 0;

        /**
         * @brief Called when the sample input toggle button changes state.
         * @param section The FilterSection instance that triggered the change.
         * @param on True if the sample input is now active, false otherwise.
         */
        virtual void sampleInputToggled(FilterSection* section, bool on) = 0;
    };

    /**
     * @brief Constructs a FilterSection with a given suffix for parameter naming, using only mono modulations.
     * @param suffix A suffix string used to identify unique parameter names for this filter.
     * @param mono_modulations A map of mono modulation outputs from the synthesizer.
     */
    FilterSection(String suffix, const vital::output_map& mono_modulations);

    /**
     * @brief Constructs a FilterSection for a given index, using both mono and poly modulations.
     * @param index The filter index (e.g., for multiple filters in a chain).
     * @param mono_modulations A map of mono modulation outputs from the synthesizer.
     * @param poly_modulations A map of poly modulation outputs from the synthesizer.
     */
    FilterSection(int index, const vital::output_map& mono_modulations, const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    ~FilterSection();

    /**
     * @brief Connects the filter's sliders to the filter response display.
     *
     * This method links the various SynthSlider components (cutoff, resonance, etc.) to the FilterResponse
     * so that the filter response graph visually represents the current slider values.
     */
    void setFilterResponseSliders();

    /**
     * @brief Paints the background of the filter section.
     * @param g The graphics context used for rendering.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints a background shadow for the filter section, if active.
     * @param g The graphics context used for rendering.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Positions UI elements in a top-bottom orientation.
     *
     * Used when the filter section is specifying input routing and thus may need a different layout.
     */
    void positionTopBottom();

    /**
     * @brief Positions UI elements in a left-right orientation.
     *
     * Used when the filter section does not specify input routing.
     */
    void positionLeftRight();

    /**
     * @brief Called when the component is resized.
     *
     * This method updates the layout of all child components (sliders, buttons, etc.) based on the current size.
     */
    void resized() override;

    /**
     * @brief Called when a button is clicked.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Sets all parameter values for this filter section.
     * @param controls A map of control values used to update the filter parameters.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Called when the previous button on the preset selector is clicked.
     *
     * This cycles to the previous filter model/style combination.
     */
    void prevClicked() override;

    /**
     * @brief Called when the next button on the preset selector is clicked.
     *
     * This cycles to the next filter model/style combination.
     */
    void nextClicked() override;

    /**
     * @brief Called when the user interacts with the text component of the preset selector.
     * @param e The mouse event information.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Returns a path that represents the left morph shape icon based on the current model/style.
     * @return A Path object representing the left morph shape.
     */
    Path getLeftMorphPath();

    /**
     * @brief Returns a path that represents the right morph shape icon based on the current model/style.
     * @return A Path object representing the right morph shape.
     */
    Path getRightMorphPath();

    /**
     * @brief Sets the active state of this filter section.
     * @param active True if the section should be active; false otherwise.
     */
    void setActive(bool active) override;

    /**
     * @brief Adds a listener to be notified of filter section events.
     * @param listener A pointer to the listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the filter model and style based on a given menu ID.
     * @param menu_id The ID selected from the popup menu for filters/styles.
     */
    void setFilterSelected(int menu_id);

    /**
     * @brief Clears the internal filter input toggle state.
     */
    void clearFilterInput() { filter_input_->setToggleState(false, sendNotification); }

    /**
     * @brief Sets the oscillator input state for a given oscillator index.
     * @param oscillator_index The index of the oscillator (0, 1, or 2).
     * @param input True to enable the oscillator input, false to disable.
     */
    void setOscillatorInput(int oscillator_index, bool input);

    /**
     * @brief Sets the sample input state.
     * @param input True to enable the sample input, false to disable.
     */
    void setSampleInput(bool input);

private:
    /**
     * @brief Constructs a FilterSection with a specified name and suffix.
     *
     * This private constructor is used internally as a common initialization method.
     * @param name The component name.
     * @param suffix A suffix string used to identify parameters.
     */
    FilterSection(String name, String suffix);

    /**
     * @brief Shows or hides knobs based on the current filter model and style.
     */
    void showModelKnobs();

    /**
     * @brief Sets the text displayed by the preset selector to match the current model and style.
     */
    void setFilterText();

    /**
     * @brief Sets the label texts for certain parameters based on the current filter model.
     */
    void setLabelText();

    /**
     * @brief Notifies the parent and the FilterResponse of changes to the current model/style.
     */
    void notifyFilterChange();

    /**
     * @brief A list of listeners registered to receive events from this FilterSection.
     */
    std::vector<Listener*> listeners_;

    std::string model_name_;
    std::string style_name_;
    int current_model_;
    int current_style_;
    bool specify_input_;

    std::unique_ptr<SynthButton> filter_on_;
    std::unique_ptr<PresetSelector> preset_selector_;
    std::unique_ptr<FilterResponse> filter_response_;
    std::unique_ptr<SynthSlider> mix_;
    std::unique_ptr<SynthSlider> cutoff_;
    std::unique_ptr<SynthSlider> resonance_;
    std::unique_ptr<SynthSlider> blend_;
    std::unique_ptr<SynthSlider> keytrack_;
    std::unique_ptr<SynthSlider> drive_;

    std::unique_ptr<SynthSlider> formant_x_;
    std::unique_ptr<SynthSlider> formant_y_;
    std::unique_ptr<SynthSlider> formant_transpose_;
    std::unique_ptr<SynthSlider> formant_resonance_;
    std::unique_ptr<SynthSlider> formant_spread_;

    std::unique_ptr<OpenGlToggleButton> osc1_input_;
    std::unique_ptr<OpenGlToggleButton> osc2_input_;
    std::unique_ptr<OpenGlToggleButton> osc3_input_;
    std::unique_ptr<OpenGlToggleButton> sample_input_;
    std::unique_ptr<SynthButton> filter_input_;

    std::unique_ptr<PlainTextComponent> filter_label_1_;
    std::unique_ptr<PlainTextComponent> filter_label_2_;

    std::unique_ptr<SynthSlider> blend_transpose_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterSection)
};
