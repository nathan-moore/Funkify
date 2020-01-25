#include <string>
#include <iostream>
#include <vector>
#include <transformation.hpp>
#include <assert.h>
#include <string.h>


#include "swaps.h"

enum class ParseReturnVal
{
	ok,
	invalid,
	other_error
};

// Used to store essential AST and WAV data
class ASTInfo {
	std::string filename; // Stores filename being used for AST
	bool isWAV = true; // Stores whether or not the host file is a natural WAV file
	std::string extension; // Stores extension of input file
	std::string tmpFilename; // Stores temporary WAV file to be deleted
	std::string extOut; // Stores output string from vgmstream / FFmpeg conversion
	int forceConvType = -1; // Stores library to be used with forced conversion

	unsigned int customSampleRate; // Stores sample rate used for AST
	unsigned int sampleRate; // Stores sample rate of original WAV file

	unsigned short numChannels; // Stores number of channels found in original WAV file
	unsigned int numSamples = 0; // Stores the number of samples being used for the AST
	unsigned short isLooped = 0x0000; // Stores value determining whether or not the AST is looped (0xFFFF = true, 0x0000 = false)
	unsigned int loopStart = 0; // Stores starting loop point
	unsigned int astSize; // Stores total file size of AST (minus 64)
	unsigned int wavSize; // Stores size of audio found in source WAV

	unsigned int blockSize = 0x00002760; // Stores block size used (default for AST is 10080 bytes)
	unsigned int excBlkSz; // Stores the size of the last block being written to the AST file
	unsigned int numBlocks; // Stores the number of blocks being used in the AST file
	unsigned int padding; // Stores a value between 0 and 32 to compensate with the final block to round it to a multiple of 32 bytes

	bool attempted = false; // Helps keep vgmstream / FFmpeg output text clean

public:
	bool getIsWAV() { return isWAV; } // Returns isWAV
	std::string getTmpFilename() { return tmpFilename; } // Returns tmpFilename

	int grabInfo(int, char**); // Retrieves header info from input WAV file, then later writes new AST file if no errors occur 
	int getWAVData(FILE*); // Grabs and stores important WAV header info
	int assignValue(char*, char*); // Parses through user arguments and overrides default settings
	int writeAST(FILE*); // Entry point for writing the AST file
	void printHeader(FILE*); // Writes AST header to output file (and swaps endianness)
	//FILE* convToWAV(std::string, int*, int); // Converts any format supported by vgmstream / vgmstream to WAV
	void setPoints(); // Sets loop points for AST file (if any) from FFmpeg / vgmstream output

	template<class T>
	void printAudio(FILE* sourceWAV, FILE* outputAST, T transformer) // Writes all audio data to AST file (Big Endian)
	{
		static_assert(std::is_base_of<transformation_interface, T>::value, "T is not derived from transformatione");

		uint32_t length = this->blockSize; // Stores size of audio chunk in block
		uint32_t paddedLength = bswap_32(length); // Stores current block size along with padding (Big Endian)
		const uint16_t zeroPad = 0; // Contains the hex of 0x0000 used for padding
		unsigned short offset = this->numChannels; // Stores an offset used in for loops to compensate with variable channels

		uint32_t diff = 32 - (this->blockSize % 32);
		if (diff == 32)
			diff = 0;

		size_t block_size = (this->blockSize + diff) * this->numChannels;
		assert((block_size % 2) == 0);
		std::vector<uint16_t> block(block_size / sizeof(uint16_t));// Used to read and store audio data from the original file

		//uint16_t* block = (uint16_t*)malloc(); 


		//uint16_t* printBlock = (uint16_t*)malloc(); // Stores all finalized audio data being printed to AST file
		size_t printBlock_size = (this->blockSize + diff) * this->numChannels;
		assert((printBlock_size % 2) == 0);
		std::vector<uint16_t> printBlock(printBlock_size / sizeof(uint16_t));

		const uint64_t headerPad[] = { 0, 0, 0 }; // Used for padding in header

		length *= this->numChannels; // Changes length from block size to audio size

		transformer.init_for_data(this->numChannels, block_size);

		for (unsigned int x = 0; x < numBlocks; ++x) {

			unsigned int blockIndex = 0; // Used for indexing the location of data in the printBlock array

			// Writes block header
			fwrite("BLCK", 4 * sizeof(char), 1, outputAST); // Writes "BLCK" at 0x00 index of block

			// Adds padding to paddedLength during the last block
			if (x == this->numBlocks - 1) {
				memset(block.data(), 0, this->blockSize * (unsigned int)this->numChannels); // Clears old audio data stored in block array (possibly unnecessary)
				paddedLength = (this->excBlkSz + this->padding);
				length = (this->excBlkSz) * this->numChannels;
				paddedLength = bswap_32(paddedLength);
			}

			fwrite(&paddedLength, sizeof(paddedLength), 1, outputAST); // Writes block size at 0x04 index of block
			fwrite(&headerPad[0], 3 * sizeof(uint64_t), 1, outputAST); // Writes 24 bytes worth of 0s at 0x08 index of block

			fread(&block.front(), length, 1, sourceWAV); // Reads one block worth of data from source WAV file

			transformer.transform_data(block, printBlock, this->numChannels);

			for (unsigned int y = 0; y < this->numChannels; ++y) {
				unsigned int z = y;
				for (; z < length / 2; z += offset) // Rearranges audio data in channel order to printBlock and swaps endianness
					block[blockIndex++] = bswap_16(printBlock[z]);

				if (x == this->numBlocks - 1) { // Adds 32-byte padding to the end of the stream
					for (z = 0; z < padding; z += 2)
						block[blockIndex++] = 0;
				}
			}
			fwrite(&block.front(), blockIndex * sizeof(uint16_t), 1, outputAST); // Writes processed audio data to output AST file
		}
	}
};
