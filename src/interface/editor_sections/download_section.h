/// @file download_section.h
/// @brief Declares the DownloadSection class, providing a UI for downloading and installing preset packs.

#pragma once

#include "JuceHeader.h"
#include "overlay.h"

class Authentication;

/// @class DownloadSection
/// @brief An overlay component handling the download and installation of factory content and packs.
///
/// The DownloadSection displays a progress bar, installation directory, and allows the user
/// to cancel or proceed with installation. It retrieves available packs, downloads them,
/// and installs them into the chosen directory.
class DownloadSection : public Overlay, public URL::DownloadTask::Listener, public Timer {
public:
    /// The URL path for the factory download.
    static const std::string kFactoryDownloadPath;
    /// The vertical offset for the download UI.
    static constexpr int kY = 180;
    /// The width of the download UI.
    static constexpr int kDownloadWidth = 450;
    /// The initial height of the download UI before any content is loaded.
    static constexpr int kDownloadInitialHeight = 380;
    /// The additional height if more content is available.
    static constexpr int kDownloadAdditionalHeight = 324;
    /// The text height for labels.
    static constexpr int kTextHeight = 15;
    /// Horizontal padding inside the UI.
    static constexpr int kPaddingX = 20;
    /// Vertical padding inside the UI.
    static constexpr int kPaddingY = 20;
    /// Height of buttons.
    static constexpr int kButtonHeight = 36;
    /// Time in milliseconds to wait before hiding the UI after completion.
    static constexpr int kCompletionWaitMs = 1000;

    /// Represents a downloadable pack with name, author, ID, URL, and local destination file.
    struct DownloadPack {
        DownloadPack(std::string n, std::string a, int i, URL u, File d) :
                name(std::move(n)), author(std::move(a)), id(i), url(u), download_location(d), finished(false) { }
        std::string name;            ///< The name of the pack.
        std::string author;          ///< The author of the pack.
        int id;                      ///< The pack's unique ID.
        URL url;                     ///< The download URL for the pack.
        File download_location;      ///< The local file where the pack is downloaded.
        bool finished;               ///< Whether the download is finished.
    };

    /// @class Listener
    /// @brief Interface for objects that need to respond to data directory changes or no-download scenarios.
    class Listener {
    public:
        virtual ~Listener() = default;

        /// Called when the data directory changes (e.g., after a successful installation).
        virtual void dataDirectoryChanged() = 0;

        /// Called when no downloads are needed (all content is up-to-date).
        virtual void noDownloadNeeded() = 0;
    };

    /// A background thread to handle downloading content without blocking the GUI.
    class DownloadThread : public Thread {
    public:
        DownloadThread(DownloadSection* ref, URL url, File dest) :
                Thread("Vial Download Thread"), ref_(ref), url_(std::move(url)), dest_(std::move(dest)) { }
        virtual ~DownloadThread() { }

        void run() override {
            ref_->startDownload(this, url_, dest_);
        }

    private:
        DownloadSection* ref_;
        URL url_;
        File dest_;
    };

    /// A background thread for installing downloaded packs.
    class InstallThread : public Thread {
    public:
        InstallThread(DownloadSection* ref) : Thread("Vial Install Thread"), ref_(ref) { }
        virtual ~InstallThread() { }

        void run() override {
            ref_->startInstall(this);
        }

    private:
        DownloadSection* ref_;
    };

    /// Constructor.
    /// @param name The name of this overlay component.
    /// @param auth The authentication object for accessing protected content.
    DownloadSection(String name, Authentication* auth);

    /// Destructor.
    virtual ~DownloadSection() {
        for (auto& thread : download_threads_)
            thread->stopThread(300);
    }

    /// Lays out the UI components within the overlay.
    void resized() override;

    /// Sets the visibility of the section, repainting if becoming visible.
    /// @param should_be_visible True to show, false to hide.
    void setVisible(bool should_be_visible) override;

    /// Timer callback used for hiding the UI after a delay.
    void timerCallback() override {
        stopTimer();
        setVisible(false);
    }

    /// Handles mouse-up events. If clicked outside the UI, hides the overlay.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Handles button clicks, either canceling download or triggering install.
    /// @param clicked_button The button clicked.
    void buttonClicked(Button* clicked_button) override;

    /// Renders OpenGL components, including the progress bar.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate If true, animates transitions.
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /// Called when a download finishes.
    /// @param task The download task.
    /// @param success True if successful, false otherwise.
    void finished(URL::DownloadTask* task, bool success) override;

    /// Called periodically with download progress updates.
    /// @param task The download task.
    /// @param bytesDownloaded The number of bytes downloaded so far.
    /// @param totalLength The total length of the file.
    void progress(URL::DownloadTask* task, int64 bytesDownloaded, int64 totalLength) override;

    /// Gets the rectangle representing the download UI area.
    /// @return The bounding rectangle of the download area.
    Rectangle<int> getDownloadRect();

    /// Triggers the process of checking available packs and downloading necessary content.
    void triggerDownload();

    /// Starts the installation process after all downloads are completed.
    void triggerInstall();

    /// Initiates a single download task.
    /// @param thread The thread starting this download.
    /// @param url The URL to download.
    /// @param dest The destination file for the downloaded content.
    void startDownload(Thread* thread, URL& url, const File& dest);

    /// Begins installation of downloaded packs.
    /// @param thread The thread performing the installation.
    void startInstall(Thread* thread);

    /// Cancels all ongoing downloads.
    void cancelDownload();

    /// Allows the user to choose a folder for installation.
    void chooseInstallFolder();

    /// Adds a listener to be notified of data directory changes or no-download scenarios.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

private:
    Authentication* auth_; ///< The authentication object for verifying access.
    OpenGlQuad body_;      ///< The main background quad for the overlay.
    bool cancel_;          ///< If true, cancels ongoing downloads.
    bool initial_download_;///< True if this is the initial download (no data directory yet).
    float progress_value_; ///< The current download progress (0.0 to 1.0).

    OpenGlQuad download_progress_;    ///< The download progress bar quad.
    OpenGlQuad download_background_;  ///< The background for the progress bar.
    OpenGlQuad install_text_background_; ///< Background for the installation text field.

    std::unique_ptr<AppLogo> logo_;          ///< Logo displayed at the top.
    std::unique_ptr<LoadingWheel> loading_wheel_; ///< Shows when download is in progress.

    std::vector<std::unique_ptr<DownloadThread>> download_threads_; ///< Threads handling downloads.
    InstallThread install_thread_;                                  ///< Thread handling installation.

    URL packs_url_;                ///< URL for fetching available packs.
    URL factory_download_url_;     ///< URL for downloading factory content.
    File available_packs_location_;///< Temporary file location for available packs data.
    std::vector<DownloadPack> awaiting_install_; ///< Packs that have been downloaded, awaiting install.
    std::vector<DownloadPack> awaiting_download_;///< Packs awaiting download.

    std::vector<std::unique_ptr<URL::DownloadTask>> download_tasks_; ///< Active download tasks.
    File install_location_; ///< The selected install directory.
    std::vector<Listener*> listeners_; ///< Listeners to be notified after installation or if no downloads are needed.

    std::unique_ptr<OpenGlShapeButton> folder_button_;   ///< Button to choose install folder.
    std::unique_ptr<PlainTextComponent> download_text_;  ///< Text showing current download status.
    std::unique_ptr<PlainTextComponent> install_location_text_; ///< Displays the install directory path.
    std::unique_ptr<OpenGlToggleButton> install_button_;  ///< Button to start installation.
    std::unique_ptr<OpenGlToggleButton> cancel_button_;   ///< Button to cancel downloads.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DownloadSection)
};
