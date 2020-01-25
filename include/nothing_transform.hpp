#ifndef nothing_transform_header
#define nothing_transform_header

#include "transformation.hpp"

class nothing_transform : public transformation_interface
{
public:
	int transform_data(const std::vector<uint16_t>& in, std::vector<uint16_t>& out) override
	{
		::memcpy(out.data(), in.data(), sizeof(uint16_t) * in.size());
		return 0;
	}
};

#endif // !nothing_transform
