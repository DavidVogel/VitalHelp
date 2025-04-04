#pragma once

#include "JuceHeader.h"

#include "common.h"
#include "futils.h"
#include "utils.h"
#include "wave_frame.h"

namespace vital {

    /**
     * @brief A class representing a wavetable, holding multiple frames of waveforms and their frequency-domain representations.
     *
     * This class encapsulates a collection of waveforms (frames) and provides methods to
     * manipulate, retrieve, and process frequency-domain and time-domain representations.
     * It leverages both mono and SIMD (poly) data types to efficiently handle frequency
     * bins and operations on multiple harmonics.
     */
    class Wavetable {
    public:
        /// Number of frequency bins (equal to number of wave bits in a frame).
        static constexpr int kFrequencyBins = WaveFrame::kWaveformBits;
        /// Size of each waveform frame.
        static constexpr int kWaveformSize = WaveFrame::kWaveformSize;
        /// Number of extra values to store beyond the main waveform size.
        static constexpr int kExtraValues = 3;
        /// Number of harmonics in the waveform (half the size plus one).
        static constexpr int kNumHarmonics = kWaveformSize / 2 + 1;
        /// The size for poly frequency buffers, ensuring alignment and vectorization.
        static constexpr int kPolyFrequencySize = 2 * kNumHarmonics / poly_float::kSize + 2;

        /**
         * @brief Struct holding all necessary data for the Wavetable, including multiple frames.
         *
         * This data structure contains both time-domain and frequency-domain
         * representations of each frame. It also stores metadata such as the number
         * of frames, the frequency ratio, sample rate, and a version number used
         * for synchronization.
         */
        struct WavetableData {
            /**
             * @brief Construct a new WavetableData object.
             *
             * @param frames        The number of frames contained in this wavetable data.
             * @param table_version The version of the wavetable data (incremented on changes).
             */
            WavetableData(int frames, int table_version) :
                    num_frames(frames), frequency_ratio(1.0f), sample_rate(kDefaultSampleRate), version(table_version) { }

            int num_frames;  ///< The number of frames in the wavetable.
            mono_float frequency_ratio;  ///< The frequency ratio used for playback.
            mono_float sample_rate;      ///< The sample rate associated with the wavetable frames.
            int version;                 ///< The version number of this wavetable data.

            /// Time-domain wave data: an array of [num_frames][kWaveformSize].
            std::unique_ptr<mono_float[][kWaveformSize]> wave_data;
            /// Frequency amplitudes: an array of [num_frames][kPolyFrequencySize].
            std::unique_ptr<poly_float[][kPolyFrequencySize]> frequency_amplitudes;
            /// Normalized frequency data: an array of [num_frames][kPolyFrequencySize].
            std::unique_ptr<poly_float[][kPolyFrequencySize]> normalized_frequencies;
            /// Phase data: an array of [num_frames][kPolyFrequencySize].
            std::unique_ptr<poly_float[][kPolyFrequencySize]> phases;
        };

        /**
         * @brief Returns a constant pointer to a zeroed waveform.
         *
         * @return A pointer to a static zero waveform.
         */
        static constexpr const mono_float* null_waveform() { return kZeroWaveform; }

        /**
         * @brief Construct a new Wavetable object with a given maximum number of frames.
         *
         * @param max_frames The maximum number of frames this wavetable can hold.
         */
        Wavetable(int max_frames);

        /**
         * @brief Load a default wavetable containing a single, default frame.
         */
        void loadDefaultWavetable();

        /**
         * @brief Set the number of frames in the wavetable.
         *
         * This method reallocates or resizes the wavetable data. It also ensures
         * that the currently active wavetable data is not being used by the audio thread
         * before modification.
         *
         * @param num_frames The new number of frames.
         */
        void setNumFrames(int num_frames);

        /**
         * @brief Set the frequency ratio for this wavetable.
         *
         * @param frequency_ratio The frequency ratio to set.
         */
        void setFrequencyRatio(float frequency_ratio);

        /**
         * @brief Set the sample rate associated with this wavetable.
         *
         * @param rate The new sample rate.
         */
        void setSampleRate(float rate);

        /**
         * @brief Get the user-defined name of this wavetable.
         *
         * @return The name as a string.
         */
        std::string getName() { return name_; }

        /**
         * @brief Get the author of this wavetable.
         *
         * @return The author name as a string.
         */
        std::string getAuthor() { return author_; }

        /**
         * @brief Set a user-defined name for this wavetable.
         *
         * @param name The name to set.
         */
        void setName(const std::string& name) { name_ = name; }

        /**
         * @brief Set the author for this wavetable.
         *
         * @param author The author name to set.
         */
        void setAuthor(const std::string& author) { author_ = author; }

        /**
         * @brief Compute a float-based frequency bin from a phase increment.
         *
         * @param phase_increment The phase increment.
         * @return The computed float frequency bin.
         */
        static force_inline mono_float getFrequencyFloatBin(mono_float phase_increment) {
            return futils::log2(1.0f / phase_increment);
        }

        /**
         * @brief Compute an integer frequency bin from a phase increment.
         *
         * @param phase_increment The phase increment.
         * @return The computed integer frequency bin.
         */
        static force_inline int getFrequencyBin(mono_float phase_increment) {
            int num_waves = 1.0f / phase_increment;
            return utils::iclamp(utils::ilog2(num_waves), 0, kFrequencyBins - 1);
        }

        /**
         * @brief Clamp a frame index to be within the valid range for the current data.
         *
         * @param frame The frame index to clamp.
         * @return A valid frame index.
         */
        force_inline int clampFrame(int frame) {
            return std::min(frame, current_data_->num_frames - 1);
        }

        /**
         * @brief Get a pointer to the current WavetableData.
         *
         * @return A pointer to the current WavetableData structure.
         */
        force_inline const WavetableData* getAllData() {
            return current_data_;
        }

        /**
         * @brief Get a pointer to the time-domain waveform buffer for a given frame.
         *
         * @param frame_index The index of the frame.
         * @return A pointer to the waveform buffer.
         */
        force_inline mono_float* getBuffer(int frame_index) {
            return current_data_->wave_data[clampFrame(frame_index)];
        }

        /**
         * @brief Get a pointer to the frequency amplitude data for a given frame.
         *
         * @param frame_index The index of the frame.
         * @return A pointer to the frequency amplitudes buffer.
         */
        force_inline poly_float* getFrequencyAmplitudes(int frame_index) {
            return current_data_->frequency_amplitudes[clampFrame(frame_index)];
        }

        /**
         * @brief Get a pointer to the normalized frequency data for a given frame.
         *
         * @param frame_index The index of the frame.
         * @return A pointer to the normalized frequencies buffer.
         */
        force_inline poly_float* getNormalizedFrequencies(int frame_index) {
            return current_data_->normalized_frequencies[clampFrame(frame_index)];
        }

        /**
         * @brief Get the version number of the current wavetable data.
         *
         * @return The version number.
         */
        force_inline int getVersion() {
            return current_data_->version;
        }

        /**
         * @brief Clamp a frame index to be within the valid range of the active (in-use) wavetable data.
         *
         * @param frame The frame index to clamp.
         * @return A valid frame index for the active data.
         */
        force_inline int clampActiveFrame(int frame) {
            return std::min(frame, active_audio_data_.load()->num_frames - 1);
        }

        /**
         * @brief Get the frequency ratio of the active wavetable data.
         *
         * @return The active frequency ratio.
         */
        force_inline float getActiveFrequencyRatio() const {
            return active_audio_data_.load()->frequency_ratio;
        };

        /**
         * @brief Get the sample rate of the active wavetable data.
         *
         * @return The active sample rate.
         */
        force_inline float getActiveSampleRate() const {
            return active_audio_data_.load()->sample_rate;
        };

        /**
         * @brief Get a pointer to the active (in-use) WavetableData.
         *
         * @return A pointer to the active WavetableData.
         */
        force_inline const WavetableData* getAllActiveData() {
            return active_audio_data_.load();
        }

        /**
         * @brief Get a pointer to the active frequency amplitudes for a given frame.
         *
         * @param frame_index The frame index.
         * @return A pointer to the active frequency amplitudes buffer.
         */
        force_inline poly_float* getActiveFrequencyAmplitudes(int frame_index) {
            return active_audio_data_.load()->frequency_amplitudes[clampActiveFrame(frame_index)];
        }

        /**
         * @brief Get a pointer to the active normalized frequencies for a given frame.
         *
         * @param frame_index The frame index.
         * @return A pointer to the active normalized frequencies buffer.
         */
        force_inline poly_float* getActiveNormalizedFrequencies(int frame_index) {
            return active_audio_data_.load()->normalized_frequencies[clampActiveFrame(frame_index)];
        }

        /**
         * @brief Get the version number of the active wavetable data.
         *
         * @return The active version number.
         */
        force_inline int getActiveVersion() {
            return active_audio_data_.load()->version;
        }

        /**
         * @brief Load a WaveFrame into the wavetable at the frame index specified by the WaveFrame.
         *
         * @param wave_frame A pointer to the WaveFrame containing time-domain and frequency-domain data.
         */
        void loadWaveFrame(const WaveFrame* wave_frame);

        /**
         * @brief Load a WaveFrame into the wavetable at a specific frame index.
         *
         * @param wave_frame A pointer to the WaveFrame containing time-domain and frequency-domain data.
         * @param to_index The index at which to place the WaveFrame data.
         */
        void loadWaveFrame(const WaveFrame* wave_frame, int to_index);

        /**
         * @brief Post-process the loaded wavetable frames, scaling them based on a maximum span.
         *
         * This can be used to normalize amplitude and correct normalized frequency data across frames.
         *
         * @param max_span The maximum amplitude span used for normalization.
         */
        void postProcess(float max_span);

        /**
         * @brief Get the number of frames in the current wavetable data.
         *
         * @return The number of frames.
         */
        force_inline int numFrames() const { return current_data_->num_frames; }

        /**
         * @brief Get the number of frames in the active wavetable data.
         *
         * @return The number of active frames.
         */
        force_inline int numActiveFrames() const { return active_audio_data_.load()->num_frames; }

        /**
         * @brief Mark the current wavetable data as used (active).
         */
        force_inline void markUsed() { active_audio_data_ = current_data_; }

        /**
         * @brief Mark the active wavetable data as unused, allowing for changes.
         */
        force_inline void markUnused() { active_audio_data_ = nullptr; }

        /**
         * @brief Enable or disable "Shepard" table mode.
         *
         * Shepard mode is a special mode for certain types of wavetable transformations.
         *
         * @param shepard True to enable Shepard mode, false to disable.
         */
        force_inline void setShepardTable(bool shepard) { shepard_table_ = shepard; }

        /**
         * @brief Check if the wavetable is currently in Shepard mode.
         *
         * @return True if Shepard mode is enabled, false otherwise.
         */
        force_inline bool isShepardTable() { return shepard_table_; }

    protected:
        /**
         * @brief Protected default constructor for the Wavetable, intended for subclassing or internal use.
         */
        Wavetable() = default;

        /**
         * @brief Load frequency amplitude data from a set of complex frequency-domain coefficients.
         *
         * @param frequencies A pointer to the complex frequency-domain data.
         * @param to_index The frame index to load the data into.
         */
        void loadFrequencyAmplitudes(const std::complex<float>* frequencies, int to_index);

        /**
         * @brief Load normalized frequency and phase data from a set of complex frequency-domain coefficients.
         *
         * @param frequencies A pointer to the complex frequency-domain data.
         * @param to_index The frame index to load the data into.
         */
        void loadNormalizedFrequencies(const std::complex<float>* frequencies, int to_index);

        /// A static zeroed-out waveform for reference or fallback.
        static const mono_float kZeroWaveform[kWaveformSize + kExtraValues];

        std::string name_;                 ///< User-defined name of the wavetable.
        std::string author_;               ///< Author of the wavetable.
        int max_frames_;                   ///< Maximum number of frames allocated for this wavetable.
        WavetableData* current_data_;      ///< Pointer to the currently editable wavetable data.
        std::atomic<WavetableData*> active_audio_data_; ///< Pointer to the currently active wavetable data used by the audio thread.
        std::unique_ptr<WavetableData> data_; ///< Owning pointer to the wavetable data.
        bool shepard_table_;               ///< Flag indicating if this wavetable is in Shepard mode.

        mono_float fft_data_[2 * kWaveformSize]; ///< Internal FFT buffer data used in processing.

        JUCE_LEAK_DETECTOR(Wavetable)
    };
} // namespace vital
