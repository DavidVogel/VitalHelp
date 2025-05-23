#include "synth_constants.h"
#include "fourier_transform.h"
#include "futils.h"
#include "wavetable.h"

namespace vital {
  /**
   * @brief Number of harmonics based on the WaveFrame size.
   */
  static constexpr int kNumHarmonics = WaveFrame::kWaveformSize / 2 + 1;

  /**
   * @brief Maximum allowable formant shift for spectral morphing.
   */
  static constexpr mono_float kMaxFormantShift = 1.0f;

  /**
   * @brief Maximum allowable even/odd formant shift for spectral morphing.
   */
  static constexpr mono_float kMaxEvenOddFormantShift = 2.0f;

  /**
   * @brief Maximum harmonic scaling factor for harmonic scaling morph.
   */
  static constexpr mono_float kMaxHarmonicScale = 4.0f;

  /**
   * @brief Maximum inharmonic scaling factor for inharmonic morph.
   */
  static constexpr mono_float kMaxInharmonicScale = 12.0f;

  /**
   * @brief Maximum split scale factor for certain spectral morph operations.
   */
  static constexpr int kMaxSplitScale = 2;

  /**
   * @brief Maximum split shift (in semitones) for spectral morph operations.
   */
  static constexpr mono_float kMaxSplitShift = 24.0f;

  /**
   * @brief Number of stages used for random amplitude morphing.
   */
  static constexpr int kRandomAmplitudeStages = 16;

  /**
   * @brief Scaling factor for phase dispersion morph operations.
   */
  static constexpr mono_float kPhaseDisperseScale = 0.05f;

  /**
   * @brief Scaling factor for skew-based morph operations.
   */
  static constexpr mono_float kSkewScale = 16.0f;

  /**
   * @brief Maximum poly index based on waveform size and poly_float vector size.
   */
  static constexpr int kMaxPolyIndex = WaveFrame::kWaveformSize / poly_float::kSize;

  /**
   * @brief Performs an inverse Fourier transform on a buffer and wraps the data to handle waveform boundaries.
   *
   * This function modifies the given buffer by calling the inverse Fourier transform and then wrapping
   * the data for seamless waveform looping. It is an internal helper function used by various morph operations.
   *
   * @param transform A pointer to a FourierTransform object for performing the inverse transform.
   * @param buffer Pointer to a mono_float buffer with waveform data.
   */
  static force_inline void transformAndWrapBuffer(FourierTransform* transform, mono_float* buffer) {
      transform->transformRealInverse(buffer + poly_float::kSize);

      for (int i = 0; i < poly_float::kSize; ++i) {
          buffer[i] = buffer[i + Wavetable::kWaveformSize];
          buffer[i + Wavetable::kWaveformSize + poly_float::kSize] = buffer[i + poly_float::kSize];
      }

#if DEBUG
      for (int i = 0; i < Wavetable::kWaveformSize + 2 * poly_float::kSize; ++i)
    VITAL_ASSERT(utils::isFinite(buffer[i]));
#endif
  }

  /**
   * @brief Overload of transformAndWrapBuffer for poly_float buffers.
   *
   * @param transform Pointer to a FourierTransform object.
   * @param buffer Pointer to a poly_float buffer with waveform data.
   */
  static force_inline void transformAndWrapBuffer(FourierTransform* transform, poly_float* buffer) {
      transformAndWrapBuffer(transform, (mono_float*)buffer);
  }

  /**
   * @brief A passthrough morph operation that outputs the wavetable data without modification.
   *
   * This serves as a baseline or identity morph. It directly uses the frequency amplitudes and normalized
   * frequencies from the wavetable data, then applies the inverse Fourier transform to produce the final waveform.
   *
   * @param wavetable_data Pointer to the Wavetable::WavetableData containing frequency domain data.
   * @param wavetable_index Index of the wavetable frame within the data.
   * @param dest Pointer to the destination buffer (poly_float) for the transformed waveform.
   * @param transform Pointer to a FourierTransform object.
   * @param shift Morph shift parameter (unused in this morph).
   * @param last_harmonic The highest harmonic index considered.
   * @param data_buffer Additional data buffer (unused in this morph).
   */
  static void passthroughMorph(const Wavetable::WavetableData* wavetable_data,
                               int wavetable_index, poly_float* dest, FourierTransform* transform,
                               float shift, int last_harmonic, const poly_float* data_buffer) {
    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    int last_index = 2 * last_harmonic / poly_float::kSize;

    for (int i = 0; i <= last_index; ++i)
      wave_start[i] = frequency_amplitudes[i] * normalized_frequencies[i];

    for (int i = last_index + 1; i < kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A Shepard-tone inspired morph that blends fundamental and higher harmonic amplitudes.
   *
   * This creates a morphing effect reminiscent of an endlessly rising or falling sound by mixing
   * fundamental and shepard-like amplitudes and phases.
   *
   * @param wavetable_data Pointer to the Wavetable::WavetableData.
   * @param wavetable_index Index of the wavetable frame.
   * @param dest Destination buffer for the transformed waveform.
   * @param transform Pointer to a FourierTransform for inverse transform.
   * @param shift The amount of Shepard morph to apply (0.0 to 1.0).
   * @param last_harmonic The highest harmonic index to process.
   * @param data_buffer Additional data buffer (unused).
   */
  static void shepardMorph(const Wavetable::WavetableData* wavetable_data,
                           int wavetable_index, poly_float* dest, FourierTransform* transform,
                           float shift, int last_harmonic, const poly_float* data_buffer) {
    static constexpr float kMinAmplitudeRatio = 2.0f;
    static constexpr float kMinAmplitudeAdd = 0.001f;
    const poly_float* poly_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* poly_normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* poly_wave_start = dest + 1;
    int last_index = 2 * last_harmonic / poly_float::kSize;

    float regular_amount = 1.0f - shift;
    for (int i = 0; i <= last_index; ++i) {
      poly_float value = poly_amplitudes[i] * poly_normalized_frequencies[i] * regular_amount;
      poly_wave_start[i] = value & constants::kSecondMask;
    }

    for (int i = last_index + 1; i < kMaxPolyIndex; ++i)
      poly_wave_start[i] = 0.0f;

    const mono_float* frequency_amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[wavetable_index];
    const mono_float* normalized = (const mono_float*)wavetable_data->normalized_frequencies[wavetable_index];
    const mono_float* phases = (const mono_float*)wavetable_data->phases[wavetable_index];
    mono_float* wave_start = (mono_float*)(dest + 1);

    for (int i = 0; i <= last_harmonic; i += 2) {
      int real_index = 2 * i;
      int imag_index = real_index + 1;

      float fundamental_amplitude = frequency_amplitudes[real_index];
      float shepard_amplitude = frequency_amplitudes[i];
      float amplitude = fundamental_amplitude + (shepard_amplitude - fundamental_amplitude) * shift;

      float ratio = (fundamental_amplitude + kMinAmplitudeAdd) / (shepard_amplitude + kMinAmplitudeAdd);
      float real, imag;
      if (ratio < kMinAmplitudeRatio && ratio > (1.0f / kMinAmplitudeRatio)) {
        float fundamental_phase = phases[real_index] * (0.5f / kPi);
        float shepard_phase = phases[i] * (0.5f / kPi);
        float delta_phase = shepard_phase - fundamental_phase;
        int wraps = delta_phase;
        wraps = (wraps + 1) / 2;
        delta_phase -= 2.0f * wraps;

        float phase = fundamental_phase + delta_phase * shift;
        real = futils::sin(utils::mod(phase + 0.75f)[0] - 0.5f);
        imag = futils::sin(utils::mod(phase + 0.5f)[0] - 0.5f);
      }
      else {
        float fundamental_real = normalized[real_index];
        real = (normalized[i] - fundamental_real) * shift + fundamental_real;
        float fundamental_imag = normalized[real_index + 1];
        imag = (normalized[i + 1] - fundamental_imag) * shift + fundamental_imag;
      }

      wave_start[real_index] = amplitude * real;
      wave_start[imag_index] = amplitude * imag;
    }

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A morph that skews the wavetable frame selection based on a shift parameter.
   *
   * If multiple wavetable frames exist, it interpolates between them depending on the shift and harmonic index,
   * creating a skewed morphing effect across frames.
   *
   * @param wavetable_data Pointer to the Wavetable::WavetableData.
   * @param wavetable_index Current frame index in the wavetable.
   * @param dest Destination buffer for the transformed waveform.
   * @param transform FourierTransform for inverse transform.
   * @param shift Amount of skew morph to apply.
   * @param last_harmonic The highest harmonic to process.
   * @param data_buffer Additional data buffer (unused).
   */
  static void wavetableSkewMorph(const Wavetable::WavetableData* wavetable_data,
                                 int wavetable_index, poly_float* dest, FourierTransform* transform,
                                 float shift, int last_harmonic, const poly_float* data_buffer) {
    mono_float* wave_start = (mono_float*)(dest + 1);

    int num_frames = wavetable_data->num_frames;
    if (num_frames <= 1) {
      passthroughMorph(wavetable_data, wavetable_index, dest, transform, shift, last_harmonic, data_buffer);
      return;
    }

    float dc_amplitude = wavetable_data->frequency_amplitudes[wavetable_index][0][0];
    float dc_real = wavetable_data->normalized_frequencies[wavetable_index][0][0];
    float dc_imag = wavetable_data->normalized_frequencies[wavetable_index][0][1];
    wave_start[0] = dc_amplitude * dc_real;
    wave_start[1] = dc_amplitude * dc_imag;

    float max_frame = kNumOscillatorWaveFrames - 1;
    float base_wavetable_t = wavetable_index / max_frame;
    for (int i = 1; i <= last_harmonic; ++i) {
      float shift_scale = futils::log2(i) / Wavetable::kFrequencyBins;
      poly_float base_value = poly_float(1.0f) - utils::mod((base_wavetable_t + shift * shift_scale) * 0.5f) * 2.0f;
      float shifted_index = (1.0f - poly_float::abs(base_value)[0]) * max_frame;
      int from_index = std::min<int>(shifted_index, num_frames - 2);
      float t = std::min(1.0f, shifted_index - from_index);
      int to_index = from_index + 1;

      int real_index = 2 * i;
      int imaginary_index = real_index + 1;
      const mono_float* from_amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[from_index];
      const mono_float* to_amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[to_index];
      float amplitude = utils::interpolate(from_amplitudes[real_index], to_amplitudes[real_index], t);

      const mono_float* from_normalized = (const mono_float*)wavetable_data->normalized_frequencies[from_index];
      const mono_float* to_normalized = (const mono_float*)wavetable_data->normalized_frequencies[to_index];
      float real = utils::interpolate(from_normalized[real_index], to_normalized[real_index], t);
      float imag = utils::interpolate(from_normalized[imaginary_index], to_normalized[imaginary_index], t);

      wave_start[real_index] = amplitude * real;
      wave_start[imaginary_index] = amplitude * imag;
    }

    for (int i = 2 * (last_harmonic + 1); i < 2 * WaveFrame::kWaveformSize; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A phase morph that alters harmonic phases based on a given phase shift amount.
   *
   * Adjusts harmonics to create timbral changes driven by phase modifications.
   *
   * @param wavetable_data Pointer to wavetable data.
   * @param wavetable_index Frame index within the wavetable.
   * @param dest Destination buffer.
   * @param transform Fourier transform for inverse processing.
   * @param phase_shift Amount of phase shift morph.
   * @param last_harmonic Highest harmonic index to process.
   * @param data_buffer Unused additional buffer.
   */
  static void phaseMorph(const Wavetable::WavetableData* wavetable_data,
                         int wavetable_index, poly_float* dest, FourierTransform* transform,
                         float phase_shift, int last_harmonic, const poly_float* data_buffer) {
    static constexpr float kCenterMorph = 24.0f;

    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    int last_index = 2 * last_harmonic / poly_float::kSize;

    float offset = -(kCenterMorph - 1.0f) * (kCenterMorph - 1.0f) * phase_shift;
    poly_float value_offset(0.0f, 0.0f, 1.0f, 1.0f);
    poly_float phase_offset(0.25f, 0.0f, 0.25f, 0.0f);
    poly_float scale = 0.5f / kPi;
    for (int i = 0; i <= last_index; ++i) {
      poly_float amplitude = frequency_amplitudes[i];
      poly_float normalized = normalized_frequencies[i];
      poly_float index = value_offset + 2.0f * i;

      poly_float delta_center = (index - kCenterMorph) * (index - kCenterMorph) * phase_shift + offset;
      poly_float phase = utils::mod(delta_center * scale + phase_offset);
      poly_float shift = futils::sin1(phase);

      poly_float match_mult = normalized * shift;
      poly_float switch_mult = utils::swapStereo(normalized) * shift;
      poly_float real = match_mult - utils::swapStereo(match_mult);
      poly_float imag = switch_mult + utils::swapStereo(switch_mult);

      wave_start[i] = amplitude * utils::maskLoad(imag, real, constants::kLeftMask);
    }
    for (int i = last_index + 1; i < kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A smear morph that progressively blurs harmonic amplitudes.
   *
   * This creates a smoother, more uniform harmonic distribution by interpolating amplitudes over harmonics.
   *
   * @param wavetable_data Wavetable data source.
   * @param wavetable_index Frame index in wavetable.
   * @param dest Destination buffer.
   * @param transform Fourier transform for inverse transform.
   * @param smear Smear amount (0.0 to 1.0).
   * @param last_harmonic Highest harmonic to consider.
   * @param data_buffer Unused additional buffer.
   */
  static void smearMorph(const Wavetable::WavetableData* wavetable_data,
                         int wavetable_index, poly_float* dest, FourierTransform* transform,
                         float smear, int last_harmonic, const poly_float* data_buffer) {
    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    int last_index = 2 * last_harmonic / poly_float::kSize;

    poly_float amplitude = frequency_amplitudes[0] * (1.0f - smear);
    wave_start[0] = amplitude * normalized_frequencies[0];

    for (int i = 1; i <= last_index; ++i) {
      poly_float original_amplitude = frequency_amplitudes[i];
      amplitude = utils::interpolate(original_amplitude, amplitude, smear);

      wave_start[i] = amplitude * normalized_frequencies[i];
      amplitude *= (i + 0.25f) / i;
    }

    for (int i = last_index + 1; i < kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A low-pass morph that gradually removes higher harmonics.
   *
   * Sets a cutoff and attenuates harmonics above that cutoff, resulting in a darker timbre.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Destination buffer.
   * @param transform Fourier transform instance.
   * @param cutoff_t Normalized cutoff parameter for low-pass effect.
   * @param last_harmonic Highest harmonic index.
   * @param data_buffer Unused additional buffer.
   */
  static void lowPassMorph(const Wavetable::WavetableData* wavetable_data,
                           int wavetable_index, poly_float* dest, FourierTransform* transform,
                           float cutoff_t, int last_harmonic, const poly_float* data_buffer) {
    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    float cutoff = futils::pow(2.0f, (Wavetable::kFrequencyBins - 1) * cutoff_t) + 1.0f;
    int last_index = 2 * last_harmonic / poly_float::kSize;
    float poly_cutoff = std::min(last_index + 1.0f, 2.0f * cutoff / poly_float::kSize);
    last_index = std::min<int>(last_index, poly_cutoff);
    float t = poly_float::kSize * (poly_cutoff - last_index) / 2.0f;

    for (int i = 0; i <= last_index; ++i)
      wave_start[i] = frequency_amplitudes[i] * normalized_frequencies[i];

    for (int i = last_index + 1; i <= kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    poly_float last_mult = 1.0f;
    if (t >= 1.0f)
      last_mult = poly_float(1.0f, 1.0f, t - 1.0f, t - 1.0f);
    else
      last_mult = poly_float(t, t, 0.0f, 0.0f);

    wave_start[last_index] = wave_start[last_index] * last_mult;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A high-pass morph that removes lower harmonics, leaving only higher ones.
   *
   * Opposite of lowPassMorph, this emphasizes the higher part of the spectrum.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Destination buffer.
   * @param transform Fourier transform.
   * @param cutoff_t Normalized cutoff for high-pass effect.
   * @param last_harmonic Highest harmonic index.
   * @param data_buffer Unused additional buffer.
   */
  static void highPassMorph(const Wavetable::WavetableData* wavetable_data,
                            int wavetable_index, poly_float* dest, FourierTransform* transform,
                            float cutoff_t, int last_harmonic, const poly_float* data_buffer) {
    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    float cutoff = futils::pow(2.0f, (Wavetable::kFrequencyBins - 1) * cutoff_t);
    cutoff *= (kNumHarmonics + 1.0f) / kNumHarmonics;
    int last_index = 2 * last_harmonic / poly_float::kSize;
    float poly_cutoff = std::min(last_index + 1.0f, 2.0f * cutoff / poly_float::kSize);
    int start_index = poly_cutoff;
    float t = poly_float::kSize * (poly_cutoff - start_index) / 2.0f;

    for (int i = 0; i < start_index; ++i)
      wave_start[i] = 0.0f;

    for (int i = start_index; i <= last_index; ++i)
      wave_start[i] = frequency_amplitudes[i] * normalized_frequencies[i];

    for (int i = last_index + 1; i <= kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    poly_float last_mult = 1.0f;
    if (t >= 1.0f)
      last_mult = poly_float(0.0f, 0.0f, 2.0f - t, 2.0f - t);
    else
      last_mult = poly_float(1.0f - t, 1.0f - t, 1.0f, 1.0f);

    wave_start[start_index] = wave_start[start_index] * last_mult;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief Morph that separates even and odd harmonics and resynthesizes them with shifts.
   *
   * This can create vocal-formant-like effects or interesting tonal changes by scaling and shifting
   * the distribution of even and odd harmonics.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Output buffer.
   * @param transform Fourier transform.
   * @param shift Shift amount affecting even/odd harmonic distribution.
   * @param last_harmonic Highest harmonic.
   * @param data_buffer Unused additional buffer.
   */
  static void evenOddVocodeMorph(const Wavetable::WavetableData* wavetable_data,
                                 int wavetable_index, poly_float* dest, FourierTransform* transform,
                                 float shift, int last_harmonic, const poly_float* data_buffer) {
    mono_float* wave_start = (mono_float*)(dest + 1);
    int last_index = std::min<int>(last_harmonic, WaveFrame::kWaveformSize / (2 * shift));

    const mono_float* amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[wavetable_index];
    const mono_float* normalized = (const mono_float*)wavetable_data->normalized_frequencies[wavetable_index];

    float dc_amplitude = amplitudes[0];
    wave_start[0] = dc_amplitude * normalized[0];
    wave_start[1] = dc_amplitude * normalized[1];

    for (int i = 1; i <= last_index; ++i) {
      float shifted_index = std::max(1.0f, i * shift);
      int index_start = shifted_index;
      index_start = index_start - (i + index_start) % 2;
      VITAL_ASSERT(index_start >= 0 && index_start < kNumHarmonics);

      float t = (shifted_index - index_start) * 0.5f;
      int real_index1 = 2 * index_start;
      int real_index2 = real_index1 + 4;
      float amplitude_from = amplitudes[real_index1];
      float amplitude_to = amplitudes[real_index2];
      float real_from = amplitude_from * normalized[real_index1];
      float real_to = amplitude_to * normalized[real_index2];
      float imag_from = amplitude_from * normalized[real_index1 + 1];
      float imag_to = amplitude_to * normalized[real_index2 + 1];

      VITAL_ASSERT(utils::isFinite(real_from) && utils::isFinite(real_to));
      VITAL_ASSERT(utils::isFinite(imag_from) && utils::isFinite(imag_to));

      int real_index = 2 * i;
      wave_start[real_index] = shift * utils::interpolate(real_from, real_to, t);
      wave_start[real_index + 1] = shift * utils::interpolate(imag_from, imag_to, t);
    }
    for (int i = 2 * (last_index + 1); i < WaveFrame::kWaveformSize; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief A morph that scales harmonic positions, effectively changing the harmonic spacing.
   *
   * This can compress or expand the harmonic series, altering the perceived pitch/timbre relationship.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Destination buffer.
   * @param transform Fourier transform.
   * @param shift Harmonic scaling factor.
   * @param last_harmonic Highest harmonic.
   * @param data_buffer Unused additional buffer.
   */
  static void harmonicScaleMorph(const Wavetable::WavetableData* wavetable_data,
                                 int wavetable_index, poly_float* dest, FourierTransform* transform,
                                 float shift, int last_harmonic, const poly_float* data_buffer) {
    mono_float* wave_start = (mono_float*)(dest + 1);
    memset(wave_start, 0, 2 * WaveFrame::kWaveformSize * sizeof(mono_float));
    int harmonics = std::min<int>(kNumHarmonics, (last_harmonic - 1) / shift + 1);

    const mono_float* amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[wavetable_index];
    const mono_float* normalized = (const mono_float*)wavetable_data->normalized_frequencies[wavetable_index];

    float dc_amplitude = amplitudes[0];
    wave_start[0] = dc_amplitude * normalized[0];
    wave_start[1] = dc_amplitude * normalized[1];

    for (int i = 1; i <= harmonics; ++i) {
      float shifted_index = std::max(1.0f, (i - 1) * shift + 1);
      int dest_index = shifted_index;
      VITAL_ASSERT(dest_index >= 0 && dest_index <= kNumHarmonics);

      float t = shifted_index - dest_index;
      float real_amount = normalized[2 * i];
      float imag_amount = normalized[2 * i + 1];
      float amplitude = amplitudes[2 * i];
      float amplitude1 = (1.0f - t) * amplitude;
      float amplitude2 = t * amplitude;

      int real_index1 = 2 * dest_index;
      int imaginary_index1 = real_index1 + 1;
      wave_start[real_index1] += amplitude1 * real_amount;
      wave_start[imaginary_index1] += amplitude1 * imag_amount;

      int real_index2 = imaginary_index1 + 1;
      int imaginary_index2 = real_index2 + 1;
      wave_start[real_index2] += amplitude2 * real_amount;
      wave_start[imaginary_index2] += amplitude2 * imag_amount;
    }

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief Inharmonic scale morph that changes the harmonic relationships to create inharmonic spectra.
   *
   * By applying a multiplier to harmonic indices, this morph creates more bell-like or inharmonic sounds.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Destination buffer.
   * @param transform Fourier transform.
   * @param mult Inharmonic scaling multiplier.
   * @param last_harmonic Highest harmonic to consider.
   * @param data_buffer Unused additional buffer.
   */
  static void inharmonicScaleMorph(const Wavetable::WavetableData* wavetable_data,
                                   int wavetable_index, poly_float* dest, FourierTransform* transform,
                                   float mult, int last_harmonic, const poly_float* data_buffer) {
    poly_float* poly_data_start = dest + 2 + kMaxPolyIndex;

    poly_float offset(0.0f, 2.0f, 1.0f, 3.0f);
    for (int i = 0; i <= kMaxPolyIndex / 2; ++i) {
      poly_float index = offset + i * 4;
      poly_float octave = futils::log2(index);
      poly_float power = octave * (1.0f / (Wavetable::kFrequencyBins - 1.0f));
      poly_float shift = futils::pow(mult, power);
      poly_float shifted_index = utils::max(1.0f, shift * (index - 1.0f) + 1.0f);
      poly_data_start[2 * i] = shifted_index;
      poly_data_start[2 * i + 1] = utils::swapStereo(shifted_index);
    }

    const mono_float* amplitudes = (const mono_float*)wavetable_data->frequency_amplitudes[wavetable_index];
    const mono_float* normalized = (const mono_float*)wavetable_data->normalized_frequencies[wavetable_index];
    mono_float* wave_start = (mono_float*)(dest + 1);
    mono_float* index_data = (mono_float*)(poly_data_start);
    memset(wave_start, 0, WaveFrame::kWaveformSize * sizeof(mono_float));

    float dc_amplitude = amplitudes[0];
    wave_start[0] = dc_amplitude * normalized[0];
    wave_start[1] = dc_amplitude * normalized[1];

    int processed_index = 1;
    for (; processed_index <= kNumHarmonics; ++processed_index) {
      int index = 2 * processed_index;
      float shifted_index = index_data[index];
      int dest_index = shifted_index;
      if (dest_index > 2 * last_harmonic)
        break;
      VITAL_ASSERT(dest_index >= 0 && dest_index <= kNumHarmonics * 2);

      float t = shifted_index - dest_index;
      float amplitude = amplitudes[index];
      float real = normalized[index];
      float imag = normalized[index + 1];
      VITAL_ASSERT(real < 10000.0f);
      VITAL_ASSERT(imag < 10000.0f);

      int real_index = 2 * dest_index;
      float value1 = (1.0f - t) * amplitude;
      wave_start[real_index] += value1 * real;
      wave_start[real_index + 1] += value1 * imag;
      float value2 = t * amplitude;
      wave_start[real_index + 2] += value2 * real;
      wave_start[real_index + 3] += value2 * imag;
    }

    transformAndWrapBuffer(transform, dest);
  }

  /**
   * @brief Random amplitude morph adds a stochastic element to harmonic amplitudes.
   *
   * Uses random stages to alter harmonic amplitudes, producing noisy, textural timbral changes.
   *
   * @param wavetable_data Wavetable data.
   * @param wavetable_index Frame index.
   * @param dest Destination buffer.
   * @param transform Fourier transform.
   * @param shift Morph amount determining randomization.
   * @param last_harmonic Highest harmonic.
   * @param data_buffer Additional data used for storing random amplitudes.
   */
  static void randomAmplitudeMorph(const Wavetable::WavetableData* wavetable_data,
                                   int wavetable_index, poly_float* dest, FourierTransform* transform,
                                   float shift, int last_harmonic, const poly_float* data_buffer) {
    const poly_float* frequency_amplitudes = wavetable_data->frequency_amplitudes[wavetable_index];
    const poly_float* normalized_frequencies = wavetable_data->normalized_frequencies[wavetable_index];

    poly_float* wave_start = dest + 1;
    int last_index = 2 * last_harmonic / poly_float::kSize;
    int index = std::min<int>(shift, kRandomAmplitudeStages - 2);
    float amount = shift / kRandomAmplitudeStages;
    float t = shift - index;
    poly_float scale = shift;
    poly_float center = poly_float(1.0f) - scale;
    poly_float mult = 1.0f + shift;

    const poly_float* buffer1 = data_buffer + index * kNumHarmonics / poly_float::kSize;
    const poly_float* buffer2 = data_buffer + (index + 1) * kNumHarmonics / poly_float::kSize;

    poly_float random_t(amount, 1.0f - amount, amount, 1.0f - amount);
    for (int i = 0; i <= last_index; ++i) {
      poly_float random_value1 = buffer1[i] & constants::kLeftMask;
      random_value1 = random_value1 + utils::swapStereo(random_value1);
      poly_float random_value2 = buffer2[i] & constants::kLeftMask;
      random_value2 = random_value2 + utils::swapStereo(random_value2);
      poly_float random1 = mult * utils::max(center - scale * random_value1, 0.0f);
      poly_float random2 = mult * utils::max(center - scale * random_value2, 0.0f);
      poly_float amplitude = utils::min(utils::interpolate(random1, random2, t) * frequency_amplitudes[i], 1024.0f);

      wave_start[i] = amplitude * normalized_frequencies[i];
    }
    for (int i = last_index + 1; i <= kMaxPolyIndex; ++i)
      wave_start[i] = 0.0f;

    transformAndWrapBuffer(transform, dest);
  }
} // namespace vital
