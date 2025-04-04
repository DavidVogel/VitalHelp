/// @file equalizer_section.h
/// @brief Declares the EqualizerSection class, providing a UI for a 3-band equalizer with adjustable modes and frequency responses.

#pragma once

#include "JuceHeader.h"
#include "equalizer_response.h"
#include "synth_section.h"

class SynthSlider;
class Spectrogram;
class TabSelector;

/// @class EqualizerSection
/// @brief A UI section for configuring a 3-band equalizer with selectable modes (Low, Band, High).
///
/// The EqualizerSection provides controls for three bands of EQ. Each band can be set to a particular mode (low-pass,
/// notch, high-pass for low/band/high) and has adjustable cutoff, resonance, and gain. It includes a Spectrogram and
/// EqualizerResponse for visual feedback, and a TabSelector for choosing which band is visible.
class EqualizerSection : public SynthSection, public EqualizerResponse::Listener {
public:
    /// Constructor.
    /// @param name The name of this equalizer section.
    /// @param mono_modulations A map of mono modulation outputs.
    EqualizerSection(String name, const vital::output_map& mono_modulations);
    /// Destructor.
    virtual ~EqualizerSection();

    /// Paints the background and labels for the EQ parameters.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow if the EQ is active.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /// Resizes and lays out child components (filter sliders, spectrogram, etc.).
    void resized() override;

    /// Sets whether the section is active. Deactivates certain controls if not active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) override;

    /// Called when the low band is selected in the EQ response.
    void lowBandSelected() override;

    /// Called when the mid band is selected in the EQ response.
    void midBandSelected() override;

    /// Called when the high band is selected in the EQ response.
    void highBandSelected() override;

    /// Called when a slider value changes, e.g., the selected band.
    /// @param slider The slider that changed.
    void sliderValueChanged(Slider* slider) override;

    /// Called when a button is clicked (e.g., band mode buttons).
    /// @param button The button that was clicked.
    void buttonClicked(Button* button) override;

    /// Enables or disables scroll-wheel input on internal components.
    /// @param enabled True if scroll wheel should be enabled.
    void setScrollWheelEnabled(bool enabled) override;

    /// Sets the gain sliders active or inactive based on the current mode and active state.
    void setGainActive();

    /// Called when the parent hierarchy changes, used to set up spectrogram memory.
    void parentHierarchyChanged() override;

    /// Renders OpenGL components (like the spectrogram) each frame.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate True if should animate transitions.
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

private:
    SynthGuiInterface* parent_; ///< Parent interface for accessing memory and settings.

    std::unique_ptr<SynthButton> on_; ///< On/off button for the EQ.

    std::unique_ptr<OpenGlShapeButton> low_mode_;   ///< Button for low band mode (low-pass).
    std::unique_ptr<OpenGlShapeButton> band_mode_;  ///< Button for band mode (notch).
    std::unique_ptr<OpenGlShapeButton> high_mode_;  ///< Button for high band mode (high-pass).

    std::unique_ptr<EqualizerResponse> eq_response_; ///< Visual EQ response.
    std::unique_ptr<Spectrogram> spectrogram_;       ///< Spectrogram for frequency content visualization.

    std::unique_ptr<SynthSlider> low_cutoff_;     ///< Low band cutoff slider.
    std::unique_ptr<SynthSlider> low_resonance_;  ///< Low band resonance slider.
    std::unique_ptr<SynthSlider> low_gain_;       ///< Low band gain slider.

    std::unique_ptr<SynthSlider> band_cutoff_;    ///< Mid band cutoff slider.
    std::unique_ptr<SynthSlider> band_resonance_; ///< Mid band resonance slider.
    std::unique_ptr<SynthSlider> band_gain_;      ///< Mid band gain slider.

    std::unique_ptr<SynthSlider> high_cutoff_;    ///< High band cutoff slider.
    std::unique_ptr<SynthSlider> high_resonance_; ///< High band resonance slider.
    std::unique_ptr<SynthSlider> high_gain_;      ///< High band gain slider.

    std::unique_ptr<TabSelector> selected_band_;   ///< Selector for choosing which band is visible/editable.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqualizerSection)
};
