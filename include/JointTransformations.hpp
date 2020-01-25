#ifndef JointTransformation_header_
#define JointTransformation_header_

#include "transformation.hpp"

template<class ... transforms>
class JointTransformations : transformation_interface
{
	std::tuple<transforms..> args;
	JointTransformations(transforms... ts) :
		args(ts)
	{}

	void init_for_data(unsigned short channels, size_t block_size) override
	{

	}
};

#endif // !JointTransformation_header_
