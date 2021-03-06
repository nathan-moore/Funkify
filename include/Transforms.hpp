#ifndef JointTransformation_header_
#define JointTransformation_header_

#include "transformation.hpp"

class JointTransformations final : transformation_interface
{
public:
	std::vector<transformation_interface*> transforms;

	void add_transformation(transformation_interface* t)
	{
		transforms.push_back(t);
	}

	void init_for_data(unsigned short channels, size_t block_size) override
	{
		for (transformation_interface* t : transforms)
		{
			t->init_for_data(channels, block_size);
		}
	}

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		std::vector<int16_t> scratch(in.size());
		if (transforms.size() > 0)
		{
			transforms[0]->transform_data(in, out, numChannels);
		}
		else
		{
			memcpy((void*)out.data(), (void*)in.data(), in.size() * sizeof(uint16_t));
			return 0;
		}

		for (size_t i = 1; i < transforms.size(); i++)
		{
			if ((i % 2) == 1)
			{
				transforms[i]->transform_data(out, scratch, numChannels);
			}
			else
			{
				transforms[i]->transform_data(scratch, out, numChannels);
			}
		}

		if ((transforms.size() % 2) == 0)
		{
			memcpy((void*)out.data(), (void*)scratch.data(), in.size() * sizeof(uint16_t));
		}

		return 0;
	}

};

template<class T>
int16_t clamp_audio(T num)
{
	return (int16_t)std::clamp(num, (T)INT16_MIN, (T)INT16_MAX);
}

int16_t no_clip_subtract(int16_t a, int16_t b)
{
	int32_t sub = (int32_t)a - b;
	return clamp_audio(sub);
}

int32_t clamp_audio_sum(int32_t num, int32_t multiplier)
{
	return (int32_t)std::clamp(num, (int32_t)INT16_MIN * multiplier, (int32_t)INT16_MAX * multiplier);
}

int16_t no_clip_add(int16_t a, int16_t b)
{
	int32_t sub = (int32_t)a + b;
	return clamp_audio(sub);
}

class derivative final : public transformation_interface
{
private:
	std::vector<uint16_t> derivatives;
public:

	void init_for_data(unsigned short channels, size_t block_size) override
	{
		for (unsigned int i = 0; i < channels; ++i)
			derivatives.push_back(0);
	}

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{

		for (size_t i = 0; i < in.size(); i += numChannels)
		{
			for (int j = 0; j < numChannels; j++)
			{
				out[i + j] = no_clip_subtract(in[i + j], derivatives[j]);
				derivatives[j] = in[i + j];
			}
		}
		return 0;
	}
};

class nothing_transform final : public transformation_interface
{
public:
	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		::memcpy(out.data(), in.data(), sizeof(uint16_t) * in.size());
		return 0;
	}
};

class integral final : public transformation_interface
{
	int32_t sum;
	int32_t number_of_samples;
public:
	integral()
		: sum(0),
		number_of_samples(0)
	{}

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		int32_t divisor = 64;
		for (size_t i = 0; i < in.size(); i++)
		{
			sum += in[i];
			sum = clamp_audio_sum(sum, divisor);
			number_of_samples++;
			out[i] = clamp_audio(sum / divisor);
		}

		return 0;
	}
};

class sum_tranform final : public transformation_interface
{
private:
	std::vector<transformation_interface*> transforms;
	std::vector<int32_t> buffer;
public:
	sum_tranform(std::initializer_list<transformation_interface*> list)
		:transforms(list)
	{}

	void add_transformation(transformation_interface* t)
	{
		transforms.push_back(t);
	}

	void init_for_data(unsigned short channels, size_t block_size) override
	{
		for (transformation_interface* t : transforms)
		{
			t->init_for_data(channels, block_size);
		}
	}

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		buffer.resize(in.size());
		memset(buffer.data(), 0, buffer.size() * sizeof(int32_t));

		for (transformation_interface* t : transforms)
		{
			t->transform_data(in, out, numChannels);
			for (size_t i = 0; i < out.size(); i++)
			{
				buffer[i] += out[i];
			}
		}

		for (size_t i = 0; i < buffer.size(); i++)
		{
			out[i] = buffer[i] / transforms.size();
		}

		return 0;
	}

};

class triangle_linear final : public transformation_interface
{
private:
	std::vector<uint16_t> history;
public:

	void init_for_data(unsigned short channels, size_t block_size) override
	{
		for (unsigned int i = 0; i < channels; ++i)
			history.push_back(0);
	}

	int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) override
	{
		for (size_t i = 0; i < in.size(); i += numChannels)
		{
			for (int j = 0; j < numChannels; j++)
			{
				if (no_clip_subtract(in[i + j], history[j]) >= 0) {
					out[i + j] = 0x0000;
				}
				else {
					out[i + j] = 0xFFFF;
				}
			}
		}
		return 0;
	}

};

#endif // !JointTransformation_header_
