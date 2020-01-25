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

	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels) override
	{
		std::vector<uint16_t> scratch(in.size());
		if (transforms.size() > 0)
		{
			transforms[0]->transform_data(in, out, numChannels);
		}
		else
		{
			memcpy((void*)out.data(), (void*)in.data(), in.size() * sizeof(uint16_t));
		}

		for (int i = 1; i < transforms.size(); i++)
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

	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels) override
	{

		for (size_t i = 0; i < in.size(); i += numChannels)
		{
			for (int j = 0; j < numChannels; j++)
			{
				out[i + j] = in[i + j] - derivatives[j];
				derivatives[j] = in[i + j];
			}
		}
		return 0;
	}
};

class nothing_transform final : public transformation_interface
{
public:
	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels) override
	{
		::memcpy(out.data(), in.data(), sizeof(uint16_t) * in.size());
		return 0;
	}
};

#endif // !JointTransformation_header_
