#include "phase_editor.h"

#include "skin.h"
#include "synth_constants.h"
#include "shaders.h"
#include "utils.h"

PhaseEditor::PhaseEditor() : OpenGlMultiQuad(kNumLines, Shaders::kColorFragment) {
    phase_ = 0.0f;
    max_tick_height_ = kDefaultHeightPercent;
    setInterceptsMouseClicks(true, false);
}

PhaseEditor::~PhaseEditor() { }

void PhaseEditor::mouseDown(const MouseEvent& e) {
    last_edit_position_ = e.getPosition();
}

void PhaseEditor::mouseUp(const MouseEvent& e) {
    updatePhase(e);

    // Notify listeners that mouse is now up, finalizing the change.
    for (Listener* listener : listeners_)
        listener->phaseChanged(phase_, true);
}

void PhaseEditor::mouseDrag(const MouseEvent& e) {
    // Update phase on each mouse drag movement, listeners get notified with mouse_up = false.
    updatePhase(e);
}

void PhaseEditor::updatePhase(const MouseEvent& e) {
    int difference = e.getPosition().x - last_edit_position_.x;
    // Convert horizontal drag into a phase change based on component width.
    phase_ += (2.0f * vital::kPi * difference) / getWidth();
    last_edit_position_ = e.getPosition();

    for (Listener* listener : listeners_)
        listener->phaseChanged(phase_, false);

    updatePositions();
}

void PhaseEditor::updatePositions() {
    float width = 2.0f / getWidth();
    for (int i = 0; i < kNumLines; ++i) {
        // Compute the fractional position along the x-axis and wrap phase around [0,1].
        float fraction = phase_ / (2.0f * vital::kPi) + (1.0f * i) / kNumLines;
        fraction -= floorf(fraction);

        float height = max_tick_height_ * 2.0f;
        // Reduce height for certain divisions, making a structured pattern of lines.
        for (int div = 2; div < kNumLines; div *= 2) {
            if (i % div)
                height /= 2.0f;
        }

        setQuad(i, 2.0f * fraction - 1.0f, -1.0f, width, height);
    }
}

void PhaseEditor::setPhase(float phase) {
    phase_ = phase;
    updatePositions();
}
