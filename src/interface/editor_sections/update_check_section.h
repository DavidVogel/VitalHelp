#pragma once

#include "JuceHeader.h"
#include "overlay.h"

/**
 * @class UpdateMemory
 * @brief A singleton class that keeps track of whether an update check should be performed.
 *
 * This class uses a reference counting mechanism (checkers_) to determine if any section
 * is interested in checking for updates. If no checkers are active, the update check might
 * be skipped. Once a section increments the checker count, update checks may proceed.
 */
class UpdateMemory : public DeletedAtShutdown {
public:
    /**
     * @brief Constructs the UpdateMemory object.
     *        Initializes the checker count based on user settings for update checks.
     */
    UpdateMemory();

    /** @brief Destructor. */
    virtual ~UpdateMemory();

    /**
     * @brief Increments the number of components interested in checking for updates.
     * @return True if this is the first component to show interest (i.e., should check).
     */
    bool incrementChecker() {
        ScopedLock lock(lock_);
        bool should_check = (checkers_ == 0);
        checkers_++;
        return should_check;
    }

    /**
     * @brief Decrements the count of interested update checkers.
     */
    void decrementChecker() {
        ScopedLock lock(lock_);
        checkers_--;
    }

    JUCE_DECLARE_SINGLETON(UpdateMemory, false)

private:
    CriticalSection lock_; ///< Mutex lock for thread safety.
    int checkers_;         ///< Reference count of interested update checkers.
};

/**
 * @class UpdateCheckSection
 * @brief A UI overlay for checking software or content updates.
 *
 * This overlay appears to inform the user about available updates for the application
 * or content (e.g., presets). It can also prompt the user if they want to overwrite
 * an existing file or download new content.
 */
class UpdateCheckSection : public Overlay, public URL::DownloadTask::Listener {
public:
    static constexpr int kUpdateCheckWidth = 340; ///< Width of the update check dialog.
    static constexpr int kUpdateCheckHeight = 160;///< Height of the update check dialog.
    static constexpr int kPaddingX = 20;          ///< Horizontal padding inside the dialog.
    static constexpr int kPaddingY = 20;          ///< Vertical padding inside the dialog.
    static constexpr int kButtonHeight = 30;      ///< Height of buttons in the dialog.

    /**
     * @class VersionRequestThread
     * @brief A thread that requests version information from the server.
     *
     * This thread allows the version check to happen asynchronously without blocking the UI.
     */
    class VersionRequestThread : public Thread {
    public:
        /**
         * @brief Constructs a VersionRequestThread.
         * @param ref Pointer to the UpdateCheckSection that will process the result.
         */
        VersionRequestThread(UpdateCheckSection* ref) : Thread("Vital Download Thread"), ref_(ref) { }

        /**
         * @brief Thread entry point. Calls checkUpdate() on the referenced UpdateCheckSection.
         */
        void run() override {
            ref_->checkUpdate();
        }

    private:
        UpdateCheckSection* ref_; ///< Reference to the parent UpdateCheckSection.
    };

    /**
     * @class Listener
     * @brief Interface for components interested in update notifications.
     */
    class Listener {
    public:
        virtual ~Listener() { }

        /**
         * @brief Called when an update is needed.
         */
        virtual void needsUpdate() = 0;
    };

    /**
     * @brief Constructs an UpdateCheckSection.
     * @param name The name of this overlay section.
     */
    UpdateCheckSection(String name);

    /**
     * @brief Destructor. Stops the version request thread.
     */
    ~UpdateCheckSection() {
        version_request_.stopThread(350);
    }

    /**
     * @brief Called when the component is resized. Updates layout of all elements.
     */
    void resized() override;

    /**
     * @brief Sets the visibility of the update overlay. When made visible, lays out elements accordingly.
     * @param should_be_visible True to show, false to hide.
     */
    void setVisible(bool should_be_visible) override;

    /**
     * @brief Informs listeners that an update is available.
     */
    void needsUpdate();

    /**
     * @brief Handles button clicks.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Closes the overlay if clicked outside the dialog area.
     * @param e The mouse event.
     */
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Called when a download task finishes.
     * @param task The download task.
     * @param success True if download succeeded, false otherwise.
     */
    void finished(URL::DownloadTask* task, bool success) override;

    /**
     * @brief Called periodically to report download progress.
     * @param task The download task.
     * @param bytesDownloaded Number of bytes downloaded so far.
     * @param totalLength Total size of the file.
     */
    void progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 totalLength) override { }

    /**
     * @brief Starts the version check by launching the VersionRequestThread.
     */
    void startCheck() {
        version_request_.startThread();
    }

    /**
     * @brief Actual method that checks for an update. Usually called from the thread.
     */
    void checkUpdate();

    /**
     * @brief Checks for content updates (e.g., new preset content).
     */
    void checkContentUpdate();

    /**
     * @brief Computes the rectangle of the update check dialog within the overlay.
     * @return The rectangle representing the dialog area.
     */
    Rectangle<int> getUpdateCheckRect();

    /**
     * @brief Adds a listener to be notified when updates are available.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

private:
    /**
     * @brief Called when the user chooses to update content based on the retrieved version info.
     * @param version The new content version string.
     */
    void updateContent(String version);

    std::vector<Listener*> listeners_; ///< Listeners that want update notifications.

    VersionRequestThread version_request_;       ///< Thread for requesting version info.
    std::unique_ptr<URL::DownloadTask> download_task_; ///< The current download task (if any).
    File version_file_;                           ///< Temporary file holding version info.

    OpenGlQuad body_;                             ///< The background quad for the dialog.
    std::unique_ptr<PlainTextComponent> notify_text_;   ///< Text notifying user of an update.
    std::unique_ptr<PlainTextComponent> version_text_;  ///< Text showing the new version number.
    std::unique_ptr<OpenGlToggleButton> download_button_;///< Button to download the update.
    std::unique_ptr<OpenGlToggleButton> nope_button_;    ///< Button to ignore the update.

    String app_version_;    ///< The retrieved new application version string.
    String content_version_;///< The retrieved new content version string.
    bool content_update_;   ///< True if this is a content update rather than an app update.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UpdateCheckSection)
};
