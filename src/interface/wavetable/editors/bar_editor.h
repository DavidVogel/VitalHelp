#pragma once

#include "JuceHeader.h"
#include "bar_renderer.h"
#include "common.h"
#include "open_gl_multi_quad.h"
#include "skin.h"
#include "utils.h"

/**
 * @class BarEditor
 * @brief An interactive component that allows editing individual bars in a bar graph visually.
 *
 * The BarEditor extends BarRenderer to handle mouse events for editing bar values. Users can click
 * and drag to modify bar values, right-click to access a popup menu with various editing options,
 * and perform actions such as clearing ranges or randomizing bar values.
 */
class BarEditor : public BarRenderer {
public:
    /**
     * @class Listener
     * @brief Interface for receiving notifications when bar values are changed.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when bar values have changed.
         *
         * @param start    The first changed bar index.
         * @param end      The last changed bar index.
         * @param mouse_up True if the mouse was released after editing, indicating the edit is complete.
         */
        virtual void barsChanged(int start, int end, bool mouse_up) = 0;
    };

    /**
     * @enum BarEditorMenu
     * @brief Popup menu actions for bar editing.
     */
    enum BarEditorMenu {
        kCancel = 0,
        kClear,
        kClearRight,
        kClearLeft,
        kClearEven,
        kClearOdd,
        kRandomize
    };

    /**
     * @brief Constructs a BarEditor for a given number of bars.
     * @param num_points Number of bars to be edited.
     */
    BarEditor(int num_points) : BarRenderer(num_points), editing_quad_(Shaders::kColorFragment),
                                random_generator_(-1.0f, 1.0f), editing_(false), clear_value_(-1.0f) {
        current_mouse_position_ = Point<int>(-10, -10);
    }

    /**
     * @brief Destructor.
     */
    virtual ~BarEditor() { }

    /**
     * @brief Initializes the OpenGL components used by this editor.
     * @param open_gl Reference to the OpenGL wrapper.
     */
    virtual void init(OpenGlWrapper& open_gl) override {
        BarRenderer::init(open_gl);
        editing_quad_.init(open_gl);
    }

    /**
     * @brief Renders the editor including the highlight of the currently hovered bar.
     * @param open_gl Reference to the OpenGL wrapper.
     * @param animate Whether to animate (not used here).
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override {
        BarRenderer::render(open_gl, animate);
        int hovered_index = getHoveredIndex(current_mouse_position_);
        if (current_mouse_position_.x < 0)
            hovered_index = -1;
        float bar_width = 2.0f * scale_ / num_points_;
        editing_quad_.setQuad(0, bar_width * hovered_index - 1.0f, -1.0f, bar_width, 2.0f);
        editing_quad_.render(open_gl, animate);
    }

    /**
     * @brief Destroys all OpenGL components.
     * @param open_gl Reference to the OpenGL wrapper.
     */
    virtual void destroy(OpenGlWrapper& open_gl) override {
        BarRenderer::destroy(open_gl);
        editing_quad_.destroy(open_gl);
    }

    /**
     * @brief Called when the editor is resized.
     */
    virtual void resized() override {
        BarRenderer::resized();
        editing_quad_.setColor(findColour(Skin::kLightenScreen, true));
    }

    /**
     * @brief Handles mouse move events to update hovered bar.
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override;

    /**
     * @brief Handles mouse down events. Initiates editing or shows popup menu.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events. Completes editing operation.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events. Updates bar values while dragging.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events. Clears hover state.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Adds a listener to receive updates when bars change.
     * @param listener Pointer to a listener.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the value used when clearing bars.
     * @param value The value to set cleared bars to.
     */
    void setClearValue(float value) { clear_value_ = value; }

    /**
     * @brief Randomizes all bars using a uniform distribution.
     */
    void randomize();

    /**
     * @brief Clears all bars to the clear value.
     */
    void clear();

    /**
     * @brief Clears bars to the right of the currently hovered bar.
     */
    void clearRight();

    /**
     * @brief Clears bars to the left of the currently hovered bar.
     */
    void clearLeft();

    /**
     * @brief Clears every even-indexed bar.
     */
    void clearEven();

    /**
     * @brief Clears every odd-indexed bar.
     */
    void clearOdd();

protected:
    /**
     * @brief Changes values based on mouse drag position.
     * @param e The mouse event.
     */
    void changeValues(const MouseEvent& e);

    /**
     * @brief Gets the index of the bar under the given position.
     * @param position The mouse position.
     * @return The hovered bar index or a clamped value.
     */
    int getHoveredIndex(Point<int> position);

    OpenGlQuad editing_quad_;             ///< Quad used to highlight the hovered bar.
    vital::utils::RandomGenerator random_generator_; ///< Random generator for randomizing bars.
    std::vector<Listener*> listeners_;    ///< List of listeners for bar changes.
    Point<int> current_mouse_position_;   ///< Current mouse position.
    Point<int> last_edit_position_;       ///< Last position during editing for interpolation.
    bool editing_;                        ///< Whether the user is currently editing bars.
    float clear_value_;                   ///< Value to clear bars to.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BarEditor)
};
