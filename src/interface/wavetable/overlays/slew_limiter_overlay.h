#pragma once

#include "JuceHeader.h"
#include "wavetable_component_overlay.h"
#include "slew_limit_modifier.h"

/**
 * @class SlewLimiterOverlay
 * @brief An overlay for editing SlewLimitModifier parameters in the wavetable editor.
 *
 * This overlay allows the user to adjust the slew rate limits for upward and downward changes in the waveform.
 * It provides two knobs: one for the upward slew limit and one for the downward slew limit.
 */
class SlewLimiterOverlay : public WavetableComponentOverlay {
public:
    /**
     * @brief Constructor.
     *
     * Initializes the slew limiter overlay with controls for adjusting up and down slew limits.
     */
    SlewLimiterOverlay();

    /**
     * @brief Called when a new frame is selected in the wavetable editor.
     * @param keyframe The selected WavetableKeyframe or nullptr if none.
     *
     * If the frame belongs to the SlewLimitModifier, updates the controls to reflect the selected frame's parameters.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a frame is dragged. Not used in this overlay.
     * @param keyframe The dragged keyframe.
     * @param position The new position of the frame (unused).
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the layout of the editing controls.
     * @param bounds The bounding rectangle.
     *
     * Positions the up and down slew limit controls horizontally within the given area.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Called when a slider value changes.
     * @param moved_slider The slider that changed.
     *
     * Updates the current frame's slew limits based on the slider value.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a slider drag operation ends.
     * @param moved_slider The slider that was dragged.
     *
     * Notifies that a final change to slew limits occurred.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Sets the SlewLimitModifier that this overlay will control.
     * @param slew_modifier The SlewLimitModifier to control.
     *
     * Resets the current frame and updates when a new frame is selected.
     */
    void setSlewLimitModifier(SlewLimitModifier* slew_modifier) {
        slew_modifier_ = slew_modifier;
        current_frame_ = nullptr;
    }

protected:
    SlewLimitModifier* slew_modifier_;                               ///< The assigned SlewLimitModifier.
    SlewLimitModifier::SlewLimitModifierKeyframe* current_frame_;    ///< Currently selected frame.

    std::unique_ptr<SynthSlider> up_slew_limit_;                     ///< Slider for upward slew limit.
    std::unique_ptr<SynthSlider> down_slew_limit_;                   ///< Slider for downward slew limit.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlewLimiterOverlay)
};
