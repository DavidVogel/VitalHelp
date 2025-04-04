#pragma once

#include "JuceHeader.h"
#include "delete_section.h"
#include "load_save.h"
#include "open_gl_image.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_image.h"
#include "open_gl_multi_quad.h"
#include "overlay.h"
#include "save_section.h"
#include "synth_section.h"

/**
 * @class PopupDisplay
 * @brief A small popup component that displays text in a styled bubble.
 *
 * This class is used for showing brief textual popups, similar to tooltips,
 * with configurable placement around a given UI element.
 */
class PopupDisplay : public SynthSection {
  public:
    /** @brief Constructs a PopupDisplay. */
    PopupDisplay();

    /** @brief Called when the component is resized. */
    void resized() override;

    /**
     * @brief Sets the content of the popup display.
     * @param text The text to display.
     * @param bounds The target bounds around which the popup is positioned.
     * @param placement The placement of the popup relative to the target.
     */
    void setContent(const std::string& text, Rectangle<int> bounds, BubbleComponent::BubblePlacement placement);

  private:
    PlainTextComponent text_;
    OpenGlQuad body_;
    OpenGlQuad border_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupDisplay)
};

/**
 * @class PopupList
 * @brief A scrollable, selectable popup list of items.
 *
 * PopupList displays a vertical list of items that can be hovered, selected, and scrolled through.
 * It supports a listener for when a new item is selected or double-clicked.
 */
class PopupList : public SynthSection, ScrollBar::Listener {
  public:
  /**
   * @class Listener
   * @brief Interface for receiving selection events from PopupList.
   */
    class Listener {
      public:
        virtual ~Listener() = default;

      /**
       * @brief Called when a new selection is made.
       * @param list Pointer to this PopupList.
       * @param id The ID of the selected item.
       * @param index The index of the selected item.
       */
      virtual void newSelection(PopupList* list, int id, int index) = 0;

      /**
       * @brief Called when the user double-clicks the selected item.
       * @param list Pointer to this PopupList.
       * @param id The ID of the clicked item.
       * @param index The index of the clicked item.
       */
        virtual void doubleClickedSelected(PopupList* list, int id, int index) { }
    };

    static constexpr float kRowHeight = 24.0f;       ///< Base row height.
    static constexpr float kScrollSensitivity = 200.0f; ///< Scroll sensitivity factor.
    static constexpr float kScrollBarWidth = 15.0f; ///< Width of the scrollbar.

    /** @brief Constructs a PopupList. */
    PopupList();

    void paintBackground(Graphics& g) override { }
    void paintBackgroundShadow(Graphics& g) override { }
    void resized() override;

    /**
     * @brief Sets the list of items to be displayed.
     * @param selections The PopupItems containing the selectable items.
     */
    void setSelections(PopupItems selections);

    /**
     * @brief Retrieves the selection items for a specified index.
     * @param index Index of the selection.
     * @return The PopupItems at the given index.
     */
    PopupItems getSelectionItems(int index) const { return selections_.items[index]; }

    /**
     * @brief Gets the row index corresponding to a given vertical position.
     * @param mouse_position The y-coordinate to translate into a row index.
     * @return The row index at that position.
     */
    int getRowFromPosition(float mouse_position);

    /**
     * @brief Returns the row height in current scaling.
     * @return The scaled row height.
     */
    int getRowHeight() { return size_ratio_ * kRowHeight; }

    /**
     * @brief Returns text padding around list items.
     * @return The text padding in pixels.
     */
    int getTextPadding() { return getRowHeight() / 4; }

    /**
     * @brief Calculates the width needed to display all items.
     * @return The width in pixels.
     */
    int getBrowseWidth();

    /**
     * @brief Gets the total height needed to display all items.
     * @return The height in pixels.
     */
    int getBrowseHeight() { return getRowHeight() * selections_.size(); }

    /**
     * @brief Gets the font used for displaying items.
     * @return The Font object.
     */
    Font getFont() {
      return Fonts::instance()->proportional_light().withPointHeight(getRowHeight() * 0.55f * getPixelMultiple());
    }
    void mouseMove(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Retrieves the selection index at a mouse event position.
     * @param e The mouse event.
     * @return The index of the selection, or -1 if none.
     */
    int getSelection(const MouseEvent& e);

    void mouseUp(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;

    /**
     * @brief Sets the currently selected item by index.
     * @param selection The index of the item to select.
     */
    void setSelected(int selection) { selected_ = selection; }

    /**
     * @brief Gets the currently selected item index.
     * @return The index of the selected item, or -1 if none.
     */
    int getSelected() const { return selected_; }
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    void resetScrollPosition();
    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;
    void setScrollBarRange();

    /**
     * @brief Gets the total scrollable range in pixels.
     * @return The scrollable range.
     */
    int getScrollableRange();

    void initOpenGlComponents(OpenGlWrapper& open_gl) override;
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /**
     * @brief Adds a PopupList::Listener to receive selection events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) {
      listeners_.push_back(listener);
    }

    /**
     * @brief Enables or disables showing a highlight for the selected row.
     * @param show True to show selection highlight, false to hide.
     */
    void showSelected(bool show) { show_selected_ = show; }

    /**
     * @brief Programmatically selects an item by its index.
     * @param select The index of the item to select.
     */
    void select(int select);

  private:
    int getViewPosition() {
      int view_height = getHeight();
      return std::max(0, std::min<int>(selections_.size() * getRowHeight() - view_height, view_position_));
    }
    void redoImage();
    void moveQuadToRow(OpenGlQuad& quad, int row);

    std::vector<Listener*> listeners_;
    PopupItems selections_;
    int selected_;
    int hovered_;
    bool show_selected_;

    float view_position_;
    std::unique_ptr<OpenGlScrollBar> scroll_bar_;
    OpenGlImage rows_;
    OpenGlQuad highlight_;
    OpenGlQuad hover_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupList)
};

/**
 * @class SelectionList
 * @brief A scrollable file/folder selection list that supports nested folders and favorites.
 *
 * SelectionList displays folders and files that can be opened or selected. It supports
 * filtering, additional folders, favorites, and user interaction such as adding or removing
 * folders. Listeners can be notified about user selections.
 */
class SelectionList : public SynthSection, ScrollBar::Listener {
  public:
    /**
     * @class Listener
     * @brief Interface for receiving selection events from SelectionList.
     */
    class Listener {
      public:
        virtual ~Listener() = default;

        /**
         * @brief Called when a new File is selected.
         * @param selection The newly selected file.
         */
        virtual void newSelection(File selection) = 0;

        /**
         * @brief Called when "All" special selection is made.
         */
        virtual void allSelected() { }

        /**
         * @brief Called when "Favorites" special selection is made.
         */
        virtual void favoritesSelected() { }

        /**
         * @brief Called when a file is double-clicked.
         * @param selection The double-clicked file.
         */
        virtual void doubleClickedSelected(File selection) = 0;
    };

    static constexpr int kNumCachedRows = 50;    ///< Number of rows cached for performance.
    static constexpr float kRowHeight = 24.0f;   ///< Base row height.
    static constexpr float kStarWidth = 38.0f;   ///< Width of the star icon area.
    static constexpr float kScrollSensitivity = 200.0f; ///< Scroll sensitivity.
    static constexpr float kScrollBarWidth = 15.0f;     ///< Scrollbar width.

    /**
     * @brief Returns a File object representing "All" selection.
     * @return Special "All" File.
     */
    static File getAllFile() { return File::getSpecialLocation(File::tempDirectory).getChildFile("All"); }

    /**
     * @brief Returns a File object representing "Favorites" selection.
     * @return Special "Favorites" File.
     */
    static File getFavoritesFile() { return File::getSpecialLocation(File::tempDirectory).getChildFile("Favorites"); }

    /**
     * @class FileNameAscendingComparator
     * @brief Comparator for sorting files by name ascending.
     */
    class FileNameAscendingComparator {
      public:
        static int compareElements(const File& first, const File& second) {
          String first_name = first.getFullPathName().toLowerCase();
          String second_name = second.getFullPathName().toLowerCase();
          return first_name.compareNatural(second_name);
        }
    };

    /** @brief Constructs a SelectionList. */
    SelectionList();

    void paintBackground(Graphics& g) override { }
    void paintBackgroundShadow(Graphics& g) override { }
    void resized() override;

    /**
     * @brief Adds a "Favorites" option to the list.
     */
    void addFavoritesOption() { favorites_option_ = true; selected_ = getAllFile(); }

    /**
     * @brief Sets the files/folders displayed in this SelectionList.
     * @param presets The array of File objects to display.
     */
    void setSelections(Array<File> presets);
    Array<File> getSelections() { return selections_; }
    Array<File> getAdditionalFolders() { return additional_roots_; }

    void resetScrollPosition();
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;
    int getRowFromPosition(float mouse_position);

    /**
     * @brief Returns the scaled row height.
     * @return The row height.
     */
    int getRowHeight() { return size_ratio_ * kRowHeight; }

    /**
     * @brief Gets the icon padding.
     * @return The icon padding in pixels.
     */
    int getIconPadding() { return getRowHeight() * 0.25f; }

    void mouseMove(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;
    void respondToMenuCallback(int result);
    void menuClick(const MouseEvent& e);
    void leftClick(const MouseEvent& e);

    /**
     * @brief Gets the File at the mouse event position.
     * @param e The mouse event.
     * @return The selected File or an invalid File if none.
     */
    File getSelection(const MouseEvent& e);

    void mouseDown(const MouseEvent& e) override;
    void mouseDoubleClick(const MouseEvent& e) override;
    void addAdditionalFolder();
    void removeAdditionalFolder(const File& folder);

    /**
     * @brief Selects a given file.
     * @param selection The file to select.
     */
    void select(const File& selection);

    /**
     * @brief Returns the currently selected file.
     * @return The selected File.
     */
    File selected() const { return selected_; }

    /**
     * @brief Sets the currently selected file.
     * @param selection The file to set as selected.
     */
    void setSelected(const File& selection) { selected_ = selection; }

    /**
     * @brief Handles selecting the icon area (favorite toggle).
     * @param selection The file associated with the icon clicked.
     */
    void selectIcon(const File& selection);

    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;
    void setScrollBarRange();

    /**
     * @brief Refreshes the cached row images.
     */
    void redoCache();

    /**
     * @brief Gets the depth of a given folder relative to open folders.
     * @param file The folder file.
     * @return The depth level.
     */
    int getFolderDepth(const File& file);

    /**
     * @brief Adds subfolder selections if a folder is open.
     * @param selection The parent folder.
     * @param selections Vector to add the subfolders into.
     */
    void addSubfolderSelections(const File& selection, std::vector<File>& selections);

    /**
     * @brief Sets the name associated with additional roots.
     * @param name The name for additional roots category.
     */
    void setAdditionalRootsName(const std::string& name);

    /**
     * @brief Filters the selection list by a given string.
     * @param filter_string The filter query.
     */
    void filter(const String& filter_string);

    /**
     * @brief Gets the index of the currently selected file.
     * @return The selected index, or -1 if none.
     */
    int getSelectedIndex();

    /**
     * @brief Gets the total scrollable range.
     * @return The total scrollable height in pixels.
     */
    int getScrollableRange();

    /**
     * @brief Selects the next item in the list.
     */
    void selectNext();

    /**
     * @brief Selects the previous item in the list.
     */
    void selectPrev();

    void initOpenGlComponents(OpenGlWrapper& open_gl) override;
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /**
     * @brief Adds a SelectionList::Listener.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) {
      listeners_.push_back(listener);
    }

    void setPassthroughFolderName(const std::string& name) { passthrough_name_ = name; }
    std::string getPassthroughFolderName() const { return passthrough_name_; }

    /**
     * @brief Checks if at least one displayed path exists.
     * @return True if at least one file/folder is valid, false otherwise.
     */
    bool hasValidPath() {
      for (File& selection : selections_) {
        if (selection.exists())
          return true;
      }
      return false;
    }

  private:
    void viewPositionChanged();
    void toggleFavorite(const File& selection);
    void toggleOpenFolder(const File& selection);
    int getViewPosition() {
      int view_height = getHeight();
      return std::max(0, std::min<int>(num_view_selections_ * getRowHeight() - view_height, view_position_));
    }
    void loadBrowserCache(int start_index, int end_index);
    void moveQuadToRow(OpenGlQuad& quad, int row, float y_offset);
    void sort();

    bool favorites_option_;
    std::vector<Listener*> listeners_;
    Array<File> selections_;
    std::string additional_roots_name_;
    Array<File> additional_roots_;
    int num_view_selections_;
    std::vector<File> filtered_selections_;
    std::set<std::string> favorites_;
    std::map<std::string, int> open_folders_;
    std::unique_ptr<OpenGlScrollBar> scroll_bar_;
    String filter_string_;
    File selected_;
    int hovered_;
    bool x_area_;

    Component browse_area_;
    int cache_position_;
    OpenGlImage rows_[kNumCachedRows];
    bool is_additional_[kNumCachedRows];
    OpenGlQuad highlight_;
    OpenGlQuad hover_;
    PlainShapeComponent remove_additional_x_;
    float view_position_;
    std::string passthrough_name_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionList)
};

/**
 * @class SinglePopupSelector
 * @brief A popup for selecting a single item from a list.
 *
 * SinglePopupSelector displays a PopupList inside a styled popup
 * container, allowing the user to choose one item.
 */
class SinglePopupSelector : public SynthSection, public PopupList::Listener {
  public:
    /** @brief Constructs a SinglePopupSelector. */
    SinglePopupSelector();

    void paintBackground(Graphics& g) override { }
    void paintBackgroundShadow(Graphics& g) override { }
    void resized() override;

    void visibilityChanged() override {
      if (isShowing() && isVisible())
        grabKeyboardFocus();
    }

    /**
     * @brief Sets the popup position relative to a given bounding area.
     * @param position The anchor position.
     * @param bounds The bounding rectangle for position adjustment.
     */
    void setPosition(Point<int> position, Rectangle<int> bounds);

    void newSelection(PopupList* list, int id, int index) override {
      if (id >= 0) {
        cancel_ = nullptr;
        callback_(id);
        setVisible(false);
      }
      else
        cancel_();
    }

    void focusLost(FocusChangeType cause) override {
      setVisible(false);
      if (cancel_)
        cancel_();
    }

    /**
     * @brief Sets the callback function called when an item is selected.
     * @param callback The callback function taking an integer ID.
     */
    void setCallback(std::function<void(int)> callback) { callback_ = std::move(callback); }

    /**
     * @brief Sets the callback function called when the selection is cancelled.
     * @param cancel The cancel callback.
     */
    void setCancelCallback(std::function<void()> cancel) { cancel_ = std::move(cancel); }

    /**
     * @brief Displays a set of selections.
     * @param selections The PopupItems to display.
     */
    void showSelections(const PopupItems& selections) {
      popup_list_->setSelections(selections);
    }

  private:
    OpenGlQuad body_;
    OpenGlQuad border_;

    std::function<void(int)> callback_;
    std::function<void()> cancel_;
    std::unique_ptr<PopupList> popup_list_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SinglePopupSelector)
};

/**
 * @class DualPopupSelector
 * @brief A popup for selecting from a hierarchical set of items in two columns.
 *
 * DualPopupSelector shows two PopupLists side-by-side. Selecting an item in the
 * left list updates the right list with related items. Selecting from the right
 * list finalizes the selection.
 */
class DualPopupSelector : public SynthSection, public PopupList::Listener {
  public:
    /** @brief Constructs a DualPopupSelector. */
    DualPopupSelector();

    void paintBackground(Graphics& g) override { }
    void paintBackgroundShadow(Graphics& g) override { }
    void resized() override;
    void visibilityChanged() override {
      if (isShowing() && isVisible())
        grabKeyboardFocus();
    }

    /**
     * @brief Positions the DualPopupSelector.
     * @param position The anchor position.
     * @param width Desired width.
     * @param bounds The bounding area for positioning.
     */
    void setPosition(Point<int> position, int width, Rectangle<int> bounds);

    void newSelection(PopupList* list, int id, int index) override;
    void doubleClickedSelected(PopupList* list, int id, int index) override { setVisible(false); }

    void focusLost(FocusChangeType cause) override { setVisible(false); }

    /**
     * @brief Sets the callback for when a final selection is made.
     * @param callback The callback function taking an integer ID.
     */
    void setCallback(std::function<void(int)> callback) { callback_ = std::move(callback); }

    /**
     * @brief Displays selections in the left list and updates the right list accordingly.
     * @param selections The PopupItems to display in the left list.
     */
    void showSelections(const PopupItems& selections) {
      left_list_->setSelections(selections);

      for (int i = 0; i < selections.size(); ++i) {
        if (selections.items[i].selected)
          right_list_->setSelections(selections.items[i]);
      }
    }

  private:
    OpenGlQuad body_;
    OpenGlQuad border_;
    OpenGlQuad divider_;

    std::function<void(int)> callback_;
    std::unique_ptr<PopupList> left_list_;
    std::unique_ptr<PopupList> right_list_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualPopupSelector)
};

/**
 * @class PopupClosingArea
 * @brief A transparent area that triggers a closing event when clicked.
 *
 * This component is used to close a popup if the user clicks outside it.
 */
class PopupClosingArea : public Component {
  public:
    PopupClosingArea() : Component("Ignore Area") { }

    /**
     * @class Listener
     * @brief Interface for receiving closing area click events.
     */
    class Listener {
      public:
        virtual ~Listener() = default;
        /**
         * @brief Called when the closing area is clicked.
         * @param closing_area Pointer to this PopupClosingArea.
         * @param e The mouse event.
         */
        virtual void closingAreaClicked(PopupClosingArea* closing_area, const MouseEvent& e) = 0;
    };

    void mouseDown(const MouseEvent& e) override {
      for (Listener* listener : listeners_)
        listener->closingAreaClicked(this, e);
    }

    /**
     * @brief Adds a Listener to receive closing events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    std::vector<Listener*> listeners_;
};

/**
 * @class PopupBrowser
 * @brief A popup browser interface for browsing and selecting files (e.g. presets).
 *
 * This class displays folders and files in a split view (folders on the left, files on the right),
 * with search and filtering capabilities, favorite handling, and the ability to add additional folders.
 * It also supports a closing area around it and launching external URLs for additional content.
 */
class PopupBrowser : public SynthSection,
                     public SelectionList::Listener,
                     public TextEditor::Listener,
                     public KeyListener,
                     public PopupClosingArea::Listener {
  public:
    /** @brief Constructs a PopupBrowser. */
    PopupBrowser();
    /** @brief Destructor. */
    ~PopupBrowser();

    void paintBackground(Graphics& g) override { }
    void paintBackgroundShadow(Graphics& g) override { }
    void resized() override;
    void buttonClicked(Button* clicked_button) override;
    void visibilityChanged() override;

    /**
     * @brief Filters the displayed presets based on the search text.
     */
    void filterPresets();
    void textEditorTextChanged(TextEditor& editor) override;
    void textEditorEscapeKeyPressed(TextEditor& editor) override;

    void newSelection(File selection) override;
    void allSelected() override;
    void favoritesSelected() override;
    void doubleClickedSelected(File selection) override;
    void closingAreaClicked(PopupClosingArea* closing_area, const MouseEvent& e) override;

    bool keyPressed(const KeyPress &key, Component *origin) override;
    bool keyStateChanged(bool is_key_down, Component *origin) override;

    /**
     * @brief Checks and updates the UI if there is no content available.
     */
    void checkNoContent();

    /**
     * @brief Checks and updates if the store button should be displayed.
     */
    void checkStoreButton();

    /**
     * @brief Loads presets from a set of directories.
     * @param directories A vector of File directories to load from.
     * @param extensions The file extensions to filter for.
     * @param passthrough_name The name for a passthrough folder (if applicable).
     * @param additional_folders_name The name used for managing additional folders.
     */
    void loadPresets(std::vector<File> directories, const String& extensions,
                     const std::string& passthrough_name, const std::string& additional_folders_name);

    /**
     * @brief Sets the owner SynthSection for which this popup browser was opened.
     * @param owner The SynthSection that owns this popup.
     */
    void setOwner(SynthSection* owner) {
      owner_ = owner;
      if (owner_)
        selection_list_->setSelected(owner_->getCurrentFile());
      checkStoreButton();
    }

    /**
     * @brief Sets the ignored bounds (the "passthrough" area) that won't close the browser.
     * @param bounds The rectangle representing the ignored area.
     */
    void setIgnoreBounds(Rectangle<int> bounds) { passthrough_bounds_ = bounds; resized(); }

    /**
     * @brief Sets the main browser bounds.
     * @param bounds The rectangle representing the browser bounds.
     */
    void setBrowserBounds(Rectangle<int> bounds) { browser_bounds_ = bounds; resized(); }

  private:
    OpenGlQuad body_;
    OpenGlQuad border_;
    OpenGlQuad horizontal_divider_;
    OpenGlQuad vertical_divider_;

    std::unique_ptr<SelectionList> folder_list_;
    std::unique_ptr<SelectionList> selection_list_;
    std::unique_ptr<OpenGlTextEditor> search_box_;
    std::unique_ptr<OpenGlShapeButton> exit_button_;
    std::unique_ptr<OpenGlToggleButton> store_button_;
    std::unique_ptr<OpenGlToggleButton> download_button_;
    Rectangle<int> passthrough_bounds_;
    Rectangle<int> browser_bounds_;
    PopupClosingArea closing_areas_[4];

    SynthSection* owner_;
    String extensions_;
    String author_;
    std::set<std::string> more_author_presets_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupBrowser)
};
