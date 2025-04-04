#pragma once

#include "JuceHeader.h"

#include "line_generator.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_image.h"
#include "open_gl_multi_quad.h"
#include "open_gl_line_renderer.h"
#include "synth_lfo.h"
#include "synth_module.h"

/**
 * @class LineEditor
 * @brief A GUI component for editing and visualizing a user-defined line shape (curve or envelope).
 *
 * The LineEditor component displays and allows interactive editing of a set of control points
 * and powers that define a curve. This curve may represent an LFO shape or an envelope. Users
 * can add, remove, and move points, as well as adjust the curve’s power segments for non-linear
 * transitions. The editor supports grid snapping, painting patterns, copying/pasting states,
 * and file loading/saving.
 *
 * The rendered curve is shown using OpenGL for efficient visualization. Hover indicators,
 * dragging handles, and visual feedback are provided. The LineEditor also supports text-based
 * entry for precise values and can handle custom mouse interactions (like painting patterns).
 */
class LineEditor : public OpenGlLineRenderer, public TextEditor::Listener {
public:
    /// Width in pixels for main position markers.
    static constexpr float kPositionWidth = 9.0f;
    /// Width in pixels for power markers (curve shaping handles).
    static constexpr float kPowerWidth = 7.0f;
    /// Fractional thickness for marker ring rendering.
    static constexpr float kRingThickness = 0.45f;
    /// Radius in pixels for detecting grabbing a point or power handle.
    static constexpr float kGrabRadius = 12.0f;
    /// Radius in pixels for dragging a point or power handle.
    static constexpr float kDragRadius = 20.0f;

    /// Resolution used for intermediate line calculations.
    static constexpr int kResolution = 64;
    /// Number of wrap points for looping lines.
    static constexpr int kNumWrapPoints = 8;
    /// Number of points drawn: resolution plus max points from LineGenerator.
    static constexpr int kDrawPoints = kResolution + LineGenerator::kMaxPoints;
    /// Total points including wrap-around.
    static constexpr int kTotalPoints = kDrawPoints + 2 * kNumWrapPoints;

    /// Maximum grid sizes for horizontal and vertical lines.
    static constexpr int kMaxGridSizeX = 32;
    static constexpr int kMaxGridSizeY = 24;

    /// Vertical padding in pixels.
    static constexpr float kPaddingY = 6.0f;
    /// Horizontal padding in pixels.
    static constexpr float kPaddingX = 0.0f;

    /// Multiplier for mouse movements when adjusting power handles.
    static constexpr float kPowerMouseMultiplier = 9.0f;
    /// Minimum horizontal distance (in pixels) between points to show power handles.
    static constexpr float kMinPointDistanceForPower = 3.0f;

    /**
     * @enum MenuOptions
     * @brief Context menu options available in the line editor.
     */
    enum MenuOptions {
        kCancel,
        kCopy,
        kPaste,
        kSave,
        kEnterPhase,
        kEnterValue,
        kResetPower,
        kRemovePoint,
        kInit,
        kFlipHorizontal,
        kFlipVertical,
        kNumMenuOptions
    };

    /**
     * @class Listener
     * @brief Interface for classes that want to receive notifications about line editor changes.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when the LFO phase or start point is changed.
         * @param phase The new phase value.
         */
        virtual void setPhase(float phase) = 0;

        /**
         * @brief Called when the user scrolls the mouse wheel over the line editor.
         * @param e The associated mouse event.
         * @param wheel The wheel details, including scroll direction and amount.
         */
        virtual void lineEditorScrolled(const MouseEvent& e, const MouseWheelDetails& wheel) = 0;

        /**
         * @brief Called when paint mode is toggled.
         * @param enabled True if paint mode is enabled.
         * @param temporary_switch True if this is a temporary toggle via a modifier key.
         */
        virtual void togglePaintMode(bool enabled, bool temporary_switch) = 0;

        /**
         * @brief Called when a file (e.g., a saved LFO shape) is loaded into the editor.
         */
        virtual void fileLoaded() = 0;

        /**
         * @brief Called when the user requests to import an LFO.
         */
        virtual void importLfo() = 0;

        /**
         * @brief Called when the user requests to export the current LFO.
         */
        virtual void exportLfo() = 0;

        /**
         * @brief Called when a point's position changes.
         * @param index The point index.
         * @param position The new point position in normalized coordinates.
         * @param mouse_up True if the mouse is no longer pressed (final position).
         */
        virtual void pointChanged(int index, Point<float> position, bool mouse_up) { }

        /**
         * @brief Called when the curve powers have changed.
         * @param mouse_up True if the mouse is no longer pressed (final state).
         */
        virtual void powersChanged(bool mouse_up) { }

        /**
         * @brief Called when a point is added to the curve.
         * @param index The index of the new point.
         * @param position The position of the new point.
         */
        virtual void pointAdded(int index, Point<float> position) { }

        /**
         * @brief Called when a point is removed from the curve.
         * @param index The index of the removed point.
         */
        virtual void pointRemoved(int index) { }

        /**
         * @brief Called when multiple points are added at once.
         * @param index The starting index of added points.
         * @param num_points_added The number of points added.
         */
        virtual void pointsAdded(int index, int num_points_added) { }

        /**
         * @brief Called when multiple points are removed at once.
         * @param index The starting index of removed points.
         * @param num_points_removed The number of points removed.
         */
        virtual void pointsRemoved(int index, int num_points_removed) { }
    };

    /**
     * @brief Constructs the LineEditor.
     * @param line_source The line source (LineGenerator) providing the curve data.
     */
    LineEditor(LineGenerator* line_source);

    /**
     * @brief Destructor.
     */
    virtual ~LineEditor();

    /**
     * @brief Resets the wave path, recalculating positions based on current points and powers.
     */
    void resetWavePath();

    void resized() override {
        OpenGlLineRenderer::resized();
        drag_circle_.setBounds(getLocalBounds());
        hover_circle_.setBounds(getLocalBounds());
        grid_lines_.setBounds(getLocalBounds());
        position_circle_.setBounds(getLocalBounds());
        point_circles_.setBounds(getLocalBounds());
        power_circles_.setBounds(getLocalBounds());
        resetPositions();
    }

    /**
     * @brief Pads a Y coordinate to fit the drawing area with vertical padding.
     * @param y The Y coordinate (unscaled).
     * @return The padded Y coordinate.
     */
    float padY(float y);

    /**
     * @brief Removes padding from a padded Y coordinate.
     * @param y The padded Y coordinate.
     * @return The unpadded Y coordinate.
     */
    float unpadY(float y);

    /**
     * @brief Pads an X coordinate to fit the drawing area with horizontal padding.
     * @param x The X coordinate (unscaled).
     * @return The padded X coordinate.
     */
    float padX(float x);

    /**
     * @brief Removes padding from a padded X coordinate.
     * @param x The padded X coordinate.
     * @return The unpadded X coordinate.
     */
    float unpadX(float x);

    virtual void mouseDown(const MouseEvent& e) override;
    virtual void mouseDoubleClick(const MouseEvent& e) override;
    virtual void mouseMove(const MouseEvent& e) override;
    virtual void mouseDrag(const MouseEvent& e) override;
    virtual void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Responds to a callback triggered by a menu option or action.
     * @param point The active point index, or -1 if none.
     * @param power The active power handle index, or -1 if none.
     * @param option The selected menu option ID.
     */
    virtual void respondToCallback(int point, int power, int option);

    /**
     * @brief Checks if the system clipboard contains a compatible line data JSON.
     * @return True if clipboard data matches a known line state format.
     */
    bool hasMatchingSystemClipboard();

    /**
     * @brief Paints the line by adding points according to a pattern when in paint mode.
     * @param e The mouse event.
     */
    void paintLine(const MouseEvent& e);

    /**
     * @brief Handles the initial mouse press (not in paint mode) for dragging points or powers.
     * @param e The mouse event.
     */
    void drawDown(const MouseEvent& e);

    /**
     * @brief Handles mouse dragging to move points or adjust powers.
     * @param e The mouse event.
     */
    void drawDrag(const MouseEvent& e);

    /**
     * @brief Handles mouse release to finalize point or power positions.
     * @param e The mouse event.
     */
    void drawUp(const MouseEvent& e);

    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    void mouseExit(const MouseEvent& e) override;
    void clearActiveMouseActions();

    /**
     * @brief Renders the grid lines that show snapping lines or painting sections.
     */
    void renderGrid(OpenGlWrapper& open_gl, bool animate);

    /**
     * @brief Renders the points and power handles on the curve.
     */
    void renderPoints(OpenGlWrapper& open_gl, bool animate);

    void init(OpenGlWrapper& open_gl) override;
    void render(OpenGlWrapper& open_gl, bool animate) override;
    void destroy(OpenGlWrapper& open_gl) override;

    /**
     * @brief Sets the size ratio, scaling UI elements accordingly.
     * @param ratio The size ratio.
     */
    void setSizeRatio(float ratio) { size_ratio_ = ratio; }

    /**
     * @brief Gets the current size ratio.
     * @return The size ratio.
     */
    float sizeRatio() const { return size_ratio_; }

    /**
     * @brief Enables or disables looping of the line at the edges.
     * @param loop True to loop the line, false otherwise.
     */
    void setLoop(bool loop) { loop_ = loop; }

    /**
     * @brief Enables or disables smoothing of the curve.
     * @param smooth True to smooth the curve, false otherwise.
     */
    void setSmooth(bool smooth) { model_->setSmooth(smooth); resetPositions(); }

    /**
     * @brief Checks if smoothing is enabled.
     * @return True if smoothing is enabled, false otherwise.
     */
    bool getSmooth() const { return model_->smooth(); }

    /**
     * @brief Enables or disables paint mode.
     * @param paint True to enable paint mode.
     */
    void setPaint(bool paint);

    /**
     * @brief Sets a pattern of points used when painting the line.
     * @param pattern A vector of (phase, amplitude) pairs defining the pattern.
     */
    void setPaintPattern(std::vector<std::pair<float, float>> pattern) { paint_pattern_ = pattern; }

    /**
     * @brief Sets the horizontal grid size.
     * @param size The number of divisions horizontally.
     */
    virtual void setGridSizeX(int size) { grid_size_x_ = size; setGridPositions(); }

    /**
     * @brief Sets the vertical grid size.
     * @param size The number of divisions vertically.
     */
    virtual void setGridSizeY(int size) { grid_size_y_ = size; setGridPositions(); }

    /**
     * @brief Gets the current horizontal grid size.
     */
    int getGridSizeX() { return grid_size_x_; }

    /**
     * @brief Gets the current vertical grid size.
     */
    int getGridSizeY() { return grid_size_y_; }

    /**
     * @brief Sets the LineGenerator model defining the curve.
     * @param model The new LineGenerator model.
     */
    void setModel(LineGenerator* model) { model_ = model; resetPositions(); }

    /**
     * @brief Gets the current LineGenerator model.
     */
    LineGenerator* getModel() { return model_; }

    /**
     * @brief Shows the text editor for entering a precise value or phase.
     */
    void showTextEntry();

    /**
     * @brief Hides the text entry editor.
     */
    void hideTextEntry();

    void textEditorReturnKeyPressed(TextEditor& editor) override;
    void textEditorFocusLost(TextEditor& editor) override;
    void textEditorEscapeKeyPressed(TextEditor& editor) override;

    /**
     * @brief Sets the position of the selected point/power from the text field.
     */
    void setSliderPositionFromText();

    /**
     * @brief Allows or disallows file loading actions (copy/paste from files).
     * @param allow True to allow file loading, false otherwise.
     */
    void setAllowFileLoading(bool allow) { allow_file_loading_ = allow; }

    /**
     * @brief Adds a listener for line editor events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets whether the line editor is active (enabled).
     * @param active True if active, false otherwise.
     */
    void setActive(bool active) { active_ = active; }

    /**
     * @brief Marks positions as needing recalculation on next render.
     */
    force_inline void resetPositions() { reset_positions_ = true; }

    /**
     * @brief Gets the OpenGlComponent for text editing (if any).
     * @return The text editor's OpenGL component, or nullptr if none.
     */
    OpenGlComponent* getTextEditorComponent() {
        if (value_entry_)
            return value_entry_->getImageComponent();
        return nullptr;
    }

protected:
    /**
     * @brief Draws a position marker at a specific fraction of the X-axis.
     * @param open_gl The OpenGL wrapper.
     * @param color The color of the marker.
     * @param fraction_x The normalized X position (0.0 to 1.0).
     */
    void drawPosition(OpenGlWrapper& open_gl, Colour color, float fraction_x);

    /**
     * @brief Sets bounds for the editing circles (hover and drag indicators).
     */
    void setEditingCircleBounds();

    /**
     * @brief Calculates and sets positions for grid lines.
     */
    void setGridPositions();

    /**
     * @brief Calculates and sets positions for point and power handle quads.
     */
    void setPointPositions();

    /**
     * @brief Updates OpenGL buffers with the latest positions if needed.
     */
    void setGlPositions();

    /**
     * @brief Gets the currently active point index.
     * @return The index of the active point, or -1 if none.
     */
    int getActivePoint() { return active_point_; }

    /**
     * @brief Gets the currently active power handle index.
     * @return The index of the active power, or -1 if none.
     */
    int getActivePower() { return active_power_; }

    /**
     * @brief Gets the currently active grid section index for painting.
     * @return The grid section index, or -1 if none.
     */
    int getActiveGridSection() { return active_grid_section_; }

    /**
     * @brief Checks if painting mode is currently active.
     */
    bool isPainting() { return paint_ != temporary_paint_toggle_; }

    /**
     * @brief Checks if paint mode is globally enabled.
     */
    bool isPaintEnabled() { return paint_; }

    /**
     * @brief Adjusts a given phase value for boost calculations.
     * @param phase The phase value as a poly_float.
     * @return Adjusted phase for smooth boosting.
     */
    vital::poly_float adjustBoostPhase(vital::poly_float phase);

    /**
     * @brief Temporarily enables or disables paint mode using a toggle (e.g., via a modifier key).
     * @param toggle True to temporarily enable, false to disable.
     */
    virtual void enableTemporaryPaintToggle(bool toggle);

    bool active_;
    std::vector<Listener*> listeners_;

private:
    float adjustBoostPhase(float phase);

    int getHoverPoint(Point<float> position);
    int getHoverPower(Point<float> position);
    float getSnapRadiusX();
    float getSnapRadiusY();
    float getSnappedX(float x);
    float getSnappedY(float y);

    void addPointAt(Point<float> position);
    void movePoint(int index, Point<float> position, bool snap);
    void movePower(int index, Point<float> position, bool all, bool alternate);
    void removePoint(int index);
    float getMinX(int index);
    float getMaxX(int index);

    Point<float> valuesToOpenGlPosition(float x, float y);
    Point<float> getPowerPosition(int index);
    bool powerActive(int index);

    LineGenerator* model_;
    int active_point_;
    int active_power_;
    int active_grid_section_;
    bool dragging_;
    bool reset_positions_;
    bool allow_file_loading_;
    Point<float> last_mouse_position_;
    int last_model_render_;
    bool loop_;
    int grid_size_x_;
    int grid_size_y_;
    bool paint_;
    bool temporary_paint_toggle_;
    std::vector<std::pair<float, float>> paint_pattern_;
    vital::poly_float last_phase_;
    vital::poly_float last_voice_;
    vital::poly_float last_last_voice_;
    float size_ratio_;

    OpenGlQuad drag_circle_;
    OpenGlQuad hover_circle_;
    OpenGlMultiQuad grid_lines_;
    OpenGlQuad position_circle_;
    OpenGlMultiQuad point_circles_;
    OpenGlMultiQuad power_circles_;
    std::unique_ptr<OpenGlTextEditor> value_entry_;
    bool entering_phase_;
    int entering_index_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LineEditor)
};
