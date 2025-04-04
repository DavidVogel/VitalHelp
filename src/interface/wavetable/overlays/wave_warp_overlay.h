#pragma once

#include "JuceHeader.h"
#include "wavetable_component_overlay.h"
#include "wave_warp_modifier.h"

/**
 * @class WaveWarpOverlay
 * @brief An overlay interface component for modifying wave warp parameters in a wavetable editor.
 *
 * WaveWarpOverlay provides UI controls for editing the warp properties of a WaveWarpModifier.
 * Users can adjust horizontal and vertical warp values and toggle asymmetric warping.
 * This overlay is displayed when a keyframe associated with a WaveWarpModifier is selected.
 */
class WaveWarpOverlay : public WavetableComponentOverlay {
public:
    /**
     * @brief Constructs a WaveWarpOverlay with default parameters and UI elements.
     *
     * Initializes sliders and toggle buttons to control warp parameters.
     */
    WaveWarpOverlay();

    /**
     * @brief Called when a new keyframe is selected.
     *
     * If the keyframe is part of the associated WaveWarpModifier, updates UI elements
     * to reflect the current keyframe's warp settings.
     *
     * @param keyframe The newly selected keyframe, or nullptr if none.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a keyframe is dragged, not implemented in this overlay.
     *
     * @param keyframe The dragged keyframe.
     * @param position The new position of the keyframe.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the bounds for the overlay's editable UI area.
     *
     * Positions the controls (sliders and toggle buttons) within the specified region.
     *
     * @param bounds The bounding rectangle for editing controls.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Called when a slider value changes.
     *
     * Updates the corresponding parameter in the current keyframe of the WaveWarpModifier.
     *
     * @param moved_slider The slider that changed.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when a slider drag ends.
     *
     * Notifies that changes should be considered final.
     *
     * @param moved_slider The slider that ended its drag operation.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Called when a button (toggle) state changes.
     *
     * Updates the WaveWarpModifier's asymmetric flags for horizontal or vertical warping.
     *
     * @param clicked_Button The button that was clicked.
     */
    void buttonClicked(Button* clicked_Button) override;

    /**
     * @brief Sets the WaveWarpModifier this overlay controls.
     *
     * Clears any currently selected frame and readies the overlay to modify the given warp_modifier.
     *
     * @param warp_modifier The WaveWarpModifier to control.
     */
    void setWaveWarpModifier(WaveWarpModifier* warp_modifier) {
        warp_modifier_ = warp_modifier;
        current_frame_ = nullptr;
    }

protected:
    WaveWarpModifier* warp_modifier_; ///< The WaveWarpModifier being controlled by this overlay.
    WaveWarpModifier::WaveWarpModifierKeyframe* current_frame_; ///< Currently selected keyframe's data.

    std::unique_ptr<SynthSlider> horizontal_warp_; ///< Slider controlling horizontal warp amount.
    std::unique_ptr<SynthSlider> vertical_warp_;   ///< Slider controlling vertical warp amount.
    std::unique_ptr<OpenGlToggleButton> horizontal_asymmetric_; ///< Toggle for horizontal asymmetry.
    std::unique_ptr<OpenGlToggleButton> vertical_asymmetric_;   ///< Toggle for vertical asymmetry.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWarpOverlay)
};
