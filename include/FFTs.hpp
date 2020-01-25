#ifndef FFTransform_header
#define FFTransform_header

#include "transformation.hpp"

#include <assert.h>
#include "fftw3.h"

class FFT final : public transformation_interface
{
private:
	fftw_plan plan;
	double* fft_in;
	fftw_complex* fft_out;
	size_t in_length;
	size_t out_length;
	fftw_plan reverse;

public:

	FFT() :
		plan((fftw_plan)nullptr),
		fft_in(nullptr),
		fft_out(nullptr),
		reverse((fftw_plan)reverse)
	{}

	FFT(FFT&& move) : FFT()
	{
		std::swap(plan, move.plan);
		std::swap(fft_in, move.fft_in);
		std::swap(fft_out, move.fft_out);
	}

	void init_for_data(unsigned short channels, size_t block_size) override
	{
		in_length = block_size / (channels * sizeof(uint16_t));
		fft_in = fftw_alloc_real(in_length);

		out_length = in_length / 2 + 1;
		fft_out = fftw_alloc_complex(out_length);

		plan = fftw_plan_dft_r2c_1d(in_length, fft_in, fft_out, FFTW_MEASURE);
		reverse = fftw_plan_dft_c2r_1d(in_length, fft_out, fft_in, FFTW_MEASURE);
	}

	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels) override
	{
		assert(in.size() / numChannels == in_length);
		assert(in.size() == out.size());

		for (int i = 0; i < numChannels; i++)
		{
			int k = 0;
			for (size_t j = i; j < in.size(); j += numChannels)
			{
				fft_in[k] = (double)*reinterpret_cast<const int16_t*>(&in[j]);
				k++;
				//printf("%d", fft_in[k]);
			}

			fftw_execute(plan);

			size_t swap_segment = out_length / 8;
			for (size_t j = 0; j < swap_segment / 2; j++)
			{
				std::swap(fft_out[j], fft_out[swap_segment - j - 1]);
			}

			fftw_execute(reverse);

			k = 0;
			for (size_t j = i; j < in.size(); j += numChannels)
			{
				int16_t normalized_val = (int16_t)(fft_in[k] / in_length);
				out[j] = *reinterpret_cast<const uint16_t*>(&normalized_val);
				k++;
			}
		}

		return 0;
	}

	~FFT()
	{
		if (fft_in != nullptr)
		{
			fftw_destroy_plan(plan);
			fftw_destroy_plan(reverse);

			fftw_free(fft_in);
			fftw_free(fft_out);

			fftw_cleanup();
		}
	}
};

#endif // !FFTransform_header
