#include "wave_frame.h"
#include "futils.h"
#include "fourier_transform.h"

namespace vital {

    void WaveFrame::clear() {
        frequency_ratio = kDefaultFrequencyRatio;
        sample_rate = kDefaultSampleRate;
        for (int i = 0; i < kWaveformSize; ++i) {
            frequency_domain[i] = 0.0f;
            time_domain[i] = 0.0f;
        }
    }

    void WaveFrame::multiply(mono_float value) {
        for (int i = 0; i < kWaveformSize; ++i) {
            time_domain[i] *= value;
            frequency_domain[i] *= value;
        }
    }

    void WaveFrame::loadTimeDomain(float* buffer) {
        for (int i = 0; i < kWaveformSize; ++i)
            time_domain[i] = buffer[i];

        toFrequencyDomain();
    }

    mono_float WaveFrame::getMaxZeroOffset() const {
        mono_float max = 0.0f;
        for (int i = 0; i < kWaveformSize; ++i)
            max = std::max(max, fabsf(time_domain[i]));

        return max;
    }

    void WaveFrame::normalize(bool allow_positive_gain) {
        constexpr mono_float kMaxInverseMult = 0.0000001f;
        mono_float max = getMaxZeroOffset();
        mono_float min = 1.0f;
        if (allow_positive_gain)
            min = kMaxInverseMult;

        mono_float normalization = 1.0f / std::max(min, max);
        for (int i = 0; i < kWaveformSize; ++i)
            time_domain[i] *= normalization;
    }

    void WaveFrame::addFrom(WaveFrame* source) {
        for (int i = 0; i < kWaveformSize; ++i) {
            time_domain[i] += source->time_domain[i];
            frequency_domain[i] += source->frequency_domain[i];
        }
    }

    void WaveFrame::copy(const WaveFrame* other) {
        for (int i = 0; i < kWaveformSize; ++i) {
            frequency_domain[i] = other->frequency_domain[i];
            time_domain[i] = other->time_domain[i];
        }
    }

    void WaveFrame::toFrequencyDomain() {
        /**
         * @brief Converts the time_domain data into the frequency_domain representation using FFT.
         *
         * This uses a forward real-to-complex FFT. The frequency_domain will then contain
         * complex frequency bins representing the spectrum of the waveform.
         */
        float* frequency_data = getFrequencyData();
        memcpy(frequency_data, time_domain, kWaveformSize * sizeof(float));
        memset(frequency_data + kWaveformSize, 0, kWaveformSize * sizeof(float));
        FFT<kWaveformBits>::transform()->transformRealForward(frequency_data);
    }

    void WaveFrame::toTimeDomain() {
        /**
         * @brief Converts the frequency_domain data back into the time_domain representation using an inverse FFT.
         *
         * This restores the waveform from its frequency bins. If modifications were made in the frequency domain,
         * this will produce the corresponding time-domain waveform.
         */
        float* frequency_data = getFrequencyData();
        memcpy(time_domain, frequency_domain, 2 * kNumRealComplex * sizeof(float));
        memset(frequency_data + 2 * kNumRealComplex, 0, 2 * kNumExtraComplex * sizeof(float));
        FFT<kWaveformBits>::transform()->transformRealInverse(time_domain);
    }

    void WaveFrame::removedDc() {
        /**
         * @brief Removes DC offset from the waveform by subtracting the imaginary part of frequency_domain[0].
         *
         * The DC offset is found in the imaginary component of the first frequency bin.
         * Subtracting this value from time_domain samples removes any constant offset.
         */
        float offset = frequency_domain[0].imag();
        frequency_domain[0] = 0.0f;
        for (int i = 0; i < kWaveformSize; ++i)
            time_domain[i] -= offset;
    }

    PredefinedWaveFrames::PredefinedWaveFrames() {
        /**
         * @brief Constructs predefined wave frames for each shape using helper methods.
         */
        createSin(wave_frames_[kSin]);
        createSaturatedSin(wave_frames_[kSaturatedSin]);
        createTriangle(wave_frames_[kTriangle]);
        createSquare(wave_frames_[kSquare]);
        createSaw(wave_frames_[kSaw]);
        createPulse(wave_frames_[kPulse]);
    }

    void PredefinedWaveFrames::createSin(WaveFrame& wave_frame) {
        /**
         * @brief Creates a sine waveform.
         *
         * For a perfect sine, only the fundamental frequency bin is set. The half-waveform is placed
         * in the frequency domain, then converted to time domain.
         */
        int half_waveform = WaveFrame::kWaveformSize / 2;
        wave_frame.frequency_domain[1] = half_waveform;
        wave_frame.toTimeDomain();
    }

    void PredefinedWaveFrames::createSaturatedSin(WaveFrame& wave_frame) {
        /**
         * @brief Creates a saturated sine waveform by first creating a strong sine,
         *        then applying a tanh function to distort it, and finally converting back to frequency domain.
         */
        wave_frame.frequency_domain[1] = WaveFrame::kWaveformSize;
        wave_frame.toTimeDomain();
        for (int i = 0; i < WaveFrame::kWaveformSize; ++i) {
            wave_frame.time_domain[i] = futils::tanh(wave_frame.time_domain[i]);
        }
        wave_frame.toFrequencyDomain();
    }

    void PredefinedWaveFrames::createTriangle(WaveFrame& wave_frame) {
        /**
         * @brief Creates a triangle waveform by linearly interpolating between values
         *        over four equal sections of the waveform cycle.
         */
        int section_size = WaveFrame::kWaveformSize / 4;
        for (int i = 0; i < section_size; ++i) {
            mono_float t = i / (section_size * 1.0f);
            wave_frame.time_domain[i] = 1.0f - t;
            wave_frame.time_domain[i + section_size] = -t;
            wave_frame.time_domain[i + 2 * section_size] = t - 1.0f;
            wave_frame.time_domain[i + 3 * section_size] = t;
        }
        wave_frame.toFrequencyDomain();
    }

    void PredefinedWaveFrames::createSquare(WaveFrame& wave_frame) {
        /**
         * @brief Creates a square waveform by setting half of the cycle to 1.0
         *        and the other half to -1.0, arranged in quarters for symmetry.
         */
        int section_size = WaveFrame::kWaveformSize / 4;
        for (int i = 0; i < section_size; ++i) {
            wave_frame.time_domain[i] = 1.0f;
            wave_frame.time_domain[i + section_size] = -1.0f;
            wave_frame.time_domain[i + 2 * section_size] = -1.0f;
            wave_frame.time_domain[i + 3 * section_size] = 1.0f;
        }
        wave_frame.toFrequencyDomain();
    }

    void PredefinedWaveFrames::createPulse(WaveFrame& wave_frame) {
        /**
         * @brief Creates a pulse waveform by having one section at 1.0 and
         *        the rest at -1.0, resulting in a single short positive pulse in each cycle.
         */
        int sections = 4;
        int pulse_size = WaveFrame::kWaveformSize / sections;

        for (int i = 0; i < pulse_size; ++i) {
            wave_frame.time_domain[i + (sections - 1) * pulse_size] = 1.0f;
            for (int s = 0; s < sections - 1; ++s)
                wave_frame.time_domain[i + s * pulse_size] = -1.0f;
        }
        wave_frame.toFrequencyDomain();
    }

    void PredefinedWaveFrames::createSaw(WaveFrame& wave_frame) {
        /**
         * @brief Creates a sawtooth waveform by linearly increasing over half the cycle
         *        and then wrapping around, creating a discontinuity typical of a sawtooth.
         */
        int section_size = WaveFrame::kWaveformSize / 2;
        for (int i = 0; i < section_size; ++i) {
            mono_float t = i / (section_size * 1.0f);
            wave_frame.time_domain[(i + WaveFrame::kWaveformSize / 4) % WaveFrame::kWaveformSize] = t - 1.0f;
            wave_frame.time_domain[(i + section_size + WaveFrame::kWaveformSize / 4) % WaveFrame::kWaveformSize] = t;
        }
        wave_frame.toFrequencyDomain();
    }

} // namespace vital
