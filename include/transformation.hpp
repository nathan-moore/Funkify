#ifndef transformation_header
#define transformation_header

#include <vector>

class transformation_interface
{
public:
	transformation_interface() = default;

	virtual int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out, unsigned short numChannels) = 0;

	virtual ~transformation_interface() {}
};

#endif // !transformation_header

