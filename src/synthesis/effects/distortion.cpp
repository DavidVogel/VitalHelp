#include "distortion.h"

#include "synth_constants.h"

#include <climits>

namespace vital {

  namespace {
    /**
     * @brief Applies a linear fold distortion.
     *
     * Adds the input sample (scaled by `drive`) to a base offset,
     * then folds it repeatedly in the [-1, 1] range.
     *
     * @param value Input sample.
     * @param drive Distortion multiplier (scale for the folding).
     * @return Distorted sample via linear folding.
     */
    force_inline poly_float linearFold(poly_float value, poly_float drive) {
      poly_float adjust = value * drive * 0.25f + 0.75f;
      poly_float range = utils::mod(adjust);
      return poly_float::abs(range * -4.0f + 2.0f) - 1.0f;
    }

    /**
     * @brief Applies a sine-based fold distortion.
     *
     * Maps the input sample (scaled by `drive`) to a sine waveform.
     *
     * @param value Input sample.
     * @param drive Distortion multiplier.
     * @return Distorted sample via sine folding.
     */
    force_inline poly_float sinFold(poly_float value, poly_float drive) {
      poly_float adjust = value * drive * -0.25f + 0.5f;
      poly_float range = utils::mod(adjust);
      return futils::sin1(range);
    }

    /**
     * @brief Applies a soft clipping distortion using tanh.
     *
     * @param value Input sample.
     * @param drive Distortion multiplier.
     * @return Soft-clipped sample.
     */
    force_inline poly_float softClip(poly_float value, poly_float drive) {
      return futils::tanh(value * drive);
    }

    /**
     * @brief Applies a hard clipping distortion.
     *
     * @param value Input sample.
     * @param drive Distortion multiplier.
     * @return Hard-clipped sample constrained to [-1, 1].
     */
    force_inline poly_float hardClip(poly_float value, poly_float drive) {
      return utils::clamp(value * drive, -1.0f, 1.0f);
    }

    /**
     * @brief Applies a basic bitcrushing distortion.
     *
     * Quantizes the sample by dividing by `drive` then rounding, then multiplying again.
     *
     * @param value Input sample.
     * @param drive The quantization step size.
     * @return Bitcrushed sample.
     */
    force_inline poly_float bitCrush(poly_float value, poly_float drive) {
      return utils::round(value / drive) * drive;
    }

    /**
     * @brief Reduces an interleaved poly buffer to half-size by combining pairs of voices.
     *
     * For example, if the poly buffer has `num_samples` frames, each containing two voices,
     * this function pairs them and writes a single sample per frame in `audio_out`.
     *
     * @param audio_out Pointer to the reduced-size output.
     * @param audio_in  Pointer to the original poly buffer.
     * @param num_samples Total number of frames in `audio_in`.
     * @return Number of frames written to `audio_out`.
     */
    force_inline int compactAudio(poly_float* audio_out, const poly_float* audio_in, int num_samples) {
      int num_full = num_samples / 2;
      for (int i = 0; i < num_full; ++i) {
        int in_index = 2 * i;
        audio_out[i] = utils::compactFirstVoices(audio_in[in_index], audio_in[in_index + 1]);
      }

      int num_remaining = num_samples % 2;

      if (num_remaining)
        audio_out[num_full] = audio_in[num_samples - 1];

      return num_full + num_remaining;
    }

    /**
     * @brief Expands a half-sized buffer back into an interleaved poly buffer.
     *
     * Inverse of `compactAudio()`. Takes each sample and duplicates it into two voices
     * in `audio_out`. If there's an odd leftover sample, it is placed in the final frame.
     *
     * @param audio_out Pointer to the expanded output buffer.
     * @param audio_in  Pointer to the compacted input buffer.
     * @param num_samples Total number of original frames to rebuild.
     */
    force_inline void expandAudio(poly_float* audio_out, const poly_float* audio_in, int num_samples) {
      int num_full = num_samples / 2;
      if (num_samples % 2)
        audio_out[num_samples - 1] = audio_in[num_full];

      for (int i = num_full - 1; i >= 0; --i) {
        int out_index = 2 * i;
        audio_out[out_index] = audio_in[i];
        audio_out[out_index + 1] = utils::swapVoices(audio_in[i]);
      }
    }
  } // namespace

  /**
   * @brief Retrieves a distorted sample given the type and drive multiplier.
   *
   * Chooses an appropriate function (softClip, hardClip, etc.) based on the `type`.
   *
   * @param type  Distortion type (e.g., soft clip, bitcrush).
   * @param value Input sample to distort.
   * @param drive The scaled drive multiplier or period factor (for downsampling).
   * @return The distorted sample.
   */
  poly_float Distortion::getDrivenValue(int type, poly_float value, poly_float drive) {
    switch(type) {
      case kSoftClip:
        return softClip(value, drive);
      case kHardClip:
        return hardClip(value, drive);
      case kLinearFold:
        return linearFold(value, drive);
      case kSinFold:
        return sinFold(value, drive);
      case kBitCrush:
        return bitCrush(value, drive);
      case kDownSample:
        return bitCrush(value, poly_float(1.001f) - poly_float(kPeriodScale) / drive);
      default:
        return value;
    }
  }

  /**
   * @brief Constructs a Distortion Processor with default parameters.
   *
   * Initializes the internal buffers for the last distorted value and sample accumulation
   * (used in downsampling).
   */
  Distortion::Distortion() : Processor(kNumInputs, kNumOutputs),
                             last_distorted_value_(0.0f), current_samples_(0.0f), type_(kNumTypes) { }

  /**
   * @brief Templated processing function for time-invariant distortions.
   *
   * Each sample is processed by a distortion function that takes a sample and scaled drive value,
   * then writes the result to `audio_out`.
   *
   * @tparam distort The distortion function pointer.
   * @tparam scale   The drive scaling function pointer.
   * @param num_samples Number of samples to process.
   * @param audio_in    Pointer to the input audio buffer.
   * @param drive       Pointer to the drive parameter buffer.
   * @param audio_out   Pointer to the output audio buffer.
   */
  template<poly_float(*distort)(poly_float, poly_float), poly_float(*scale)(poly_float)>
  void Distortion::processTimeInvariant(int num_samples, const poly_float* audio_in, const poly_float* drive,
                                        poly_float* audio_out) {
    for (int i = 0; i < num_samples; ++i) {
      // Convert drive in dB to a linear scale appropriate for this distortion
      poly_float current_drive = scale(drive[i]);
      poly_float sample = audio_in[i];
      audio_out[i] = distort(sample, current_drive);
      VITAL_ASSERT(utils::isContained(audio_out[i]));
    }
  }

  void Distortion::processDownSample(int num_samples, const poly_float* audio_in, const poly_float* drive,
                                     poly_float* audio_out) {
    mono_float sample_rate = getSampleRate();
    poly_float current_samples = current_samples_;

    for (int i = 0; i < num_samples; ++i) {
      poly_float current_period = downSampleScale(drive[i]) * sample_rate;
      current_samples += 1.0f;

      poly_float current_sample = audio_in[i];
      poly_float current_downsample = current_sample & constants::kFirstMask;
      current_downsample += utils::swapVoices(current_downsample);

      poly_mask update = poly_float::greaterThanOrEqual(current_samples, current_period);
      last_distorted_value_ = utils::maskLoad(last_distorted_value_, current_downsample, update);
      current_samples = utils::maskLoad(current_samples, current_samples - current_period, update);
      audio_out[i] = last_distorted_value_;
    }

    current_samples_ = current_samples;
  }

  void Distortion::processWithInput(const poly_float* audio_in, int num_samples) {
    VITAL_ASSERT(checkInputAndOutputSize(num_samples));

    int type = static_cast<int>(input(kType)->at(0)[0]);
    poly_float* audio_out = output(kAudioOut)->buffer;
    poly_float* drive_out = output(kDriveOut)->buffer;

    int compact_samples = compactAudio(audio_out, audio_in, num_samples);
    compactAudio(drive_out, input(kDrive)->source->buffer, num_samples);

    if (type != type_) {
      type_ = type;
      last_distorted_value_ = 0.0f;
      current_samples_ = 0.0f;
    }

    switch(type) {
      case kSoftClip:
        processTimeInvariant<softClip, driveDbScale>(compact_samples, audio_out, drive_out, audio_out);
        break;
      case kHardClip:
        processTimeInvariant<hardClip, driveDbScale>(compact_samples, audio_out, drive_out, audio_out);
        break;
      case kLinearFold:
        processTimeInvariant<linearFold, driveDbScale>(compact_samples, audio_out, drive_out, audio_out);
        break;
      case kSinFold:
        processTimeInvariant<sinFold, driveDbScale>(compact_samples, audio_out, drive_out, audio_out);
        break;
      case kBitCrush:
        processTimeInvariant<bitCrush, bitCrushScale>(compact_samples, audio_out, drive_out, audio_out);
        break;
      case kDownSample:
        processDownSample(compact_samples, audio_out, drive_out, audio_out);
        break;
      default:
        utils::copyBuffer(audio_out, audio_in, num_samples);
        return;
    }

    expandAudio(audio_out, audio_out, num_samples);
  }

  void Distortion::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }
} // namespace vital
