#pragma once

#include "processor.h"

#include "memory.h"
#include "one_pole_filter.h"

namespace vital {

    /**
     * @class Delay
     * @brief A flexible delay line effect processor that can operate in various styles and apply filtering.
     *
     * The Delay class is templated on a MemoryType, allowing for different memory implementations (e.g., stereo or mono).
     * It supports multiple delay styles (mono, stereo, ping-pong, damped, etc.) and can apply filtering to the delayed
     * signal via internal one-pole filters.
     *
     * Inputs:
     * - kAudio:        The input audio signal.
     * - kWet:          The wet mix percentage (0.0 to 1.0).
     * - kFrequency:    The base delay frequency in Hz (controls the delay time).
     * - kFrequencyAux: The secondary delay frequency, used for stereo or ping-pong modes.
     * - kFeedback:     The amount of delayed signal fed back into the input.
     * - kDamping:      Controls the damping frequency applied in certain modes.
     * - kStyle:        The style of delay (mono, stereo, ping-pong, etc.).
     * - kFilterCutoff: The cutoff frequency (in MIDI note) for internal filtering.
     * - kFilterSpread: The frequency spread around the cutoff for the filter bands.
     *
     * Output:
     * - One audio output containing the processed delay signal.
     *
     * Styles:
     * - kMono: A simple mono delay.
     * - kStereo: Stereo delay with separate delay times for left and right channels.
     * - kPingPong: A ping-pong delay that alternates the delayed signal between left and right.
     * - kMidPingPong: A ping-pong style delay starting from a mid-image source.
     * - kClampedDampened: Delay with clamped dampening filters.
     * - kClampedUnfiltered: Delay with clamped parameters but no filtering.
     * - kUnclampedUnfiltered: Unclamped, no filtering delay (clean).
     *
     * The delay time is derived from the input frequency parameters and can be modulated over time.
     * Internal filtering is handled by one-pole high-pass and low-pass filters applied to the delayed signal.
     *
     * @tparam MemoryType The memory implementation type (e.g., Memory or StereoMemory).
     */
    template<class MemoryType>
    class Delay : public Processor {
    public:
        /// Constants for internal calculations.
        static constexpr mono_float kSpreadOctaveRange = 8.0f;       ///< Octave range of filter spread.
        static constexpr mono_float kDefaultPeriod = 100.0f;         ///< Default delay period in samples.
        static constexpr mono_float kDelayHalfLife = 0.02f;          ///< Time constant for smoothing frequency transitions.
        static constexpr mono_float kMinDampNote = 60.0f;            ///< Minimum MIDI note for damping frequency.
        static constexpr mono_float kMaxDampNote = 136.0f;           ///< Maximum MIDI note for damping frequency.

        /**
         * @brief Computes the filter radius based on spread.
         *
         * @param spread The input spread (0.0 to 1.0).
         * @return Filter radius in semitones.
         */
        static poly_float getFilterRadius(poly_float spread) {
            return utils::max(spread * kSpreadOctaveRange * kNotesPerOctave, 0.0f);
        }

        /// Input indices for the delay processor.
        enum {
            kAudio,          ///< Input audio signal.
            kWet,            ///< Wet mix amount.
            kFrequency,      ///< Base delay frequency.
            kFrequencyAux,   ///< Auxiliary delay frequency (for stereo/ping-pong).
            kFeedback,       ///< Feedback amount.
            kDamping,        ///< Damping control.
            kStyle,          ///< Delay style selection.
            kFilterCutoff,   ///< Filter cutoff (in MIDI note).
            kFilterSpread,   ///< Filter spread around cutoff.
            kNumInputs
        };

        /// Styles of delay.
        enum Style {
            kMono,
            kStereo,
            kPingPong,
            kMidPingPong,
            kNumStyles,
            kClampedDampened,
            kClampedUnfiltered,
            kUnclampedUnfiltered,
        };

        /**
         * @brief Constructs a Delay processor with a given memory size.
         *
         * @param size The maximum delay size in samples.
         */
        Delay(int size) : Processor(Delay::kNumInputs, 1) {
            memory_ = std::make_unique<MemoryType>(size);
            last_frequency_ = 2.0f;
            feedback_ = 0.0f;
            wet_ = 0.0f;
            dry_ = 0.0f;

            filter_gain_ = 0.0f;
            low_coefficient_ = 0.0f;
            high_coefficient_ = 0.0f;
            period_ = utils::min(kDefaultPeriod, size - 1);
            hardReset();
        }

        /**
         * @brief Virtual destructor.
         */
        virtual ~Delay() { }

        Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

        /**
         * @brief Hard-resets the delay line and internal filters.
         */
        void hardReset() override;

        /**
         * @brief Sets the maximum number of samples for the delay.
         *
         * @param max_samples The new maximum size of the delay buffer.
         */
        void setMaxSamples(int max_samples);

        /**
         * @brief Processes a block of audio using the connected inputs.
         *
         * @param num_samples Number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Processes a block of audio from a given input buffer.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Processes a clean, unfiltered delay without clamping or filtering.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void processCleanUnfiltered(const poly_float* audio_in, int num_samples,
                                    poly_float current_period, poly_float current_feedback,
                                    poly_float current_wet, poly_float current_dry);

        /**
         * @brief Processes an unfiltered delay with possible feedback saturation.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void processUnfiltered(const poly_float* audio_in, int num_samples,
                               poly_float current_period, poly_float current_feedback,
                               poly_float current_wet, poly_float current_dry);

        /**
         * @brief Processes a filtered delay applying low-pass and high-pass filtering.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_filter_gain Current gain factor for filtering.
         * @param current_low_coefficient Current low-pass filter coefficient.
         * @param current_high_coefficient Current high-pass filter coefficient.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void process(const poly_float* audio_in, int num_samples,
                     poly_float current_period, poly_float current_feedback, poly_float current_filter_gain,
                     poly_float current_low_coefficient, poly_float current_high_coefficient,
                     poly_float current_wet, poly_float current_dry);

        /**
         * @brief Processes a damped delay line using a low-pass filter for damping.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_low_coefficient Current low-pass filter coefficient.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void processDamped(const poly_float* audio_in, int num_samples,
                           poly_float current_period, poly_float current_feedback,
                           poly_float current_low_coefficient,
                           poly_float current_wet, poly_float current_dry);

        /**
         * @brief Processes a ping-pong delay, alternating the delayed signal between channels.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_filter_gain Filter gain factor.
         * @param current_low_coefficient Low-pass filter coefficient.
         * @param current_high_coefficient High-pass filter coefficient.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void processPingPong(const poly_float* audio_in, int num_samples,
                             poly_float current_period, poly_float current_feedback, poly_float current_filter_gain,
                             poly_float current_low_coefficient, poly_float current_high_coefficient,
                             poly_float current_wet, poly_float current_dry);

        /**
         * @brief Processes a mono ping-pong delay, collapsing input before ping-ponging.
         *
         * @param audio_in The input audio buffer.
         * @param num_samples Number of samples to process.
         * @param current_period Current delay period.
         * @param current_feedback Current feedback value.
         * @param current_filter_gain Filter gain factor.
         * @param current_low_coefficient Low-pass filter coefficient.
         * @param current_high_coefficient High-pass filter coefficient.
         * @param current_wet Current wet mix.
         * @param current_dry Current dry mix.
         */
        void processMonoPingPong(const poly_float* audio_in, int num_samples,
                                 poly_float current_period, poly_float current_feedback, poly_float current_filter_gain,
                                 poly_float current_low_coefficient, poly_float current_high_coefficient,
                                 poly_float current_wet, poly_float current_dry);

        /**
         * @brief A single-sample tick for a clean, unfiltered delay line.
         */
        poly_float tickCleanUnfiltered(poly_float audio_in, poly_float period, poly_float feedback,
                                       poly_float wet, poly_float dry);

        /**
         * @brief A single-sample tick for an unfiltered delay line with saturation.
         */
        poly_float tickUnfiltered(poly_float audio_in, poly_float period, poly_float feedback,
                                  poly_float wet, poly_float dry);

        /**
         * @brief A single-sample tick for a filtered delay line.
         */
        poly_float tick(poly_float audio_in, poly_float period, poly_float feedback,
                        poly_float filter_gain, poly_float low_coefficient, poly_float high_coefficient,
                        poly_float wet, poly_float dry);

        /**
         * @brief A single-sample tick for a damped delay line using a low-pass filter.
         */
        poly_float tickDamped(poly_float audio_in, poly_float period,
                              poly_float feedback, poly_float low_coefficient,
                              poly_float wet, poly_float dry);

        /**
         * @brief A single-sample tick for a ping-pong delay line.
         */
        poly_float tickPingPong(poly_float audio_in, poly_float period, poly_float feedback,
                                poly_float filter_gain, poly_float low_coefficient, poly_float high_coefficient,
                                poly_float wet, poly_float dry);

        /**
         * @brief A single-sample tick for a mono ping-pong delay line.
         */
        poly_float tickMonoPingPong(poly_float audio_in, poly_float period, poly_float feedback,
                                    poly_float filter_gain, poly_float low_coefficient, poly_float high_coefficient,
                                    poly_float wet, poly_float dry);

    protected:
        /**
         * @brief Protected default constructor for derived classes.
         */
        Delay() : Processor(0, 0) { }

        std::unique_ptr<MemoryType> memory_; ///< Internal memory buffer for delay line.
        poly_float last_frequency_;          ///< Tracks last frequency for smoothing delay time changes.
        poly_float feedback_;               ///< Current feedback value.
        poly_float wet_;                    ///< Current wet mix value.
        poly_float dry_;                    ///< Current dry mix value.
        poly_float period_;                 ///< Current delay period in samples.

        poly_float low_coefficient_;        ///< Low-pass filter coefficient.
        poly_float high_coefficient_;       ///< High-pass filter coefficient.
        poly_float filter_gain_;            ///< Gain applied before filtering stages.

        OnePoleFilter<> low_pass_;          ///< Low-pass filter for damping/frequency shaping.
        OnePoleFilter<> high_pass_;         ///< High-pass filter for shaping.

        JUCE_LEAK_DETECTOR(Delay)
    };

    /// StereoDelay is a Delay processor specialized with StereoMemory.
    typedef Delay<StereoMemory> StereoDelay;

    /// MultiDelay is a Delay processor specialized with Memory.
    typedef Delay<Memory> MultiDelay;

} // namespace vital
