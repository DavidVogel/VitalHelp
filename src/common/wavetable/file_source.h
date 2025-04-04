/*
Summary:
FileSource is a wavetable component that constructs wavetables from an external audio file buffer. It supports various interpolation/fade styles for shaping the final waveform and different phase manipulation strategies (e.g., randomizing phases for a vocoder effect). FileSourceKeyframe encapsulates one particular configuration of start position, window fade, and style settings. Together, they enable flexible and creative wavetable generation from raw audio samples.
 */

#pragma once

#include "JuceHeader.h"
#include "pitch_detector.h"
#include "wavetable_component.h"
#include "wave_frame.h"
#include "wave_source.h"
#include "utils.h"

/**
 * @brief A WavetableComponent that uses an external audio sample as its source.
 *
 * FileSource loads a segment of audio data (e.g., from a sample file) and transforms it into wavetables.
 * It provides multiple interpolation (fade) styles and phase manipulation options for constructing
 * a wavetable frame. Users can set how the audio frames are extracted (window size, fade style),
 * and how phases are handled (e.g., vocoding).
 */
class FileSource : public WavetableComponent {
  public:
    /// Maximum number of samples allowed from the file source (in samples).
    static constexpr float kMaxFileSourceSamples = 176400;
    /// Extra samples saved for safe interpolation and boundary conditions.
    static constexpr int kExtraSaveSamples = 4;
    /// Additional buffer samples for safe reading beyond boundaries.
    static constexpr int kExtraBufferSamples = 4;
    /// Maximum period for pitch detection to limit CPU usage.
    static constexpr int kPitchDetectMaxPeriod = 8096;

    /**
     * @enum FadeStyle
     * @brief Different methods to blend or interpolate the loaded audio window into a wavetable frame.
     */
    enum FadeStyle {
        kWaveBlend,      ///< Blend windowed segments into each other.
        kNoInterpolate,  ///< Use a single segment, no blending.
        kTimeInterpolate,///< Interpolate in time domain between cycles.
        kFreqInterpolate,///< Interpolate in frequency domain between cycles.
        kNumFadeStyles
    };

    /**
     * @enum PhaseStyle
     * @brief Methods for handling phase information in the transformed wave.
     */
    enum PhaseStyle {
        kNone,    ///< Keep phases as-is.
        kClear,   ///< Clear phases to a known pattern.
        kVocode,  ///< Assign random phases for vocoding-like effect.
        kNumPhaseStyles
    };

    /**
     * @brief A simple structure holding a buffer of samples loaded from the file source.
     */
    struct SampleBuffer {
        SampleBuffer() : size(0), sample_rate(0) { }
        std::unique_ptr<float[]> data; ///< Pointer to raw audio data.
        int size;                      ///< Number of samples in the buffer.
        int sample_rate;               ///< Sample rate of the audio data.
    };

    /**
     * @brief A specific WavetableKeyframe that uses the FileSource’s audio buffer.
     *
     * FileSourceKeyframe represents one "snapshot" of the file-based wavetable. It provides methods
     * to render the wave frame based on a chosen fade/interpolation style and phase style, and can
     * interpolate between keyframes or compute normalization scales.
     */
    class FileSourceKeyframe : public WavetableKeyframe {
      public:
        /**
         * @brief Constructs a keyframe tied to a given SampleBuffer.
         *
         * @param sample_buffer A pointer to the buffer of audio samples associated with this file source.
         */
        FileSourceKeyframe(SampleBuffer* sample_buffer);
        virtual ~FileSourceKeyframe() { }

        void copy(const WavetableKeyframe* keyframe) override;
        void interpolate(const WavetableKeyframe* from_keyframe,
                         const WavetableKeyframe* to_keyframe, float t) override;

        /**
         * @brief Computes the normalization scale factor for the current wave segment.
         *
         * Ensures consistent amplitude for rendering.
         * @return A scale factor for normalization.
         */
        float getNormalizationScale();

        void render(vital::WaveFrame* wave_frame) override;
        void renderWaveBlend(vital::WaveFrame* wave_frame);
        void renderNoInterpolate(vital::WaveFrame* wave_frame);
        void renderTimeInterpolate(vital::WaveFrame* wave_frame);
        void renderFreqInterpolate(vital::WaveFrame* wave_frame);
        json stateToJson() override;
        void jsonToState(json data) override;

        /**
         * @brief Gets the current start position of the wave segment in samples.
         */
        double getStartPosition() { return start_position_; }
        /**
         * @brief Gets the size of the extracted window in samples.
         */
        double getWindowSize() { return window_size_; }
        /**
         * @brief Gets the fade size for window blending.
         */
        double getWindowFade() { return window_fade_; }
        double getWindowFadeSamples() { return window_fade_ * window_size_; }
        int getSamplesNeeded() { return getWindowSize() + getWindowFadeSamples(); }

        force_inline void setStartPosition(double start_position) { start_position_ = start_position; }
        force_inline void setWindowFade(double window_fade) { window_fade_ = window_fade; }
        force_inline void setWindowSize(double window_size) { window_size_ = window_size; }
        force_inline void setFadeStyle(FadeStyle fade_style) { fade_style_ = fade_style; }
        force_inline void setPhaseStyle(PhaseStyle phase_style) { phase_style_ = phase_style; }
        force_inline void setOverriddenPhaseBuffer(const float* buffer) { overridden_phase_ = buffer; }
        force_inline const float* getDataBuffer() {
          if (sample_buffer_ == nullptr || sample_buffer_->data == nullptr)
            return nullptr;
          return sample_buffer_->data.get() + 1;
        }
        force_inline const float* getCubicInterpolationBuffer() {
          if (sample_buffer_ == nullptr)
            return nullptr;
          return sample_buffer_->data.get();
        }

        float getScaledInterpolatedSample(float time);

        void setInterpolateFromFrame(WaveSourceKeyframe* frame) {
          interpolate_from_frame_ = frame;
        }

        void setInterpolateToFrame(WaveSourceKeyframe* frame) {
          interpolate_to_frame_ = frame;
        }

      protected:
        SampleBuffer* sample_buffer_;
        const float* overridden_phase_;
        WaveSourceKeyframe* interpolate_from_frame_;
        WaveSourceKeyframe* interpolate_to_frame_;

        double start_position_;
        double window_fade_;
        double window_size_;
        FadeStyle fade_style_;
        PhaseStyle phase_style_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileSourceKeyframe)
    };

    FileSource();
    virtual ~FileSource() { }

    WavetableKeyframe* createKeyframe(int position) override;
    void render(vital::WaveFrame* wave_frame, float position) override;
    WavetableComponentFactory::ComponentType getType() override;
    json stateToJson() override;
    void jsonToState(json data) override;

    FileSourceKeyframe* getKeyframe(int index);
    const SampleBuffer* buffer() const { return &sample_buffer_; }
    FadeStyle getFadeStyle() { return fade_style_; }
    PhaseStyle getPhaseStyle() { return phase_style_; }
    bool getNormalizeGain() { return normalize_gain_; }

    void setNormalizeGain(bool normalize_gain) { normalize_gain_ = normalize_gain; }
    void setWindowSize(double window_size) { window_size_ = window_size; }
    void setFadeStyle(FadeStyle fade_style) { fade_style_ = fade_style; }
    void setPhaseStyle(PhaseStyle phase_style);
    void writePhaseOverrideBuffer();
    double getWindowSize() { return window_size_; }

    /**
     * @brief Loads audio data into the file source buffer.
     *
     * @param buffer Pointer to the audio samples.
     * @param size Number of samples.
     * @param sample_rate The sample rate of the audio data.
     */
    void loadBuffer(const float* buffer, int size, int sample_rate);

    /**
     * @brief Attempts to detect pitch in the loaded sample to determine window size automatically.
     *
     * @param max_period Maximum period considered for pitch detection.
     */
    void detectPitch(int max_period = vital::WaveFrame::kWaveformSize);

    /**
     * @brief Detects if the source audio can form a WaveEdit-style wavetable (special format).
     */
    void detectWaveEditTable();

    force_inline const float* getDataBuffer() {
      if (sample_buffer_.data == nullptr)
        return nullptr;
      return sample_buffer_.data.get() + 1;
    }
    force_inline const float* getCubicInterpolationBuffer() { return sample_buffer_.data.get(); }

  protected:
    FileSourceKeyframe compute_frame_;
    WaveSourceKeyframe interpolate_from_frame_;
    WaveSourceKeyframe interpolate_to_frame_;

    SampleBuffer sample_buffer_;
    float overridden_phase_[vital::WaveFrame::kWaveformSize];
    FadeStyle fade_style_;
    PhaseStyle phase_style_;
    bool normalize_gain_;
    bool normalize_mult_;
    double window_size_;

    int random_seed_;
    vital::utils::RandomGenerator random_generator_;
    PitchDetector pitch_detector_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileSource)
};

