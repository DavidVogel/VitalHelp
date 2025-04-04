/// @file wavetable_3d.h
/// @brief Declares the Wavetable3d class, which provides a 3D and 2D visualization for wavetables.
///
/// The Wavetable3d class can render waveforms in 3D perspective, 2D waveforms, or their spectral amplitudes.
/// It integrates with modulation outputs, responds to user interaction for modifying the waveform frame,
/// and can load and save wavetables or interpret audio files as wavetables.

#pragma once

#include "JuceHeader.h"
#include "open_gl_line_renderer.h"
#include "audio_file_drop_source.h"
#include "common.h"
#include "open_gl_image.h"
#include "synth_oscillator.h"
#include "synth_types.h"
#include "wavetable.h"
#include "wavetable_creator.h"

namespace vital {
    struct Output;
    class Wavetable;
}

/// @class Wavetable3d
/// @brief OpenGL-based component for visualizing wavetables in different formats.
///
/// The Wavetable3d component can render:
/// - A 3D representation of multiple frames of a wavetable.
/// - A 2D single-frame waveform view.
/// - A frequency amplitude view of the wavetable.
///
/// It supports loading wavetables from JSON, copying/pasting, resynthesis, and text-to-wavetable conversions.
/// It also integrates with parameter modulation for frame, spectral morph, and distortion adjustments.
class Wavetable3d : public OpenGlComponent, public AudioFileDropSource {
public:
    /// Default angles and scaling parameters for 3D rendering.
    static constexpr float kDefaultVerticalAngle = 1.132f;
    static constexpr float kDefaultHorizontalAngle = -0.28f;
    static constexpr float kDefaultDrawWidthPercent = 0.728f;
    static constexpr float kDefaultWaveHeightPercent = 0.083f;
    static constexpr float kPositionWidth = 8.0f;
    static constexpr float kPositionLineWidthRatio = 1.8f;
    static constexpr int kColorJump = 16;
    static constexpr int kDownsampleResolutionAmount = 0;
    static constexpr int kResolution = vital::Wavetable::kWaveformSize >> kDownsampleResolutionAmount;
    static constexpr int kNumBits = vital::WaveFrame::kWaveformBits;
    static constexpr int kBackgroundResolution = 128;
    static constexpr int kExtraShadows = 20;
    static constexpr float k2dWaveHeightPercent = 0.25f;

    /// Static helper method for painting a single 3D line (waveform frame) onto a Graphics object.
    static void paint3dLine(Graphics& g, vital::Wavetable* wavetable, int index, Colour color,
                            float width, float height, float wave_height_percent,
                            float wave_range_x, float frame_range_x, float wave_range_y, float frame_range_y,
                            float start_x, float start_y, float offset_x, float offset_y);

    /// Static helper method for painting the 3D background of the wavetable (all frames) onto a Graphics object.
    static void paint3dBackground(Graphics& g, vital::Wavetable* wavetable, bool active,
                                  Colour background_color, Colour wave_color1, Colour wave_color2,
                                  float width, float height,
                                  float wave_height_percent,
                                  float wave_range_x, float frame_range_x, float wave_range_y, float frame_range_y,
                                  float start_x, float start_y, float offset_x, float offset_y);

    /// Menu options for right-click context menu.
    enum MenuOptions {
        kCancel,
        kCopy,
        kPaste,
        kInit,
        kSave,
        kTextToWavetable,
        kResynthesizePreset,
        kLogIn,
        kNumMenuOptions
    };

    /// Render types for different visualization modes.
    enum RenderType {
        kWave3d,
        kWave2d,
        kFrequencyAmplitudes,
        kNumRenderTypes
    };

    /// @class Listener
    /// @brief Interface for components that need to respond to wavetable loading or transformations.
    class Listener {
    public:
        virtual ~Listener() { }

        /// Called when audio data is loaded as a wavetable.
        virtual bool loadAudioAsWavetable(String name, InputStream* audio_stream,
                                          WavetableCreator::AudioFileLoadStyle style) = 0;

        /// Called when a JSON representation of a wavetable is loaded.
        virtual void loadWavetable(json& wavetable_data) = 0;

        /// Called to initialize (reset) the wavetable to a default state.
        virtual void loadDefaultWavetable() = 0;

        /// Called to resynthesize the current preset into a wavetable.
        virtual void resynthesizeToWavetable() = 0;

        /// Called to run a "text to wavetable" conversion.
        virtual void textToWavetable() = 0;

        /// Called to save the current wavetable.
        virtual void saveWavetable() = 0;
    };

    /// Constructor.
    /// @param index The oscillator index this wavetable visualization represents.
/// @param mono_modulations A map of mono output parameters.
/// @param poly_modulations A map of poly output parameters.
    Wavetable3d(int index, const vital::output_map& mono_modulations, const vital::output_map& poly_modulations);

    /// Destructor.
    virtual ~Wavetable3d();

    /// Initializes OpenGL resources.
    /// @param open_gl The OpenGlWrapper managing state.
    void init(OpenGlWrapper& open_gl) override;

    /// Renders the wavetable visualization each frame.
    /// @param open_gl The OpenGlWrapper managing state.
    /// @param animate If true, apply animations.
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /// Renders the wave visualization (2D or 3D).
    /// @param open_gl The OpenGlWrapper managing state.
    void renderWave(OpenGlWrapper& open_gl);

    /// Renders the frequency amplitude visualization.
    /// @param open_gl The OpenGlWrapper managing state.
    void renderSpectrum(OpenGlWrapper& open_gl);

    /// Destroys OpenGL resources.
    /// @param open_gl The OpenGlWrapper managing state.
    void destroy(OpenGlWrapper& open_gl) override;

    /// Paints the background, including the 3D wavetable shadow lines.
    /// @param g The JUCE Graphics context.
    void paintBackground(Graphics& g) override;

    /// Handles component resizing, recalculating layout and scaling.
    void resized() override;

    /// Handles mouse-down events for context menu and frame dragging.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Handles mouse dragging to change the frame slider value.
    /// @param e The mouse event.
    void mouseDrag(const MouseEvent& e) override;

    /// Hides the frame slider popup on mouse exit.
    /// @param e The mouse event.
    void mouseExit(const MouseEvent& e) override;

    /// Handles mouse wheel to change the frame slider value.
    /// @param e The mouse event.
/// @param wheel The mouse wheel details.
    void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override;

    /// Sets the slider controlling the wavetable frame.
    /// @param slider The SynthSlider for frame control.
    void setFrameSlider(SynthSlider* slider) { frame_slider_ = slider; }

    /// Sets the slider controlling spectral morph amount.
    /// @param slider The Slider for spectral morph.
    void setSpectralMorphSlider(Slider* slider) { spectral_morph_slider_ = slider; }

    /// Sets the slider controlling distortion amount.
    /// @param slider The Slider for distortion.
    void setDistortionSlider(Slider* slider) { distortion_slider_ = slider; }

    /// Sets the slider controlling distortion phase.
    /// @param slider The Slider for distortion phase.
    void setDistortionPhaseSlider(Slider* slider) { distortion_phase_slider_ = slider; }

    /// Sets view settings for the 3D display (angles, width, height, offset).
    void setViewSettings(float horizontal_angle, float vertical_angle,
                         float draw_width, float wave_height, float y_offset);

    /// Sets the render type (3D wave, 2D wave, or frequency amplitude).
    /// @param render_type The render type.
    void setRenderType(RenderType render_type);

    /// Gets the current render type.
    /// @return The current render type.
    RenderType getRenderType() const { return render_type_; }

    /// Sets the spectral morph type index.
    /// @param spectral_morph_type The spectral morph type.
    void setSpectralMorphType(int spectral_morph_type) { spectral_morph_type_ = spectral_morph_type; }

    /// Sets the distortion type index.
    /// @param distortion_type The distortion type.
    void setDistortionType(int distortion_type) { distortion_type_ = distortion_type; }

    /// Responds to a menu callback action (copy, paste, init, etc.).
    /// @param option The chosen menu option.
    void respondToMenuCallback(int option);

    /// Checks if the system clipboard currently has a matching wavetable JSON.
    /// @return True if clipboard data is a valid wavetable.
    bool hasMatchingSystemClipboard();

    /// Sets the active state of this visualization.
    /// @param active True if active, false otherwise.
    void setActive(bool active) { active_ = active; }

    /// Checks if the visualization is active.
    /// @return True if active, false otherwise.
    bool isActive() { return active_; }

    /// Called when an audio file is loaded by dragging onto the component.
    /// @param file The loaded file.
    void audioFileLoaded(const File& file) override;

    /// Updates the dragging position to determine how the audio file will be interpreted.
    /// @param x The x-position of the drag.
    /// @param y The y-position of the drag.
    void updateDraggingPosition(int x, int y);

    /// Handles file drag enter events.
    void fileDragEnter(const StringArray& files, int x, int y) override {
        updateDraggingPosition(x, y);
    }

    /// Handles file drag move events.
    void fileDragMove(const StringArray& files, int x, int y) override {
        updateDraggingPosition(x, y);
    }

    /// Handles file drag exit events, resetting the load style.
    void fileDragExit(const StringArray& files) override {
        drag_load_style_ = WavetableCreator::kNone;
    }

    /// Adds a listener to be notified of wavetable changes.
    /// @param listener The listener.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /// Sets whether the wavetable is currently loading or being changed.
    /// @param loading True if currently loading a wavetable.
    void setLoadingWavetable(bool loading) { loading_wavetable_ = loading; }

    /// Marks the internal state as dirty, forcing a redraw.
    void setDirty() { last_spectral_morph_type_ = -1; }

    /// Gets the current wavetable.
    /// @return A pointer to the current vital::Wavetable.
    vital::Wavetable* getWavetable() { return wavetable_; }

private:
    /// Checks if new render data is needed and updates internal parameters.
    /// @return True if line data needs to be reloaded.
    bool updateRenderValues();

    /// Loads the selected frame into time domain data.
    void loadIntoTimeDomain(int index);

    /// Loads waveform data for rendering time-domain waves.
    /// @param index Left (0) or right (1) channel index.
    void loadWaveData(int index);

    /// Loads frequency domain data for spectrum rendering.
    /// @param index Left (0) or right (1) channel index.
    void loadSpectrumData(int index);

    /// Draws the position indicator at the start/end of the waveform line.
    /// @param open_gl The OpenGlWrapper.
    /// @param index Left (0) or right (1) channel index.
    void drawPosition(OpenGlWrapper& open_gl, int index);

    /// Sets the dimension values for the 3D perspective.
    void setDimensionValues();

    /// Sets the color scheme for lines and fills.
    void setColors();

    /// Gets the total distortion value from modulation and slider.
    vital::poly_float getDistortionValue();

    /// Gets the total spectral morph value from modulation and slider.
    vital::poly_float getSpectralMorphValue();

    /// Gets the distortion phase value.
    vital::poly_int getDistortionPhaseValue();

    /// Loads frequency-domain data from the wavetable for processing.
    void loadFrequencyData(int index);

    /// Applies spectral morph to convert frequency data to time domain.
    void warpSpectrumToWave(int index);

    /// Applies distortion phase warp to the processed data.
    void warpPhase(int index);

    /// Combines mono and poly output values.
    inline vital::poly_float getOutputsTotal(std::pair<vital::Output*, vital::Output*> outputs,
                                             vital::poly_float default_value);

    OpenGlLineRenderer left_line_renderer_;   ///< Line renderer for left channel (or single channel).
    OpenGlLineRenderer right_line_renderer_;  ///< Line renderer for right channel.
    OpenGlMultiQuad end_caps_;                ///< Caps showing position indicators.

    Colour import_text_color_;
    OpenGlQuad import_overlay_;              ///< Overlay when dragging files to import.
    std::unique_ptr<PlainTextComponent> wavetable_import_text_;
    std::unique_ptr<PlainTextComponent> vocode_import_text_;
    std::unique_ptr<PlainTextComponent> pitch_splice_import_text_;

    Colour body_color_;
    Colour line_left_color_;
    Colour line_right_color_;
    Colour line_disabled_color_;
    Colour fill_left_color_;
    Colour fill_right_color_;
    Colour fill_disabled_color_;

    std::vector<Listener*> listeners_;                      ///< Registered listeners.
    std::pair<vital::Output*, vital::Output*> wave_frame_outputs_;
    std::pair<vital::Output*, vital::Output*> spectral_morph_outputs_;
    std::pair<vital::Output*, vital::Output*> distortion_outputs_;
    std::pair<vital::Output*, vital::Output*> distortion_phase_outputs_;

    int last_spectral_morph_type_;
    int last_distortion_type_;
    int spectral_morph_type_;
    int distortion_type_;
    vital::poly_float wave_frame_;
    vital::poly_float spectral_morph_value_;
    vital::poly_float distortion_value_;
    vital::poly_int distortion_phase_;

    SynthSlider* frame_slider_;
    Slider* spectral_morph_slider_;
    Slider* distortion_slider_;
    Slider* distortion_phase_slider_;
    Point<int> last_edit_position_;
    WavetableCreator::AudioFileLoadStyle drag_load_style_;
    vital::WaveFrame process_frame_;
    vital::FourierTransform transform_;
    vital::poly_float process_wave_data_[vital::SynthOscillator::kSpectralBufferSize];
    const vital::Wavetable::WavetableData* current_wavetable_data_;
    int wavetable_index_;

    bool animate_;
    bool loading_wavetable_;
    bool last_loading_wavetable_;
    RenderType render_type_;
    RenderType last_render_type_;
    bool active_;
    int size_;
    int index_;
    vital::Wavetable* wavetable_;

    double current_value_;
    float vertical_angle_;
    float horizontal_angle_;
    float draw_width_percent_;
    float wave_height_percent_;
    float y_offset_;

    float wave_range_x_;
    float frame_range_x_;
    float wave_range_y_;
    float frame_range_y_;
    float start_x_;
    float start_y_;
    float offset_x_;
    float offset_y_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Wavetable3d)
};
