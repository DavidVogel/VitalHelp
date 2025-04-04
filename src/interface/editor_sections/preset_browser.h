#pragma once

#include "JuceHeader.h"
#include "delete_section.h"
#include "load_save.h"
#include "open_gl_image.h"
#include "open_gl_image_component.h"
#include "open_gl_multi_image.h"
#include "open_gl_multi_quad.h"
#include "overlay.h"
#include "popup_browser.h"
#include "save_section.h"
#include "synth_section.h"

/**
 * @class PresetInfoCache
 * @brief A cache for preset metadata such as author and style for faster repeated lookups.
 *
 * This class caches the author and style of presets to avoid multiple file reads.
 */
class PresetInfoCache {
  public:
    /**
     * @brief Retrieves the author of a given preset, caching the result.
     * @param preset The preset file.
     * @return The author name as a std::string.
     */
    std::string getAuthor(const File& preset) {
      std::string path = preset.getFullPathName().toStdString();
      if (author_cache_.count(path) == 0)
        author_cache_[path] = LoadSave::getAuthorFromFile(preset).toStdString();

      return author_cache_[path];
    }

    /**
     * @brief Retrieves the style of a given preset, caching the result.
     * @param preset The preset file.
     * @return The style (in lowercase) as a std::string.
     */
    std::string getStyle(const File& preset) {
      std::string path = preset.getFullPathName().toStdString();
      if (style_cache_.count(path) == 0)
        style_cache_[path] = LoadSave::getStyleFromFile(preset).toLowerCase().toStdString();

      return style_cache_[path];
    }

  private:
    std::map<std::string, std::string> author_cache_;
    std::map<std::string, std::string> style_cache_;
};

/**
 * @class PresetList
 * @brief A UI component displaying a list of presets with sorting, filtering, and favorite management.
 *
 * The PresetList shows presets in a table-like layout with columns for name, style, author, date, and a favorite star.
 * Users can filter, sort, and rename presets, as well as toggle favorites and select presets.
 */
class PresetList : public SynthSection, public TextEditor::Listener, ScrollBar::Listener {
  public:
    /**
     * @class Listener
     * @brief Interface for receiving preset selection and deletion requests.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when a new preset is selected.
         * @param preset The selected preset file.
         */
        virtual void newPresetSelected(File preset) = 0;

        /**
         * @brief Called when a preset deletion is requested.
         * @param preset The preset file to delete.
         */
        virtual void deleteRequested(File preset) = 0;
    };

    /** Columns in the preset list. */
    enum Column {
      kNone,
      kStar,
      kName,
      kStyle,
      kAuthor,
      kDate,
      kNumColumns
    };

    /** Context menu actions for a selected preset. */
    enum MenuOptions {
      kCancel,
      kOpenFileLocation,
      kRename,
      kDelete,
      kNumMenuOptions
    };

    // Layout and scaling constants.
    static constexpr int kNumCachedRows = 50;
    static constexpr float kRowSizeHeightPercent = 0.04f;
    static constexpr float kStarWidthPercent = 0.04f;
    static constexpr float kNameWidthPercent = 0.35f;
    static constexpr float kStyleWidthPercent = 0.18f;
    static constexpr float kAuthorWidthPercent = 0.25f;
    static constexpr float kDateWidthPercent = 0.18f;
    static constexpr float kScrollSensitivity = 200.0f;

    // Various comparators for sorting presets.
    class FileNameAscendingComparator {
      public:
        static int compareElements(File first, File second) {
          String first_name = first.getFileNameWithoutExtension().toLowerCase();
          String second_name = second.getFileNameWithoutExtension().toLowerCase();
          return first_name.compareNatural(second_name);
        }
    };

    class FileNameDescendingComparator {
      public:
        static int compareElements(File first, File second) {
          return FileNameAscendingComparator::compareElements(second, first);
        }
    };

    class AuthorAscendingComparator {
      public:
        AuthorAscendingComparator(PresetInfoCache* preset_cache) : cache_(preset_cache) { }

        int compareElements(File first, File second) {
          String first_author = cache_->getAuthor(first);
          String second_author = cache_->getAuthor(second);
          return first_author.compareNatural(second_author);
        }

      private:
        PresetInfoCache* cache_;
    };

    class AuthorDescendingComparator {
      public:
        AuthorDescendingComparator(PresetInfoCache* preset_cache) : cache_(preset_cache) { }

        int compareElements(File first, File second) {
          String first_author = cache_->getAuthor(first);
          String second_author = cache_->getAuthor(second);
          return -first_author.compareNatural(second_author);
        }

      private:
        PresetInfoCache* cache_;
    };

    class StyleAscendingComparator {
      public:
        StyleAscendingComparator(PresetInfoCache* preset_cache) : cache_(preset_cache) { }

        int compareElements(File first, File second) {
          String first_style = cache_->getStyle(first);
          String second_style = cache_->getStyle(second);
          return first_style.compareNatural(second_style);
        }

      private:
        PresetInfoCache* cache_;
    };

    class StyleDescendingComparator {
      public:
        StyleDescendingComparator(PresetInfoCache* preset_cache) : cache_(preset_cache) { }

        int compareElements(File first, File second) {
          String first_style = cache_->getStyle(first);
          String second_style = cache_->getStyle(second);
          return -first_style.compareNatural(second_style);
        }

      private:
        PresetInfoCache* cache_;
    };

    class FileDateAscendingComparator {
      public:
        static int compareElements(File first, File second) {
          RelativeTime relative_time = first.getCreationTime() - second.getCreationTime();
          double days = relative_time.inDays();
          return days < 0.0 ? 1 : (days > 0.0f ? -1 : 0);
        }
    };

    class FileDateDescendingComparator {
      public:
        static int compareElements(File first, File second) {
          return FileDateAscendingComparator::compareElements(second, first);
        }
    };

    class FavoriteComparator {
      public:
        FavoriteComparator() {
          favorites_ = LoadSave::getFavorites();
        }

        bool isFavorite(const File& file) {
          return favorites_.count(file.getFullPathName().toStdString());
        }

        int compare(File first, File second) {
          if (isFavorite(first)) {
            if (isFavorite(second))
              return 0;
            return -1;
          }
          if (isFavorite(second))
            return 1;
          return 0;
        }

      private:
        std::set<std::string> favorites_;
    };

    class FavoriteAscendingComparator : public FavoriteComparator {
      public:
        int compareElements(File first, File second) {
          return compare(first, second);
        }
    };

    class FavoriteDescendingComparator : public FavoriteComparator {
      public:
        int compareElements(File first, File second) {
          return compare(second, first);
        }
    };

    /**
     * @brief Constructs a PresetList.
     */
    PresetList();

    void paintBackground(Graphics& g) override;
    void paintBackgroundShadow(Graphics& g) override { paintTabShadow(g); }
    void resized() override;

    /**
     * @brief Sets the array of presets to display.
     * @param presets An array of File objects representing presets.
     */
    void setPresets(Array<File> presets);
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /**
     * @brief Converts a mouse Y position into a row index.
     * @param mouse_position The Y coordinate.
     * @return The row index, or -1 if out of range.
     */
    int getRowFromPosition(float mouse_position);

    /**
     * @brief Gets the row height in pixels.
     * @return The scaled row height.
     */
    int getRowHeight() { return getHeight() * kRowSizeHeightPercent; }
    void mouseMove(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;

    /**
     * @brief Handles actions from the preset context menu.
     * @param result The selected menu option ID.
     */
    void respondToMenuCallback(int result);

    /**
     * @brief Called on right-click to show a context menu.
     * @param e The mouse event.
     */
    void menuClick(const MouseEvent& e);

    /**
     * @brief Called on left-click to select, rename, or favorite a preset.
     * @param e The mouse event.
     */
    void leftClick(const MouseEvent& e);

    void mouseDown(const MouseEvent& e) override;

    // TextEditor callbacks for renaming.
    void textEditorReturnKeyPressed(TextEditor& text_editor) override;
    void textEditorFocusLost(TextEditor& text_editor) override;
    void textEditorEscapeKeyPressed(TextEditor& editor) override;

    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;
    void setScrollBarRange();

    /**
     * @brief Finalizes a preset rename operation.
     */
    void finishRename();

    /**
     * @brief Reloads the currently displayed presets from disk.
     */
    void reloadPresets();

    /**
     * @brief Moves the selected preset up or down by a number of indices.
     * @param indices The number of steps to move.
     */
    void shiftSelectedPreset(int indices);

    /**
     * @brief Updates the cached images for rows after sorting or filtering.
     */
    void redoCache();

    /**
     * @brief Filters the displayed presets by name, author, and styles.
     * @param filter_string The string to filter by.
     * @param styles A set of styles to include.
     */
    void filter(String filter_string, const std::set<std::string>& styles);

    /**
     * @brief Gets the index of the currently selected preset.
     * @return The index of the selected preset, or -1 if none selected.
     */
    int getSelectedIndex();

    /**
     * @brief Gets the scrollable height of the presets.
     * @return The total scrollable range in pixels.
     */
    int getScrollableRange();

    void initOpenGlComponents(OpenGlWrapper& open_gl) override;
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /**
     * @brief Adds a listener for preset events.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) {
      listeners_.push_back(listener);
    }

    /**
     * @brief Sets the current folder to display presets from.
     * @param folder The directory containing presets.
     */
    void setCurrentFolder(const File& folder) {
      current_folder_ = folder;
      reloadPresets();
    }

  private:
    void viewPositionChanged();
    int getViewPosition() {
      int view_height = getHeight() - getTitleWidth();
      return std::max(0, std::min<int>(num_view_presets_ * getRowHeight() - view_height, view_position_));
    }
    void loadBrowserCache(int start_index, int end_index);
    void moveQuadToRow(OpenGlQuad& quad, int row, float y_offset);
    void sort();

    std::vector<Listener*> listeners_;
    Array<File> presets_;
    int num_view_presets_;
    std::vector<File> filtered_presets_;
    std::set<std::string> favorites_;
    std::unique_ptr<OpenGlTextEditor> rename_editor_;
    std::unique_ptr<OpenGlScrollBar> scroll_bar_;
    String filter_string_;
    std::set<std::string> filter_styles_;
    File selected_preset_;
    File renaming_preset_;
    File current_folder_;
    int hover_preset_;
    int click_preset_;

    PresetInfoCache preset_info_cache_;

    Component browse_area_;
    int cache_position_;
    OpenGlImage rows_[kNumCachedRows];
    OpenGlQuad highlight_;
    OpenGlQuad hover_;
    float view_position_;
    Column sort_column_;
    bool sort_ascending_;
};

/**
 * @class PresetBrowser
 * @brief A UI for browsing, loading, and organizing presets.
 *
 * The PresetBrowser combines a PresetList (for preset files) and a SelectionList (for folders),
 * and includes searching, filtering by style, displaying metadata (author, comments), and links
 * to browse or purchase more presets.
 */
class PresetBrowser : public SynthSection,
                      public PresetList::Listener,
                      public TextEditor::Listener,
                      public KeyListener,
                      public SaveSection::Listener,
                      public DeleteSection::Listener,
                      public SelectionList::Listener {
  public:
    static constexpr int kLeftPadding = 24;
    static constexpr int kTopPadding = 24;
    static constexpr int kMiddlePadding = 15;
    static constexpr int kNameFontHeight = 26;
    static constexpr int kAuthorFontHeight = 19;
    static constexpr int kStoreHeight = 33;
    static constexpr int kCommentsFontHeight = 15;

    /**
     * @class Listener
     * @brief Interface for events from the PresetBrowser (e.g. preset selected, deleted, or browser hidden).
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when a new preset is selected.
         * @param preset The selected preset file.
         */
        virtual void newPresetSelected(File preset) = 0;

        /**
         * @brief Called when a preset is requested to be deleted.
         * @param preset The preset file to delete.
         */
        virtual void deleteRequested(File preset) = 0;

        /**
         * @brief Called when the preset browser should be hidden.
         */
        virtual void hidePresetBrowser() = 0;
    };

    /**
     * @brief Constructs a PresetBrowser.
     */
    PresetBrowser();
    /** @brief Destructor. */
    ~PresetBrowser();

    void paintBackground(Graphics& g) override;
    void paintBackgroundShadow(Graphics& g) override;
    void resized() override;
    void buttonClicked(Button* clicked_button) override;
    bool keyPressed(const KeyPress &key, Component *origin) override;
    bool keyStateChanged(bool is_key_down, Component *origin) override;
    void visibilityChanged() override;

    /**
     * @brief Returns the rectangle reserved for the search area.
     * @return The search rectangle.
     */
    Rectangle<int> getSearchRect();

    /**
     * @brief Returns the rectangle reserved for the preset info area.
     * @return The info rectangle.
     */
    Rectangle<int> getInfoRect();

    /**
     * @brief Filters the displayed presets based on search text and selected styles.
     */
    void filterPresets();

    void textEditorTextChanged(TextEditor& editor) override;
    void textEditorEscapeKeyPressed(TextEditor& editor) override;

    void newPresetSelected(File preset) override {
      for (Listener* listener : listeners_)
        listener->newPresetSelected(preset);
      loadPresetInfo();

      String author = author_text_->getText();
      store_button_->setText("Get more presets by " + author);
      bool visible = more_author_presets_.count(author.removeCharacters(" _.").toLowerCase().toStdString());
      bool was_visible = store_button_->isVisible();
      store_button_->setVisible(visible);
      if (was_visible != visible)
        setCommentsBounds();
    }

    void deleteRequested(File preset) override {
      for (Listener* listener : listeners_)
        listener->deleteRequested(preset);
    }

    /**
     * @brief Loads all presets from known directories.
     */
    void loadPresets();

    /**
     * @brief Called after saving a preset.
     * @param preset The saved preset file.
     */
    void save(File preset) override;

    /**
     * @brief Called after a file is deleted.
     * @param deleted_file The deleted file.
     */
    void fileDeleted(File deleted_file) override;

    /**
     * @brief Jumps to a preset a certain number of steps away.
     * @param indices The number of steps (negative for previous).
     */
    void jumpToPreset(int indices);

    /**
     * @brief Loads the next preset in the list.
     */
    void loadNextPreset();

    /**
     * @brief Loads the previous preset in the list.
     */
    void loadPrevPreset();

    /**
     * @brief Loads an external preset file and sets it as the current selection.
     * @param file The external preset file.
     */
    void externalPresetLoaded(File file);

    /**
     * @brief Clears the reference to any external preset.
     */
    void clearExternalPreset() { external_preset_ = File(); }

    /**
     * @brief Adds a listener to receive events from the PresetBrowser.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener);

    /**
     * @brief Sets the SaveSection for handling preset saving.
     * @param save_section The SaveSection instance.
     */
    void setSaveSection(SaveSection* save_section);

    /**
     * @brief Sets the DeleteSection for handling preset deletion.
     * @param delete_section The DeleteSection instance.
     */
    void setDeleteSection(DeleteSection* delete_section);

    void newSelection(File selection) override;
    void allSelected() override;
    void favoritesSelected() override;
    void doubleClickedSelected(File selection) override { }

  private:
    bool loadFromFile(File& preset);
    void loadPresetInfo();
    void setCommentsBounds();
    void setPresetInfo(File& preset);

    std::vector<Listener*> listeners_;
    std::unique_ptr<PresetList> preset_list_;
    std::unique_ptr<OpenGlTextEditor> search_box_;
    std::unique_ptr<SelectionList> folder_list_;
    std::unique_ptr<PlainTextComponent> preset_text_;
    std::unique_ptr<PlainTextComponent> author_text_;
    std::unique_ptr<OpenGlToggleButton> style_buttons_[LoadSave::kNumPresetStyles];
    std::unique_ptr<OpenGlToggleButton> store_button_;

    SaveSection* save_section_;
    DeleteSection* delete_section_;

    std::unique_ptr<OpenGlTextEditor> comments_;
    File external_preset_;
    String author_;
    String license_;
    std::set<std::string> more_author_presets_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
