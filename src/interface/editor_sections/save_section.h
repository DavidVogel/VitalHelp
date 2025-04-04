#pragma once

#include "JuceHeader.h"

#include "load_save.h"
#include "open_gl_multi_quad.h"
#include "overlay.h"
#include "synth_constants.h"
#include "json/json.h"

using json = nlohmann::json;

/**
 * @class SaveSection
 * @brief A UI overlay for saving presets or other files.
 *
 * This class provides a dialog overlay allowing the user to specify a file name,
 * author, style, and comments before saving a preset or other file. It also supports
 * prompting the user when a file already exists (overwrite confirmation).
 */
class SaveSection : public Overlay, public TextEditor::Listener {
  public:
    static constexpr int kSaveWidth = 630;       ///< Base width of the save dialog.
    static constexpr int kSavePresetHeight = 450;///< Base height of the preset save dialog.
    static constexpr int kStylePaddingX = 4;     ///< Horizontal padding for style buttons.
    static constexpr int kStylePaddingY = 4;     ///< Vertical padding for style buttons.
    static constexpr int kStyleButtonHeight = 24;///< Height of each style button.
    static constexpr int kOverwriteWidth = 340;  ///< Width of overwrite confirmation dialog.
    static constexpr int kOverwriteHeight = 160; ///< Height of overwrite confirmation dialog.
    static constexpr int kTextEditorHeight = 37; ///< Height of each text editor component.
    static constexpr int kLabelHeight = 15;      ///< Height of labels.
    static constexpr int kButtonHeight = 40;     ///< Height of buttons.
    static constexpr int kAddFolderHeight = 20;  ///< Height for additional spacing.
    static constexpr int kDivision = 150;        ///< Horizontal division for layout.
    static constexpr int kPaddingX = 25;         ///< Horizontal padding inside the dialog.
    static constexpr int kPaddingY = 20;         ///< Vertical padding inside the dialog.
    static constexpr int kExtraTopPadding = 10;  ///< Extra top padding inside the dialog.

    /**
     * @class Listener
     * @brief Interface for objects interested in the result of the save action.
     */
    class Listener {
      public:
        virtual ~Listener() = default;

        /**
         * @brief Called after a file has been saved successfully.
         * @param preset The saved preset file.
         */
        virtual void save(File preset) = 0;
    };

    /**
     * @brief Constructs a SaveSection with a given name.
     * @param name The name of this overlay section.
     */
    SaveSection(String name);
    virtual ~SaveSection() = default;

    /**
     * @brief Called when the component is resized. Updates layout and positions of all elements.
     */
    void resized() override;

    /**
     * @brief Sets the visibility of this overlay. Adjusts layout when becoming visible.
     * @param should_be_visible True to show the overlay, false to hide.
     */
    void setVisible(bool should_be_visible) override;

    /**
     * @brief Sets the layout and components for the normal save mode (not overwrite).
     */
    void setSaveBounds();

    /**
     * @brief Sets the layout and components for the overwrite confirmation mode.
     */
    void setOverwriteBounds();

    /**
     * @brief Configures text editor colors and placeholder text.
     * @param editor Pointer to the OpenGlTextEditor to configure.
     * @param empty_string The placeholder text to show when empty.
     */
    void setTextColors(OpenGlTextEditor* editor, String empty_string);

    /**
     * @brief Called when the return key is pressed in any text editor.
     * @param editor Reference to the text editor that triggered the event.
     */
    void textEditorReturnKeyPressed(TextEditor& editor) override;

    /**
     * @brief Called when a button is clicked.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Called when the mouse is released. Used to close the overlay if clicked outside.
     * @param e The MouseEvent details.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Sets the file type label shown in the dialog.
     * @param type The type of file being saved (e.g., "Preset").
     */
    void setFileType(const String& type) { file_type_ = type; repaint(); }

    /**
     * @brief Sets the file extension to use when saving.
     * @param extension The extension (without dot) for the saved file (e.g., "vital").
     */
    void setFileExtension(const String& extension) { file_extension_ = extension; }

    /**
     * @brief Sets the directory where the file will be saved.
     * @param directory The target directory.
     */
    void setDirectory(const File& directory) { file_directory_ = directory; }

    /**
     * @brief Sets the data to be saved (if not saving a preset).
     * @param data A JSON object containing file data.
     */
    void setFileData(const json& data) { file_data_ = data; }

    /**
     * @brief Configures the section for saving a preset (true) or another file type.
     * @param preset True if saving a preset, false otherwise.
     */
    void setIsPreset(bool preset) {
      saving_preset_ = preset;

      if (preset) {
        setFileExtension(vital::kPresetExtension);
        setFileType("Preset");
        setDirectory(LoadSave::getUserPresetDirectory());
      }
    }

    /**
     * @brief Gets the rectangle bounds of the main save dialog.
     * @return The rectangle bounds.
     */
    Rectangle<int> getSaveRect();

    /**
     * @brief Gets the rectangle bounds of the overwrite confirmation dialog.
     * @return The rectangle bounds.
     */
    Rectangle<int> getOverwriteRect();

    /**
     * @brief Adds a listener to be notified when saving occurs.
     * @param listener The listener to add.
     */
    void addSaveListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    /**
     * @brief Handles the actual save process when "Save" or "Overwrite" is triggered.
     */
    void save();

    bool overwrite_;       ///< True if we are showing the overwrite confirmation dialog.
    bool saving_preset_;   ///< True if we are currently saving a preset.

    String file_type_;     ///< Displayed file type name.
    String file_extension_;///< File extension for saved file.
    File file_directory_;  ///< Directory to save the file in.
    json file_data_;       ///< JSON data if saving a non-preset file.

    OpenGlQuad body_;      ///< Background quad for the dialog.

    std::unique_ptr<OpenGlTextEditor> name_;
    std::unique_ptr<OpenGlTextEditor> author_;
    std::unique_ptr<OpenGlTextEditor> comments_;

    std::unique_ptr<OpenGlToggleButton> save_button_;
    std::unique_ptr<OpenGlToggleButton> overwrite_button_;
    std::unique_ptr<OpenGlToggleButton> cancel_button_;

    std::unique_ptr<OpenGlToggleButton> style_buttons_[LoadSave::kNumPresetStyles];

    std::unique_ptr<PlainTextComponent> preset_text_;
    std::unique_ptr<PlainTextComponent> author_text_;
    std::unique_ptr<PlainTextComponent> style_text_;
    std::unique_ptr<PlainTextComponent> comments_text_;
    std::unique_ptr<PlainTextComponent> overwrite_text_;

    std::vector<Listener*> listeners_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SaveSection)
};

