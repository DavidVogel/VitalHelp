/*
Summary:

WavetablePlayheadInfo is a component that displays the current integer position of a wavetable playhead as text. It implements the WavetablePlayhead::Listener interface, updating the displayed value whenever the playhead moves. The display is styled according to the current UI skin.
 */

#pragma once

#include "JuceHeader.h"
#include "wavetable_playhead.h"

/**
 * @class WavetablePlayheadInfo
 * @brief A UI component that displays the current playhead position in a wavetable editor.
 *
 * WavetablePlayheadInfo listens to a WavetablePlayhead and shows the current frame
 * position as a textual number. Whenever the playhead moves, this component updates
 * and redraws the display.
 */
class WavetablePlayheadInfo : public Component, public WavetablePlayhead::Listener {
public:
    /**
     * @brief Constructs the WavetablePlayheadInfo component.
     *
     * Initializes the displayed playhead position to zero.
     */
    WavetablePlayheadInfo() : playhead_position_(0) { }

    /**
     * @brief Paints the current playhead position text onto the component.
     * @param g The graphics context used for painting.
     */
    void paint(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * Triggers a repaint to ensure that the displayed text is positioned correctly
     * within the new bounds.
     */
    void resized() override;

    /**
     * @brief Called when the associated playhead moves to a new position.
     * @param position The new playhead position.
     *
     * Updates the displayed position and repaints the component.
     */
    void playheadMoved(int position) override;

protected:
    int playhead_position_; ///< The current playhead position being displayed.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetablePlayheadInfo)
};
