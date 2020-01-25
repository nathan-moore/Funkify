#ifndef derivativeTrans_header
#define derivativeTrans_header

class derivative : public transformation_interface
{
private:

public:
	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels, std::vector<uint16_t> *derivatives) override
	{

		for (int i = 0; i < in.size(); i += numChannels)
		{
			for (int j = 0; j < numChannels; j++)
			{
				out[i + j] = in[i + j] - derivatives->at(j);
				derivatives->at(j) = in[i + j];
			}
		}
		//::memcpy(out.data(), in.data(), sizeof(uint16_t) * in.size());
		return 0;
	}
};

#endif // !derivativeTrans_header
