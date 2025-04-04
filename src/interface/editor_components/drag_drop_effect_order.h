#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "synth_constants.h"
#include "synth_button.h"
#include "synth_slider.h"
#include "open_gl_image_component.h"

/**
 * @class DraggableEffect
 * @brief A UI component representing an individual effect that can be enabled, disabled, and rearranged.
 *
 * DraggableEffect displays an effect’s icon, name, and a toggle button to enable or disable it.
 * It is intended to be moved (dragged) within an order list, thus allowing users to rearrange
 * the processing chain. The hover state and the enabled state are visually indicated.
 */
class DraggableEffect : public SynthSection {
public:
    /**
     * @class Listener
     * @brief Listener interface for responding to changes in the DraggableEffect’s enabled state.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the DraggableEffect’s enabled state changes.
         * @param effect The DraggableEffect that changed state.
         * @param enabled True if enabled, false otherwise.
         */
        virtual void effectEnabledChanged(DraggableEffect* effect, bool enabled) = 0;
    };

    /**
     * @brief Constructs a DraggableEffect.
     * @param name The name of the effect.
     * @param order The initial order (position) of this effect.
     */
    DraggableEffect(const String& name, int order);

    /**
     * @brief Paints the DraggableEffect’s background and icon.
     * @param g The graphics context used for drawing.
     */
    void paint(Graphics& g) override;

    /**
     * @brief Paints the background of the DraggableEffect.
     * @param g The graphics context.
     *
     * This is intentionally empty as the background painting is integrated in paint().
     */
    void paintBackground(Graphics& g) override { }

    /**
     * @brief Renders all nested OpenGL components.
     * @param open_gl The OpenGL wrapper.
     * @param animate Whether the current frame is part of an animation.
     */
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override {
        SynthSection::renderOpenGlComponents(open_gl, animate);
    }

    /**
     * @brief Positions the subcomponents (like enable button) within the DraggableEffect.
     */
    void resized() override;

    /**
     * @brief Handles button clicks, primarily the enable button.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Adds a listener for this DraggableEffect’s enable state changes.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Checks if this effect is currently enabled.
     * @return True if enabled, false otherwise.
     */
    bool enabled() const { return enable_->getToggleState(); }

    /**
     * @brief Gets the current order (position) of this effect.
     * @return The order index.
     */
    int order() const { return order_; }

    /**
     * @brief Updates the hover state visually.
     * @param hover True to indicate mouse is hovering, false otherwise.
     */
    void hover(bool hover);

private:
    Path icon_;                                   ///< The path used to draw the effect’s icon.
    int order_;                                   ///< The current order (position) of the effect.
    bool hover_;                                  ///< Indicates whether the mouse is hovering over the effect.
    std::unique_ptr<SynthButton> enable_;         ///< The enable/disable button.
    std::unique_ptr<OpenGlImageComponent> background_; ///< The background image component.
    std::vector<Listener*> listeners_;            ///< List of registered listeners.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DraggableEffect)
};

/**
 * @class DragDropEffectOrder
 * @brief A container managing multiple DraggableEffect components, allowing reordering via drag and drop.
 *
 * DragDropEffectOrder organizes a collection of DraggableEffect objects representing different effects.
 * Users can rearrange the effect chain by dragging these components. The class also communicates order
 * changes and effect enable state changes to its listeners.
 */
class DragDropEffectOrder : public SynthSection, public DraggableEffect::Listener {
public:
    /// Padding between individual effects.
    static constexpr int kEffectPadding = 6;

    /**
     * @class Listener
     * @brief Listener interface for responding to changes in the drag/drop order or effect states.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the order of effects changes due to a drag/drop operation.
         * @param order The DragDropEffectOrder instance.
         */
        virtual void orderChanged(DragDropEffectOrder* order) = 0;

        /**
         * @brief Called when a particular effect’s enabled state changes.
         * @param order_index The index of the effect whose state changed.
         * @param enabled True if enabled, false otherwise.
         */
        virtual void effectEnabledChanged(int order_index, bool enabled) = 0;
    };

    /**
     * @brief Constructs a DragDropEffectOrder component.
     * @param name The name of the section.
     */
    DragDropEffectOrder(String name);

    /**
     * @brief Destructor.
     */
    virtual ~DragDropEffectOrder();

    /**
     * @brief Positions the DraggableEffect components based on the current order.
     */
    void resized() override;

    /**
     * @brief Paints the background behind the effects.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Renders all nested OpenGL components, including dragged effects.
     * @param open_gl The OpenGL wrapper.
     * @param animate Indicates if the current rendering is part of an animation.
     */
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Handles mouse movement to update hover states.
     * @param e The mouse event.
     */
    void mouseMove(const MouseEvent& e) override;

    /**
     * @brief Handles mouse down events to initiate dragging.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events to reorder the effects dynamically.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events to finalize the dragged effect’s position.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Resets hover states when the mouse exits the component.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Responds to enable state changes from a DraggableEffect.
     * @param effect The DraggableEffect that changed.
     * @param enabled True if enabled, false otherwise.
     */
    void effectEnabledChanged(DraggableEffect* effect, bool enabled) override;

    /**
     * @brief Sets all values from a control map (e.g., restoring order from saved state).
     * @param controls The control map to retrieve values from.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Moves an effect from one position to another, adjusting the order of all affected effects.
     * @param start_index The original index of the effect being moved.
     * @param end_index The target index to move the effect to.
     */
    void moveEffect(int start_index, int end_index);

    /**
     * @brief Repositions the effect at the given index to its stationary position (no dragging).
     * @param index The index of the effect to reposition.
     */
    void setStationaryEffectPosition(int index);

    /**
     * @brief Adds a listener for order or state changes.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Gets the effect’s internal index from an ordered position.
     * @param index The ordered position.
     * @return The internal effect index.
     */
    int getEffectIndex(int index) const;

    /**
     * @brief Gets the effect component at a given ordered position.
     * @param index The ordered position of the effect.
     * @return A pointer to the effect component.
     */
    Component* getEffect(int index) const;

    /**
     * @brief Checks if the effect at a given ordered position is enabled.
     * @param index The ordered position of the effect.
     * @return True if enabled, false otherwise.
     */
    bool effectEnabled(int index) const;

    /**
     * @brief Converts a vertical mouse position (y) to an ordered effect index.
     * @param y The y-position of the mouse.
     * @return The corresponding effect index.
     */
    int getEffectIndexFromY(float y) const;

private:
    /**
     * @brief Gets the vertical position of an effect’s top edge based on its index.
     * @param index The effect’s ordered index.
     * @return The y-coordinate for that effect.
     */
    int getEffectY(int index) const;

    std::vector<Listener*> listeners_;           ///< List of registered listeners for order/enable changes.
    DraggableEffect* currently_dragged_;         ///< The effect currently being dragged, if any.
    DraggableEffect* currently_hovered_;         ///< The effect currently hovered over by the mouse.
    int last_dragged_index_;                     ///< The last index of the dragged effect before reordering.
    int mouse_down_y_;                           ///< The mouse’s y-coordinate at the time of mouseDown.
    int dragged_starting_y_;                     ///< The starting y-position of the dragged effect.
    std::vector<std::unique_ptr<DraggableEffect>> effect_list_; ///< The list of effect components.
    int effect_order_[vital::constants::kNumEffects]; ///< The current order mapping for the effects.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragDropEffectOrder)
};
