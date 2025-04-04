/// @file sample_viewer.h
/// @brief Declares the SampleViewer class that displays and animates a waveform sample.

#pragma once

#include "JuceHeader.h"

#include "audio_file_drop_source.h"
#include "common.h"
#include "open_gl_line_renderer.h"
#include "synth_types.h"
#include "sample_source.h"
#include "synth_module.h"

/// @class SampleViewer
/// @brief A UI component for visually displaying and interacting with an audio sample waveform.
///
/// The SampleViewer uses OpenGL rendering to show a waveform of a loaded sample.
/// It can react to audio file drops, updates from the synthesizer engine, and
/// provides animated line boosts that visualize sample playback position.
class SampleViewer : public OpenGlLineRenderer, public AudioFileDropSource {
public:
    /// @brief The resolution of the waveform, in number of points.
    static constexpr float kResolution = 256;
    /// @brief The decay factor for line boosts.
    static constexpr float kBoostDecay = 0.9f;
    /// @brief The multiplier for decay when lines move quickly.
    static constexpr float kSpeedDecayMult = 5.0f;

    /// @class Listener
    /// @brief Interface for objects that want to be notified when a sample is loaded.
    class Listener {
    public:
        /// Virtual destructor for proper cleanup.
        virtual ~Listener() { }

        /// Called when a new sample file is loaded.
        /// @param file The file that was loaded.
        virtual void sampleLoaded(const File& file) = 0;
    };

    /// Constructor.
    SampleViewer();
    /// Destructor.
    virtual ~SampleViewer();

    /// Initializes OpenGL resources for this component.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    void init(OpenGlWrapper& open_gl) override {
        OpenGlLineRenderer::init(open_gl);
        bottom_.init(open_gl);
        dragging_overlay_.init(open_gl);
    }

    /// Renders the waveform and overlays.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    /// @param animate If true, the component should animate.
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /// Destroys OpenGL resources associated with this component.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    void destroy(OpenGlWrapper& open_gl) override {
        OpenGlLineRenderer::destroy(open_gl);
        bottom_.destroy(open_gl);
        dragging_overlay_.destroy(open_gl);
    }

    /// Handles component resizing. Adjusts waveform geometry.
    void resized() override;

    /// Sets the active state of the SampleViewer.
    /// @param active True to mark it as active, false otherwise.
    void setActive(bool active) { active_ = active; }

    /// Checks if the SampleViewer is active.
    /// @return True if active, false otherwise.
    bool isActive() const { return active_; }

    /// Called when an audio file is loaded. Notifies listeners and updates the waveform.
    /// @param file The loaded audio file.
    void audioFileLoaded(const File& file) override;

    /// Repaints the waveform after an audio update.
    void repaintAudio();

    /// Sets the line positions (y-values) of the waveform based on the current sample.
    void setLinePositions();

    /// Called when an audio file drag enters the viewer region.
    /// @param files The dragged files.
    /// @param x The x-position of the mouse.
    /// @param y The y-position of the mouse.
    void fileDragEnter(const StringArray& files, int x, int y) override {
        dragging_audio_file_ = true;
    }

    /// Called when an audio file drag exits the viewer region.
    /// @param files The dragged files.
    void fileDragExit(const StringArray& files) override {
        dragging_audio_file_ = false;
    }

    /// Gets the name of the currently loaded sample.
    /// @return The sample name if a sample is loaded, otherwise an empty string.
    std::string getName();

    /// Adds a listener to receive events from this SampleViewer.
    /// @param listener The listener to add.
    void addListener(Listener* listener) {
        listeners_.push_back(listener);
    }

    /// Sets the sample to be viewed.
    /// @param sample The sample to display.
    void setSample(vital::Sample* sample) { sample_ = sample; setLinePositions(); }

private:
    std::vector<Listener*> listeners_;      ///< Registered listeners to notify of sample loads.

    const vital::StatusOutput* sample_phase_output_;  ///< Phase output from the synthesizer.
    vital::poly_float last_phase_;                    ///< The last known playback phase.
    vital::poly_float last_voice_;                    ///< The last known voice state.
    vital::Sample* sample_;                           ///< The currently loaded sample.

    OpenGlLineRenderer bottom_;   ///< A secondary line renderer for the bottom part of the waveform.
    OpenGlQuad dragging_overlay_; ///< Overlay drawn when a file is being dragged.

    bool dragging_audio_file_; ///< Indicates if an audio file is currently being dragged over the viewer.
    bool animate_;             ///< Indicates if the waveform should animate.
    bool active_;              ///< Active state of the viewer.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleViewer)
};
