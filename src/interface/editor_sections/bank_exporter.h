/// @file bank_exporter.h
/// @brief Declares the ContentList and BankExporter classes for exporting banks of presets, wavetables, LFOs, and samples.

#pragma once

#include "JuceHeader.h"
#include "delete_section.h"
#include "load_save.h"
#include "open_gl_image.h"
#include "overlay.h"
#include "save_section.h"
#include "synth_section.h"

/// @class ContentList
/// @brief Displays a list of files (presets, wavetables, LFOs, samples) that can be selected and exported as a bank.
///
/// The ContentList class supports sorting by name, date, and selection status. It uses an OpenGL-based
/// rendering strategy for scrolling through potentially large lists of files efficiently.
class ContentList : public SynthSection, ScrollBar::Listener {
  public:
    /// @class Listener
    /// @brief Interface for objects that need to respond to changes in selected presets.
    class Listener {
      public:
        virtual ~Listener() { }

        /// Called when the user selection of presets changes.
        virtual void selectedPresetsChanged() = 0;
    };

    /// Columns used in the list for sorting and display.
    enum Column {
        kNone,    ///< No column
        kAdded,   ///< Column representing selection status
        kName,    ///< Column representing file name
        kDate,    ///< Column representing file date
        kNumColumns
    };

    /// Number of rows to keep cached.
    static constexpr int kNumCachedRows = 40;
    /// Height of each row.
    static constexpr float kRowHeight = 26.0f;
    /// Width ratio allocated to the "add" (selection) column.
    static constexpr float kAddWidthRatio = 0.04f;
    /// Width ratio allocated to the name column.
    static constexpr float kNameWidthRatio = 0.76f;
    /// Width ratio allocated to the date column.
    static constexpr float kDateWidthRatio = 0.2f;
    /// Scroll sensitivity factor.
    static constexpr float kScrollSensitivity = 200.0f;

    /// Comparator classes for sorting files by name or date.
    class FileNameAscendingComparator {
      public:
        static int compareElements(const File& first, const File& second) {
          String first_name = first.getFileNameWithoutExtension().toLowerCase();
          String second_name = second.getFileNameWithoutExtension().toLowerCase();
          return first_name.compareNatural(second_name);
        }
    };

    class FileNameDescendingComparator {
      public:
        static int compareElements(const File& first, const File& second) {
          return -FileNameAscendingComparator::compareElements(first, second);
        }
    };

    class FileDateAscendingComparator {
      public:
        static int compareElements(const File& first, const File& second) {
          RelativeTime relative_time = first.getCreationTime() - second.getCreationTime();
          double days = relative_time.inDays();
          return days < 0.0 ? 1 : (days > 0.0f ? -1 : 0);
        }
    };

    class FileDateDescendingComparator {
      public:
        static int compareElements(const File& first, const File& second) {
          return -FileDateAscendingComparator::compareElements(first, second);
        }
    };

    /// Comparator for sorting by whether a file is selected or not.
    class SelectedComparator {
      public:
        SelectedComparator(std::set<std::string> selected, bool ascending) :
            selected_(std::move(selected)), ascending_(ascending) { }

        inline bool isSelected(const File& file) {
          return selected_.count(file.getFullPathName().toStdString());
        }

        int compareElements(const File& first, const File& second) {
          int order_value = ascending_ ? 1 : -1;
          if (isSelected(first)) {
            if (isSelected(second))
              return 0;
            return -order_value;
          }
          if (isSelected(second))
            return order_value;
          return 0;
        }

      private:
        std::set<std::string> selected_;
        bool ascending_;
    };

    /// Constructor.
    /// @param name The name of this ContentList.
    ContentList(const std::string& name);

    /// Paints the background including column headers.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints a background shadow for better visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }

    /// Resizes and lays out child components.
    void resized() override;

    /// Sets the contents of the list.
    /// @param presets The array of files to display.
    void setContent(Array<File> presets);

    /// Handles mouse wheel events for scrolling.
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /// Gets the row index from a mouse position.
    /// @param mouse_position The y-position of the mouse.
    /// @return The row index at that position.
    int getRowFromPosition(float mouse_position);

    /// Gets the row height in pixels.
    /// @return The row height.
    int getRowHeight() { return kRowHeight * size_ratio_; }

    /// Handles mouse move events for hover effects.
    void mouseMove(const MouseEvent& e) override;

    /// Handles mouse exit events to clear hover state.
    void mouseExit(const MouseEvent& e) override;

    /// Handles mouse down events for selection and sorting actions.
    void mouseDown(const MouseEvent& e) override;

    /// Called when the scrollbar position changes.
    /// @param scroll_bar The scrollbar that moved.
    /// @param range_start The new start position.
    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;

    /// Updates the scrollbar range based on content size and view position.
    void setScrollBarRange();

    /// Reloads cached rows after content changes.
    void redoCache();

    /// Gets the total scrollable range in pixels.
    /// @return The scrollable range.
    int getScrollableRange();

    /// Initializes the OpenGL components.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    void initOpenGlComponents(OpenGlWrapper& open_gl) override;

    /// Renders OpenGL components each frame.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate Whether to animate transitions.
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /// Destroys OpenGL components.
    /// @param open_gl The OpenGlWrapper.
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /// Adds a listener to be notified when selection changes.
    /// @param listener The listener to add.
    void addListener(Listener* listener) {
      listeners_.push_back(listener);
    }

    /// Gets a set of selected file paths.
    /// @return The set of selected file paths.
    std::set<std::string> selectedFiles() { return selected_files_; };

  private:
    /// Updates the view position and reloads cached rows if needed.
    void viewPositionChanged();

    /// Gets the current vertical view position in pixels.
    /// @return The current vertical offset.
    int getViewPosition() {
      int view_height = getHeight() - getTitleWidth();
      return std::max(0, std::min<int>(contents_.size() * getRowHeight() - view_height, view_position_));
    }

    /// Loads the specified range of rows into the cache.
    /// @param start_index The start row index.
    /// @param end_index The end row index.
    void loadBrowserCache(int start_index, int end_index);

    /// Positions a quad (highlight or hover) at a specific row.
    /// @param quad The quad to position.
    /// @param index The quad index.
    /// @param row The row to position at.
    /// @param y_offset The vertical offset factor.
    void moveQuadToRow(OpenGlMultiQuad* quad, int index, int row, float y_offset);

    /// Sorts the contents according to the current sort column and order.
    void sort();

    /// Finalizes selection of highlighted files.
    /// @param clicked_index The index of the clicked row.
    void selectHighlighted(int clicked_index);

    /// Handles click to highlight selection.
    /// @param e The mouse event.
    /// @param clicked_index The row clicked.
    void highlightClick(const MouseEvent& e, int clicked_index);

    /// Selects a range of files from the last selected index to the clicked index.
    /// @param clicked_index The row clicked.
    void selectRange(int clicked_index);

    std::vector<Listener*> listeners_;        ///< Listeners for selection changes.
    Array<File> contents_;                    ///< The files displayed in the list.
    int num_contents_;                        ///< Number of files.
    std::set<std::string> selected_files_;    ///< Set of selected files.
    std::set<std::string> highlighted_files_; ///< Set of highlighted files.
    std::unique_ptr<OpenGlScrollBar> scroll_bar_; ///< Scrollbar for the list.
    int last_selected_index_;                 ///< Last clicked index for shift-selection.
    int hover_index_;                         ///< Row index under mouse hover, or -1 if none.

    Component browse_area_;                   ///< The area where rows are displayed.
    int cache_position_;                      ///< Start position of cached rows.
    float view_position_;                     ///< Current vertical scroll position.
    Column sort_column_;                      ///< Current column used for sorting.
    bool sort_ascending_;                     ///< True if sorting ascending, false if descending.

    OpenGlImage rows_[kNumCachedRows];        ///< Cached row images for performance.
    bool selected_[kNumCachedRows];           ///< Whether each cached row is selected.
    OpenGlMultiQuad highlight_;               ///< Overlay to show selected rows.
    OpenGlQuad hover_;                        ///< Overlay to show hover row.
};

/// @class BankExporter
/// @brief A UI component for exporting a selection of presets, wavetables, LFOs, and samples as a bank.
///
/// The BankExporter presents multiple ContentList components for selecting items
/// to include in a bank. Users can specify a bank name and export a ZIP file containing
/// the selected items.
class BankExporter : public SynthSection, public TextEditor::Listener, public KeyListener {
  public:
    /// @class Listener
    /// @brief Interface for objects that need to respond to the BankExporter being hidden.
    class Listener {
      public:
        virtual ~Listener() = default;

        /// Called when the BankExporter should be hidden.
        virtual void hideBankExporter() = 0;
    };

    /// Constructor.
    BankExporter();
    /// Destructor.
    ~BankExporter();

    /// Paints the background of the BankExporter area.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints background shadows for visual depth.
    /// @param g The graphics context.
    void paintBackgroundShadow(Graphics& g) override;

    /// Resizes and lays out child components.
    void resized() override;

    /// Handles key presses.
    /// @param key The key press.
    /// @param origin The originating component.
    /// @return True if handled.
    bool keyPressed(const KeyPress &key, Component *origin) override;

    /// Handles changes in key state.
    /// @param is_key_down Whether a key is down.
    /// @param origin The originating component.
    /// @return True if handled.
    bool keyStateChanged(bool is_key_down, Component *origin) override;

    /// Called when visibility changes, loads files if becoming visible.
    void visibilityChanged() override;

    /// Handles button click events, specifically the "Export Bank" button.
    /// @param clicked_button The button clicked.
    void buttonClicked(Button* clicked_button) override;

    /// Handles text editor changes, enabling or disabling export based on bank name.
    /// @param editor The text editor.
    void textEditorTextChanged(TextEditor& editor) override;

    /// Adds a listener to be notified when the exporter is hidden.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    /// Sets the button colors based on the enabled state.
    void setButtonColors();

    /// Performs the bank export, creating a ZIP file with selected items.
    void exportBank();

    /// Loads files from the user's directories into the respective lists.
    void loadFiles();

    std::unique_ptr<ContentList> preset_list_;    ///< List of user presets.
    std::unique_ptr<ContentList> wavetable_list_; ///< List of user wavetables.
    std::unique_ptr<ContentList> lfo_list_;       ///< List of user LFO shapes.
    std::unique_ptr<ContentList> sample_list_;    ///< List of user samples.

    std::unique_ptr<OpenGlTextEditor> bank_name_box_; ///< Bank name input field.
    std::unique_ptr<OpenGlToggleButton> export_bank_button_; ///< "Export Bank" button.

    std::vector<Listener*> listeners_;           ///< Registered listeners to notify when hidden.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BankExporter)
};
