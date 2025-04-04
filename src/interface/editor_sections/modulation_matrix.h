#pragma once

#include "JuceHeader.h"
#include "line_map_editor.h"
#include "open_gl_image.h"
#include "overlay.h"
#include "preset_selector.h"
#include "synth_section.h"
#include "synth_constants.h"

class ModulationMatrixRow;
class ModulationMeterReadouts;
class SynthButton;
class SynthGuiInterface;
class PaintPatternSelector;

/**
 * @class ModulationSelector
 * @brief A specialized slider-like component allowing selection of modulation sources or destinations from a popup menu.
 */
class ModulationSelector : public OpenGlSlider {
  public:
    /**
     * @brief Callback for modulation popup menu selections.
     * @param result The menu item index selected.
     * @param selector The ModulationSelector instance.
     */
    static void modulationSelectionCallback(int result, ModulationSelector* selector) {
      if (selector != nullptr && result != 0)
        selector->setValue(result - 1);
    }

    /**
     * @brief Constructs a ModulationSelector.
     * @param name The name for the selector.
     * @param selections A pointer to a vector of selection strings.
     * @param popup_items A pointer to a PopupItems structure for building menus.
     * @param dual_menu Whether the selector uses dual-level menus.
     */
    ModulationSelector(String name, const std::vector<String>* selections, PopupItems* popup_items, bool dual_menu) :
        OpenGlSlider(std::move(name)), selections_(selections), popup_items_(popup_items), dual_menu_(dual_menu) {
      setRange(0.0, selections_->size() - 1.0, 1.0);
      setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    }

    /**
     * @brief Converts a numeric value to a display text representing the currently selected modulation.
     * @param value The numeric value representing the selection index.
     * @return The display text.
     */
    String getTextFromValue(double value) override;

    /**
     * @brief Gets the currently selected modulation string.
     * @return The selected modulation string.
     */
    String getSelection() {
      int index = std::round(getValue());
      return selections_->at(index);
    }

    /**
     * @brief Checks if the selector is currently connected (not at default/off selection).
     * @return True if connected, false otherwise.
     */
    bool connected() const {
      return getValue() != 0.0f;
    }

    /**
     * @brief Sets the current value based on a given name.
     * @param name The name to match.
     * @param notification_type The notification type for value changes.
     */
    void setValueFromName(const String& name, NotificationType notification_type);

    /**
     * @brief Handles mouse down events, showing the popup selection menu.
     * @param e The mouse event.
     */
    void mouseDown(const juce::MouseEvent &e) override;

  private:
    const std::vector<String>* selections_; ///< A pointer to the available selection strings.
    PopupItems* popup_items_; ///< Popup menu items for selection.
    bool dual_menu_; ///< Whether the selector uses a dual-level menu.
};

/**
 * @class ModulationViewport
 * @brief A specialized viewport for the modulation matrix allowing for scroll listeners.
 */
class ModulationViewport : public Viewport {
  public:
    /**
     * @class Listener
     * @brief A listener interface for responding to scrolling events in the modulation viewport.
     */
    class Listener {
      public:
        virtual ~Listener() = default;

        /**
         * @brief Called when the modulation view is scrolled.
         * @param position The current vertical scroll position.
         */
        virtual void modulationScrolled(int position) = 0;

        /**
         * @brief Called when scrolling starts.
         */
        virtual void startScroll() = 0;

        /**
         * @brief Called when scrolling ends.
         */
        virtual void endScroll() = 0;
    };

    /**
     * @brief Handles mouse wheel movement, notifying listeners before and after scrolling.
     * @param e The mouse event.
     * @param wheel Details about the wheel movement.
     */
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override {
      for (Listener* listener : listeners_)
        listener->startScroll();

      Viewport::mouseWheelMove(e, wheel);

      for (Listener* listener : listeners_)
        listener->endScroll();
    }

    /**
     * @brief Adds a listener to be notified of scrolling changes.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Called when the visible area changes, updating listeners with the new scroll position.
     * @param visible_area The new visible area.
     */
    void visibleAreaChanged(const Rectangle<int>& visible_area) override {
      for (Listener* listener : listeners_)
        listener->modulationScrolled(visible_area.getY());

      Viewport::visibleAreaChanged(visible_area);
    }

  private:
    std::vector<Listener*> listeners_; ///< The list of registered listeners.
};

/**
 * @class ModulationMatrixRow
 * @brief Represents a single row in the modulation matrix, showing source, destination, and associated parameters.
 */
class ModulationMatrixRow : public SynthSection {
  public:
    /**
     * @class Listener
     * @brief Interface for objects that need to respond to row selection changes.
     */
    class Listener {
      public:
        virtual ~Listener() = default;

        /**
         * @brief Called when this row is selected.
         * @param selected_row The row that was selected.
         */
        virtual void rowSelected(ModulationMatrixRow* selected_row) = 0;
    };

    /**
     * @brief Constructs a ModulationMatrixRow.
     * @param index The row index.
     * @param source_items Popup items for the source selector.
     * @param destination_items Popup items for the destination selector.
     * @param sources A vector of all possible sources.
     * @param destinations A vector of all possible destinations.
     */
    ModulationMatrixRow(int index, PopupItems* source_items, PopupItems* destination_items,
                        const std::vector<String>* sources, const std::vector<String>* destinations);

    /**
     * @brief Handles component resizing.
     */
    void resized() override;

    /**
     * @brief Overrides to repaint the background. (Empty override)
     */
    void repaintBackground() override { }

    /**
     * @brief Sets the parent SynthGuiInterface.
     * @param parent The parent interface.
     */
    void setGuiParent(SynthGuiInterface* parent) { parent_ = parent; }

    /**
     * @brief Assigns a modulation connection to this row.
     * @param connection The modulation connection.
     */
    void setConnection(vital::ModulationConnection* connection) { connection_ = connection; }

    /**
     * @brief Paints the background of this row.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Callback when a slider's value changes.
     * @param changed_slider The slider that changed.
     */
    void sliderValueChanged(Slider* changed_slider) override;

    /**
     * @brief Callback when a button is clicked.
     * @param button The button that was clicked.
     */
    void buttonClicked(Button* button) override;

    /**
     * @brief Updates the display of the row based on the current connection state.
     */
    void updateDisplay();

    /**
     * @brief Updates only the display values (like amount) without changing source/destination selection.
     */
    void updateDisplayValue();

    /**
     * @brief Checks if this row represents a connected modulation.
     * @return True if connected, false otherwise.
     */
    bool connected() const;

    /**
     * @brief Checks if this row matches the given source and destination names.
     * @param source The source name to match.
     * @param destination The destination name to match.
     * @return True if both match, false otherwise.
     */
    bool matchesSourceAndDestination(const std::string& source, const std::string& destination) const;

    /**
     * @brief Gets the bounds where a modulation meter may be drawn.
     * @return The meter bounds as a Rectangle.
     */
    Rectangle<int> getMeterBounds();

    /**
     * @brief Selects this row and notifies listeners.
     */
    void select() {
      for (Listener* listener : listeners_)
        listener->rowSelected(this);
    }

    /**
     * @brief Called when the mouse is pressed down. Selects this row.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override { select(); }

    /**
     * @brief Changes the visual "selected" state of this row.
     * @param select True to select, false to deselect.
     */
    void select(bool select);

    /**
     * @brief Checks if this row is currently selected.
     * @return True if selected, false otherwise.
     */
    bool selected() const { return selected_; }

    /**
     * @brief Adds a listener to this row.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Gets the row index.
     * @return The row index.
     */
    force_inline int index() const { return index_; }

    /**
     * @brief Gets the selected source index.
     * @return The source index.
     */
    force_inline int source() const { return source_->getValue(); }

    /**
     * @brief Gets the selected destination index.
     * @return The destination index.
     */
    force_inline int destination() const { return destination_->getValue(); }

    /**
     * @brief Gets whether this row is set to stereo modulation.
     * @return Non-zero if stereo is enabled.
     */
    force_inline int stereo() const { return stereo_->getToggleState(); }

    /**
     * @brief Gets whether this row is set to bipolar modulation.
     * @return Non-zero if bipolar is enabled.
     */
    force_inline int bipolar() const { return bipolar_->getToggleState(); }

    /**
     * @brief Gets the morph value of this modulation.
     * @return The morph value.
     */
    force_inline float morph() const { return power_slider_->getValue(); }

    /**
     * @brief Gets the modulation amount value.
     * @return The amount value.
     */
    force_inline float amount() const { return amount_slider_->getValue(); }

  protected:
    std::vector<Listener*> listeners_; ///< Registered row listeners.

    int index_; ///< The index of this row.
    vital::ModulationConnection* connection_; ///< The modulation connection for this row.
    SynthGuiInterface* parent_; ///< The parent GUI interface.

    std::unique_ptr<ModulationSelector> source_; ///< The source selector.
    std::unique_ptr<ModulationSelector> destination_; ///< The destination selector.
    double last_source_value_; ///< The last known source value.
    double last_destination_value_; ///< The last known destination value.
    double last_amount_value_; ///< The last known amount value.
    std::unique_ptr<SynthSlider> amount_slider_; ///< The slider for modulation amount.
    std::unique_ptr<SynthSlider> power_slider_; ///< The slider for the morph/power parameter.
    std::unique_ptr<OpenGlShapeButton> bipolar_; ///< The bipolar toggle button.
    std::unique_ptr<SynthButton> stereo_; ///< The stereo toggle button.
    std::unique_ptr<SynthButton> bypass_; ///< The bypass button for this modulation row.
    OverlayBackgroundRenderer highlight_; ///< Renders a highlight overlay when selected.

    bool updating_; ///< Indicates if the row is currently updating to avoid recursive changes.
    bool selected_; ///< Whether this row is currently selected.
};

/**
 * @class ModulationMatrix
 * @brief The main modulation matrix component displaying multiple modulation rows and related controls.
 */
class ModulationMatrix : public SynthSection, public ModulationViewport::Listener,
                         public ModulationMatrixRow::Listener, public ScrollBar::Listener,
                         public PresetSelector::Listener, public LineEditor::Listener {
  public:
    /** Padding in rows between each modulation row. */
    static constexpr int kRowPadding = 1;
    /** Default grid sizes for line editor. */
    static constexpr int kDefaultGridSizeX = 8;
    static constexpr int kDefaultGridSizeY = 1;

    /**
     * @brief Returns a user-friendly display name for a given source string in menu context.
     * @param original The original source string.
     * @return A transformed user-friendly display name.
     */
    static String getMenuSourceDisplayName(const String& original);

    /**
     * @brief Returns a display name suitable for the UI given a source string.
     * @param original The original source string.
     * @return A transformed UI display name.
     */
    static String getUiSourceDisplayName(const String& original);

    /**
     * @enum SortColumn
     * @brief Columns available for sorting the modulation matrix rows.
     */
    enum SortColumn {
      kNumber,
      kSource,
      kBipolar,
      kStereo,
      kMorph,
      kAmount,
      kDestination,
      kNumColumns
    };

    /**
     * @class Listener
     * @brief Interface for objects that need to respond to modulation matrix scrolling events.
     */
    class Listener {
      public:
        virtual ~Listener() = default;
        /**
         * @brief Called when the modulation matrix is scrolled.
         */
        virtual void modulationsScrolled() = 0;
    };

    /**
     * @brief Constructs a ModulationMatrix.
     * @param sources A map of all available modulation sources.
     * @param destinations A map of all available modulation destinations.
     */
    ModulationMatrix(const vital::output_map& sources, const vital::output_map& destinations);

    /**
     * @brief Destructor.
     */
    virtual ~ModulationMatrix();

    /**
     * @brief Paints the background of the modulation matrix.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints background shadows for the matrix sections.
     * @param g The graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override;

    /**
     * @brief Paints the portion of the matrix that scrolls, i.e., the rows.
     */
    void paintScrollableBackground();

    /**
     * @brief Handles resizing of the component.
     */
    void resized() override;

    /**
     * @brief Sets the meter bounds for each modulation row.
     */
    void setMeterBounds();

    /**
     * @brief Sets visibility of this component, and updates modulations if visible.
     * @param should_be_visible True if should be visible.
     */
    void setVisible(bool should_be_visible) override {
      SynthSection::setVisible(should_be_visible);
      updateModulations();
    }

    /**
     * @brief Positions rows within the viewport.
     */
    void setRowPositions();

    /**
     * @brief Called when this component's parent hierarchy changes, used to initialize rows and connections.
     */
    void parentHierarchyChanged() override;

    /**
     * @brief Handles slider value changes.
     * @param changed_slider The slider that changed.
     */
    void sliderValueChanged(Slider* changed_slider) override;

    /**
     * @brief Handles button clicks.
     * @param button The clicked button.
     */
    void buttonClicked(Button* button) override;

    /**
     * @brief Sets all parameter values from a control map.
     * @param controls The map of controls to set.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Initializes OpenGL components.
     * @param open_gl The OpenGL wrapper.
     */
    void initOpenGlComponents(OpenGlWrapper& open_gl) override;

    /**
     * @brief Renders OpenGL components, including animated elements.
     * @param open_gl The OpenGL wrapper.
     * @param animate Whether to animate components.
     */
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Destroys OpenGL components.
     * @param open_gl The OpenGL wrapper.
     */
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /**
     * @brief Called when 'previous' is clicked on the preset selector.
     */
    void prevClicked() override;

    /**
     * @brief Called when 'next' is clicked on the preset selector.
     */
    void nextClicked() override;

    /**
     * @brief Handles mouse down events on text components (for loading browser).
     * @param e The mouse event.
     */
    void textMouseDown(const MouseEvent& e) override;

    /**
     * @brief Sets the LFO phase (no-op in this class).
     * @param phase The new phase.
     */
    void setPhase(float phase) override { }

    /**
     * @brief Handles line editor scrolling for pattern or grid adjustments.
     * @param e The mouse event.
     * @param wheel The wheel details.
     */
    void lineEditorScrolled(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Toggles paint mode for the line editors.
     * @param enabled True if paint mode is enabled.
     * @param temporary_switch True if the switch is temporary.
     */
    void togglePaintMode(bool enabled, bool temporary_switch) override;

    /**
     * @brief Imports an LFO file to the current line editor.
     */
    void importLfo() override;

    /**
     * @brief Exports the current LFO to a file.
     */
    void exportLfo() override;

    /**
     * @brief Called when a file is loaded.
     */
    void fileLoaded() override;

    /**
     * @brief Loads a specific file (LFO configuration).
     * @param file The file to load.
     */
    void loadFile(const File& file) override;

    /**
     * @brief Gets the currently loaded file.
     * @return The current file.
     */
    File getCurrentFile() override { return current_file_; }

    /**
     * @brief Called when the scroll bar moves.
     * @param scroll_bar The scroll bar.
     * @param range_start The start of the new visible range.
     */
    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;

    /**
     * @brief Sets the range of the scroll bar based on the current rows.
     */
    void setScrollBarRange();

    /**
     * @brief Updates the displayed modulations when changes occur.
     */
    void updateModulations();

    /**
     * @brief Updates the modulation value for a given index.
     * @param index The row index to update.
     */
    void updateModulationValue(int index);

    /**
     * @brief Ensures the correct number of modulation rows is displayed based on connectivity.
     */
    void checkNumModulationsShown();

    /**
     * @brief Adds a listener to the modulation matrix.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Called when scrolling starts (from ModulationViewport::Listener).
     */
    void startScroll() override {
      open_gl_critical_section_.enter();
    }

    /**
     * @brief Called when scrolling ends (from ModulationViewport::Listener).
     */
    void endScroll() override {
      open_gl_critical_section_.exit();
    }

    /**
     * @brief Called when the modulation viewport is scrolled (from ModulationViewport::Listener).
     * @param position The new scroll position.
     */
    void modulationScrolled(int position) override {
      setScrollBarRange();
      scroll_bar_->setCurrentRange(position, viewport_.getHeight());
      for (Listener* listener : listeners_)
        listener->modulationsScrolled();
    }

    /**
     * @brief Called when a modulation row is selected (from ModulationMatrixRow::Listener).
     * @param selected_row The row that was selected.
     */
    void rowSelected(ModulationMatrixRow* selected_row) override;

    /**
     * @brief Handles mouse down events for sorting column selection.
     * @param e The mouse event.
     */
    void mouseDown(const MouseEvent& e) override;

  private:
    /**
     * @brief Sorts the modulation rows based on the current sorting criteria.
     */
    void sort();

    /**
     * @brief Gets the height of each row.
     * @return The row height.
     */
    int getRowHeight() { return getSizeRatio() * 34.0f; }

    std::vector<Listener*> listeners_; ///< Registered listeners.

    PopupItems source_popup_items_; ///< Popup items for source selection.
    PopupItems destination_popup_items_; ///< Popup items for destination selection.

    File current_file_; ///< The currently loaded file.
    SortColumn sort_column_; ///< The currently selected sort column.
    bool sort_ascending_; ///< Whether sorting is ascending or descending.
    int selected_index_; ///< The currently selected row index.
    int num_shown_; ///< The number of rows currently shown.
    std::vector<ModulationMatrixRow*> row_order_; ///< The current order of rows.
    std::unique_ptr<OpenGlScrollBar> scroll_bar_; ///< The vertical scroll bar.

    CriticalSection open_gl_critical_section_; ///< Critical section for OpenGL operations.
    std::unique_ptr<ModulationMatrixRow> rows_[vital::kMaxModulationConnections]; ///< All modulation rows.
    std::unique_ptr<LineMapEditor> map_editors_[vital::kMaxModulationConnections]; ///< Associated line editors.
    std::vector<String> source_strings_; ///< All available source strings.
    std::vector<String> destination_strings_; ///< All available destination strings.
    std::unique_ptr<ModulationMeterReadouts> readouts_; ///< Renders modulation meter readouts.

    ModulationViewport viewport_; ///< The main viewport for scrolling the matrix rows.
    Component container_; ///< The container that holds all rows.

    OpenGlImage background_; ///< Background image for the scrollable section.

    std::unique_ptr<PlainTextComponent> remap_name_; ///< Displays the name of the current remap preset.
    std::unique_ptr<PresetSelector> preset_selector_; ///< Preset selector for line editors.
    std::unique_ptr<PaintPatternSelector> paint_pattern_; ///< Pattern selector for painting line maps.

    std::unique_ptr<SynthSlider> grid_size_x_; ///< Horizontal grid size slider.
    std::unique_ptr<SynthSlider> grid_size_y_; ///< Vertical grid size slider.
    std::unique_ptr<OpenGlShapeButton> paint_; ///< Paint mode toggle button.
    std::unique_ptr<OpenGlShapeButton> smooth_; ///< Smooth mode toggle button.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationMatrix)
};

