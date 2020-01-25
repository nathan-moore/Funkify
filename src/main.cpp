#include "ASTInfo.hpp"
#include <cstdio>
#include <filesystem>

/**
 * Written by Gregory Heskett (gheskett)
 *
 * This is a command line tool intended to convert audio into a lossless encoding of the Nintendo AST format found in games such as Super Mario Galaxy and Mario Kart: Double Dash.
 * The resulting audio file is also compatible with lossy interpretations of AST as seen in The Legend of Zelda: Twilight Princess.
 *
 * v1.3 Released 9/3/17
 *
 */


 /**
 * Usage: ASTCreate.exe <input file> [optional arguments]
 *
 * OPTIONAL ARGUMENTS
 *	-o [output file]                           (default: same as input, not including extension)
 *	-c [vgmstream / ffmpeg]                    (force vgmstream / FFmpeg conversion)
 *	-r [sample rate]                           (default: same as source file / argument intended to change speed of audio rather than size)
 *	-b [block size]                            (default: 10080, or 0x00002760)
 *	-n                                         (enable looping)
 *	-s [loop start sample]                     (default: 0)
 *	-t [loop start in microseconds]            (default: 0)
 *	-e [loop end sample / total samples]       (default: number of samples in source file)
 *	-f [loop end in microseconds / total time] (default: length of source audio)
 *	-h                                         (show help text)
 *
 * USAGE EXAMPLES
 *	ASTCreate.exe inputfile.wav -o outputfile.ast -s 158462 -e 7485124
 *	ASTCreate.exe "use quotations if filename contains spaces.wav" -n -f 95000000
 *
 * Note: This program works with WAV files (.wav) encoded with 16-bit PCM. If the source file is anything other than a WAV file, the program will attempt to make a separate conversion with vgmstream or FFmpeg. Make sure you place the vgmstream contents or ffmpeg.exe somewhere Windows can find them.
 *
 */

std::string help; // Stores help text

 // Sets help text
void defineHelp(char* arg) {
	std::string str = arg;
	str = str.substr(str.find_last_of("\\/") + 1);

	std::string s1 = "\nUsage: ";
	std::string s2 = " <input file> [optional arguments]\n\nOPTIONAL ARGUMENTS\n"
		"	-o [output file]                           (default: same as input, not including extension)\n"
		//		"	-c [vgmstream / ffmpeg]                    (force vgmstream / FFmpeg conversion)\n"
		//		"	-r [sample rate]                           (default: same as source file / argument intended to change speed of audio rather than size)\n"
		"	-b [block size]                            (default: 16384, or 0x00004000)\n"
		"	-l                                         (enable looping)\n"
		"	-s [loop start sample]                     (default: 0)\n"
		"	-t [loop start in microseconds]            (default: 0)\n"
		"	-e [loop end sample / total samples]       (default: number of samples in source file)\n"
		"	-f [loop end in microseconds / total time] (default: length of source audio)\n"
		"	-x                                         (enable FFT encoding)\n"
		"	-y                                         (enable derivative encoding)\n"
		"	-h                                         (show help text)\n\n"
		"USAGE EXAMPLES\n	";
	std::string s3 = " inputfile.wav -o outputfile.funk -s 158462 -e 7485124\n	";
	std::string s4 = " \"use quotations if filename contains spaces.wav\" -n -f 95000000\n\n"
		"Note: This program works with WAV files (.wav) encoded with 16-bit PCM. If the source file is anything other than a WAV file, the program will attempt to make a separate conversion using vgmstream or FFmpeg. Make sure you place the vgmstream contents or ffmpeg.exe somewhere Windows can find them.\n\n";

	help = s1 + str + s2 + str + s3 + str + s4;
}


int main(int argc, char** argv)
{
	namespace fs = std::filesystem;

	if (argc < 1)
		return 1;

	defineHelp(argv[0]);

	// Displays an error if there are not at least two arguments provided
	if (argc < 2) {
		puts(help.c_str());
		return 1;
	}

	ASTInfo createFile; // Creates a class used for storing essential AST and WAV data

	// Returns nonzero value if the program runs into an error
	int returnVal = createFile.grabInfo(argc, argv);
	if (!createFile.getIsWAV()) {
		int res = fs::remove_all(createFile.getTmpFilename().c_str());
		if (!res) {
			printf("WARNING: Temporary file deletion failed!\n");
		}
	}

	return returnVal;
}