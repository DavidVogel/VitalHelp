#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "preset_selector.h"

class RandomViewer;
class SynthSlider;
class TempoSelector;
class TextSelector;

/**
 * @class RandomSection
 * @brief A section of the UI dedicated to controlling a random mod source, such as a random LFO.
 *
 * This section allows the user to configure frequency, tempo sync, stereo behavior, style of randomness,
 * and key tracking parameters. It includes a visualization of the random waveform.
 */
class RandomSection : public SynthSection {
public:
    /**
     * @brief Constructs a RandomSection.
     * @param name The name of this section.
     * @param value_prepend A string to prepend to parameter names (for parameter routing).
     * @param mono_modulations Mono modulation outputs map.
     * @param poly_modulations Poly modulation outputs map.
     */
    RandomSection(String name, std::string value_prepend,
                  const vital::output_map& mono_modulations,
                  const vital::output_map& poly_modulations);

    /** @brief Destructor. */
    ~RandomSection();

    /**
     * @brief Paints the background of the random section including labels and backgrounds.
     * @param g The Graphics context for drawing.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component is resized. Arranges the layout of sliders, buttons, and viewer.
     */
    void resized() override;

    /**
     * @brief Sets all control values from a given control map.
     * @param controls The control map with parameter values.
     */
    void setAllValues(vital::control_map& controls) override {
        SynthSection::setAllValues(controls);
        transpose_tune_divider_->setVisible(sync_->isKeytrack());
    }

    /**
     * @brief Called when a slider's value changes.
     * @param changed_slider The slider that triggered the event.
     */
    void sliderValueChanged(Slider* changed_slider) override {
        SynthSection::sliderValueChanged(changed_slider);
        transpose_tune_divider_->setVisible(sync_->isKeytrack());
    }

private:
    std::unique_ptr<RandomViewer> viewer_;           ///< The OpenGL viewer displaying the random waveform.

    std::unique_ptr<SynthSlider> frequency_;         ///< Frequency slider for free-running mode.
    std::unique_ptr<SynthSlider> tempo_;            ///< Tempo slider for sync mode.
    std::unique_ptr<SynthButton> stereo_;           ///< Stereo toggle button.
    std::unique_ptr<TempoSelector> sync_;           ///< Tempo sync selector (switches between frequency and tempo).
    std::unique_ptr<SynthButton> sync_type_;        ///< Sync type toggle button.
    std::unique_ptr<TextSelector> style_;           ///< Style selector for random mode.

    std::unique_ptr<SynthSlider> keytrack_transpose_; ///< Key tracking transpose slider.
    std::unique_ptr<SynthSlider> keytrack_tune_;      ///< Key tracking fine tune slider.

    std::unique_ptr<OpenGlQuad> transpose_tune_divider_; ///< Visual divider between transpose and tune parameters.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RandomSection)
};
