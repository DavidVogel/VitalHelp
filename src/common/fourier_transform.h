#pragma once

#include "JuceHeader.h"

/**
 * @namespace vital
 * @brief Contains classes and functions used within the Vital synthesizer framework.
 */
namespace vital {

#if INTEL_IPP

    #include "ipps.h"

/**
 * @brief A Fourier transform implementation using the Intel IPP library.
 *
 * This implementation uses Intel's IPP library to perform forward and inverse
 * real-valued Fast Fourier Transforms (FFTs). It assumes the input is real-valued,
 * and uses a specific data manipulation scheme to handle the frequency-domain
 * representation.
 *
 * @note This class is only compiled when INTEL_IPP is defined.
 */
class FourierTransform {
  public:
    /**
     * @brief Constructs a FourierTransform with the given FFT size specified by bits.
     *
     * The FFT size is `2^bits`. The constructor initializes IPP FFT specification
     * structures and allocates the necessary buffers for forward and inverse transforms.
     *
     * @param bits The power-of-two exponent defining the FFT size. For example, bits=10 leads to a size of 1024.
     */
    FourierTransform(int bits) : size_(1 << bits) {
      int spec_size = 0;
      int spec_buffer_size = 0;
      int buffer_size = 0;
      ippsFFTGetSize_R_32f(bits, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &spec_size, &spec_buffer_size, &buffer_size);

      spec_ = std::make_unique<Ipp8u[]>(spec_size);
      spec_buffer_ = std::make_unique<Ipp8u[]>(spec_buffer_size);
      buffer_ = std::make_unique<Ipp8u[]>(buffer_size);

      ippsFFTInit_R_32f(&ipp_specs_, bits, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, spec_.get(), spec_buffer_.get());
    }

    /**
     * @brief Performs an in-place forward real FFT on the provided data.
     *
     * The input data array should have a length of `size_ + 2` (to accommodate the manipulation of the Nyquist component).
     * After transformation, the data is stored in a frequency-domain format suitable for real-valued transforms.
     *
     * @param data Pointer to the input/output data buffer. On return, this contains the frequency-domain representation.
     */
    void transformRealForward(float* data) {
      data[size_] = 0.0f;
      ippsFFTFwd_RToPerm_32f_I((Ipp32f*)data, ipp_specs_, buffer_.get());
      data[size_] = data[1];
      data[size_ + 1] = 0.0f;
      data[1] = 0.0f;
    }

    /**
     * @brief Performs an in-place inverse real FFT on the provided data.
     *
     * The data is expected to be in the frequency-domain format produced by transformRealForward().
     * After the inverse transform, the data is converted back to the time-domain.
     *
     * @param data Pointer to the input/output data buffer. On return, this contains the time-domain samples.
     */
    void transformRealInverse(float* data) {
      data[1] = data[size_];
      ippsFFTInv_PermToR_32f_I((Ipp32f*)data, ipp_specs_, buffer_.get());
      memset(data + size_, 0, size_ * sizeof(float));
    }

  private:
    int size_;                              ///< The size of the FFT (2^bits).
    IppsFFTSpec_R_32f* ipp_specs_;           ///< IPP FFT specification structure.
    std::unique_ptr<Ipp8u[]> spec_;          ///< IPP FFT specification memory.
    std::unique_ptr<Ipp8u[]> spec_buffer_;   ///< IPP FFT specification buffer.
    std::unique_ptr<Ipp8u[]> buffer_;        ///< Temporary buffer for FFT computations.

    JUCE_LEAK_DETECTOR(FourierTransform)
};

#elif JUCE_MODULE_AVAILABLE_juce_dsp

/**
 * @brief A Fourier transform implementation using JUCE's built-in DSP module.
 *
 * This implementation uses JUCE's `dsp::FFT` class to perform forward and inverse
 * real-only FFTs. It relies on JUCE's platform-agnostic FFT routines.
 *
 * @note This class is only compiled when JUCE's dsp module is available.
 */
    class FourierTransform {
    public:
        /**
         * @brief Constructs a FourierTransform using JUCE's dsp::FFT with the given bits.
         *
         * @param bits The power-of-two exponent defining the FFT size.
         */
        FourierTransform(int bits) : fft_(bits) { }

        /**
         * @brief Performs an in-place forward real FFT.
         *
         * @param data Pointer to the input/output data buffer. On return, this contains the frequency-domain representation.
         */
        void transformRealForward(float* data) { fft_.performRealOnlyForwardTransform(data, true); }

        /**
         * @brief Performs an in-place inverse real FFT.
         *
         * @param data Pointer to the input/output data buffer, which should be in frequency-domain format. On return,
         *             it contains time-domain samples.
         */
        void transformRealInverse(float* data) { fft_.performRealOnlyInverseTransform(data); }

    private:
        dsp::FFT fft_; ///< JUCE FFT instance for handling forward and inverse transforms.

        JUCE_LEAK_DETECTOR(FourierTransform)
    };

#elif __APPLE__

    #define VIMAGE_H
#include <Accelerate/Accelerate.h>

/**
 * @brief A Fourier transform implementation using Apple's Accelerate framework.
 *
 * On Apple platforms, this implementation uses `vDSP_fft_zrip` for efficient real-valued FFTs.
 * It manages the complex split representation internally and scales the results appropriately.
 *
 * @note This class is only compiled on Apple platforms.
 */
class FourierTransform {
  public:
    /**
     * @brief Constructs a FourierTransform with the given bits using Accelerate's vDSP.
     *
     * @param bits The power-of-two exponent defining the FFT size.
     */
    FourierTransform(vDSP_Length bits) : setup_(vDSP_create_fftsetup(bits, 2)), bits_(bits), size_(1 << bits) { }

    /**
     * @brief Destructor. Destroys the FFT setup allocated by vDSP.
     */
    ~FourierTransform() {
      vDSP_destroy_fftsetup(setup_);
    }

    /**
     * @brief Performs an in-place forward real FFT using vDSP.
     *
     * @param data Pointer to the input/output data buffer. On return, data contains the frequency-domain representation.
     */
    void transformRealForward(float* data) {
      static const float kMult = 0.5f;
      data[size_] = 0.0f;
      DSPSplitComplex split = { data, data + 1 };
      vDSP_fft_zrip(setup_, &split, 2, bits_, kFFTDirection_Forward);
      vDSP_vsmul(data, 1, &kMult, data, 1, size_);

      data[size_] = data[1];
      data[size_ + 1] = 0.0f;
      data[1] = 0.0f;
    }

    /**
     * @brief Performs an in-place inverse real FFT using vDSP.
     *
     * @param data Pointer to the input/output data buffer, in frequency-domain format. On return, it holds time-domain samples.
     */
    void transformRealInverse(float* data) {
      float multiplier = 1.0f / size_;
      DSPSplitComplex split = { data, data + 1 };
      data[1] = data[size_];

      vDSP_fft_zrip(setup_, &split, 2, bits_, kFFTDirection_Inverse);
      vDSP_vsmul(data, 1, &multiplier, data, 1, size_ * 2);
      memset(data + size_, 0, size_ * sizeof(float));
    }

  private:
    FFTSetup setup_;      ///< vDSP FFT setup structure.
    vDSP_Length bits_;    ///< The exponent defining the FFT size.
    vDSP_Length size_;    ///< The FFT size (2^bits).

    JUCE_LEAK_DETECTOR(FourierTransform)
};

#else

#include "kissfft/kissfft.h"

/**
 * @brief A Fourier transform implementation using KissFFT for platforms where other accelerations are unavailable.
 *
 * KissFFT is a simple, universally compatible FFT library. This implementation manually arranges
 * real-only data into a complex form before performing forward and inverse transforms, and
 * includes logic to manipulate the Nyquist component.
 *
 * @note This class is used as a fallback when no other specialized FFT backends are available.
 */
class FourierTransform {
  public:
    /**
     * @brief Constructs a FourierTransform with the given bits using KissFFT.
     *
     * @param bits The exponent defining the FFT size. The FFT size is 2^bits.
     */
    FourierTransform(size_t bits) : bits_(bits), size_(1 << bits), forward_(size_, false), inverse_(size_, true) {
      buffer_ = std::make_unique<std::complex<float>[]>(size_);
    }

    /**
     * @brief Default destructor.
     */
    ~FourierTransform() { }

    /**
     * @brief Performs an in-place forward real FFT using KissFFT.
     *
     * The method rearranges the input array to form a complex array and then calls the forward KissFFT transform.
     * After the transform, it sets the Nyquist and DC components to handle the real-only FFT format.
     *
     * @param data Pointer to the input/output data buffer.
     */
    void transformRealForward(float* data) {
      for (int i = size_ - 1; i >= 0; --i) {
        data[2 * i] = data[i];
        data[2 * i + 1] = 0.0f;
      }

      forward_.transform((std::complex<float>*)data, buffer_.get());

      int num_floats = size_ * 2;
      memcpy(data, buffer_.get(), num_floats * sizeof(float));
      data[size_] = data[1];
      data[size_ + 1] = 0.0f;
      data[1] = 0.0f;
    }

    /**
     * @brief Performs an in-place inverse real FFT using KissFFT.
     *
     * This method reconstructs the complex data from the given frequency-domain buffer, calls the inverse transform,
     * and then extracts the real data. It also normalizes the resulting time-domain samples appropriately.
     *
     * @param data Pointer to the input/output data buffer in frequency-domain format.
     */
    void transformRealInverse(float* data) {
      data[0] *= 0.5f;
      data[1] = data[size_];
      inverse_.transform((std::complex<float>*)data, buffer_.get());
      int num_floats = size_ * 2;

      float multiplier = 2.0f / size_;
      for (int i = 0; i < size_; ++i)
        data[i] = buffer_[i].real() * multiplier;

      memset(data + size_, 0, size_ * sizeof(float));
    }

  private:
    size_t bits_;                                     ///< The exponent defining the FFT size.
    size_t size_;                                     ///< The FFT size (2^bits).
    std::unique_ptr<std::complex<float>[]> buffer_;    ///< Buffer for complex frequency-domain data.
    kissfft<float> forward_;                          ///< Forward KissFFT transform.
    kissfft<float> inverse_;                          ///< Inverse KissFFT transform.

    JUCE_LEAK_DETECTOR(FourierTransform)
};

#endif

/**
 * @brief A template class to provide a statically allocated FourierTransform instance for a given number of bits.
 *
 * This class maintains a static instance of FourierTransform for the given number of bits.
 * It can be useful for caching and reusing FFT instances without recreating them multiple times.
 *
 * @tparam bits The exponent defining the FFT size. The resulting size is 2^bits.
 */
    template <size_t bits>
    class FFT {
    public:
        /**
         * @brief Provides access to a static FourierTransform instance for the specified bits.
         *
         * @return A pointer to the static FourierTransform instance.
         */
        static FourierTransform* transform() {
            static FFT<bits> instance;
            return &instance.fourier_transform_;
        }

    private:
        /**
         * @brief Constructs an FFT instance, initializing the static FourierTransform.
         */
        FFT() : fourier_transform_(bits) { }

        FourierTransform fourier_transform_; ///< The static FourierTransform instance for this FFT size.
    };

} // namespace vital
