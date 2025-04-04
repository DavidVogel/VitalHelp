#pragma once

#include "JuceHeader.h"
#include "equalizer_response.h"
#include "synth_section.h"

class SynthButton;

/**
 * @class ReverbSection
 * @brief A UI section for controlling and visualizing reverb parameters.
 *
 * This class provides controls for a reverb effect, including time, filters,
 * chorus, size, delay, and dry/wet mix. It also incorporates an EqualizerResponse
 * to visualize and control the feedback EQ of the reverb.
 */
class ReverbSection : public SynthSection, public EqualizerResponse::Listener {
public:
    /** @brief Used to scale the feedback filter EQ buffer. */
    static constexpr float kFeedbackFilterBuffer = 0.4f;

    /**
     * @brief Constructs a ReverbSection.
     * @param name The name of this section.
     * @param mono_modulations A map of mono modulation outputs for connecting to parameters.
     */
    ReverbSection(String name, const vital::output_map& mono_modulations);

    /** @brief Destructor. */
    ~ReverbSection();

    /**
     * @brief Paints the reverb section background, labels, and any custom UI elements.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints a background shadow if active.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Called when the component is resized. Arranges the sliders, buttons, and EQ visualization.
     */
    void resized() override;

    /**
     * @brief Sets the active state of this section.
     * @param active True to activate the section, false otherwise.
     */
    void setActive(bool active) override;

    /**
     * @brief Called when a slider's value changes.
     * @param slider The slider that changed value.
     */
    void sliderValueChanged(Slider* slider) override;

    /**
     * @brief Called when the low EQ band is selected in the EqualizerResponse.
     */
    void lowBandSelected() override;

    /**
     * @brief Called when the mid band is selected in the EqualizerResponse (unused in this section).
     */
    void midBandSelected() override { }

    /**
     * @brief Called when the high EQ band is selected in the EqualizerResponse.
     */
    void highBandSelected() override;

private:
    std::unique_ptr<SynthButton> on_;                  ///< Button to toggle the reverb effect on/off.
    std::unique_ptr<EqualizerResponse> feedback_eq_response_; ///< Visual and interactive EQ for reverb feedback.
    std::unique_ptr<TabSelector> selected_eq_band_;    ///< Tab selector for choosing low or high EQ band.
    std::unique_ptr<SynthSlider> decay_time_;          ///< Controls the decay time of the reverb.
    std::unique_ptr<SynthSlider> low_pre_cutoff_;      ///< Low cutoff filter before reverb.
    std::unique_ptr<SynthSlider> high_pre_cutoff_;     ///< High cutoff filter before reverb.
    std::unique_ptr<SynthSlider> low_cutoff_;          ///< Low shelf cutoff frequency in the feedback EQ.
    std::unique_ptr<SynthSlider> low_gain_;            ///< Low shelf gain in the feedback EQ.
    std::unique_ptr<SynthSlider> high_cutoff_;         ///< High shelf cutoff frequency in the feedback EQ.
    std::unique_ptr<SynthSlider> high_gain_;           ///< High shelf gain in the feedback EQ.
    std::unique_ptr<SynthSlider> chorus_amount_;       ///< Amount of chorus applied in the reverb.
    std::unique_ptr<SynthSlider> chorus_frequency_;    ///< Frequency rate of chorus modulation.
    std::unique_ptr<SynthSlider> size_;                ///< Controls the size of the reverb space.
    std::unique_ptr<SynthSlider> delay_;               ///< Pre-delay time before reverb onset.
    std::unique_ptr<SynthSlider> dry_wet_;             ///< Dry/Wet mix control for the reverb.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbSection)
};
