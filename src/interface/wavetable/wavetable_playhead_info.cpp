/*
Summary:

WavetablePlayheadInfo is a component that displays the current integer position of a wavetable playhead as text. It implements the WavetablePlayhead::Listener interface, updating the displayed value whenever the playhead moves. The display is styled according to the current UI skin.
 */
#include "wavetable_playhead_info.h"
#include "skin.h"

void WavetablePlayheadInfo::paint(Graphics& g) {
    // Fill background with the main body colour.
    g.fillAll(findColour(Skin::kBody, true));

    // Convert the current position to a string for display.
    String position_text(playhead_position_);

    // Draw the text in a colour that contrasts the background (e.g. body text).
    g.setColour(findColour(Skin::kBodyText, true));
    Rectangle<int> bounds = getLocalBounds();
    // Adjust bounds to leave some space on the right side.
    bounds.setWidth(bounds.getWidth() - bounds.getHeight() * 0.5f);

    // Draw the current position text, right-aligned.
    g.drawText(position_text, bounds, Justification::centredRight);
}

void WavetablePlayheadInfo::resized() {
    // Ensures that if the size changes, we update the display region.
    repaint();
}

void WavetablePlayheadInfo::playheadMoved(int position) {
    // Update the stored position and repaint to show the new value.
    playhead_position_ = position;
    repaint();
}
