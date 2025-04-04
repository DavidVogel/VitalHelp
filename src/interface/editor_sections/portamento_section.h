#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

class SynthButton;
class SynthSlider;
class TextSlider;

/**
 * @class PortamentoSection
 * @brief A UI section for controlling portamento (glide) settings in a synthesizer.
 *
 * This section provides controls for portamento time, slope, octave scaling, forced glide,
 * and legato mode. It inherits from SynthSection and handles painting, resizing, and
 * updating associated sliders and buttons.
 */
class PortamentoSection : public SynthSection {
public:
    /**
     * @brief Constructs a new PortamentoSection with a given name.
     * @param name The name of this section.
     */
    PortamentoSection(String name);

    /** @brief Destructor. */
    virtual ~PortamentoSection();

    /**
     * @brief Paints the background of the portamento section, including labels and backgrounds.
     * @param g The Graphics context to draw into.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the background shadow for the portamento section.
     * @param g The Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }

    /**
     * @brief Called when the component is resized. Lays out sliders and buttons.
     */
    void resized() override;

    /**
     * @brief Called when a slider value changes to update dependent controls.
     * @param changed_slider Pointer to the slider that changed.
     */
    void sliderValueChanged(Slider* changed_slider) override;

    /**
     * @brief Sets all parameter values from a given control map.
     * @param controls The control map containing parameter values.
     */
    void setAllValues(vital::control_map& controls) override;

private:
    std::unique_ptr<SynthSlider> portamento_;       ///< Slider for portamento (glide) time.
    std::unique_ptr<SynthSlider> portamento_slope_; ///< Slider for controlling the portamento slope.
    std::unique_ptr<SynthButton> portamento_scale_; ///< Button to toggle octave scaling of portamento.
    std::unique_ptr<SynthButton> portamento_force_; ///< Button to force glide always.
    std::unique_ptr<SynthButton> legato_;           ///< Button to enable legato playing mode.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PortamentoSection)
};
