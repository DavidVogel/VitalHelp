#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

/**
 * @class VoiceSection
 * @brief A UI section for controlling various global voice parameters.
 *
 * The VoiceSection provides controls such as polyphony, velocity tracking,
 * pitch bend range, and stereo routing modes. It displays and updates these
 * parameters and their associated labels.
 */
class VoiceSection : public SynthSection {
public:
    /**
     * @brief Constructs a VoiceSection with a given name.
     * @param name The section name (for display and identification).
     */
    VoiceSection(String name);

    /** @brief Destructor. */
    virtual ~VoiceSection();

    /**
     * @brief Paints the background of the voice section, including labels and shadows.
     * @param g The Graphics context to draw into.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints a background shadow if the section is active.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }

    /**
     * @brief Lays out child components and adjusts sizing after a resize event.
     */
    void resized() override;

    /**
     * @brief Handles button click events for this section.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Updates all parameter values from a given control map.
     * @param controls A map of parameter names and values.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Sets the selected stereo mode.
     * @param selection The index of the selected stereo mode.
     */
    void setStereoModeSelected(int selection);

private:
    std::unique_ptr<SynthSlider> polyphony_;        ///< Slider for polyphony setting.
    std::unique_ptr<SynthSlider> velocity_track_;   ///< Slider for velocity tracking amount.
    std::unique_ptr<SynthSlider> pitch_bend_range_; ///< Slider for pitch bend range.
    std::unique_ptr<SynthSlider> stereo_routing_;   ///< Slider for stereo routing.

    std::unique_ptr<PlainTextComponent> stereo_mode_text_; ///< Text label for current stereo mode.
    std::unique_ptr<ShapeButton> stereo_mode_type_selector_; ///< Button for choosing stereo mode.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceSection)
};
