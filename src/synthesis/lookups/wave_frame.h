#pragma once

#include "common.h"

#include <complex>

namespace vital {

    /**
     * @brief Represents a single frame of a wavetable, containing both time-domain and frequency-domain data.
     *
     * A WaveFrame holds a waveform in both time-domain and frequency-domain forms.
     * It allows for operations such as normalization, clearing, transforming between
     * time and frequency domains, and other utility functions to manipulate or analyze the waveform.
     */
    class WaveFrame {
    public:
        /// The number of bits that define the size of the waveform (2^kWaveformBits = kWaveformSize).
        static constexpr int kWaveformBits = 11;
        /// The size of the waveform (number of samples per frame).
        static constexpr int kWaveformSize = 1 << kWaveformBits;
        /// The number of real-valued frequency components (half the size + 1).
        static constexpr int kNumRealComplex = kWaveformSize / 2 + 1;
        /// The number of "extra" complex bins to pad after the real frequency components.
        static constexpr int kNumExtraComplex = kWaveformSize - kNumRealComplex;
        /// The default frequency ratio for a WaveFrame (usually 1.0f).
        static constexpr float kDefaultFrequencyRatio = 1.0f;
        /// The default sample rate for a WaveFrame.
        static constexpr float kDefaultSampleRate = 44100.0f;

        /**
         * @brief Constructs a WaveFrame with default frequency ratio and sample rate.
         *
         * The waveform data is initialized to zero, and frequency/time domains are ready for loading or processing.
         */
        WaveFrame() : index(0), frequency_ratio(kDefaultFrequencyRatio), sample_rate(kDefaultSampleRate),
                      time_domain(), frequency_domain() { }

        /**
         * @brief Retrieves the maximum absolute amplitude in the time-domain waveform.
         *
         * @return The maximum absolute sample value found in the time_domain array.
         */
        mono_float getMaxZeroOffset() const;

        /**
         * @brief Normalizes the time-domain waveform samples to have a maximum absolute value of 1.0.
         *
         * @param allow_positive_gain If true, allows scaling up the waveform if it's below the
         *                            normalization threshold, otherwise only scales down.
         */
        void normalize(bool allow_positive_gain = false);

        /**
         * @brief Clears the waveform data, resetting it to default states.
         *
         * This sets the frequency ratio, sample rate to defaults, and zeros all frequency_domain and time_domain samples.
         */
        void clear();

        /**
         * @brief Sets the frequency ratio for this wave frame.
         *
         * @param ratio The new frequency ratio value.
         */
        void setFrequencyRatio(float ratio) { frequency_ratio = ratio; }

        /**
         * @brief Sets the sample rate for this wave frame.
         *
         * @param rate The new sample rate.
         */
        void setSampleRate(float rate) { sample_rate = rate; }

        /**
         * @brief Multiplies all samples in both time and frequency domains by a given value.
         *
         * @param value The value by which to scale the waveform data.
         */
        void multiply(mono_float value);

        /**
         * @brief Loads time-domain data from a given buffer and updates the frequency domain accordingly.
         *
         * @param buffer Pointer to a float array of length kWaveformSize containing the time-domain samples.
         */
        void loadTimeDomain(float* buffer);

        /**
         * @brief Adds another WaveFrame's data to this one, sample-by-sample, in both time and frequency domains.
         *
         * @param source Pointer to the source WaveFrame whose data will be added to this one.
         */
        void addFrom(WaveFrame* source);

        /**
         * @brief Copies another WaveFrame's time and frequency domain data into this one.
         *
         * @param other Pointer to the WaveFrame whose data will be copied.
         */
        void copy(const WaveFrame* other);

        /**
         * @brief Converts the currently loaded time-domain data into frequency-domain representation.
         */
        void toFrequencyDomain();

        /**
         * @brief Converts the currently loaded frequency-domain data into time-domain representation.
         */
        void toTimeDomain();

        /**
         * @brief Removes the DC offset from the waveform.
         *
         * This modifies the time_domain samples by subtracting the DC component. The DC component
         * is derived from the frequency_domain[0] bin.
         */
        void removedDc();

        int index;              ///< The index of this frame in a wavetable.
        float frequency_ratio;  ///< The frequency ratio for this frame (e.g., for pitch scaling).
        float sample_rate;      ///< The sample rate associated with this frame.
        mono_float time_domain[2 * kWaveformSize];   ///< The time-domain data, extended buffer size for FFT alignment.
        std::complex<float> frequency_domain[kWaveformSize]; ///< The frequency-domain representation (complex spectrum).

        /**
         * @brief Gets a pointer to the frequency-domain data interpreted as floats.
         *
         * @return A float pointer to the frequency-domain data (real and imaginary interleaved).
         */
        float* getFrequencyData() { return reinterpret_cast<float*>(frequency_domain); }

        JUCE_LEAK_DETECTOR(WaveFrame)
    };

    /**
     * @brief Holds a set of predefined WaveFrame shapes that can be used as basic building blocks.
     *
     * PredefinedWaveFrames generates standard wave shapes (sin, square, saw, etc.) in a WaveFrame format
     * that can be used for initializing or shaping other wavetables.
     */
    class PredefinedWaveFrames {
    public:
        /// Supported predefined shapes.
        enum Shape {
            kSin,
            kSaturatedSin,
            kTriangle,
            kSquare,
            kPulse,
            kSaw,
            kNumShapes
        };

        /**
         * @brief Constructs the PredefinedWaveFrames, initializing all predefined shapes.
         */
        PredefinedWaveFrames();

        /**
         * @brief Retrieves a pointer to a WaveFrame representing a predefined shape.
         *
         * @param shape The shape enumerated by Shape.
         * @return A pointer to the corresponding predefined WaveFrame.
         */
        static const WaveFrame* getWaveFrame(Shape shape) { return &instance()->wave_frames_[shape]; }

    private:
        /**
         * @brief Gets a singleton instance of PredefinedWaveFrames.
         *
         * @return A pointer to the static instance containing all predefined wave frames.
         */
        static const PredefinedWaveFrames* instance() {
            static const PredefinedWaveFrames wave_frames;
            return &wave_frames;
        }

        void createSin(WaveFrame& wave_frame);
        void createSaturatedSin(WaveFrame& wave_frame);
        void createTriangle(WaveFrame& wave_frame);
        void createSquare(WaveFrame& wave_frame);
        void createPulse(WaveFrame& wave_frame);
        void createSaw(WaveFrame& wave_frame);

        WaveFrame wave_frames_[kNumShapes]; ///< An array of WaveFrames representing the predefined shapes.
    };
} // namespace vital
