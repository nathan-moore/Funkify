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

	void freeCurrentFields()
	{
		fftw_destroy_plan(plan);
		fftw_destroy_plan(reverse);

		fftw_free(fft_in);
		fftw_free(fft_out);

		plan = nullptr;
		reverse = nullptr;
		fft_in = nullptr;
		fft_out = nullptr;
	}

public:

	FFT() :
		plan((fftw_plan)nullptr),
		fft_in(nullptr),
		fft_out(nullptr),
		in_length(0),
		out_length(0),
		reverse((fftw_plan)reverse)
	{}

	FFT(FFT&& move) noexcept : FFT()
	{
		std::swap(plan, move.plan);
		std::swap(fft_in, move.fft_in);
		std::swap(fft_out, move.fft_out);
		std::swap(in_length, move.in_length);
		std::swap(out_length, move.out_length);
		std::swap(reverse, move.reverse);
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

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		assert(in.size() == out.size());

		if ((in.size() / numChannels) != in_length)
		{
			//assert(in.capacity() != in.size());
			freeCurrentFields();
			init_for_data(numChannels, in.size() * sizeof(uint16_t));
		}

		for (int i = 0; i < numChannels; i++)
		{
			int k = 0;
			for (size_t j = i; j < in.size(); j += numChannels)
			{
				fft_in[k] = (double)in[j];
				k++;
			}

			fftw_execute(plan);

			const int num_of_segments = 32;
			size_t swap_segment_size = out_length / num_of_segments;
			for (int segment = 0; segment < num_of_segments; segment++)
			{
				size_t swap_segment = swap_segment_size * segment;
				for (size_t j = 0; j < swap_segment_size / 2; j++)
				{
					std::swap(fft_out[j + swap_segment], fft_out[swap_segment + swap_segment_size - j - 1]);
				}
			}


			fftw_execute(reverse);

			k = 0;
			size_t j = i;
			if (in.size() > 8 * numChannels) {
				for (; j < 8 * numChannels; j += numChannels)
				{
					size_t tmp = j / numChannels;
					double mult = ((double) tmp) / 8.0;
					double norm = fft_in[k] / in_length * mult;
					int16_t normalized_val = (int16_t)std::clamp(norm, (double)INT16_MIN, (double)INT16_MAX);
					out[in.size() - 1 - j] = normalized_val;
					k++;
				}
			}
			for (; j < in.size() - (8 * numChannels); j += numChannels)
			{
				double norm = fft_in[k] / in_length;
				int16_t normalized_val = (int16_t)std::clamp(norm, (double)INT16_MIN, (double)INT16_MAX);
				out[in.size() - 1 - j] = normalized_val;
				k++;
			}
			for (; j < in.size(); j += numChannels)
			{
				size_t tmp = (in.size() - j - 1) / numChannels;
				double mult = ((double) tmp) / 8.0;
				double norm = fft_in[k] / in_length * mult;
				int16_t normalized_val = (int16_t)std::clamp(norm, (double)INT16_MIN, (double)INT16_MAX);
				out[in.size() - 1 - j] = normalized_val;
				k++;
			}
		}

		return 0;
	}

	~FFT()
	{
		if (fft_in != nullptr)
		{
			freeCurrentFields();
			fftw_cleanup();
		}
	}
};

#endif // !FFTransform_header
