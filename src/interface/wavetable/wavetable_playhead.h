/*
Summary:

WavetablePlayhead represents a movable playhead line indicating a current position within a set of frames. Users can interact with it using mouse events to set a specific frame. The class supports customizable tick marks at regular intervals and notifies listeners whenever the playhead is moved.
 */
#pragma once

#include "JuceHeader.h"

/**
 * @class WavetablePlayhead
 * @brief A UI component representing a playhead position over a range of frames in a wavetable editor.
 *
 * WavetablePlayhead displays and controls a playhead line that can be moved horizontally
 * to select a particular position among a given number of frames. This allows the user to
 * quickly navigate through frames in the wavetable. Listeners can be notified when the
 * playhead moves to a new position.
 */
class WavetablePlayhead : public SynthSection {
public:
    /// Used to determine which ticks are larger (every kBigLineSkip) and which are smaller (every kLineSkip).
    static constexpr int kBigLineSkip = 16;
    static constexpr int kLineSkip = 4;

    /**
     * @class Listener
     * @brief A listener interface for objects interested in playhead position changes.
     *
     * Implementing classes can register to receive a callback when the playhead position changes.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the playhead is moved to a new position.
         * @param new_position The new position of the playhead.
         */
        virtual void playheadMoved(int new_position) = 0;
    };

    /**
     * @brief Constructs a WavetablePlayhead.
     * @param num_positions The total number of positions (frames) that the playhead can navigate through.
     */
    WavetablePlayhead(int num_positions);

    /**
     * @brief Gets the current playhead position.
     * @return The current position of the playhead.
     */
    int position() { return position_; }

    /**
     * @brief Sets the playhead to a specific position.
     * @param position The new position to set.
     *
     * Clamps and updates the playhead position, and notifies listeners of the change.
     */
    void setPosition(int position);

    /**
     * @brief Updates the visual position of the playhead quad based on the current position and size.
     */
    void setPositionQuad();

    /**
     * @brief Handles mouse down events for interaction.
     * @param event The mouse event information.
     */
    void mouseDown(const MouseEvent& event) override;

    /**
     * @brief Handles mouse drag events, moving the playhead position accordingly.
     * @param event The mouse event information.
     */
    void mouseDrag(const MouseEvent& event) override;

    /**
     * @brief Internal method for handling mouse events to change playhead position.
     * @param event The mouse event information.
     */
    void mouseEvent(const MouseEvent& event);

    /**
     * @brief Paints the background ticks and line indicators for the playhead.
     * @param g The graphics context used for painting.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component is resized, updates the playhead position display.
     */
    void resized() override;

    /**
     * @brief Registers a listener interested in changes to the playhead position.
     * @param listener The listener to register.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the horizontal padding around the playhead display area.
     * @param padding The amount of padding in pixels.
     */
    void setPadding(float padding) { padding_ = padding; setPositionQuad(); }

protected:
    OpenGlQuad position_quad_; ///< The visual quad representing the playhead line.

    std::vector<Listener*> listeners_; ///< Registered listeners to notify on position changes.

    float padding_;       ///< Extra horizontal padding for the display area.
    int num_positions_;   ///< Total number of positions (frames) available.
    int position_;        ///< Current playhead position.
    int drag_start_x_;    ///< Starting x position of the mouse when dragging.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetablePlayhead)
};
