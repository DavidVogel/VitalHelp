#pragma once

#include "JuceHeader.h"

#include "audio_file_drop_source.h"
#include "wavetable_component_overlay.h"
#include "file_source.h"
#include "pitch_detector.h"
#include "sample_viewer.h"
#include "text_selector.h"
#include "synth_section.h"

class AudioFileViewer;
class AudioFilePosition;

/**
 * @class AudioFileViewer
 * @brief A component for visualizing and interacting with an audio waveform.
 *
 * The AudioFileViewer displays an audio waveform and allows the user to adjust
 * the window position and size used by a FileSource. It supports file drops
 * to load audio files directly.
 */
class AudioFileViewer : public SynthSection, public AudioFileDropSource {
public:
    /**
     * @class DragListener
     * @brief Interface for listening to mouse drag movements relative to the waveform.
     *
     * Implement this interface to respond to changes in position as the user drags
     * over the audio waveform viewer.
     */
    class DragListener {
    public:
        virtual ~DragListener() { }

        /**
         * @brief Called when the user moves the mouse while dragging over the audio waveform.
         *
         * @param ratio    The relative ratio of the movement across the waveform width.
         * @param mouse_up True if this call is triggered by mouse release.
         */
        virtual void positionMovedRelative(float ratio, bool mouse_up) = 0;
    };

    static constexpr float kResolution = 256; /**< Resolution of the waveform display. */

    /**
     * @brief Constructs an AudioFileViewer.
     */
    AudioFileViewer();
    virtual ~AudioFileViewer() { }

    /**
     * @brief Resizes the component, adjusting the waveform display and line positions.
     */
    void resized() override;

    /**
     * @brief Clears the currently displayed audio positions in the waveform.
     */
    void clearAudioPositions();

    /**
     * @brief Updates the waveform visualization based on the current audio file data.
     */
    void setAudioPositions();

    /**
     * @brief Updates the display to reflect new window position and size.
     */
    void setWindowValues();

    /**
     * @brief Sets the window start position for display.
     * @param window_position New window start position in normalized units [0, 1].
     */
    void setWindowPosition(float window_position) { window_position_ = window_position; setWindowValues(); }

    /**
     * @brief Sets the window size for display.
     * @param window_size New window size in normalized units relative to file length.
     */
    void setWindowSize(float window_size) { window_size_ = window_size; setWindowValues(); }

    /**
     * @brief Sets the window fade amount, controlling the fade shape on the edges of the window.
     * @param window_fade New window fade amount [0, 1].
     */
    void setWindowFade(float window_fade) { window_fade_ = window_fade; setWindowValues(); }

    /**
     * @brief Called when an audio file is loaded via drag-and-drop.
     * @param file The loaded audio file.
     */
    void audioFileLoaded(const File& file) override;

    /**
     * @brief Called when files are dragged into this component.
     */
    void fileDragEnter(const StringArray& files, int x, int y) override;

    /**
     * @brief Called when files are dragged out of this component.
     */
    void fileDragExit(const StringArray& files) override;

    /**
     * @brief Updates the mouse position and returns the relative movement ratio.
     * @param position Current mouse position.
     * @return Relative ratio of movement since the last mouse position update.
     */
    float updateMousePosition(Point<float> position);

    /**
     * @brief Mouse event handlers for adjusting audio window start on drag.
     */
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    /**
     * @brief Gets the underlying sample buffer of the loaded audio.
     * @return Pointer to the audio sample buffer.
     */
    AudioSampleBuffer* getSampleBuffer() { return &sample_buffer_; }

    /**
     * @brief Gets the sample rate of the loaded audio.
     * @return The sample rate in Hz.
     */
    int getSampleRate() const { return sample_rate_; }

    /**
     * @brief Sets the FileSource reference for retrieving audio data.
     * @param file_source The FileSource to associate with this viewer.
     */
    void setFileSource(FileSource* file_source) { file_source_ = file_source; setAudioPositions(); }

    /**
     * @brief Adds a drag listener to receive callbacks on waveform drags.
     * @param listener The listener to add.
     */
    void addDragListener(DragListener* listener) { drag_listeners_.push_back(listener); }

private:
    std::vector<DragListener*> drag_listeners_;

    OpenGlLineRenderer top_;
    OpenGlLineRenderer bottom_;
    OpenGlQuad dragging_quad_;

    float window_position_;
    float window_size_;
    float window_fade_;

    AudioSampleBuffer sample_buffer_;
    int sample_rate_;
    FileSource* file_source_;
    Point<float> last_mouse_position_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioFileViewer)
};

/**
 * @class FileSourceOverlay
 * @brief An overlay UI component for editing FileSource Wavetable components.
 *
 * The FileSourceOverlay allows configuring the start position, window size, fade style,
 * and other parameters for a FileSource-based keyframe in a Wavetable. It provides controls
 * for loading a file, adjusting window parameters, normalization, fade style, and phase style.
 */
class FileSourceOverlay : public WavetableComponentOverlay, TextEditor::Listener,
                          public AudioFileDropSource::Listener,
                          public AudioFileViewer::DragListener {
public:
    /**
     * @brief Constructs a FileSourceOverlay.
     */
    FileSourceOverlay();
    virtual ~FileSourceOverlay();

    /**
     * @brief Called when a keyframe is selected in the Wavetable.
     * @param keyframe The newly selected keyframe, or nullptr if none.
     */
    virtual void frameSelected(WavetableKeyframe* keyframe) override;

    /**
     * @brief Called when a keyframe is dragged, not used here.
     */
    virtual void frameDragged(WavetableKeyframe* keyframe, int position) override { }

    /**
     * @brief Sets the editing bounds for controls in this overlay.
     * @param bounds The rectangular area allocated for editing controls.
     */
    virtual void setEditBounds(Rectangle<int> bounds) override;

    /**
     * @brief Responds to slider changes (window fade, fade style, phase style).
     * @param moved_slider The slider that changed.
     */
    void sliderValueChanged(Slider* moved_slider) override;

    /**
     * @brief Called when slider drag ends, finalizing changes.
     * @param moved_slider The slider that stopped dragging.
     */
    void sliderDragEnded(Slider* moved_slider) override;

    /**
     * @brief Handles button clicks (e.g., load button, normalize toggle).
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Called when an audio file is loaded (via drag-drop or load button).
     * @param file The loaded audio file.
     */
    void audioFileLoaded(const File& file) override;

    /**
     * @brief Called when return key is pressed in text editors (start position, window size).
     * @param text_editor The text editor where return was pressed.
     */
    void textEditorReturnKeyPressed(TextEditor& text_editor) override;

    /**
     * @brief Called when text editors lose focus, updating values.
     * @param text_editor The text editor that lost focus.
     */
    void textEditorFocusLost(TextEditor& text_editor) override;

    /**
     * @brief Called by AudioFileViewer when the mouse moves relative to waveform position.
     * @param ratio The normalized movement ratio.
     * @param mouse_up True if triggered by mouse release.
     */
    void positionMovedRelative(float ratio, bool mouse_up) override;

    /**
     * @brief Sets the FileSource this overlay edits.
     * @param file_source The FileSource to associate with this overlay.
     */
    void setFileSource(FileSource* file_source);

    /**
     * @brief Loads an audio file into the FileSource and updates UI.
     * @param file The audio file to load.
     */
    void loadFile(const File& file) override;

protected:
    /**
     * @brief Applies visuals and font settings to a text editor.
     * @param text_editor The text editor to style.
     * @param height The target height for font size calculation.
     */
    void setTextEditorVisuals(TextEditor* text_editor, float height);

    /**
     * @brief Updates the FileSource's window size from text input.
     */
    void loadWindowSizeText();

    /**
     * @brief Updates the FileSource's starting position from text input.
     */
    void loadStartingPositionText();

    /**
     * @brief Ensures the starting position is within valid range.
     */
    void clampStartingPosition();

    /**
     * @brief Opens a file chooser to load an audio file into the FileSource.
     */
    void loadFilePressed();

    FileSource* file_source_;
    FileSource::FileSourceKeyframe* current_frame_;

    std::unique_ptr<OpenGlTextEditor> start_position_;
    std::unique_ptr<OpenGlTextEditor> window_size_;
    std::unique_ptr<SynthSlider> window_fade_;
    std::unique_ptr<TextButton> load_button_;
    std::unique_ptr<TextSelector> fade_style_;
    std::unique_ptr<TextSelector> phase_style_;
    std::unique_ptr<OpenGlToggleButton> normalize_gain_;
    std::unique_ptr<AudioFileViewer> audio_thumbnail_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileSourceOverlay)
};
