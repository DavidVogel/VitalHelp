#pragma once

#include "JuceHeader.h"
#include "memory.h"
#include "fourier_transform.h"
#include "open_gl_line_renderer.h"

/**
 * @class Oscilloscope
 * @brief Renders a time-domain waveform using OpenGL.
 *
 * The Oscilloscope class displays a waveform in real-time by reading from a memory buffer of audio samples.
 * It uses the OpenGlLineRenderer base class to handle rendering a line representing the waveform.
 */
class Oscilloscope : public OpenGlLineRenderer {
public:
    /// The number of points used to represent the waveform.
    static constexpr int kResolution = 512;

    /**
     * @brief Constructs the Oscilloscope and sets up fill and corners.
     */
    Oscilloscope();
    /**
     * @brief Destructor.
     */
    virtual ~Oscilloscope();

    /**
     * @brief Draws the waveform line for a given channel index.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param index The channel index (0 or 1).
     */
    void drawWaveform(OpenGlWrapper& open_gl, int index);

    /**
     * @brief Renders the oscilloscope line for both channels.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param animate If true, animations/boosts may be applied.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Sets the memory buffer that the oscilloscope reads from.
     * @param memory A pointer to a vital::poly_float array of audio data.
     */
    void setOscilloscopeMemory(const vital::poly_float* memory) { memory_ = memory; }

private:
    const vital::poly_float* memory_; ///< Pointer to memory buffer holding waveform samples.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oscilloscope)
};

/**
 * @class Spectrogram
 * @brief Renders a frequency-domain representation (spectrogram) using OpenGL.
 *
 * The Spectrogram class uses a Fourier transform to display the frequency content of audio data over time.
 * It reads from a StereoMemory buffer, applies a window function, performs an FFT, and then visualizes
 * the amplitude across frequencies as a line. Both left and right channels are represented, and various
 * frequency and dB range parameters can be set.
 */
class Spectrogram : public OpenGlLineRenderer {
public:
    static constexpr int kResolution = 300;         ///< Number of points in the frequency line.
    static constexpr float kDecayMult = 0.008f;     ///< Decay multiplier for amplitude smoothing.
    static constexpr int kBits = 14;                ///< Number of bits, defining the transform size (2^kBits).
    static constexpr int kAudioSize = 1 << kBits;   ///< Size of audio block for the FFT.
    static constexpr float kDefaultMaxDb = 0.0f;    ///< Default maximum dB level displayed.
    static constexpr float kDefaultMinDb = -50.0f;  ///< Default minimum dB level displayed.
    static constexpr float kDefaultMinFrequency = 9.2f;   ///< Default minimum frequency.
    static constexpr float kDefaultMaxFrequency = 21000.0f; ///< Default maximum frequency.
    static constexpr float kDbSlopePerOctave = 3.0f;       ///< dB slope per octave for visualization.

    /**
     * @brief Constructs the Spectrogram with default parameters.
     */
    Spectrogram();

    /**
     * @brief Destructor.
     */
    virtual ~Spectrogram();

    /**
     * @brief Draws the frequency-domain waveform (spectrogram line) for a given channel index.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param index The channel index (0 for left, 1 for right).
     */
    void drawWaveform(OpenGlWrapper& open_gl, int index);

    /**
     * @brief Renders the spectrogram for both channels.
     * @param open_gl The OpenGlWrapper providing the current OpenGL context.
     * @param animate If true, animations/boosts may be applied.
     */
    void render(OpenGlWrapper& open_gl, bool animate) override;

    /**
     * @brief Sets the StereoMemory from which audio data is read for the FFT.
     * @param memory Pointer to a vital::StereoMemory holding stereo audio samples.
     */
    void setAudioMemory(const vital::StereoMemory* memory) { memory_ = memory; }

    /**
     * @brief Paints a background using JUCE's Graphics (e.g., frequency lines).
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Sets the oversampling amount used for adjusting frequency scaling.
     * @param oversample The oversample factor.
     */
    void setOversampleAmount(int oversample) { oversample_amount_ = oversample; }

    /**
     * @brief Sets the minimum frequency displayed in the spectrogram.
     * @param frequency The new minimum frequency in Hz.
     */
    void setMinFrequency(float frequency) { min_frequency_ = frequency; }

    /**
     * @brief Sets the maximum frequency displayed in the spectrogram.
     * @param frequency The new maximum frequency in Hz.
     */
    void setMaxFrequency(float frequency) { max_frequency_ = frequency; }

    /**
     * @brief Sets the minimum dB level displayed.
     * @param db The new minimum dB level.
     */
    void setMinDb(float db) { min_db_ = db; }

    /**
     * @brief Sets the maximum dB level displayed.
     * @param db The new maximum dB level.
     */
    void setMaxDb(float db) { max_db_ = db; }

    /**
     * @brief Enables or disables painting background frequency lines.
     * @param paint True to paint background lines, false otherwise.
     */
    void paintBackgroundLines(bool paint) { paint_background_lines_ = paint; }

private:
    /**
     * @brief Updates amplitude data after performing the FFT for a given channel and offset.
     * @param index The channel index (0 or 1).
     * @param offset The sample offset to read from in the memory.
     */
    void updateAmplitudes(int index, int offset);

    /**
     * @brief Updates amplitude data with no offset.
     * @param index The channel index (0 or 1).
     */
    void updateAmplitudes(int index);

    /**
     * @brief Applies a window function to the audio data before performing the FFT.
     */
    void applyWindow();

    int sample_rate_;         ///< Audio sample rate.
    int oversample_amount_;   ///< Oversampling factor for frequency scaling.
    float min_frequency_;     ///< Minimum frequency displayed.
    float max_frequency_;     ///< Maximum frequency displayed.
    float min_db_;            ///< Minimum dB level displayed.
    float max_db_;            ///< Maximum dB level displayed.
    bool paint_background_lines_; ///< Whether to paint background frequency lines.

    float transform_buffer_[2 * kAudioSize]; ///< FFT input/output buffer.
    float left_amps_[kAudioSize];            ///< Left channel amplitude array.
    float right_amps_[kAudioSize];           ///< Right channel amplitude array.
    const vital::StereoMemory* memory_;      ///< Pointer to stereo audio memory for FFT source.
    vital::FourierTransform transform_;      ///< Fourier transform instance for FFT operations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrogram)
};
