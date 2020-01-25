class AudioOut
{
	AudioOut(istream in);

	ConvertChunk(std:vector<int32_t>& in, std::vector<int32_t>& out) = 0;
};