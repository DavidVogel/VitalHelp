#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "preset_selector.h"
#include "sample_viewer.h"
#include "transpose_quantize.h"

class SynthSlider;
class OpenGlShapeButton;

/**
 * @class SampleSection
 * @brief A UI section for managing and editing a sample source.
 *
 * The SampleSection allows loading, browsing, and editing sample-based audio sources.
 * Users can adjust pitch, tuning, panning, level, looping, key tracking, and random phase.
 * It integrates with a SampleViewer for waveform display and a PresetSelector for browsing samples.
 * It also supports setting the routing destination for the sample output.
 */
class SampleSection : public SynthSection,
                      public SampleViewer::Listener,
                      public PresetSelector::Listener,
                      public TransposeQuantizeButton::Listener {
public:
    /**
     * @class Listener
     * @brief Interface for receiving events when the sample destination changes.
     */
    class Listener {
    public:
        virtual ~Listener() {}
        /**
         * @brief Called when the sample output routing destination changes.
         * @param sample Pointer to this SampleSection.
         * @param destination The new destination index.
         */
        virtual void sampleDestinationChanged(SampleSection* sample, int destination) = 0;
    };

    /**
     * @brief Constructs a SampleSection with a given name.
     * @param name The name of this section.
     */
    SampleSection(String name);

    /** @brief Destructor. */
    ~SampleSection();

    /**
     * @brief Called when the parent hierarchy changes.
     *        Used to acquire a pointer to the synth and associated sample object.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Paints the background of the sample section, including labels and visual layouts.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the background shadow if the section is active.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Activates or deactivates the section.
     * @param active True to activate, false to deactivate.
     */
    void setActive(bool active) override;

    /**
     * @brief Called when the component is resized to lay out children and controls.
     */
    void resized() override;

    /**
     * @brief Resets the sample section, updating display and reloading the sample if needed.
     */
    void reset() override;

    /**
     * @brief Sets all control values from a given control map.
     * @param controls The control map containing values for parameters.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Handles button click events for this section.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Sets the sample output routing destination and updates the UI.
     * @param selection The destination index to set.
     */
    void setDestinationSelected(int selection);

    /**
     * @brief Configures the destination text and notifies listeners of the new destination.
     */
    void setupDestination();

    /**
     * @brief Toggles the filter input routing on or off for a particular filter index.
     * @param filter_index The index of the filter (0 or 1).
     * @param on True to enable filter input, false to disable.
     */
    void toggleFilterInput(int filter_index, bool on);

    /**
     * @brief Loads a sample file and updates the sample viewer and presets.
     * @param file The file to load.
     */
    void loadFile(const File& file) override;

    /**
     * @brief Callback for when a sample is loaded externally through the SampleViewer.
     * @param file The file that was loaded.
     */
    void sampleLoaded(const File& file) override { loadFile(file); }

    /**
     * @brief Gets the currently loaded sample file.
     * @return The currently loaded sample file.
     */
    File getCurrentFile() override { return File(sample_->getLastBrowsedFile()); }

    /**
     * @brief Called when the "previous" button is clicked to cycle through samples.
     */
    void prevClicked() override;

    /**
     * @brief Called when the "next" button is clicked to cycle through samples.
     */
    void nextClicked() override;

    /**
     * @brief Called when the user clicks on the text area (e.g., preset name) to browse samples.
     * @param e The mouse event.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Called when quantization is updated from the TransposeQuantizeButton.
     */
    void quantizeUpdated() override;

    /**
     * @brief Adds a listener for destination change events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

private:
    std::vector<Listener*> listeners_;

    std::unique_ptr<TransposeQuantizeButton> transpose_quantize_button_;
    std::unique_ptr<SynthSlider> transpose_;
    std::unique_ptr<SynthSlider> tune_;
    std::unique_ptr<SynthSlider> pan_;
    std::unique_ptr<SynthSlider> level_;
    std::unique_ptr<SampleViewer> sample_viewer_;
    std::unique_ptr<PresetSelector> preset_selector_;

    int current_destination_;
    std::string destination_control_name_;
    std::unique_ptr<PlainTextComponent> destination_text_;
    std::unique_ptr<ShapeButton> destination_selector_;
    std::unique_ptr<OpenGlShapeButton> prev_destination_;
    std::unique_ptr<OpenGlShapeButton> next_destination_;

    std::unique_ptr<SynthButton> on_;
    std::unique_ptr<OpenGlShapeButton> loop_;
    std::unique_ptr<OpenGlShapeButton> bounce_;
    std::unique_ptr<OpenGlShapeButton> keytrack_;
    std::unique_ptr<OpenGlShapeButton> random_phase_;

    AudioSampleBuffer sample_buffer_;
    vital::Sample* sample_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSection)
};
