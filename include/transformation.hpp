#ifndef transformation_header
#define transformation_header

#include <vector>

class transformation_interface
{
public:
	transformation_interface() = default;

	virtual void init_for_data(unsigned short channels, size_t block_size) 
	{
		return;
	}

	virtual int transform_data(const std::vector<int16_t>& in, std::vector<int16_t>& out, unsigned short numChannels) = 0;

	virtual ~transformation_interface() {}
};

#endif // !transformation_header

