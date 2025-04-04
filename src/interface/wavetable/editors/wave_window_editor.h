#pragma once

#include "JuceHeader.h"
#include "open_gl_line_renderer.h"
#include "common.h"
#include "wave_window_modifier.h"

/**
 * @class WaveWindowEditor
 * @brief A UI component that allows editing of a windowing function applied to a waveform.
 *
 * The WaveWindowEditor displays a graphical window shape that can be adjusted by dragging handles
 * on the left and right sides. Users can interact with these handles (left and right) to shift
 * the window's range, and the resulting window shape is updated in real-time. Different window
 * shapes can be applied and listeners can be notified of changes.
 */
class WaveWindowEditor : public OpenGlLineRenderer {
public:
    /** The radius (in normalized units) around the handles to detect mouse hover or dragging. */
    static constexpr float kGrabRadius = 0.05f;
    /** The number of points per window section to ensure a smooth curve. */
    static constexpr int kPointsPerSection = 50;
    /** The total number of points plotted for the full window shape. */
    static constexpr int kTotalPoints = 4 * kPointsPerSection;

    /**
     * @enum ActiveSide
     * @brief Enum representing which side (left or right) is being edited or hovered.
     */
    enum ActiveSide {
        kNone,  /**< No side is active. */
        kLeft,  /**< The left handle is active. */
        kRight  /**< The right handle is active. */
    };

    /**
     * @class Listener
     * @brief Interface for receiving notifications about window position changes.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the window boundaries have changed.
         *
         * @param left     True if the left boundary changed, false if the right boundary changed.
         * @param mouse_up True if the mouse button was just released.
         */
        virtual void windowChanged(bool left, bool mouse_up) = 0;
    };

    /**
     * @brief Constructs a WaveWindowEditor instance.
     */
    WaveWindowEditor();

    /**
     * @brief Destructor.
     */
    virtual ~WaveWindowEditor();

    /**
     * @brief Paints the background. Currently does nothing as rendering is handled by OpenGL.
     * @param g The graphics context to draw into.
     */
    void paintBackground(Graphics& g) override { }

    /**
     * @brief Called when the component is resized. Updates the rendering and layout.
     */
    void resized() override;

    /**
     * @brief Initializes OpenGL components.
     * @param open_gl The OpenGL wrapper to use.
     */
    virtual void init(OpenGlWrapper& open_gl) override {
        OpenGlLineRenderer::init(open_gl);
        edit_bars_.init(open_gl);
    }

    /**
     * @brief Renders the window editor. Draws the line and the handle bars.
     * @param open_gl The OpenGL wrapper to use.
     * @param animate True if animation is enabled.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
        OpenGlLineRenderer::render(open_gl, animate);
        edit_bars_.render(open_gl, animate);
    }

    /**
     * @brief Destroys OpenGL resources.
     * @param open_gl The OpenGL wrapper to use.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override {
        OpenGlLineRenderer::destroy(open_gl);
        edit_bars_.destroy(open_gl);
    }

    /**
     * @brief Handles mouse down events, determining which side (if any) is selected.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events, finalizing changes if a handle was being moved.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse move events, updating hover states over handles.
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events, resetting hover states.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events, adjusting the window boundaries.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Adds a listener for window changes.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Determines which handle (left or right) is being hovered based on a position.
     * @param position The position to check.
     * @return The active side being hovered, or kNone if none is hovered.
     */
    ActiveSide getHover(Point<int> position);

    /**
     * @brief Gets the left boundary's normalized position.
     * @return The left boundary position in [0, 1].
     */
    float getLeftPosition() { return left_position_; }

    /**
     * @brief Gets the right boundary's normalized position.
     * @return The right boundary position in [0, 1].
     */
    float getRightPosition() { return right_position_; }

    /**
     * @brief Sets the left and right boundary positions for the window.
     * @param left  New left position in [0, 1].
     * @param right New right position in [0, 1].
     */
    void setPositions(float left, float right) {
        left_position_ = left;
        right_position_ = right;
        setPoints();
    }

    /**
     * @brief Sets the window shape type.
     * @param window_shape The new window shape.
     */
    void setWindowShape(WaveWindowModifier::WindowShape window_shape) {
        window_shape_ = window_shape;
        setPoints();
    }

private:
    /**
     * @brief Notifies listeners that the window has changed.
     * @param mouse_up True if triggered by a mouse release.
     */
    void notifyWindowChanged(bool mouse_up);

    /**
     * @brief Adjusts window values based on a mouse event.
     * @param e The mouse event.
     */
    void changeValues(const MouseEvent& e);

    /**
     * @brief Sets the editing quad positions for handles.
     */
    void setEditingQuads();

    /**
     * @brief Recomputes the points for the window line visualization.
     */
    void setPoints();

    std::vector<Listener*> listeners_;
    Point<int> last_edit_position_;

    OpenGlMultiQuad edit_bars_;

    WaveWindowModifier::WindowShape window_shape_;
    ActiveSide hovering_;
    ActiveSide editing_;
    float left_position_;
    float right_position_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveWindowEditor)
};
