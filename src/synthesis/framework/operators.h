#pragma once

#include "futils.h"
#include "processor.h"
#include "utils.h"

namespace vital {

  /**
   * @class Operator
   * @brief Base class for operator nodes that perform arithmetic or other transformations in the processing graph.
   *
   * An Operator is a Processor that can be enabled or disabled based on whether sufficient inputs
   * are connected and whether it has been externally enabled.
   */
  class Operator : public Processor {
    public:
      /**
       * @brief Constructs an Operator with a specified number of inputs/outputs and control-rate setting.
       * @param num_inputs Number of input channels to this operator.
       * @param num_outputs Number of output channels from this operator.
       * @param control_rate True if operating at control rate, false if audio rate.
       */
      Operator(int num_inputs, int num_outputs, bool control_rate = false)
          : Processor(num_inputs, num_outputs, control_rate) {
        externally_enabled_ = true;
        Processor::enable(false);
      }

      /**
       * @brief Checks if this Operator has at least one connected input.
       * @return True if at least one input is connected.
       */
      force_inline bool hasEnoughInputs() {
        return connectedInputs() > 0;
      }

      /**
       * @brief Updates this operator’s enabled state based on connected inputs and external status.
       */
      void setEnabled() {
        bool will_enable = hasEnoughInputs() && externally_enabled_;
        Processor::enable(will_enable);
        if (!will_enable) {
          // If not enabled, clear output buffer and perform a minimal process.
          for (int i = 0; i < numOutputs(); ++i)
            output(i)->clearBuffer();
          process(1);
        }
      }

      /**
       * @brief Called when the number of inputs changes (e.g., dynamically connected or disconnected).
       */
      void numInputsChanged() override {
        Processor::numInputsChanged();
        setEnabled();
      }

      /**
       * @brief Enables or disables the Operator, storing the external enable state.
       * @param enable True to enable, false to disable.
       */
      void enable(bool enable) override {
        externally_enabled_ = enable;
        setEnabled();
      }

      /**
       * @brief Indicates whether this operator has internal state that must be preserved or reset.
       * @return False by default, meaning no internal state is stored.
       */
      virtual bool hasState() const override { return false; }

    private:
      /**
       * @brief Default constructor (disabled).
       */
      Operator() : Processor(0, 0), externally_enabled_(false) { }

      bool externally_enabled_;

      JUCE_LEAK_DETECTOR(Operator)
  };

  /**
   * @class Clamp
   * @brief Clamps each sample to a specified [min, max] range.
   */
  class Clamp : public Operator {
    public:
      /**
       * @brief Constructs a Clamp operator with the given min and max range.
       * @param min The minimum clamping value.
       * @param max The maximum clamping value.
       */
      Clamp(mono_float min = -1, mono_float max = 1)
          : Operator(1, 1), min_(min), max_(max) { }

      virtual Processor* clone() const override { return new Clamp(*this); }

      /**
       * @brief Processes the input buffer, clamping each sample between `min_` and `max_`.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      mono_float min_, max_;

      JUCE_LEAK_DETECTOR(Clamp)
  };

  /**
   * @class Negate
   * @brief Negates each sample (multiplies by -1).
   */
  class Negate : public Operator {
    public:
      Negate() : Operator(1, 1) { }

      virtual Processor* clone() const override { return new Negate(*this); }

      /**
       * @brief Processes the input buffer, negating each sample.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Negate)
  };

  /**
   * @class Inverse
   * @brief Computes 1 / x for each sample.
   *
   * Use caution with zero or near-zero inputs, as this could produce infinities or NaNs.
   */
  class Inverse : public Operator {
    public:
      Inverse() : Operator(1, 1) { }

      virtual Processor* clone() const override { return new Inverse(*this); }

      /**
       * @brief Processes the input buffer, storing the inverse of each sample.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Inverse)
  };

  /**
   * @class LinearScale
   * @brief Multiplies each sample by a fixed scale factor.
   */
  class LinearScale : public Operator {
    public:
      /**
       * @brief Constructs a LinearScale operator with an initial scale factor.
       * @param scale The factor to multiply each sample by.
       */
      LinearScale(mono_float scale = 1.0f) : Operator(1, 1), scale_(scale) { }

      virtual Processor* clone() const override { return new LinearScale(*this); }

      /**
       * @brief Processes each sample, multiplying by the scale factor.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      mono_float scale_;

      JUCE_LEAK_DETECTOR(LinearScale)
  };

  /**
   * @class Square
   * @brief Squares each sample (sample * sample).
   */
  class Square : public Operator {
    public:
      Square() : Operator(1, 1) { }
      virtual Processor* clone() const override { return new Square(*this); }

      /**
       * @brief Processes the input buffer, squaring each sample.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Square)
  };

  /**
   * @class Add
   * @brief Adds two input buffers sample-by-sample.
   */
  class Add : public Operator {
    public:
      Add() : Operator(2, 1) { }

      virtual Processor* clone() const override { return new Add(*this); }

      /**
       * @brief Processes two input buffers and writes their sum to the output.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Add)
  };

  /**
   * @class VariableAdd
   * @brief Adds together an arbitrary number of inputs.
   *
   * The number of inputs can be changed dynamically, and each sample in the output is the sum
   * of the corresponding samples from all inputs.
   */
  class VariableAdd : public Operator {
    public:
      /**
       * @brief Constructs a VariableAdd operator with a specified number of inputs.
       * @param num_inputs Initial number of inputs.
       */
      VariableAdd(int num_inputs = 0) : Operator(num_inputs, 1) { }

      virtual Processor* clone() const override {
        return new VariableAdd(*this);
      }

      /**
       * @brief Sums all input channels for each sample and writes the result to the output.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(VariableAdd)
  };

  /**
   * @class ModulationSum
   * @brief A special sum operator that can accumulate control-rate and audio-rate modulation signals.
   *
   * Control-rate inputs are smoothed over the audio block, while audio-rate inputs are summed per-sample.
   */
  class ModulationSum : public Operator {
    public:
      enum {
        kReset,
        kNumStaticInputs
      };

      /**
       * @brief Constructs a ModulationSum with optional dynamic inputs, plus static inputs (kNumStaticInputs).
       * @param num_inputs Number of dynamic inputs to add.
       */
      ModulationSum(int num_inputs = 0)
          : Operator(num_inputs + kNumStaticInputs, 1) {
        setPluggingStart(kNumStaticInputs);
      }

      virtual Processor* clone() const override {
        return new ModulationSum(*this);
      }

      /**
       * @brief Processes control-rate inputs by smoothing them over the block, and adds audio-rate inputs sample by sample.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

      /**
       * @brief Indicates that this operator has a small amount of state (the smoothed control value).
       * @return True to indicate it must be reset appropriately.
       */
      virtual bool hasState() const override { return true; }

    private:
      poly_float control_value_;

      JUCE_LEAK_DETECTOR(ModulationSum)
  };

  /**
   * @class Subtract
   * @brief Subtracts the second input buffer from the first, sample-by-sample.
   */
  class Subtract : public Operator {
    public:
      Subtract() : Operator(2, 1) { }

      virtual Processor* clone() const override { return new Subtract(*this); }

      /**
       * @brief Processes two inputs, storing the result of left - right.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Subtract)
  };

  /**
   * @class Multiply
   * @brief Multiplies two input buffers sample-by-sample.
   */
  class Multiply : public Operator {
    public:
      Multiply() : Operator(2, 1) { }

      virtual ~Multiply() { }

      virtual Processor* clone() const override { return new Multiply(*this); }

      /**
       * @brief Processes two inputs, storing the product of each sample pair.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(Multiply)
  };

  /**
   * @class SmoothMultiply
   * @brief Multiplies an audio-rate input by a smoothly changing control-rate parameter over one block.
   *
   * Provides smoothing to avoid clicks or pops when the multiplier changes abruptly.
   */
  class SmoothMultiply : public Operator {
    public:
      enum {
        kAudioRate,
        kControlRate,
        kReset,
        kNumInputs
      };

      SmoothMultiply() : Operator(kNumInputs, 1), multiply_(0.0f) { }
      virtual ~SmoothMultiply() { }

      virtual Processor* clone() const override { return new SmoothMultiply(*this); }

      /**
       * @brief Indicates that this operator has internal state (the last multiplier).
       * @return True to support resetting.
       */
      bool hasState() const override { return true; }

      /**
       * @brief Processes the audio input, multiplying by a smoothed control-rate value.
       * @param num_samples Number of samples to process.
       */
      virtual void process(int num_samples) override;

    protected:
      /**
       * @brief Internal function to perform the per-sample smoothing of the multiplier and multiplication.
       * @param num_samples Number of samples to process.
       * @param multiply The target multiplier for the block.
       */
      void processMultiply(int num_samples, poly_float multiply);

      poly_float multiply_;

    private:
      JUCE_LEAK_DETECTOR(SmoothMultiply)
  };

  /**
   * @class SmoothVolume
   * @brief A specialized SmoothMultiply that interprets the control-rate input in dB for volume adjustments.
   *
   * It clamps the dB input to a [kMinDb, max_db_] range, then converts to a linear multiplier and applies smoothing.
   */
  class SmoothVolume : public SmoothMultiply {
    public:
      static constexpr int kDb = kControlRate;
      static constexpr mono_float kMinDb = -80.0f;
      static constexpr mono_float kDefaultMaxDb = 12.2f;

      /**
       * @brief Constructs a SmoothVolume operator with a specified maximum dB level.
       * @param max_db The upper limit for the volume in dB.
       */
      SmoothVolume(mono_float max_db = kDefaultMaxDb) : SmoothMultiply(), max_db_(max_db) { }
      virtual ~SmoothVolume() { }

      virtual Processor* clone() const override { return new SmoothVolume(*this); }

      /**
       * @brief Processes volume in dB, smoothing across one audio block before multiplying the audio input.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      mono_float max_db_;

      JUCE_LEAK_DETECTOR(SmoothVolume)
  };

  /**
   * @class Interpolate
   * @brief Interpolates between two input buffers (From, To) based on a fractional value [0..1].
   */
  class Interpolate : public Operator {
    public:
      enum {
        kFrom,
        kTo,
        kFractional,
        kReset,
        kNumInputs
      };

      Interpolate() : Operator(kNumInputs, 1) { }

      virtual Processor* clone() const override { return new Interpolate(*this); }

      /**
       * @brief Processes two input buffers and interpolates them by a fractional control input.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      poly_float fraction_;

      JUCE_LEAK_DETECTOR(Interpolate)
  };

  /**
   * @class BilinearInterpolate
   * @brief Performs bilinear interpolation among four corners (top-left, top-right, bottom-left, bottom-right).
   *
   * The interpolation factors are provided via two control-rate inputs (XPosition, YPosition).
   */
  class BilinearInterpolate : public Operator {
    public:
      enum {
        kTopLeft,
        kTopRight,
        kBottomLeft,
        kBottomRight,
        kXPosition,
        kYPosition,
        kNumInputs
      };

      /**
       * @brief Index of the first corner input for convenience.
       */
      static const int kPositionStart = kTopLeft;

      BilinearInterpolate() : Operator(kNumInputs, 1) { }

      virtual Processor* clone() const override {
        return new BilinearInterpolate(*this);
      }

      /**
       * @brief Processes the corner values, interpolating horizontally and then vertically.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(BilinearInterpolate)
  };

  /**
   * @class SampleAndHoldBuffer
   * @brief Grabs the first sample from the input, then repeats it for all samples in the output.
   */
  class SampleAndHoldBuffer : public Operator {
    public:
      SampleAndHoldBuffer() : Operator(1, 1) { }

      virtual Processor* clone() const override {
        return new SampleAndHoldBuffer(*this);
      }

      /**
       * @brief Processes the input buffer, outputting a constant value (first sample) for the entire block.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(SampleAndHoldBuffer)
  };

  /**
   * @class StereoEncoder
   * @brief Encodes or decodes a stereo signal by rotating or centering the phase of the left and right channels.
   *
   * The mode (spread vs. rotate) and the encoding value are used to compute rotation or mix between channels.
   */
  class StereoEncoder : public Operator {
    public:
      enum {
        kAudio,
        kEncodingValue,
        kMode,
        kNumInputs
      };

      enum {
        kSpread,
        kRotate,
        kNumStereoModes
      };

      /**
       * @brief Constructs a StereoEncoder. If decoding is true, the math is adjusted for decoding instead of encoding.
       * @param decoding Set to true for decoding, false for standard encoding.
       */
      StereoEncoder(bool decoding = false) : Operator(kNumInputs, 1), cos_mult_(0.0f), sin_mult_(0.0f) {
        decoding_mult_ = 1.0f;
        if (decoding)
          decoding_mult_ = -1.0f;
      }

      virtual Processor* clone() const override {
        return new StereoEncoder(*this);
      }

      /**
       * @brief Processes the stereo input, either applying rotation or a center/spread mix.
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

      /**
       * @brief Indicates that this operator has internal state (cos/sin multipliers) that may change over time.
       * @return True for stateful operators.
       */
      bool hasState() const override { return true; }

    protected:
      /**
       * @brief Processes the stereo signal in rotate mode (phase rotation).
       * @param num_samples Number of samples to process.
       */
      void processRotate(int num_samples);

      /**
       * @brief Processes the stereo signal in spread/center mode.
       * @param num_samples Number of samples to process.
       */
      void processCenter(int num_samples);

      poly_float cos_mult_;
      poly_float sin_mult_;
      mono_float decoding_mult_;

    private:
      JUCE_LEAK_DETECTOR(StereoEncoder)
  };

  /**
   * @class TempoChooser
   * @brief Chooses a frequency based on tempo sync or direct frequency modes.
   *
   * Supports dotted, triplet, and keytrack modes, merging various inputs into a single frequency output.
   */
  class TempoChooser : public Operator {
    public:
      enum {
        kFrequencyMode,
        kTempoMode,
        kDottedMode,
        kTripletMode,
        kKeytrack,
        kNumSyncModes
      };

      enum {
        kFrequency,
        kTempoIndex,
        kBeatsPerSecond,
        kSync,
        kMidi,
        kKeytrackTranspose,
        kKeytrackTune,
        kNumInputs
      };

      /**
       * @brief Constructs a TempoChooser operating at control rate.
       */
      TempoChooser() : Operator(kNumInputs, 1, true) { }

      virtual Processor* clone() const override {
        return new TempoChooser(*this);
      }

      /**
       * @brief Reads sync mode and other parameters, producing a final frequency.
       * @param num_samples Number of samples (control-rate typically processes one sample).
       */
      void process(int num_samples) override;

    private:
      JUCE_LEAK_DETECTOR(TempoChooser)
  };

  //-----------------------------------------------------------------------------------------------
  // The following classes are within the 'cr' namespace, indicating control-rate operators
  // that typically process a single sample rather than an entire audio block.
  //-----------------------------------------------------------------------------------------------
  namespace cr {

    /**
     * @class Clamp
     * @brief Control-rate clamping of a single value.
     */
    class Clamp : public Operator {
      public:
        /**
         * @brief Constructs a control-rate Clamp operator for single-value clamping.
         * @param min The minimum value to clamp to.
         * @param max The maximum value to clamp to.
         */
        Clamp(mono_float min = -1, mono_float max = 1)
            : Operator(1, 1, true), min_(min), max_(max) { }

        virtual Processor* clone() const override { return new Clamp(*this); }

        /**
         * @brief Processes one control sample, clamping it to [min_, max_].
         * @param num_samples Typically 1 at control rate.
         */
        void process(int num_samples) override {
          output()->buffer[0] = utils::clamp(input()->at(0), min_, max_);
        }

      private:
        mono_float min_, max_;
        JUCE_LEAK_DETECTOR(Clamp)
    };

    /**
     * @class LowerBound
     * @brief Clamps a single control value to be at least `min_`.
     */
    class LowerBound : public Operator {
      public:
        LowerBound(mono_float min = 0.0f) : Operator(1, 1, true), min_(min) { }

        virtual Processor* clone() const override { return new LowerBound(*this); }

        void process(int num_samples) override {
          output()->buffer[0] = utils::max(input()->at(0), min_);
        }

      private:
        mono_float min_;

        JUCE_LEAK_DETECTOR(LowerBound)
    };

    /**
     * @class UpperBound
     * @brief Clamps a single control value to be at most `max_`.
     */
    class UpperBound : public Operator {
      public:
        UpperBound(mono_float max = 0.0f) : Operator(1, 1, true), max_(max) { }

        virtual Processor* clone() const override { return new UpperBound(*this); }

        void process(int num_samples) override {
          output()->buffer[0] = utils::min(input()->at(0), max_);
        }

      private:
        mono_float max_;

        JUCE_LEAK_DETECTOR(UpperBound)
    };

    /**
     * @class Add
     * @brief Control-rate addition of two values.
     */
    class Add : public Operator {
      public:
        Add() : Operator(2, 1, true) {
        }

        virtual Processor* clone() const override { return new Add(*this); }

        void process(int num_samples) override {
          output()->buffer[0] = input(0)->at(0) + input(1)->at(0);
        }

      private:
        JUCE_LEAK_DETECTOR(Add)
    };

    /**
     * @class Multiply
     * @brief Control-rate multiplication of two values.
     */
    class Multiply : public Operator {
      public:
        Multiply() : Operator(2, 1, true) {
        }

        virtual ~Multiply() { }

        virtual Processor* clone() const override { return new Multiply(*this); }

        void process(int num_samples) override {
          output()->buffer[0] = input(0)->at(0) * input(1)->at(0);
        }

      private:
        JUCE_LEAK_DETECTOR(Multiply)
    };

    /**
     * @class Interpolate
     * @brief Control-rate interpolation between two values based on a fraction.
     */
    class Interpolate : public Operator {
      public:
        enum {
          kFrom,
          kTo,
          kFractional,
          kNumInputs
        };

        Interpolate() : Operator(kNumInputs, 1, true) { }

        virtual Processor* clone() const override { return new Interpolate(*this); }

        void process(int num_samples) override {
          poly_float from = input(kFrom)->at(0);
          poly_float to = input(kTo)->at(0);
          poly_float fraction = input(kFractional)->at(0);
          output()->buffer[0] = utils::interpolate(from, to, fraction);
        }

      private:
        JUCE_LEAK_DETECTOR(Interpolate)
    };

    /**
     * @class Square
     * @brief Control-rate operator squaring a single value.
     */
    class Square : public Operator {
      public:
        Square() : Operator(1, 1, true) { }
        virtual Processor* clone() const override { return new Square(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          output()->buffer[0] = value * value;
        }

      private:
        JUCE_LEAK_DETECTOR(Square)
    };

    /**
     * @class Cube
     * @brief Control-rate operator cubing a single value.
     */
    class Cube : public Operator {
      public:
        Cube() : Operator(1, 1, true) { }
        virtual Processor* clone() const override { return new Cube(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          output()->buffer[0] = value * value * value;
        }

      private:
        JUCE_LEAK_DETECTOR(Cube)
    };

    /**
     * @class Quart
     * @brief Control-rate operator raising a single value to the 4th power.
     */
    class Quart : public Operator {
      public:
        Quart() : Operator(1, 1, true) { }
        virtual Processor* clone() const override { return new Quart(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          value *= value;
          output()->buffer[0] = value * value;
        }

      private:
        JUCE_LEAK_DETECTOR(Quart)
    };

    /**
     * @class Quadratic
     * @brief Control-rate operator computing x^2 + offset.
     */
    class Quadratic : public Operator {
      public:
        Quadratic(mono_float offset)
            : Operator(1, 1, true), offset_(offset) { }

        virtual Processor* clone() const override { return new Quadratic(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          output()->buffer[0] = value * value + offset_;
        }

      private:
        mono_float offset_;

        JUCE_LEAK_DETECTOR(Quadratic)
    };

    /**
     * @class Cubic
     * @brief Control-rate operator computing x^3 + offset.
     */
    class Cubic : public Operator {
      public:
        Cubic(mono_float offset)
            : Operator(1, 1, true), offset_(offset) { }

        virtual Processor* clone() const override { return new Cubic(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          output()->buffer[0] = value * value * value + offset_;
        }

      private:
        mono_float offset_;

        JUCE_LEAK_DETECTOR(Cubic)
    };

    /**
     * @class Quartic
     * @brief Control-rate operator computing x^4 + offset.
     */
    class Quartic : public Operator {
      public:
        Quartic(mono_float offset)
            : Operator(1, 1, true), offset_(offset) { }

        virtual Processor* clone() const override { return new Quartic(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          value *= value;
          output()->buffer[0] = value * value + offset_;
        }

      private:
        mono_float offset_;

        JUCE_LEAK_DETECTOR(Quartic)
    };

    /**
     * @class Root
     * @brief Control-rate operator computing sqrt(x) + offset.
     */
    class Root : public Operator {
      public:
        Root(mono_float offset)
            : Operator(1, 1, true), offset_(offset) { }

        virtual Processor* clone() const override { return new Root(*this); }

        void process(int num_samples) override {
          poly_float value = utils::max(input()->at(0), 0.0f);
          output()->buffer[0] = utils::sqrt(value) + offset_;
        }

      private:
        mono_float offset_;

        JUCE_LEAK_DETECTOR(Root)
    };

    /**
     * @class ExponentialScale
     * @brief Raises `scale_` to the power of the input value (clamped to [min_, max_]).
     */
    class ExponentialScale : public Operator {
      public:
        /**
         * @param min Minimum input value.
         * @param max Maximum input value.
         * @param scale Base for exponentiation.
         * @param offset Currently unused.
         */
        ExponentialScale(mono_float min, mono_float max, mono_float scale = 1, mono_float offset = 0.0f)
            : Operator(1, 1, true), min_(min), max_(max), scale_(scale), offset_(offset) { }

        virtual Processor* clone() const override {
          return new ExponentialScale(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = futils::pow(scale_, utils::clamp(input()->at(0), min_, max_));
        }

      private:
        mono_float min_;
        mono_float max_;
        mono_float scale_;
        mono_float offset_;

        JUCE_LEAK_DETECTOR(ExponentialScale)
    };

    /**
     * @class VariableAdd
     * @brief Control-rate version of summing multiple inputs into one.
     */
    class VariableAdd : public Operator {
      public:
        VariableAdd(int num_inputs = 0) : Operator(num_inputs, 1, true) {
        }

        virtual Processor* clone() const override {
          return new VariableAdd(*this);
        }

        void process(int num_samples) override {
          int num_inputs = static_cast<int>(inputs_->size());
          poly_float value = 0.0;

          for (int in = 0; in < num_inputs; ++in)
            value += input(in)->at(0);

          output()->buffer[0] = value;
        }

      private:
        JUCE_LEAK_DETECTOR(VariableAdd)
    };

    /**
     * @class FrequencyToPhase
     * @brief Converts a frequency to a normalized phase increment (freq / sample_rate).
     */
    class FrequencyToPhase : public Operator {
      public:
        FrequencyToPhase() : Operator(1, 1, true) { }

        virtual Processor* clone() const override {
          return new FrequencyToPhase(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = input()->at(0) * (1.0f / getSampleRate());
        }

      private:
        JUCE_LEAK_DETECTOR(FrequencyToPhase)
    };

    /**
     * @class FrequencyToSamples
     * @brief Converts a frequency to a period in samples (sample_rate / freq).
     */
    class FrequencyToSamples : public Operator {
      public:
        FrequencyToSamples() : Operator(1, 1, true) { }

        virtual Processor* clone() const override {
          return new FrequencyToSamples(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = poly_float(getSampleRate()) / input()->at(0);
        }

      private:
        JUCE_LEAK_DETECTOR(FrequencyToSamples)
    };

    /**
     * @class TimeToSamples
     * @brief Converts a time in seconds to a number of samples (time * sample_rate).
     */
    class TimeToSamples : public Operator {
      public:
        TimeToSamples() : Operator(1, 1, true) { }

        virtual Processor* clone() const override {
          return new TimeToSamples(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = input()->at(0) * getSampleRate();
        }

      private:
        JUCE_LEAK_DETECTOR(TimeToSamples)
    };

    /**
     * @class MagnitudeScale
     * @brief Converts a dB value to a linear magnitude at control rate.
     */
    class MagnitudeScale : public Operator {
      public:
        MagnitudeScale() : Operator(1, 1, true) { }

        virtual Processor* clone() const override {
          return new MagnitudeScale(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = futils::dbToMagnitude(input()->at(0));
        }

      private:
        JUCE_LEAK_DETECTOR(MagnitudeScale)
    };

    /**
     * @class MidiScale
     * @brief Converts a MIDI note (in semitones) to a frequency at control rate.
     */
    class MidiScale : public Operator {
      public:
        MidiScale() : Operator(1, 1, true) { }

        virtual Processor* clone() const override {
          return new MidiScale(*this);
        }

        void process(int num_samples) override {
          output()->buffer[0] = utils::midiCentsToFrequency(input()->at(0));
        }

      private:
        JUCE_LEAK_DETECTOR(MidiScale)
    };

    /**
     * @class BilinearInterpolate
     * @brief Control-rate bilinear interpolation between four corner values.
     */
    class BilinearInterpolate : public Operator {
      public:
        enum {
          kTopLeft,
          kTopRight,
          kBottomLeft,
          kBottomRight,
          kXPosition,
          kYPosition,
          kNumInputs
        };

        /**
         * @brief Index of the first corner input for convenience.
         */
        static const int kPositionStart = kTopLeft;

        BilinearInterpolate() : Operator(kNumInputs, 1, true) { }

        virtual Processor* clone() const override {
          return new BilinearInterpolate(*this);
        }

        /**
         * @brief Reads corner values and interpolation positions (X, Y), then performs bilinear interpolation.
         * @param num_samples Typically 1 for control-rate processing.
         */
        void process(int num_samples) override {
          poly_float top = utils::interpolate(input(kTopLeft)->at(0),
                                              input(kTopRight)->at(0),
                                              input(kXPosition)->at(0));
          poly_float bottom = utils::interpolate(input(kBottomLeft)->at(0),
                                                 input(kBottomRight)->at(0),
                                                 input(kXPosition)->at(0));
          output()->buffer[0] = utils::interpolate(top, bottom, input(kYPosition)->at(0));
        }

      private:
        JUCE_LEAK_DETECTOR(BilinearInterpolate)
    };

  } // namespace cr
} // namespace vital
