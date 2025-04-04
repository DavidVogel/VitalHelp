/// @file compressor_section.h
/// @brief Declares the CompressorSection class, providing a UI for a multiband compressor.

#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "text_selector.h"

class CompressorEditor;
class SynthButton;
class SynthSlider;

/// @class CompressorSection
/// @brief A UI section for configuring a multiband compressor effect.
///
/// The CompressorSection provides sliders for adjusting compressor parameters such as attack,
/// release, mix, and gain for low, band, and high frequency bands. It also includes a mode selector
/// (single band, low band, high band, or multiband) and a visual editor to display and manipulate
/// the compressor's band settings.
class CompressorSection : public SynthSection {
public:
    /// Constructor.
    /// @param name The name of this section.
    CompressorSection(const String& name);

    /// Destructor.
    ~CompressorSection();

    /// Paints the background, including labels for the various controls.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /// Resizes and lays out the compressor's knobs and editor.
    void resized() override;

    /// Sets all control values from a given map of controls.
    /// @param controls The control map with updated values.
    void setAllValues(vital::control_map& controls) override;

    /// Sets whether this section is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) override;

    /// Called when a slider value changes, to handle changes in the compressor mode or parameters.
    /// @param changed_slider The slider that changed.
    void sliderValueChanged(Slider* changed_slider) override;

private:
    /// Updates the compressor editor and gain controls based on the selected compressor mode.
    void setCompressorActiveBands();

    std::unique_ptr<SynthButton> on_;                 ///< On/off button for the compressor.
    std::unique_ptr<SynthSlider> mix_;                ///< Dry/Wet mix slider.
    std::unique_ptr<SynthSlider> attack_;             ///< Attack time slider.
    std::unique_ptr<SynthSlider> release_;            ///< Release time slider.
    std::unique_ptr<SynthSlider> low_gain_;           ///< Low band gain slider.
    std::unique_ptr<SynthSlider> band_gain_;          ///< Middle band gain slider.
    std::unique_ptr<SynthSlider> high_gain_;          ///< High band gain slider.
    std::unique_ptr<TextSelector> enabled_bands_;     ///< Selector for compressor mode (which bands are active).
    std::unique_ptr<CompressorEditor> compressor_editor_; ///< Visual editor for the compressor bands.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorSection)
};
