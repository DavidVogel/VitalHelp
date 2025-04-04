/// @file chorus_section.h
/// @brief Declares the ChorusSection class, which provides a UI for configuring chorus effects.

#pragma once

#include "JuceHeader.h"
#include "delay_section.h"
#include "synth_section.h"

class ChorusViewer;
class SynthButton;
class TempoSelector;

/// @class ChorusSection
/// @brief A UI section for editing chorus effect parameters.
///
/// The ChorusSection provides controls for chorus parameters, including frequency, sync mode,
/// number of voices, feedback, modulation depth, delays, dry/wet mix, and filtering options.
/// It includes a graphical viewer to display chorus delay lines and a filter viewer for
/// customizing the chorus effect's frequency response.
class ChorusSection : public SynthSection, public DelayFilterViewer::Listener {
public:
    /// Constructor.
    /// @param name The name of this section.
    /// @param mono_modulations A map of mono modulation outputs from the synth.
    ChorusSection(const String& name, const vital::output_map& mono_modulations);

    /// Destructor.
    ~ChorusSection();

    /// Paints the background of the chorus section, including labels and separators.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /// Resizes and lays out child components and controls.
    void resized() override;

    /// Sets whether this section is active.
    /// @param active True if active, false otherwise.
    void setActive(bool active) override;

    /// Called when the user drags on the delay filter viewer.
    /// Adjusts filter cutoff and spread based on mouse movement.
    /// @param x The horizontal delta.
    /// @param y The vertical delta.
    void deltaMovement(float x, float y) override;

private:
    std::unique_ptr<SynthButton> on_;               ///< On/off button for the chorus effect.
    std::unique_ptr<SynthSlider> frequency_;        ///< Chorus modulation frequency slider.
    std::unique_ptr<SynthSlider> tempo_;            ///< Chorus tempo slider (when synced).
    std::unique_ptr<TempoSelector> sync_;           ///< Tempo sync mode selector.
    std::unique_ptr<SynthSlider> voices_;           ///< Number of chorus voices slider.
    std::unique_ptr<ChorusViewer> chorus_viewer_;   ///< Visualizes chorus delay lines.
    std::unique_ptr<DelayFilterViewer> filter_viewer_; ///< Viewer for the chorus filter response.

    std::unique_ptr<SynthSlider> feedback_;         ///< Chorus feedback slider.
    std::unique_ptr<SynthSlider> mod_depth_;        ///< Chorus modulation depth slider.
    std::unique_ptr<SynthSlider> delay_1_;          ///< First delay time slider.
    std::unique_ptr<SynthSlider> delay_2_;          ///< Second delay time slider.
    std::unique_ptr<SynthSlider> dry_wet_;          ///< Dry/wet mix slider.
    std::unique_ptr<SynthSlider> filter_cutoff_;    ///< Chorus filter cutoff slider.
    std::unique_ptr<SynthSlider> filter_spread_;    ///< Chorus filter spread slider.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChorusSection)
};
