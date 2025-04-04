#pragma once

#include "JuceHeader.h"
#include "open_gl_multi_image.h"
#include "synth_section.h"
#include "wavetable_component_factory.h"

class WavetableComponent;
class WavetableCreator;
class WavetableGroup;

/**
 * @class WavetableComponentViewport
 * @brief A Viewport subclass that notifies listeners when the visible area changes.
 *
 * WavetableComponentViewport allows registering listeners to be informed whenever
 * the user scrolls through the contained components. This helps in synchronizing
 * UI elements or other overlays as the viewport content shifts.
 */
class WavetableComponentViewport : public Viewport {
  public:
    /**
     * @class Listener
     * @brief Interface for objects wanting to know when the viewport scrolls.
     */
    class Listener {
      public:
      virtual ~Listener() { }
      /**
       * @brief Called whenever the visible area changes, e.g., due to scrolling.
       */
      virtual void componentsScrolled() = 0;
    };

    /**
     * @brief Adds a listener that will be notified on scroll events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Overridden to notify listeners when the visible area changes.
     * @param newVisibleArea The new visible area in the component's coordinates.
     */
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea) override {
      for (Listener* listener : listeners_)
        listener->componentsScrolled();

      Viewport::visibleAreaChanged(newVisibleArea);
    }

  private:
    std::vector<Listener*> listeners_;
};

/**
 * @class WavetableComponentList
 * @brief A UI component that lists and manages the wavetable sources and modifiers.
 *
 * This class displays a scrollable list of wavetable source groups and their modifiers.
 * It allows adding, removing, and reordering sources and modifiers, as well as resetting them.
 * It interacts with WavetableCreator to maintain and modify the structure of the wavetable.
 */
class WavetableComponentList : public SynthSection, ScrollBar::Listener,
                               public WavetableComponentViewport::Listener {
  public:
    static constexpr int kMaxRows = 128;   ///< Maximum number of rows for components.
    static constexpr int kMaxSources = 16; ///< Maximum number of source groups.

    /**
     * @enum ComponentRowMenu
     * @brief Menu options for component rows.
     */
    enum ComponentRowMenu {
      kRowCancel = 0,
      kReset,
      kMoveUp,
      kMoveDown,
      kRemove
    };

    /**
     * @class Listener
     * @brief Interface for objects wanting to know when the component list changes.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when a component is added.
         * @param component The newly added component.
         */
        virtual void componentAdded(WavetableComponent* component) = 0;

        /**
         * @brief Called when a component is removed.
         * @param component The component that was removed.
         */
        virtual void componentRemoved(WavetableComponent* component) = 0;

        /**
         * @brief Called when components are reordered.
         */
        virtual void componentsReordered() = 0;

        /**
         * @brief Called when components change (e.g., after add/remove/reset).
         */
        virtual void componentsChanged() = 0;

        /**
         * @brief Called when components are scrolled.
         * @param offset The vertical offset of the scroll.
         */
        virtual void componentsScrolled(int offset) { }
    };

    /**
     * @brief Constructs a WavetableComponentList for managing sources and modifiers.
     * @param wavetable_creator The WavetableCreator that manages the actual data.
     */
    WavetableComponentList(WavetableCreator* wavetable_creator);

    /**
     * @brief Clears the list of sources and modifiers.
     */
    void clear();

    /**
     * @brief Initializes the component list after construction.
     */
    void init();

    /**
     * @brief Overridden to layout and refresh the UI.
     */
    void resized() override;

    /**
     * @brief Overridden to paint the background and related elements.
     * @param g The Graphics context to use.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Adds a listener interested in component changes.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the height of each row in the list.
     * @param row_height The new row height in pixels.
     */
    void setRowHeight(int row_height) {
      row_height_ = row_height;
      resetGroups();
    }

    /**
     * @brief Retrieves the group and component indices for a given row index.
     * @param row_index The row index.
     * @return A pair {group_index, component_index} for that row.
     */
    std::pair<int, int> getIndicesForRow(int row_index);

    /**
     * @brief Displays a menu for the group row at the given index.
     * @param row_index The row index.
     */
    void groupMenuClicked(int row_index);

    /**
     * @brief Displays a menu for the modifier row at the given index.
     * @param row_index The row index.
     */
    void modifierMenuClicked(int row_index);

    /**
     * @brief Displays a general menu for the given row index, determining if it's a group or modifier.
     * @param row_index The row index.
     */
    void menuClicked(int row_index);

    /**
     * @brief Called when the "Add Modifier" button is clicked for a particular group.
     * @param group_index The index of the group.
     */
    void addModifierClicked(int group_index);
    /**
     * @brief Called when the "Add Source" button is clicked.
     */
    void addSourceClicked();

    /**
     * @brief Overridden to handle button clicks (menus, add source/modifier).
     * @param button The button that was clicked.
     */
    void buttonClicked(Button* button) override;

    /**
     * @brief Called when the components are scrolled.
     */
    void componentsScrolled() override;

    /**
     * @brief Adds a new source of the given index (maps to a component type).
     * @param index The index representing a source type.
     */
    void addSource(int index);

    /**
     * @brief Removes a source group by index.
     * @param index The group index to remove.
     */
    void removeGroup(int index);

    /**
     * @brief Adds a new component (modifier) of the given type.
     * @param type The type index (maps to a modifier component type).
     */
    void addComponent(int type);

    /**
     * @brief Removes the currently selected component.
     */
    void removeComponent();

    /**
     * @brief Resets the currently selected component to default state.
     */
    void resetComponent();

    /**
     * @brief Removes the currently selected group.
     */
    void removeGroup();

    /**
     * @brief Moves the currently selected group up in the list.
     */
    void moveGroupUp();

    /**
     * @brief Moves the currently selected group down in the list.
     */
    void moveGroupDown();
    /**
     * @brief Moves the currently selected modifier up in its group.
     */
    void moveModifierUp();

    /**
     * @brief Moves the currently selected modifier down in its group.
     */
    void moveModifierDown();

    /**
     * @brief Returns the number of groups in the wavetable.
     */
    int numGroups() { return wavetable_creator_->numGroups(); }

    /**
     * @brief Notifies listeners that a component was added.
     * @param component The component that was added.
     */
    void notifyComponentAdded(WavetableComponent* component);

    /**
     * @brief Notifies listeners that a component was removed.
     * @param component The component that was removed.
     */
    void notifyComponentRemoved(WavetableComponent* component);

    /**
     * @brief Notifies listeners that components have been reordered.
     */
    void notifyComponentsReordered();

    /**
     * @brief Notifies listeners that components have changed.
     */
    void notifyComponentsChanged();

    /**
     * @brief Called when the scroll bar moves.
     * @param scroll_bar The scrollbar that moved.
     * @param range_start The new start range of the scroll.
     */
    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;

    /**
     * @brief Scrolls the viewport by mouse wheel.
     * @param e The mouse event.
     * @param wheel The wheel details.
     */
    void scroll(const MouseEvent& e, const MouseWheelDetails& wheel) {
      viewport_.mouseWheelMove(e, wheel);
    }

  protected:
    /**
     * @brief Resets the display of groups and modifiers in the UI.
     */
    void resetGroups();

    /**
     * @brief Positions the groups and modifiers in the scrollable list.
     */
    void positionGroups();

    /**
     * @brief Sets the scrollbar range based on content size.
     */
    void setScrollBarRange();

    WavetableComponentViewport viewport_;            ///< Viewport for scrolling the component list.
    Component component_container_;                  ///< Container holding all UI elements for the list.
    std::unique_ptr<OpenGlScrollBar> scroll_bar_;    ///< A custom scrollbar for vertical scrolling.

    WavetableCreator* wavetable_creator_;            ///< The wavetable creator managing group/components.
    int current_group_index_;                        ///< The currently selected group index.
    int current_component_index_;                    ///< The currently selected component index.
    std::vector<Listener*> listeners_;               ///< Listeners for changes in the component list.
    OpenGlMultiQuad component_backgrounds_;          ///< Background rectangles for each row.
    std::unique_ptr<PlainTextComponent> names_[kMaxRows];      ///< Text labels for each component row.
    std::unique_ptr<OpenGlShapeButton> menu_buttons_[kMaxRows];///< Menu buttons for each row.
    std::unique_ptr<OpenGlToggleButton> create_component_button_; ///< Button to add a new source.
    std::unique_ptr<OpenGlToggleButton> add_modifier_buttons_[kMaxSources]; ///< Buttons to add modifiers.
    std::unique_ptr<PlainShapeComponent> plus_icons_[kMaxSources + 1];       ///< Plus icons next to buttons.
    int row_height_;                                ///< Height of each row in pixels.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableComponentList)
};
