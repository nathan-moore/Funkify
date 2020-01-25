#include <algorithm>
#include <filesystem>
#include <cstring>
#include <cctype>
#include <filesystem>

#include "swaps.h"
#include "ASTInfo.hpp"
#include "nothing_transform.hpp"
#include "derivativeTrans.hpp"

using namespace std;
namespace fs = std::filesystem;

extern std::string help;

// Retrieves header info from input WAV file, then later writes new AST file if no errors occur 
int ASTInfo::grabInfo(int argc, char** argv) {
	this->filename = argv[1];

	bool isForced = false;
	for (int count = 2; count < argc - 1; ++count) {
		if (!strcmp(argv[count], "-c")) {
			count++;
			string argForce;
			for (size_t count2 = 0; count2 < strlen(argv[count]); ++count2) {
				argForce += tolower(argv[count][count2]);
			}
			if (!argForce.compare("vgmstream") || !argForce.compare("test.exe") || !argForce.compare("0")) {
				this->forceConvType = 0;
			}
			else if (!argForce.compare("ffmpeg") || !argForce.compare("ffmpeg.exe") || !argForce.compare("1")) {
				this->forceConvType = 1;
			}
			else {
				puts(help.c_str());
				return 1;
			}
			isForced = true;
		}
	}

	// Checks for input of more than one input file (via *)
	if (this->filename.find("*") != -1) {
		printf("ERROR: Program is only capable of opening a single input file at a time. Please enter an exact file name (avoid using '*').\n\n%s", help.c_str());
		return 1;
	}

	// Opens input file
	FILE* sourceWAV = fopen(this->filename.c_str(), "rb");
	if (!sourceWAV) {
		if (this->filename.compare("-h") == 0 && argc == 2)
			puts(help.c_str());
		else
			printf("ERROR: Cannot find/open input file!\n\n%s", help.c_str());
		return 1;
	}

	// Checks for file (extention) validity
	string tmp = "";
	size_t lastOf = this->filename.find_last_of("."); // Grabs extension
	if (lastOf == string::npos) {
		printf("ERROR: Source file contains no extension!\n\n%s", help.c_str());
		fclose(sourceWAV);
		return 1;
	}
	tmp = this->filename.substr(lastOf, this->filename.length() - lastOf); // Checks whether or not filename is .wav extension
	transform(tmp.begin(), tmp.end(), tmp.begin(), [](unsigned char c) { return std::tolower(c); });
	this->extension = tmp;

	if (isForced || (tmp.compare(".wav") != 0 && tmp.compare(".wave") != 0)) {
		int ret1 = 0;
		FILE* tmpFile;
		if (this->forceConvType == -1) {
			int ret2 = 0;
			//tmpFile = this->convToWAV(this->filename, &ret1, 0); // Attempts to convert file to WAV using vgmstream (0)
			//if (ret1 != 0) {
			//	if (ret1 > 0)
			//		printf("Format either unsupported by vgmstream or vgmstream cannot be located. Retrying conversion with FFmpeg:\n");
			//	tmpFile = this->convToWAV(this->filename, &ret2, 1); // Attempts to convert file to WAV using FFmpeg (1)
			//}

			fclose(sourceWAV);
			if (ret1 != 0 && ret2 != 0) {
				if (ret1 == -1 && ret2 == -1) {
					printf("ERROR: Neither vgmstream's test.exe nor ffmpeg.exe could be located. Please either make a seperate conversion to WAV or place one or both of the libraries somewhere Windows can find them.\n\n%s", help.c_str());
				}
				else if (ret1 > 0 && ret2 > 0) {
					printf("ERROR: Unsupported file format / library structure! Please use valid audio for the conversion.\n\n%s", help.c_str());
				}
				else if (ret1 > 0) {
					printf("ERROR: ffmpeg.exe could not be located and vgmstream either failed or was not found.\n\n%s", help.c_str());
				}
				else {
					printf("ERROR: Unsupported file format! Please consider using a WAV file, vgmstream or something else supported by FFmpeg.\n\n%s", help.c_str());
				}
				return 1;
			}
		}
		else {
			//tmpFile = this->convToWAV(this->filename, &ret1, this->forceConvType); // Attempts to convert file to WAV using forced type
			fclose(sourceWAV);
			if (ret1 != 0) {
				if (ret1 > 0) {
					if (this->forceConvType == 0)
						printf("ERROR: Either the file type is unsupported, the vgmstream library structure is incomplete, or test.exe cannot be located.\n\n%s", help.c_str());
					else
						printf("ERROR: Unsupported file format! Please use valid audio for the conversion with FFmpeg.\n\n%s", help.c_str());
				}
				else {
					if (this->forceConvType == 0)
						printf("ERROR: vgmstream's test.exe could not be located. Please place the vgmstream library somewhere Windows can find it.\n\n%s", help.c_str());
					else
						printf("ERROR: ffmpeg.exe could not be located. Please place ffmpeg.exe somewhere Windows can find it.\n\n%s", help.c_str());
				}
				return 1;
			}
		}
		//sourceWAV = tmpFile;
	}

	if (!sourceWAV) {
		printf("ERROR: File pointer manipulation failed.\n");
		return 1;
	}

	// Changes .wav extension to .ast
	this->filename = this->filename.substr(0, lastOf);
	this->filename += ".ast";

	int exit = this->getWAVData(sourceWAV); // Grabs WAV header info
	if (exit == 1) {
		fclose(sourceWAV);
		return 1;
	}

	if (!this->isWAV)
		this->setPoints(); // Sets loop points for AST file (if any) from FFmpeg / vgmstream output

	// Parses through user arguments
	bool helpState = false; // Stores boolean value determining whether or not to print out help text
	for (int count = 2; count < argc; count++) {
		if (argv[count][0] == '-') {
			if (strlen(argv[count]) != 2) { // Ensures arguments are two characters
				if (this->isWAV)
					puts(help.c_str());
				else
					printf("ERROR: Provided command line structure is invalid!\n\n%s", help.c_str());
				fclose(sourceWAV);
				return 1;
			}
			if (argc - 1 == count) {
				if (argv[count][1] != 'n' && argv[count][1] != 'h')
					exit = 1;
				else
					exit = assignValue(argv[count], NULL);
			}
			else {
				exit = assignValue(argv[count], argv[count + 1]);
			}
			if (argv[count][1] != 'n' && argv[count][1] != 'h')
				count++;
			else if (argv[count][1] == 'h')
				helpState = true;
		}
		else {
			exit = 1;
		}
		if (exit == 1) { // Exits the program if user arguments are invalid
			if (this->isWAV)
				puts(help.c_str());
			else
				printf("ERROR: Provided command line arguments are invalid!\n\n%s", help.c_str());
			fclose(sourceWAV);
			return 1;
		}
	}
	if (helpState == true) // Prints help text if prompted
		puts(help.c_str());

	exit = this->writeAST(sourceWAV);
	fclose(sourceWAV);
	return exit;

}

// Parses through user arguments and overrides default settings
int ASTInfo::assignValue(char* c1, char* c2) {

	unsigned int tmp;
	int slash, colon;
	uint64_t time;
	long double rounded;
	uint64_t samples;
	string c2str;

	char value = c1[1];
	switch (value) {
	case 'h': // Breaks out if asking for help text since it is printed somewhere else in the program
		break;
	case 'c': // Breaks out if forcing FFmpeg / vgmstream conversion since it is handled elsewhere
		break;
	case 'n': // Disables looping
		this->isLooped = 0xFFFF;
		break;
	case 'b': // Sets custom block size
		tmp = (unsigned int)atoi(c2);
		if (strlen(c2) > 2) {
			string tmpstr = c2;
			if (!tmpstr.substr(1, 1).compare("x") || !tmpstr.substr(1, 1).compare("X")) {
				tmpstr = tmpstr.substr(2);
				tmp = (unsigned int)strtoul(tmpstr.c_str(), NULL, 16);
			}
		}
		if (tmp == 0) {
			printf("WARNING: Invalid/zero argument used for block size!\n");
		}
		else if (tmp % 2 == 1) {
			printf("WARNING: Cannot use odd numbered block size due to each sample being 2 bytes in size.\n");
		}
		else {
			if (tmp % 32 != 0)
				printf("NOTE: Recommended block size should be a multiple of 32 bytes. No padding will be used.\n");
			this->blockSize = tmp;
		}
		break;
	case 'o': // Changes name of output file (if given legal filename)
		c2str = c2;
		slash = c2str.find_last_of("/\\");
		if (c2str.find_last_of("/\\") == string::npos)
			slash = -1;
		colon = c2str.find_last_of(":");
		if (c2str.find_last_of(":") == string::npos)
			colon = -1;

		if (c2str.find("*") != string::npos || c2str.find("?") != string::npos || c2str.find("\"") != string::npos || colon > slash
			|| c2str.find("<") != string::npos || c2str.find(">") != string::npos || c2str.find("|") != string::npos) {
			printf("WARNING: Output filename \"%s\" contains illegal format/characters. Output argument will be ignored.\n", c2);
		}
		else {
			if (c2str.find_last_of("/\\") + 1 == c2str.length()) {
				if (this->filename.find_last_of("/\\") != string::npos)
					c2str += this->filename.substr(this->filename.find_last_of("/\\") + 1, this->filename.length());
				else
					c2str += this->filename;
			}
			this->filename = c2str;
		}
		break;
	case 's': // Sets starting loop point
		this->loopStart = atoi(c2);
		if (!this->isWAV)
			this->isLooped = 0xFFFF;
		break;
	case 't': // Sets starting loop point (in microseconds)
		time = atol(c2);
		rounded = ((long double)time / 1000000.0);
		rounded = rounded * (long double)this->sampleRate + 0.5;
		time = (uint64_t)rounded;
		this->loopStart = (unsigned int)rounded;
		if (!this->isWAV)
			this->isLooped = 0xFFFF;
		break;
	case 'e': // Sets end point of AST file
		samples = atoi(c2);
		if (samples == 0) {
			printf("ERROR: Total number of samples cannot be zero!\n");
			return 1;
		}
		if (this->numSamples < (unsigned int)samples)
			samples = (uint64_t)this->numSamples;
		else
			this->numSamples = (unsigned int)samples;
		this->wavSize = this->numSamples * 2 * this->numChannels;
		break;
	case 'f': // Sets end point of AST file (in microseconds)
		time = atol(c2);
		if (time == 0) {
			printf("ERROR: Ending point of AST cannot be set to zero microseconds!\n");
			return 1;
		}
		rounded = ((long double)time / 1000000.0);
		rounded = rounded * (long double)this->sampleRate + 0.5;
		samples = (uint64_t)rounded;
		if (samples == 0) {
			printf("ERROR: End point of AST is effectively zero! Please enter a larger value of microseconds (not milliseconds).\n");
			return 1;
		}
		if (this->numSamples < (unsigned int)samples)
			samples = (uint64_t)this->numSamples;
		else
			this->numSamples = (unsigned int)samples;
		this->wavSize = this->numSamples * 2 * this->numChannels;
		break;
	case 'r': // Sets custom sample rate (does not affect loop times entered, but does intentionally affect playback speed)
		this->customSampleRate = atoi(c2);
		if (this->customSampleRate == 0)
			this->customSampleRate = this->sampleRate;
		break;
	default: // Returns error if argument is invalid
		return 1;
	}
	return 0;
}

// Grabs and stores important WAV header info
int ASTInfo::getWAVData(FILE* sourceWAV) {
	const char _riff[] = "RIFF";
	const char _wavefmt[] = "WAVE";
	const char _data[] = "data";
	const char _fmt[] = "fmt ";

	// Checks for use of RIFF WAV file
	char riff[5];
	char wavefmt[5];
	fseek(sourceWAV, 0, SEEK_SET);
	fread(&riff, 4, 1, sourceWAV);
	fseek(sourceWAV, 8, SEEK_SET);
	fread(&wavefmt, 4, 1, sourceWAV);
	riff[4] = '\0';
	wavefmt[4] = '\0';
	if (strcmp(_riff, riff) != 0 || strcmp(_wavefmt, wavefmt) != 0) {
		printf("ERROR: Header contents of WAV are invalid or corrupted. Please be sure your input file is a RIFF WAV audio file, or manually force a vgmstream / FFmpeg conversion with the -c flag.\n");
		return 1;
	}

	// Stores size of chunks
	unsigned int chunkSZ;

	// Searches for fmt chunk
	char fmt[5];
	bool isFmt = false;
	while (fread(&fmt, 4, 1, sourceWAV) == 1) {
		fmt[4] = '\0';
		if (strcmp(_fmt, fmt) == 0) {
			isFmt = true;
			break;
		}
		if (fread(&chunkSZ, 4, 1, sourceWAV) != 1)
			break;
		fseek(sourceWAV, chunkSZ, SEEK_CUR);
	}
	if (!isFmt) {
		printf("ERROR: No 'fmt ' chunk could be found in WAV file. The source file is likely corrupted.\n");
		return 1;
	}

	// Checks for use of PCM
	unsigned short PCM;
	fseek(sourceWAV, 4, SEEK_CUR);
	fread(&PCM, 2, 1, sourceWAV);
	if (PCM != 0x0001 && PCM != 0xFFFE)
		printf("CRITICAL WARNING: Source WAV file may not use PCM!\n");

	// Ensures source file uses anywhere between 1 and 16 channels total
	fread(&this->numChannels, 2, 1, sourceWAV);
	if (this->numChannels > 16 || this->numChannels < 1) {
		printf("NOTE: You are using %d channels! Is this intentional?\n", this->numChannels);
	}

	// Sets sample rate
	fread(&this->sampleRate, 4, 1, sourceWAV);
	this->customSampleRate = this->sampleRate;

	// Checks to see if bit rate is 16 bits per sample
	short bitrate;
	fseek(sourceWAV, 6, SEEK_CUR);
	fread(&bitrate, 2, 1, sourceWAV);
	if (bitrate != 16) {
		printf("ERROR: Invalid bit depth! Please make sure you are using 16-bit PCM.\n");
		return 1;
	}

	// Searches for data chunk
	char data[5];
	bool isData = false;
	fseek(sourceWAV, 12, SEEK_SET);
	while (fread(&data, 4, 1, sourceWAV) == 1) {
		data[4] = '\0';
		if (strcmp(_data, data) == 0) {
			isData = true;
			break;
		}
		if (fread(&chunkSZ, 4, 1, sourceWAV) != 1)
			break;
		fseek(sourceWAV, chunkSZ, SEEK_CUR);
	}
	if (!isData) {
		printf("ERROR: No 'data' chunk could be found in WAV file. Either the source is corrupted or contains no audio.\n");
		return 1;
	}

	fread(&this->wavSize, 4, 1, sourceWAV); // Sets total size of audio

	unsigned int tmpSz = (unsigned int)(this->wavSize) / ((unsigned int)this->numChannels * 2); // Sets total number of audio samples
	if (numSamples == 0 || tmpSz < numSamples) {
		numSamples = tmpSz;
		this->wavSize = this->numSamples * 2 * this->numChannels;
	}

	return 0;
}

// Entry point for writing the AST file
int ASTInfo::writeAST(FILE* sourceWAV)
{
	// Calculates number of blocks and size of last block
	this->excBlkSz = (this->numSamples * 2) % this->blockSize;
	this->numBlocks = (this->numSamples * 2) / this->blockSize;
	if (this->excBlkSz != 0)
		this->numBlocks++;
	this->padding = 32 - (this->excBlkSz % 32);
	if (this->padding == 32 || this->blockSize % 32 != 0)
		this->padding = 0;

	// Ensures resulting file size isn't too large
	if ((uint64_t)wavSize + (uint64_t)(this->numBlocks * 32) + (uint64_t)(this->padding * this->numChannels) >= 4294967232) {
		printf("ERROR: Input file is too large!");
		return 1;
	}

	this->astSize = wavSize + (this->numBlocks * 32) + (this->padding * this->numChannels); // Stores size of AST

	// Ensures output file extension is .ast
	string tmp = "";
	if (this->filename.length() >= 4)
		tmp = this->filename.substr(this->filename.length() - 4, this->filename.length());
	if (strcmp(tmp.c_str(), ".ast") != 0)
		this->filename += ".ast";
	if (strcmp(this->filename.c_str(), ".ast") == 0) {
		printf("ERROR: Output filename cannot be restricted exclusively to .ast extension!\n\n%s", help.c_str());
		return 1;
	}

	// Ensures WAV file has audio
	if (this->numBlocks == 0) {
		printf("ERROR: Source WAV contains no audio data!\n");
		return 1;
	}

	// Compensates by setting extra block size to the general block size if it's set to zero
	if (this->excBlkSz == 0)
		this->excBlkSz = this->blockSize;

	// Prevents starting loop point from being as large or larger than the end point
	if (this->loopStart >= this->numSamples)
		this->loopStart = 0;

	// Checks to make sure sample rate is not zero
	if (this->customSampleRate == 0) {
		printf("ERROR: Source file has a sample rate of 0 Hz!\n");
		return 1;
	}

	// Creates directory if needed
	if (this->filename.find("\\") != string::npos || this->filename.find("/") != string::npos) {
		int size = this->filename.find_last_of("/\\");
		fs::create_directory(this->filename.substr(0, size + 1).c_str());
	}

	// Creates AST file
	FILE* outputAST = fopen(this->filename.c_str(), "wb");
	if (!outputAST) {
		printf("ERROR: Couldn't create file.\n");
		return 1;
	}

	// Prints AST information to user
	string loopStatus = "true";
	if (this->isLooped == 0x0000) {
		loopStatus = "false";
		this->loopStart = 0;
	}

	uint64_t startTime = (uint64_t)((long double)this->loopStart / (long double)this->customSampleRate * 1000000.0 + 0.5);
	uint64_t endTime = (uint64_t)((long double)this->numSamples / (long double)this->customSampleRate * 1000000.0 + 0.5);

	printf("WAV file opened successfully!\n\n	AST file size: %d bytes\n	Sample rate: %d Hz\n	Is looped: %s\n	Block size: %d (%#010x)\n", this->astSize + 64, this->customSampleRate, loopStatus.c_str(), this->blockSize, this->blockSize);
	if (this->isLooped == 0xFFFF)
		printf("	Starting loop point: %d samples (time: %d:%02d.%06d)\n", this->loopStart, (int)(startTime / 60000000), (int)(startTime / 1000000) % 60, (int)(startTime % 1000000));
	printf("	End of stream: %d samples (time: %d:%02d.%06d)\n	Number of channels: %d", this->numSamples, (int)(endTime / 60000000), (int)(endTime / 1000000) % 60, (int)(endTime % 1000000), this->numChannels);
	if (this->numChannels == 1)
		printf(" (mono)");
	else if (this->numChannels == 2)
		printf(" (stereo)");

	printf("\n\nWriting %s...", this->filename.c_str());

	printHeader(outputAST); // Writes header info to output

	derivative t{};
	printAudio(sourceWAV, outputAST, t); // Writes audio to AST file

	printf("...DONE!\n");
	fclose(outputAST);
	return 0;
}

// Writes AST header to output file (and swaps endianness)
void ASTInfo::printHeader(FILE* outputAST) {
	fwrite("STRM", 4 * sizeof(char), 1, outputAST); // Prints "STRM" at 0x00

	uint32_t fourByteInt = bswap_32(this->astSize); // Prints total size of all future AST block chunks (file size - 64) at 0x04
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	fourByteInt = 0x10000100; // Prints a hex of big endian 0x00010010 at 0x08 (contains PCM16 encoding information)
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	uint16_t twoByteShort = bswap_16(this->numChannels); // Prints number of channels at 0x0C
	fwrite(&twoByteShort, sizeof(twoByteShort), 1, outputAST);

	fwrite(&this->isLooped, sizeof(this->isLooped), 1, outputAST); // Prints 0xFFFF if looped and 0x0000 if not looped at 0x0E

	fourByteInt = bswap_32(this->customSampleRate); // Prints sample rate at 0x10
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	fourByteInt = bswap_32(this->numSamples); // Prints total number of samples at 0x14
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	fourByteInt = bswap_32(this->loopStart); // Prints starting loop point (in samples) at 0x18
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	fourByteInt = bswap_32(this->numSamples); // Prints end loop point (in samples) at 0x1C (same as 0x14)
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	// Prints size of first block at 0x20
	if (this->numBlocks == 1) {
		fourByteInt = bswap_32(this->excBlkSz + padding);
	}
	else {
		fourByteInt = bswap_32(this->blockSize);
	}
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	// Fills in last 28 bytes with all 0s (except for at 0x28, which has a hex of 0x7F)
	fourByteInt = 0;
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);
	fourByteInt = 0x7F; // Likely denotes playback volume of AST (always set to 127, or 0x7F)
	fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);
	fourByteInt = 0;
	for (int x = 0; x < 5; ++x)
		fwrite(&fourByteInt, sizeof(fourByteInt), 1, outputAST);

	return;
}
//
//// Writes all audio data to AST file (Big Endian)
//void ASTInfo::printAudio(FILE* sourceWAV, FILE* outputAST) {
//
//}
//
// Converts any format supported by vgmstream / FFmpeg to WAV
//FILE* ASTInfo::convToWAV(string filename, int* ret, int cnvType) {
//	string args;
//	string origFilename = filename;
//	if (cnvType == 0) {
//		args = "test.exe -o \"";
//	}
//	else {
//		args = "ffmpeg.exe -i \"";
//		args += origFilename + "\" \"";
//	}
//
//
//	// Creates temporary file and checks that it isn't overwriting another existing file
//	srand((unsigned int)time(NULL));
//	while (true) {
//		string tmp = "";
//		for (int i = 0; i < 16; ++i) {
//			int rchar = (rand() % 16) + 48;
//			if (rchar > 57)
//				rchar += 7;
//			tmp += (char)rchar;
//		}
//		tmp += ".wav";
//		FILE* exist = fopen(tmp.c_str(), "rb");
//		if (!exist) {
//			filename = tmp;
//			break;
//		}
//		fclose(exist);
//	}
//
//	// Piping needed to obtain vgmstream / FFmpeg output
//	_dup2(1, 3);
//	int pin = _dup(0);
//	int pout;
//	if (cnvType == 0)
//		pout = _dup(1);
//	else
//		pout = _dup(2);
//	int outPipe[2];
//	_pipe(outPipe, 16777216, O_TEXT);
//	if (cnvType == 0)
//		_dup2(outPipe[1], 1);
//	else
//		_dup2(outPipe[1], 2);
//
//	args += filename + "\"";
//
//	if (cnvType == 0)
//		args += " -l 1 \"" + origFilename + "\"";
//
//	TCHAR sysdir[MAX_PATH];
//	GetWindowsDirectory(sysdir, MAX_PATH);
//	string args2 = "vgmstream\\" + args;
//	string args3 = "libraries\\" + args;
//	string args4 = (string)sysdir + "\\SysNative\\" + args;
//	string args5 = (string)sysdir + "\\vgmstream\\" + args;
//
//	STARTUPINFO sinfo = { sizeof(STARTUPINFO) };
//	PROCESS_INFORMATION pinfo;
//
//	// Calls vgmstream / ffmpeg.exe
//	int status;
//	if (cnvType == 0 && CreateProcess(NULL, (LPSTR)args2.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)) {
//		status = 1;
//	}
//
//	else if (CreateProcess(NULL, (LPSTR)args3.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)) {
//		status = 2;
//	}
//	else if (CreateProcess(NULL, (LPSTR)args4.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)) {
//		status = 3;
//	}
//	else if (CreateProcess(NULL, (LPSTR)args5.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo)) {
//		status = 4;
//	}
//	else if (CreateProcess(NULL, (LPSTR)args.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &pinfo))
//	{
//		status = 0;
//	}
//	else {
//		_dup2(pin, 0);
//		_close(pin);
//		if (cnvType == 0)
//			_dup2(pout, 1);
//		else
//			_dup2(pout, 2);
//		_close(pout);
//		_close(outPipe[0]);
//		_close(outPipe[1]);
//		status = -1;
//		*ret = -1;
//		return NULL;
//	}
//	if (!this->attempted) {
//		char* str1 = "File opened successfully!\nAttempting to convert host file to temporary WAV file with vgmstream's test.exe:\n";
//		if (cnvType == 1)
//			str1 = "File opened successfully!\nAttempting to convert host file to temporary WAV file with ffmpeg.exe:\n";
//		_write(3, str1, strlen(str1));
//	}
//	this->attempted = true;
//	char* str2 = "Converting to WAV...";
//	_write(3, str2, strlen(str2));
//	WaitForSingleObject(pinfo.hProcess, INFINITE); // Waits for conversion to finish
//	_dup2(pin, 0);
//	_close(pin);
//	if (cnvType == 0)
//		_dup2(pout, 1);
//	else
//		_dup2(pout, 2);
//	_close(pout);
//	_close(outPipe[1]);
//	DWORD exCode;
//	GetExitCodeProcess(pinfo.hProcess, &exCode);
//	CloseHandle(pinfo.hProcess);
//	CloseHandle(pinfo.hThread);
//	*ret = (int)exCode;
//
//	char* str3 = "...FAILED!\n";
//	if (*ret != 0) {
//		_write(3, str3, strlen(str3));
//		return NULL;
//	}
//
//	// Checks that the new file exists
//	this->isWAV = false;
//	this->tmpFilename = filename;
//	FILE* sourceWAV = fopen(this->tmpFilename.c_str(), "rb");
//	if (!sourceWAV) {
//		_write(3, str3, strlen(str3));
//		if (cnvType == 0)
//			str3 = "ERROR: vgmstream failed to write file somehow. This error should never pop up.\n";
//		else
//			str3 = "ERROR: ffmpeg.exe failed to write file somehow. This error should never pop up.\n";
//		_write(3, str3, strlen(str3));
//		*ret = -3;
//		return NULL;
//	}
//	char* str4 = "...DONE! Proceeding with AST conversion:\n";
//	_write(3, str4, strlen(str4));
//
//	// Reads and stores output from ffmpeg.exe / vgmstream
//	char tmpResult[4096];
//	int sz;
//	do {
//		sz = _read(outPipe[0], &tmpResult, 4096);
//		if (sz != 4096)
//			tmpResult[sz] = '\0';
//		this->extOut += tmpResult;
//	} while (sz == 4096);
//	_close(outPipe[0]);
//
//	//_write(3, this->extOut.c_str(), strlen(this->extOut.c_str()));
//	//_write(3, "\n", 1);
//	return sourceWAV;
//}

// Sets loop points for AST file (if any) from FFmpeg / vgmstream output
void ASTInfo::setPoints() {
	bool isLooped = false;

	int type = 0;
	// Starting loop point
	size_t loopPt = this->extOut.find("loop start");
	if (loopPt == string::npos) {
		loopPt = this->extOut.find("loop_start");
		type = 1;
	}
	if (loopPt != string::npos) {
		isLooped = true;
		loopPt = this->extOut.find(": ", loopPt);
		if (loopPt != string::npos) {
			string toIntPoint = this->extOut.substr(loopPt + 2, this->extOut.find_first_of("\n ", loopPt));
			if (type == 0) {
				this->loopStart = atoi(toIntPoint.c_str());
			}
			else {
				uint64_t time = atol(toIntPoint.c_str());
				long double rounded = ((long double)time / 1000000.0);
				rounded = rounded * (long double)this->sampleRate + 0.5;
				time = (uint64_t)rounded;
				this->loopStart = (unsigned int)rounded;
			}
		}
	}

	type = 0;
	// Ending loop point
	loopPt = this->extOut.find("loop end");
	if (loopPt == string::npos) {
		loopPt = this->extOut.find("loop_end");
		type = 1;
	}
	if (loopPt != string::npos) {
		isLooped = true;
		loopPt = this->extOut.find(": ", loopPt);
		if (loopPt != string::npos) {
			string toIntPoint = this->extOut.substr(loopPt + 2, this->extOut.find_first_of("\n ", loopPt));
			uint64_t samples;
			if (type == 0) {
				samples = atoi(toIntPoint.c_str());
			}
			else {
				uint64_t time = atol(toIntPoint.c_str());
				if (time == 0)
					return;
				long double rounded = ((long double)time / 1000000.0);
				rounded = rounded * (long double)this->sampleRate + 0.5;
				samples = (uint64_t)rounded;
			}
			if (samples == 0)
				return;
			if (this->numSamples >= (unsigned int)samples)
				this->numSamples = (unsigned int)samples;
			this->wavSize = this->numSamples * 2 * this->numChannels;
		}
	}
	this->isLooped = 0xFFFF;

	// Checks whether or not a format is looped to make drag-and-drop of non-looped video game tracks possible
	// Most common audio formats are excluded for their sake of simple drag-and-drop (the -n flag still works)
	string tmp = this->extension;
	if (!isLooped && (tmp.compare(".wav") && tmp.compare(".flac") && tmp.compare(".mp3") && tmp.compare(".mp4") && tmp.compare(".aac") && tmp.compare(".aiff")
		&& tmp.compare(".m4a") && tmp.compare(".ogg") && tmp.compare(".opus") && tmp.compare(".raw") && tmp.compare(".wma") && tmp.compare(".avi"))) {

		this->isLooped = 0x0000;
	}
}