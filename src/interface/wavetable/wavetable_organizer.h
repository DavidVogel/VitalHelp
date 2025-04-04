#pragma once

#include "JuceHeader.h"
#include "synth_slider.h"
#include "wavetable_component_list.h"
#include "wavetable_creator.h"
#include "wavetable_playhead.h"

/**
 * @class DraggableFrame
 * @brief A visual frame representing a single wavetable keyframe, which can be dragged by the user.
 *
 * The DraggableFrame is used to indicate a wavetable keyframe's position and allow mouse interaction,
 * such as clicking, dragging, and selection. It can represent a full-frame keyframe or a smaller frame
 * depending on the wavetable component type.
 */
class DraggableFrame : public Component {
  public:
    /**
     * @brief Constructs a DraggableFrame.
     * @param full_frame Set to true if this frame covers the entire row, false if it's a small diamond.
     */
    DraggableFrame(bool full_frame = false) {
      setInterceptsMouseClicks(false, true);
      selected_ = false;
      full_frame_ = full_frame;
    }

    /**
     * @brief Checks if a point is inside the frame's clickable area.
     * @param x The x-coordinate relative to the frame's bounds.
     * @param y The y-coordinate relative to the frame's bounds.
     * @return True if the point is considered inside the frame's area, false otherwise.
     */
    bool isInside(int x, int y);

    /**
     * @brief Determines if the frame represents a full row or a small diamond.
     * @return True if the frame is full-row sized, otherwise false.
     */
    bool fullFrame() const { return full_frame_; }

    /**
     * @brief Sets the frame's selection state.
     * @param selected True if the frame should be marked as selected.
     */
    force_inline void select(bool selected) { selected_ = selected; }

    /**
     * @brief Checks if the frame is currently selected.
     * @return True if the frame is selected, otherwise false.
     */
    force_inline bool selected() const { return selected_; }

  protected:
    bool selected_;
    bool full_frame_;
};

/**
 * @class WavetableOrganizer
 * @brief Manages the display and interaction of wavetable keyframes and groups on a timeline.
 *
 * WavetableOrganizer provides a scrollable, interactive timeline of wavetable groups and their components'
 * keyframes. Users can create, remove, select, and drag keyframes to rearrange positions. The organizer
 * integrates with a WavetableCreator and updates keyframe positions within components.
 *
 * It also notifies attached listeners about changes in selected frames and frame positions.
 */
class WavetableOrganizer : public SynthSection,
                           public WavetablePlayhead::Listener,
                           public WavetableComponentList::Listener {
  public:
    /// Fraction of the total height used for the handle area.
    static constexpr float kHandleHeightPercent = 1.0f / 8.0f;
    static constexpr int kDrawSkip = 4;
    static constexpr int kDrawSkipLarge = 32;
    static constexpr int kMaxKeyframes = 2048;

    /**
     * @enum OrganizerMenu
     * @brief Menu actions available in the organizer context menus.
     */
    enum OrganizerMenu {
      kCancel = 0,
      kCreate,
      kRemove
    };

    /**
     * @enum MouseMode
     * @brief The current mouse interaction mode used by the organizer.
     */
    enum MouseMode {
      kWaiting,
      kSelecting,
      kDragging,
      kRightClick,
      kNumMouseModes
    };

    /**
     * @class Listener
     * @brief Interface for objects that need to respond to organizer events.
     *
     * Listeners receive notifications when keyframe positions change, when frames are selected or dragged,
     * and can respond to mouse wheel events.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when keyframe positions or arrangement changes.
         */
        virtual void positionsUpdated() { }

        /**
         * @brief Called when a frame is selected.
         * @param keyframe The newly selected keyframe or nullptr if none are selected.
         */
        virtual void frameSelected(WavetableKeyframe* keyframe) = 0;

        /**
         * @brief Called when a frame is being dragged by the user.
         * @param keyframe The dragged keyframe.
         * @param position The new proposed position for the keyframe.
         */
        virtual void frameDragged(WavetableKeyframe* keyframe, int position) = 0;

        /**
         * @brief Called when the mouse wheel is moved over the organizer.
         * @param e The mouse event.
         * @param wheel The wheel details (direction, amount).
         */
        virtual void wheelMoved(const MouseEvent& e, const MouseWheelDetails& wheel) { }
    };

    /**
     * @brief Constructs a WavetableOrganizer.
     * @param wavetable_creator The WavetableCreator that this organizer interacts with.
     * @param max_frames The maximum number of frames that can be displayed on the timeline.
     */
    WavetableOrganizer(WavetableCreator* wavetable_creator, int max_frames);
    virtual ~WavetableOrganizer();

    /**
     * @brief Paints the background, including a grid and handle areas.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Handles resizing and rearranges the displayed frames and rows.
     */
    void resized() override;

    /**
     * @brief Called when the mouse button is pressed.
     * Used for selecting frames, starting drags, or displaying a menu.
     */
    void mouseDown(const MouseEvent& event) override;

    /**
     * @brief Called as the mouse is dragged.
     * Used for dragging frames or adjusting selection rectangles.
     */
    void mouseDrag(const MouseEvent& event) override;

    /**
     * @brief Called when the mouse button is released.
     * Finalizes drags, completes selections, or handles context menu actions.
     */
    void mouseUp(const MouseEvent& event) override;

    /**
     * @brief Called on a double-click.
     * Used for quickly adding or removing keyframes.
     */
    void mouseDoubleClick(const MouseEvent& event) override;

    /**
     * @brief Called when the mouse wheel is moved.
     * Passes wheel movements to listeners for additional behaviors (like scrolling).
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override {
      for (Listener* listener : listeners_)
        listener->wheelMoved(e, wheel);
    }

    /**
     * @brief Called when the playhead moves, updates the visual playhead position.
     * @param position The new playhead position.
     */
    void playheadMoved(int position) override;

    /**
     * @brief Called when a component is added to the wavetable.
     * Updates frame display accordingly.
     */
    void componentAdded(WavetableComponent* component) override;

    /**
     * @brief Called when a component is removed from the wavetable.
     * Ensures selected frames are updated accordingly.
     */
    void componentRemoved(WavetableComponent* component) override;

    /**
     * @brief Called when components are reordered. Refreshes visuals.
     */
    void componentsReordered() override { }

    /**
     * @brief Called when components change (e.g. frames added/removed).
     * Refreshes the displayed frames.
     */
    void componentsChanged() override { recreateVisibleFrames(); }

    /**
     * @brief Called when components are scrolled via the WavetableComponentList.
     * Adjusts vertical offset of rows.
     * @param offset The vertical offset after scrolling.
     */
    void componentsScrolled(int offset) override;

    /**
     * @brief Adds a listener for organizer events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Removes a previously added listener.
     * @param listener The listener to remove.
     */
    void removeListener(Listener* listener) {
      listeners_.erase(std::find(listeners_.begin(), listeners_.end(), listener));
    }

    /**
     * @brief Gets the width of the handle area for rows.
     * @return The width in pixels of the handle area.
     */
    int handleWidth() const;

    /**
     * @brief Deletes all currently selected keyframes.
     */
    void deleteSelectedKeyframes();

    /**
     * @brief Shows a menu to create a keyframe at a specific position.
     */
    void createKeyframeAtMenu();

    /**
     * @brief Selects a default frame (usually the first one) if available.
     */
    void selectDefaultFrame();

    /**
     * @brief Clears all visible frames and selection.
     */
    void clear() {
      clearVisibleFrames();
      currently_selected_.clear();
    }

    /**
     * @brief Initializes the organizer and sets up frames.
     */
    void init() {
      recreateVisibleFrames();
    }

    /**
     * @brief Checks if there is at least one selected frame.
     * @return True if one or more frames are selected.
     */
    bool hasSelectedFrames() const { return !currently_selected_.empty(); }

  private:
    void clearVisibleFrames();
    void recreateVisibleFrames();
    void repositionVisibleFrames();
    WavetableComponent* getComponentAtRow(int row);
    WavetableKeyframe* getFrameAtMouseEvent(const MouseEvent& event);
    void deselect();
    void deleteKeyframe(WavetableKeyframe* keyframe);
    void createKeyframeAtPosition(Point<int> position);
    void selectFrame(WavetableKeyframe* keyframe);
    void selectFrames(std::vector<WavetableKeyframe*> keyframes);
    void positionSelectionBox(const MouseEvent& event);
    void setRowQuads();
    void setFrameQuads();

    int getRowFromY(int y);
    int getPositionFromX(int x);
    int getUnclampedPositionFromX(int x);
    bool isSelected(WavetableKeyframe* keyframe);

    WavetableCreator* wavetable_creator_;
    std::vector<Listener*> listeners_;
    std::map<WavetableKeyframe*, std::unique_ptr<DraggableFrame>> frame_lookup_;
    OpenGlMultiQuad unselected_frame_quads_;
    OpenGlMultiQuad selected_frame_quads_;
    OpenGlMultiQuad active_rows_;
    OpenGlQuad selection_quad_;
    OpenGlQuad playhead_quad_;

    MouseMode mouse_mode_;
    Point<int> mouse_down_position_;
    Point<int> menu_created_position_;
    std::vector<WavetableKeyframe*> currently_selected_;
    WavetableKeyframe* currently_dragged_;
    int dragged_start_x_;

    int draw_vertical_offset_;
    int playhead_position_;
    int max_frames_;
    float frame_width_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableOrganizer)
};

