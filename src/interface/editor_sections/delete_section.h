/// @file delete_section.h
/// @brief Declares the DeleteSection class, which provides a confirmation overlay for deleting a preset file.

#pragma once

#include "JuceHeader.h"
#include "overlay.h"

/// @class DeleteSection
/// @brief An overlay that asks the user to confirm deletion of a preset file.
///
/// The DeleteSection displays a message and the name of the preset file to be deleted,
/// along with "Delete" and "Cancel" buttons. If the user confirms, it deletes the file
/// and notifies registered listeners.
class DeleteSection : public Overlay {
public:
    /// Width of the delete confirmation box.
    static constexpr int kDeleteWidth = 340;
    /// Height of the delete confirmation box.
    static constexpr int kDeleteHeight = 140;
    /// The height of the text within the confirmation box.
    static constexpr int kTextHeight = 15;
    /// Horizontal padding inside the confirmation box.
    static constexpr int kPaddingX = 25;
    /// Vertical padding inside the confirmation box.
    static constexpr int kPaddingY = 20;
    /// Height of the buttons inside the confirmation box.
    static constexpr int kButtonHeight = 30;

    /// @class Listener
    /// @brief Interface for objects that need to respond when a file is deleted.
    class Listener {
    public:
        /// Virtual destructor for proper cleanup.
        virtual ~Listener() = default;

        /// Called when a file is successfully deleted.
        /// @param save_file The file that was deleted.
        virtual void fileDeleted(File save_file) = 0;
    };

    /// Constructor.
    /// @param name The name of this overlay component.
    DeleteSection(const String& name);

    /// Destructor.
    virtual ~DeleteSection() = default;

    /// Lays out the components inside the delete confirmation box.
    void resized() override;

    /// Sets the visibility of this component, and repaints if becoming visible.
    /// @param should_be_visible True to show, false to hide.
    void setVisible(bool should_be_visible) override;

    /// Handles mouse-up events. If clicked outside the confirmation box, hides the overlay.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Handles button clicks for the "Delete" and "Cancel" buttons.
    /// @param clicked_button The button that was clicked.
    void buttonClicked(Button* clicked_button) override;

    /// Sets the file that will be deleted if the user confirms.
    /// @param file The file to delete.
    void setFileToDelete(File file) {
        file_ = std::move(file);
        preset_text_->setText(file_.getFileNameWithoutExtension());
    }

    /// Gets the rectangle of the delete confirmation box.
    /// @return The rectangle representing the confirmation box area.
    Rectangle<int> getDeleteRect();

    /// Adds a listener to be notified when the file is deleted.
    /// @param listener The listener to add.
    void addDeleteListener(Listener* listener) { listeners_.add(listener); }

    /// Removes a previously added delete listener.
    /// @param listener The listener to remove.
    void removeDeleteListener(Listener* listener) { listeners_.removeAllInstancesOf(listener); }

private:
    File file_; ///< The file to be deleted if the user confirms.

    OpenGlQuad body_; ///< Background quad for the confirmation box.

    std::unique_ptr<PlainTextComponent> delete_text_; ///< Instruction text: "Are you sure..."
    std::unique_ptr<PlainTextComponent> preset_text_; ///< Displays the name of the preset file to delete.

    std::unique_ptr<OpenGlToggleButton> delete_button_; ///< "Delete" confirmation button.
    std::unique_ptr<OpenGlToggleButton> cancel_button_; ///< "Cancel" button.

    Array<Listener*> listeners_; ///< Registered listeners to notify after deletion.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeleteSection)
};
