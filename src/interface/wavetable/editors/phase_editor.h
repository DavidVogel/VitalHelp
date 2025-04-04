#pragma once

#include "JuceHeader.h"
#include "open_gl_multi_quad.h"
#include "common.h"

/**
 * @class PhaseEditor
 * @brief A UI component for interactively editing a phase value using a horizontal dragging gesture.
 *
 * The PhaseEditor displays a series of vertical lines (tick marks) that represent phase divisions.
 * Users can click and drag horizontally to adjust the phase, and listeners can be notified of
 * changes. Each tick line's position reflects the current phase offset.
 */
class PhaseEditor : public OpenGlMultiQuad {
public:
    /** The number of vertical lines to draw representing divisions of phase. */
    static constexpr int kNumLines = 16;
    /** The default relative height for each tick line. */
    static constexpr float kDefaultHeightPercent = 0.2f;

    /**
     * @class Listener
     * @brief Interface for receiving notifications when the phase value changes.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the phase value has been changed by user interaction.
         *
         * @param phase    The updated phase value (radians).
         * @param mouse_up True if this change was due to a mouse release, false if still dragging.
         */
        virtual void phaseChanged(float phase, bool mouse_up) = 0;
    };

    /**
     * @brief Constructs a PhaseEditor.
     */
    PhaseEditor();

    /**
     * @brief Destructor.
     */
    virtual ~PhaseEditor();

    /**
     * @brief Renders the tick lines representing the current phase.
     *
     * @param open_gl  The OpenGL wrapper with context and shaders.
     * @param animate  True if animations are enabled, false otherwise.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
        OpenGlMultiQuad::render(open_gl, animate);
    }

    /**
     * @brief Called when the mouse is pressed down on the component.
     *
     * Records the initial mouse position for phase calculations.
     *
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Called when the mouse is released after interaction.
     *
     * Updates the phase and notifies listeners with mouse_up = true.
     *
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Called while the mouse is dragged horizontally.
     *
     * Continually updates the phase and notifies listeners with mouse_up = false.
     *
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Adds a listener to be notified when the phase changes.
     *
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the current phase value and updates the display.
     *
     * @param phase The new phase value in radians.
     */
    void setPhase(float phase);

    /**
     * @brief Sets the maximum tick line height relative to the component height.
     *
     * @param height The new max tick height factor.
     */
    void setMaxTickHeight(float height) { max_tick_height_ = height; }

private:
    /**
     * @brief Updates the phase based on current mouse position.
     *
     * Calculates the horizontal drag distance to modify the phase and notifies listeners.
     *
     * @param e The mouse event.
     */
    void updatePhase(const MouseEvent& e);

    /**
     * @brief Updates the positions of the tick lines based on the current phase.
     *
     * Each line is repositioned to visually represent the new phase offset.
     */
    void updatePositions();

    std::vector<Listener*> listeners_;  ///< Listeners for phase changes.
    Point<int> last_edit_position_;     ///< Last mouse position during editing.

    float phase_;                       ///< Current phase value in radians.
    float max_tick_height_;             ///< Max tick line height proportion.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseEditor)
};
