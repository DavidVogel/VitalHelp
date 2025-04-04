#include "operators.h"
#include "synth_constants.h"

namespace vital {

  /**
   * @brief Clamps each sample in the input to [min_, max_].
   * @param num_samples Number of samples to process.
   */
  void Clamp::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize());

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = utils::clamp(source[i], min_, max_);
  }

  /**
   * @brief Negates each sample (multiplies by -1).
   * @param num_samples Number of samples to process.
   */
  void Negate::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize());

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = -source[i];
  }

  /**
   * @brief Inverts each sample by computing 1.0 / sample.
   * @param num_samples Number of samples to process.
   */
  void Inverse::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize());

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = poly_float(1.0f) / source[i];
  }

  /**
   * @brief Multiplies each sample by a constant scale factor.
   * @param num_samples Number of samples to process.
   */
  void LinearScale::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize());

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = source[i] * scale_;
  }

  /**
   * @brief Squares each input sample.
   * @param num_samples Number of samples to process.
   */
  void Square::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(0));
    poly_float* dest = output()->buffer;
    const poly_float* source = input()->source->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float value = source[i];
      dest[i] = value * value;
    }
  }

  /**
   * @brief Adds two input buffers sample-by-sample.
   * @param num_samples Number of samples to process.
   */
  void Add::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(0));
    VITAL_ASSERT(inputMatchesBufferSize(1));

    poly_float* dest = output()->buffer;
    const poly_float* source_left = input(0)->source->buffer;
    const poly_float* source_right = input(1)->source->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = source_left[i] + source_right[i];
  }

  /**
   * @brief Subtracts the second input from the first, sample-by-sample.
   * @param num_samples Number of samples to process.
   */
  void Subtract::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(0));
    VITAL_ASSERT(inputMatchesBufferSize(1));

    poly_float* dest = output()->buffer;
    const poly_float* source_left = input(0)->source->buffer;
    const poly_float* source_right = input(1)->source->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = source_left[i] - source_right[i];
  }

  /**
   * @brief Multiplies two input buffers sample-by-sample.
   * @param num_samples Number of samples to process.
   */
  void Multiply::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(0));
    VITAL_ASSERT(inputMatchesBufferSize(1));

    poly_float* dest = output()->buffer;
    const poly_float* source_left = input(0)->source->buffer;
    const poly_float* source_right = input(1)->source->buffer;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = source_left[i] * source_right[i];
  }

  /**
   * @brief Applies a multiplier that is smoothed over the course of the audio block.
   * @param num_samples Number of samples to process.
   */
  void SmoothMultiply::process(int num_samples) {
    processMultiply(num_samples, input(kControlRate)->at(0));
  }

  /**
   * @brief Internal function for per-sample smoothing and multiplication.
   * @param num_samples Number of samples to process.
   * @param multiply The final target multiplier for this block.
   */
  void SmoothMultiply::processMultiply(int num_samples, poly_float multiply) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudioRate));

    poly_float* audio_out = output()->buffer;
    const poly_float* audio_in = input(kAudioRate)->source->buffer;

    poly_float current_multiply = multiply_;
    multiply_ = multiply;

    // If we need to reset multiplier for specific voices
    current_multiply = utils::maskLoad(current_multiply, multiply_, getResetMask(kReset));
    poly_float delta_multiply = (multiply_ - current_multiply) * (1.0f / num_samples);

    for (int i = 0; i < num_samples; ++i) {
      current_multiply += delta_multiply;
      audio_out[i] = audio_in[i] * current_multiply;
    }
  }

  /**
   * @brief Processes a volume in dB, converting and smoothing to a linear multiplier for the audio.
   * @param num_samples Number of samples to process.
   */
  void SmoothVolume::process(int num_samples) {
    poly_float db = utils::clamp(input(kDb)->at(0), kMinDb, max_db_);
    poly_mask zero_mask = poly_float::lessThanOrEqual(db, kMinDb);

    poly_float amplitude = futils::dbToMagnitude(db);
    // If dB is at or below kMinDb, amplitude becomes 0
    amplitude = utils::maskLoad(amplitude, 0.0f, zero_mask);

    processMultiply(num_samples, amplitude);
  }

  /**
   * @brief Interpolates between two input buffers based on a fractional input, supporting smoothing if fraction is CR.
   * @param num_samples Number of samples to process.
   */
  void Interpolate::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kFrom));
    VITAL_ASSERT(inputMatchesBufferSize(kTo));

    poly_float* dest = output()->buffer;
    const poly_float* from = input(kFrom)->source->buffer;
    const poly_float* to = input(kTo)->source->buffer;
    const poly_float* fractional = input(kFractional)->source->buffer;

    // If fractional is control rate, we smooth it over the block
    if (input(kFractional)->source->isControlRate()) {
      poly_float current_fraction = fraction_;
      fraction_ = fractional[0];
      current_fraction = utils::maskLoad(current_fraction, fraction_, getResetMask(kReset));
      poly_float delta_fraction = (fraction_ - current_fraction) * (1.0f / num_samples);

      for (int i = 0; i < num_samples; ++i) {
        current_fraction += delta_fraction;
        dest[i] = utils::interpolate(from[i], to[i], current_fraction);
      }
    }
    else {
      // Fractional is audio-rate or per-sample
      for (int i = 0; i < num_samples; ++i)
        dest[i] = utils::interpolate(from[i], to[i], fractional[i]);
    }
  }

  /**
   * @brief Performs bilinear interpolation among four corners, using X and Y for horizontal and vertical interpolation.
   * @param num_samples Number of samples to process.
   */
  void BilinearInterpolate::process(int num_samples) {
    static constexpr float kMaxOffset = 1.0f;

    VITAL_ASSERT(inputMatchesBufferSize(kXPosition));
    VITAL_ASSERT(inputMatchesBufferSize(kYPosition));

    poly_float top_left = input(kTopLeft)->at(0);
    poly_float top_right = input(kTopRight)->at(0);
    poly_float bottom_left = input(kBottomLeft)->at(0);
    poly_float bottom_right = input(kBottomRight)->at(0);

    const poly_float* x_position = input(kXPosition)->source->buffer;
    const poly_float* y_position = input(kYPosition)->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float x = utils::clamp(x_position[i], -kMaxOffset, 1.0f + kMaxOffset);
      poly_float y = utils::clamp(y_position[i], -kMaxOffset, 1.0f + kMaxOffset);
      poly_float top = utils::interpolate(top_left, top_right, x);
      poly_float bottom = utils::interpolate(bottom_left, bottom_right, x);
      dest[i] = utils::interpolate(top, bottom, y);
    }
  }

  /**
   * @brief Sums together an arbitrary number of input buffers, sample-by-sample.
   * @param num_samples Number of samples to process.
   */
  void VariableAdd::process(int num_samples) {
#if DEBUG
    // Verify each input buffer size matches
    for (int i = 0; i < inputs_->size(); ++i)
      VITAL_ASSERT(inputMatchesBufferSize(i));
#endif

    poly_float* dest = output()->buffer;
    int num_inputs = static_cast<int>(inputs_->size());

    if (isControlRate()) {
      // Summation for single-sample control-rate
      dest[0] = 0.0f;
      for (int i = 0; i < num_inputs; ++i)
        dest[0] += input(i)->at(0);
    }
    else {
      // Summation for audio-rate
      utils::zeroBuffer(dest, num_samples);

      for (int i = 0; i < num_inputs; ++i) {
        if (input(i)->source != &Processor::null_source_) {
          const poly_float* source = input(i)->source->buffer;
          for (int s = 0; s < num_samples; ++s)
            dest[s] += source[s];
        }
      }
    }
  }

  /**
   * @brief Summation operator that smoothly integrates control-rate and audio-rate mod signals.
   * @param num_samples Number of samples to process.
   */
  void ModulationSum::process(int num_samples) {
    VITAL_ASSERT(output()->buffer_size >= num_samples);

    poly_float* dest = output()->buffer;
    int num_inputs = static_cast<int>(inputs_->size());

    // Smooth control-rate inputs
    poly_float current_control_value = control_value_;
    control_value_ = 0.0f;

    for (int i = kNumStaticInputs; i < num_inputs; ++i) {
      if (input(i)->source != &Processor::null_source_ && input(i)->source->owner->isControlRate())
        control_value_ += input(i)->source->buffer[0];
    }

    // Smoothing the control value
    current_control_value = utils::maskLoad(current_control_value, control_value_, getResetMask(kReset));
    poly_float delta_control_value = (control_value_ - current_control_value) * (1.0f / num_samples);

    for (int s = 0; s < num_samples; ++s) {
      current_control_value += delta_control_value;
      dest[s] = current_control_value;
    }

    // Add audio-rate inputs
    for (int i = kNumStaticInputs; i < num_inputs; ++i) {
      if (input(i)->source != &Processor::null_source_ && !input(i)->source->owner->isControlRate()) {
        VITAL_ASSERT(inputMatchesBufferSize(i));
        const poly_float* source = input(i)->source->buffer;

        for (int s = 0; s < num_samples; ++s) {
          dest[s] += source[s];
        }
      }
    }

    output()->trigger_value = dest[0];
  }

  /**
   * @brief Outputs a constant value taken from the first sample of the input buffer over the entire block.
   * @param num_samples Number of samples to process.
   */
  void SampleAndHoldBuffer::process(int num_samples) {
    poly_float value = input()->source->buffer[0];
    poly_float* dest = output()->buffer;

    // If value is already the same, no change needed
    if (utils::equal(value, dest[0]))
      return;

    for (int i = 0; i < num_samples; ++i)
      dest[i] = value;
  }

  /**
   * @brief Processes stereo rotation/spread encoding or decoding based on the selected mode.
   * @param num_samples Number of samples to process.
   */
  void StereoEncoder::process(int num_samples) {
    if (input(kMode)->at(0)[0])
      processRotate(num_samples);
    else
      processCenter(num_samples);
  }

  /**
   * @brief Applies a rotation transform to the stereo signal (L,R) -> (L*cos - R*sin, R*cos + L*sin).
   * @param num_samples Number of samples to process.
   */
  void StereoEncoder::processRotate(int num_samples) {
    static const poly_float kSign(1.0f, -1.0f);
    VITAL_ASSERT(inputMatchesBufferSize());

    poly_float current_cos_mult = cos_mult_;
    poly_float current_sin_mult = sin_mult_;
    poly_float encoding = utils::clamp(input(kEncodingValue)->at(0), 0.0f, 1.0f)
                           * decoding_mult_ * (2.0f * kPi);

    // Compute target cos/sin
    cos_mult_ = utils::cos(encoding);
    sin_mult_ = utils::sin(encoding);

    mono_float delta_tick = 1.0f / num_samples;
    poly_float delta_cos = (cos_mult_ - current_cos_mult) * delta_tick;
    poly_float delta_sin = (sin_mult_ - current_sin_mult) * delta_tick;

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_cos_mult += delta_cos;
      current_sin_mult += delta_sin;
      // swapStereo swaps L and R, then multiply by (1, -1) for sign inversion
      poly_float swap = kSign * utils::swapStereo(source[i]);
      dest[i] = source[i] * current_cos_mult + swap * current_sin_mult;
    }
  }

  /**
   * @brief Applies a center or spread transform to the stereo signal, blending L and R.
   * @param num_samples Number of samples to process.
   */
  void StereoEncoder::processCenter(int num_samples) {
    poly_float current_cos_mult = cos_mult_;
    poly_float current_sin_mult = sin_mult_;
    // encoding ∈ [0..1], mapped to an angle in [π/4..0].
    poly_float encoding = utils::clamp(input(kEncodingValue)->at(0), 0.0f, 1.0f);
    poly_float phase = (poly_float(1.0f) - encoding) * (0.25f * kPi);

    cos_mult_ = utils::cos(phase);
    sin_mult_ = utils::sin(phase);

    mono_float delta_tick = 1.0f / num_samples;
    poly_float delta_cos = (cos_mult_ - current_cos_mult) * delta_tick;
    poly_float delta_sin = (sin_mult_ - current_sin_mult) * delta_tick;

    const poly_float* source = input()->source->buffer;
    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_cos_mult += delta_cos;
      current_sin_mult += delta_sin;
      poly_float swap = utils::swapStereo(source[i]);
      dest[i] = source[i] * current_cos_mult + swap * current_sin_mult;
    }
  }

  /**
   * @brief Chooses an output frequency by reading various inputs (frequency, tempo index, sync mode, etc.).
   * @param num_samples Number of samples to process (typically 1 for control-rate).
   */
  void TempoChooser::process(int num_samples) {
    static const poly_float dotted_ratio = 2.0f / 3.0f;
    static const poly_float triplet_ratio = 3.0f / 2.0f;

    poly_float tempo = utils::clamp(input(kTempoIndex)->at(0), 0.0f, constants::kNumSyncedFrequencyRatios - 1);
    poly_int tempo_index = utils::toInt(tempo + 0.3f);

    // Retrieve the base ratio from the frequency table
    poly_float tempo_value = 0.0f;
    for (int i = 0; i < poly_float::kSize; ++i)
      tempo_value.set(i, constants::kSyncedFrequencyRatios[tempo_index[i]]);

    poly_float beats_per_second = input(kBeatsPerSecond)->at(0);
    tempo_value *= beats_per_second;

    poly_float sync = input(kSync)->at(0);
    poly_mask triplet_mask = poly_float::equal(sync, kTripletMode);
    poly_mask dotted_mask = poly_float::equal(sync, kDottedMode) & ~triplet_mask;

    // Apply triplet or dotted multipliers
    poly_float triplet_mult = utils::maskLoad(1.0f, triplet_ratio, triplet_mask);
    poly_float dotted_mult = utils::maskLoad(1.0f, dotted_ratio, dotted_mask);
    poly_float tempo_adjusted = triplet_mult * dotted_mult * tempo_value;

    poly_mask frequency_mask = poly_float::equal(sync, kFrequencyMode);
    poly_mask keytrack_mask = poly_float::equal(sync, kKeytrack);

    // Keytrack uses MIDI to produce a frequency
    poly_float midi = input(kKeytrackTranspose)->at(0)
                    + input(kKeytrackTune)->at(0)
                    + input(kMidi)->at(0);
    poly_float keytrack_frequency = utils::midiNoteToFrequency(midi);

    // If in frequency mode, just return frequency. Otherwise, use tempo-based calculation.
    poly_float result = utils::maskLoad(tempo_adjusted, input(kFrequency)->at(0), frequency_mask);

    // If in keytrack mode, override with keytrack frequency
    output()->buffer[0] = utils::maskLoad(result, keytrack_frequency, keytrack_mask);
  }

} // namespace vital
