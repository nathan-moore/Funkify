#ifndef derivativeTrans_header
#define derivativeTrans_header

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

#endif // !derivativeTrans_header
