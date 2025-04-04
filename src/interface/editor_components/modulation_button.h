#pragma once

#include "JuceHeader.h"
#include "open_gl_image_component.h"

namespace vital {
    struct ModulationConnection;
} // namespace vital

class SynthGuiInterface;

/**
 * @class ModulationButton
 * @brief A component representing a modulation source or connection button.
 *
 * The ModulationButton displays and manages a modulation source, allowing the user
 * to connect it to various parameters, disconnect existing connections, and visualize
 * modulation activity. It supports interaction modes like selecting, dragging out to map
 * modulation, and invoking context menus to disconnect modulations.
 */
class ModulationButton : public PlainShapeComponent {
public:
    /// Ratio of the component's width used to display the text (font area).
    static constexpr float kFontAreaHeightRatio = 0.3f;
    /// Number of modulation knob columns.
    static constexpr int kModulationKnobColumns = 3;
    /// Number of modulation knob rows.
    static constexpr int kModulationKnobRows = 2;
    /// Maximum number of modulation knobs displayed.
    static constexpr int kMaxModulationKnobs = kModulationKnobRows * kModulationKnobColumns;
    /// Ratio of the meter area to the total width.
    static constexpr float kMeterAreaRatio = 0.05f;

    /**
     * @enum MenuId
     * @brief Identifiers for menu actions on the ModulationButton.
     */
    enum MenuId {
        kCancel = 0,
        kDisconnect,
        kModulationList
    };

    /**
     * @enum MouseState
     * @brief States representing the current mouse interaction state.
     */
    enum MouseState {
        kNone,
        kHover,
        kMouseDown,
        kMouseDragging,
        kDraggingOut
    };

    /**
     * @class Listener
     * @brief Interface for receiving notifications about modulation events from the ModulationButton.
     */
    class Listener {
    public:
        virtual ~Listener() = default;

        /**
         * @brief Called when a modulation connection is changed (e.g., added or removed).
         */
        virtual void modulationConnectionChanged() { }

        /**
         * @brief Called when a modulation connection is disconnected.
         * @param connection The disconnected modulation connection.
         * @param last True if this was the last modulation connection for that destination.
         */
        virtual void modulationDisconnected(vital::ModulationConnection* connection, bool last) { }

        /**
         * @brief Called when this modulation button is selected.
         * @param source The modulation button that was selected.
         */
        virtual void modulationSelected(ModulationButton* source) { }

        /**
         * @brief Called when the modulation button loses keyboard focus.
         * @param source The modulation button that lost focus.
         */
        virtual void modulationLostFocus(ModulationButton* source) { }

        /**
         * @brief Called when the user begins dragging out to map the modulation (dragging out of the button).
         * @param source The modulation button source.
         * @param e The mouse event triggering this action.
         */
        virtual void startModulationMap(ModulationButton* source, const MouseEvent& e) { }

        /**
         * @brief Called while modulation is being dragged out (during mouse drag).
         * @param e The mouse event during the drag.
         */
        virtual void modulationDragged(const MouseEvent& e) { }

        /**
         * @brief Called when the mouse wheel moves while interacting with the modulation button.
         * @param e The mouse event.
         * @param wheel Wheel details such as delta.
         */
        virtual void modulationWheelMoved(const MouseEvent& e, const MouseWheelDetails& wheel) { }

        /**
         * @brief Called when the user finishes dragging out modulation connections.
         */
        virtual void endModulationMap() { }

        /**
         * @brief Called when the modulation button is clicked (mouse up) without dragging out.
         * @param source The modulation button that was clicked.
         */
        virtual void modulationClicked(ModulationButton* source) { }

        /**
         * @brief Called when all modulations associated with this button have been cleared.
         */
        virtual void modulationCleared() { }
    };

    /**
     * @brief Constructs a ModulationButton.
     * @param name The name of the modulation source this button represents.
     */
    ModulationButton(String name);

    /**
     * @brief Destructor.
     */
    virtual ~ModulationButton();

    /**
     * @brief Paints the background of the button, including meter areas and text.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component's parent hierarchy changes.
     *
     * Used to find the parent SynthGuiInterface and enable/disable modulation sources accordingly.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Called when the component is resized. Repositions internal sub-components.
     */
    void resized() override;

    /**
     * @brief Renders the drag-drop shape overlay using OpenGL.
     * @param open_gl The OpenGlWrapper.
     * @param animate Whether to animate changes.
     */
    virtual void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Handles mouse down events to select or show disconnect menus.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

    /**
     * @brief Handles mouse drag events to start modulation mapping or continue dragging.
     * @param e The mouse event.
     */
    void mouseDrag(const MouseEvent& e) override;

    /**
     * @brief Handles mouse up events to finalize selection or modulation mappings.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Handles mouse enter events to show the drag-drop overlay.
     * @param e The mouse event.
     */
    void mouseEnter(const MouseEvent& e) override;

    /**
     * @brief Handles mouse exit events to hide the drag-drop overlay.
     * @param e The mouse event.
     */
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Handles mouse wheel events to adjust modulation parameters.
     * @param e The mouse event.
     * @param wheel Details of the wheel movement.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Called when the component loses keyboard focus.
     * @param cause The reason for losing focus.
     */
    void focusLost(FocusChangeType cause) override;

    /**
     * @brief Adds a Listener to receive events from this ModulationButton.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener);

    /**
     * @brief Disconnects a modulation connection at a given index in the popup menu.
     * @param index The selected popup menu index.
     */
    void disconnectIndex(int index);

    /**
     * @brief Selects or deselects this modulation source.
     * @param select True to select, false to deselect.
     */
    void select(bool select);

    /**
     * @brief Checks if the modulation button is currently selected.
     * @return True if selected, false otherwise.
     */
    bool isSelected() const { return selected_; }

    /**
     * @brief Sets whether this modulation source is actively modulating something.
     * @param active True if active, false otherwise.
     */
    void setActiveModulation(bool active);

    /**
     * @brief Checks if the modulation source is active.
     * @return True if active, false otherwise.
     */
    bool isActiveModulation() const { return active_modulation_; }

    /**
     * @brief Forces the enable state of this modulation source in the parent synthesizer interface.
     */
    void setForceEnableModulationSource();

    /**
     * @brief Checks if this modulation source is currently connected to any parameters.
     * @return True if any connections exist, false otherwise.
     */
    bool hasAnyModulation();

    /**
     * @brief Sets the font size for the modulation source text.
     * @param size The font size in points.
     */
    void setFontSize(float size) { font_size_ = size; }

    /**
     * @brief Gets the bounds for a modulation amount knob at a given index.
     * @param index The index of the knob.
     * @param total The total number of modulation knobs.
     * @return The bounds of the knob as a Rectangle.
     */
    Rectangle<int> getModulationAmountBounds(int index, int total);

    /**
     * @brief Gets the bounding area used for modulation knobs.
     * @return A rectangle defining the modulation area.
     */
    Rectangle<int> getModulationAreaBounds();

    /**
     * @brief Gets the bounding area used for a modulation meter.
     * @return A rectangle defining the meter area.
     */
    Rectangle<int> getMeterBounds();

    /**
     * @brief Sets whether the modulation button should connect to the right edge.
     * @param connect True if the button connects from the right, false otherwise.
     */
    void setConnectRight(bool connect) { connect_right_ = connect; repaint(); }

    /**
     * @brief Sets whether a border should be drawn around the button.
     * @param border True to draw border, false otherwise.
     */
    void setDrawBorder(bool border) { draw_border_ = border; repaint(); }

    /**
     * @brief Overrides the displayed text on the modulation button.
     * @param text The new text to display.
     */
    void overrideText(String text) { text_override_ = std::move(text); repaint(); }

private:
    /**
     * @brief Disconnects a given modulation connection.
     * @param connection The modulation connection to disconnect.
     */
    void disconnectModulation(vital::ModulationConnection* connection);

    String text_override_;           ///< Optional override text for this modulation source.
    SynthGuiInterface* parent_;      ///< Pointer to the parent SynthGuiInterface.
    std::vector<Listener*> listeners_; ///< Listeners receiving events from this button.
    MouseState mouse_state_;         ///< Current state of the mouse interaction.
    bool selected_;                  ///< Whether this modulation source is currently selected.
    bool connect_right_;             ///< Whether to visually connect this button from the right.
    bool draw_border_;               ///< Whether to draw a border around the button.
    bool active_modulation_;         ///< Whether this modulation source is actively modulating parameters.
    OpenGlImageComponent drag_drop_; ///< OpenGL component for drag-drop overlays.
    Component drag_drop_area_;       ///< Component representing the area for drag-drop rendering.
    float font_size_;                ///< Font size for the displayed text.

    Colour drag_drop_color_;         ///< Color used when showing drag-drop overlays.
    bool show_drag_drop_;            ///< Whether to show the drag-drop overlay.
    float drag_drop_alpha_;          ///< Current alpha value for drag-drop overlay transitions.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationButton)
};
