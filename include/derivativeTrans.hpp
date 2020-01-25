#ifndef derivativeTrans_header
#define derivativeTrans_header

class derivative : public transformation_interface
{
private:
	uint32_t lastOne = 0;

public:
	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out) override
	{
		for (int i = 0; i < in.size(); ++i)
		{
			out[i] = in[i] - lastOne;
			lastOne = in[i];
		}
		//::memcpy(out.data(), in.data(), sizeof(uint16_t) * in.size());
		return 0;
	}
};

#endif // !derivativeTrans_header
