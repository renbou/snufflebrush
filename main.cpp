#include "seedgen.h"
#include "png++/png.hpp"
#include <fstream>
#include <iostream>

typedef std::string string;
auto& cout = std::cout;
auto& cin = std::cin;
string g_helpMessage =
		"Usage:\n"
		"  ./snufflebrush -e -i <input> -o <output> -f <file> -p <password> [-c (colours)] [-b (bits)] [-eq]\n"
		"  ./snufflebrush -d -i <input> -o <output> -p <password> [-c (colours)] [-b (bits)] [-eq]\n"
		"WARNING: SPECIFY ALL OPTIONS IN THE ORDER IN WHICH THEY ARE WRITTEN IN THIS MESSAGE.\n"
		"\n"
		"Options:\n"
		"  -h, --help					Show this help message.\n"
		"  -e, --encode					Set mode to encoding.\n"
		"  -d, --decode					Set mode to decoding.\n"
		"  (Specify only one of the 	previous 2 options)\n"
		"  -i, --input <input>			Base image for stego (carrier). \n"
		"  -o, --output <output>		Result image (or extracted data).\n"
		"  -f, --file <file>			File to hide (this option is only for encode mode).\n"
		"  -p, --password <password>	Use specified password.\n"
		"Additional functionality:\n"
		"  -c, --colours (colours)		Only use specified colours, converting image if needed.\n"
		"    Example values: rgba; r; grb; gra.\n"
		"    Default value: rgba.\n"
		"  -b, --bits (bits)			Only use specified bits (from lower to higher).\n"
		"    Example values: 4 (use first 4); 2-4 (use lower bits 2,3,4); 1,3,5 (use lower bits 1,3,5)\n"
		"    Default value: 4.\n"
		"  -eq, --equal					Use equally spaced out bits of data across image.";

class argException: public std::exception {
	const char* msg;
public:
	argException(const char* msg_): msg(msg_) {}
	const char * what() const throw () { return msg; }
};

class imgException: public std::exception {
	const char* msg;
public:
	imgException(const char* msg_): msg(msg_) {}
	const char * what() const throw () { return msg; }
};

void initializeIO() {
	std::ios_base::sync_with_stdio(0);
	cin.tie(0);
	cout.tie(0);
	return;
}

void printHelpMessage(std::ostream & out) {
	out << g_helpMessage;
	out.flush();
	return;
}

void convertOptionsToStrings (int &argc, char **argv, string res[]) {
	for (int i = 1; i < argc; i++)
		res[i-1] = argv[i];
	return;
}

bool checkNumeric(char c) {
	if (c < '0' || c > '9')
		return 0;
	return 1;
}

bool getOptions(int &argc, char **argv, bool &mode, string files[], string &password, int &numColours, int * &colours, int &numLayers, int * &layers, bool &equality) {
	string * args = new string [argc];
	convertOptionsToStrings(argc, argv, args);
	argc--;

	if (argc == 0) {
		printHelpMessage(cout);
		throw argException("No options specified.");
	}

	if (args[0] == "-h" || args[0] == "--help") {
		printHelpMessage(cout);
		return 0;
	}

	if (argc > 0 && (args[0] == "-e" || args[0] == "--encode"))
		mode = 1;
	else if (args[0] == "-d" || args[0] == "--decode")
		mode = 0;
	else {
		printHelpMessage(cout);
		throw argException("Invalid option for mode.");
	}

	if (argc > 1 && (args[1] == "-i" || args[1] == "--input"))
		if (argc > 2)
			files[0] = args[2];
		else {
			printHelpMessage(cout);
			throw argException("Input file not specified.");
		}
	else {
		printHelpMessage(cout);
		throw argException("Invalid second option, has to be input.");
	}

	if (argc > 3 && (args[3] == "-o" || args[3] == "--output"))
		if (argc > 4)
			files[1] = args[4];
		else {
			printHelpMessage(cout);
			throw argException("Output file not specified.");
		}
	else {
		printHelpMessage(cout);
		throw argException("Invalid third option, has to be output.");
	}

	if (mode) {
		if (argc > 5 && (args[5] == "-f" || args[5] == "--file"))
			if (argc > 6)
				files[2] = args[6];
			else {
				printHelpMessage(cout);
				throw argException("File to hide not specified.");
			}
		else {
			printHelpMessage(cout);
			throw argException("Invalid fourth option, has to be file.");
		}
	}

	int offset = mode*2;

	if (argc > (5+offset) && (args[5 + offset] == "-p" || args[5 + offset] == "--password"))
		if (argc > 6+offset)
			password = args[6 + offset];
		else {
			printHelpMessage(cout);
			throw argException("Password not specified.");
		}
	else {
		printHelpMessage(cout);
		if (mode)
			throw argException("Invalid fifth option, has to be password.");
		else
			throw argException("Invalid fourth option, has to be password.");
	}

	if (argc > (7+offset)) {
		if (args[7+offset] == "-c" || args[7+offset] == "--colours") {
			if (argc > (8+offset)) {
				string cls = args[8+offset];
				std::sort(cls.begin(), cls.end());
				string check = "abgr";
				for (auto &c: cls) {
					if (check.find(c) == string::npos) {
						printHelpMessage(cout);
						throw argException("Invalid colours specified, have to be some form of \"rgba\"");
					}
				}
				numColours = cls.size();
				colours = new int[numColours];
				int clrind = 0;
				if (cls.find('r') != string::npos)
					colours[clrind++] = 0;
				if (cls.find('g') != string::npos)
					colours[clrind++] = 1;
				if (cls.find('b') != string::npos)
					colours[clrind++] = 2;
				if (cls.find('a') != string::npos)
					colours[clrind++] = 3;

				offset += 2;

			} else {
				printHelpMessage(cout);
				throw argException("Colours not specified.");
			}
		} else if (args[7+offset] != "-b" && args[7+offset] != "--bits") {
			printHelpMessage(cout);
			throw argException("Invalid extra option specified.");
		}
	}

	if (argc > (7+offset)) {
		if (args[7+offset] == "-b" || args[7+offset] == "--bits") {
			if (argc > (8+offset)) {
				string bits = args[8+offset];
				if (bits.size() == 1) {
					if (bits != "0") {
						if (bits[0] > '8') {
							printHelpMessage(cout);
							throw argException("Bits have to be in range [1;8].");
						}
						if (!checkNumeric(bits[0])) {
							printHelpMessage(cout);
							throw argException("Specified \"bits\" are not numbers.");
						}
						numLayers = bits[0]-'0';
						layers = new int[numLayers];
						for (int i = 0; i < numLayers; i++)
							layers[i] = i;
						offset += 2;
					} else {
						printHelpMessage(cout);
						throw argException("Minimum 1 bit has to be used for stego.");
					}
				} else if (bits.size() == 3 && bits[1] == '-') {
					if (!checkNumeric(bits[0]) || !checkNumeric(bits[2])) {
						printHelpMessage(cout);
						throw argException("Specified \"bits\" are not numbers.");
					}
					int firstbit = bits[0]-'0';
					int lastbit = bits[2]-'0';
					if (firstbit < 1 || firstbit > 8 || lastbit < 1 || lastbit > 8) {
						printHelpMessage(cout);
						throw argException("Specified bits have to be in the range [1;8].");
					}
					if (lastbit < firstbit)
						std::swap(firstbit, lastbit);
					numLayers = lastbit-firstbit+1;
					layers = new int[numLayers];
					for (int i = 0; i < numLayers; i++, firstbit++)
						layers[i] = firstbit-1;
					offset += 2;
				} else {
					numLayers = bits.size()/2+1;
					int * bitsAr = new int[numLayers];
					for (int i = 0; i < bits.size(); i++) {
						if (i == bits.size()-1 && !checkNumeric(bits[i])) {
							printHelpMessage(cout);
							throw argException("Invalid format for specifying bits.");
						}
						if (~i&1) {
							if (checkNumeric(bits[i])) {
								bitsAr[i/2] = bits[i] - '0';
								if (bitsAr[i/2] < 1 || bitsAr[i/2] > 8) {
									printHelpMessage(cout);
									throw argException("Specified bits have to be in the range [1;8].");
								}
							} else {
								printHelpMessage(cout);
								throw argException("One of the specified \"bits\" is not a number.");
							}
						} else if (bits[i] != ',') {
							printHelpMessage(cout);
							throw argException("Invalid format for specifying bits to use.");
						}
					}
					std::sort(bitsAr, bitsAr+numLayers);
					layers = new int[numLayers];
					for (int i = 0; i < numLayers; i++)
						layers[i] = bitsAr[i]-1;
					offset += 2;
				}
			} else {
				printHelpMessage(cout);
				throw argException("Bits not specified.");
			}
		} else {
			printHelpMessage(cout);
			throw argException("Invalid extra option specified.");
		}
	}

	if (argc > (7+offset)) {
		if (args[7+offset] == "-eq" || args[7+offset] == "--equal") {
			equality = 1;
		} else {
			printHelpMessage(cout);
			throw argException("Invalid extra option specified.");
		}
	}

	if (numColours == -1) {
		numColours = 4;
		colours = new int[4];
		for (int i = 0; i < 4; i++)
			colours[i] = i;
	}

	if (numLayers == -1) {
		numLayers = 4;
		layers = new int[4];
		for (int i = 0; i < 4; i++)
			layers[i] = i;
	}

	return true;
}

struct stegoImage {
	png::image <png::rgba_pixel> img;
	int numLayers, numColours;
	int *colours, *layers;
	bool equality;
	stegoImage() {};
	png::byte & getByte(uint x, uint y, int colour) {
		if (colour == 0) {
			return img[y][x].red;
		} else if (colour == 1) {
			return img[y][x].green;
		} else if (colour == 2) {
			return img[y][x].blue;
		} else {
			return img[y][x].alpha;
		}
	}
};

void loadImage(png::image<png::rgba_pixel> &image, string filename) {
	try {
		image.read(filename);
	}
	catch (std::exception E) {
		throw imgException(E.what());
	}
	return;
}

void writeImage(png::image<png::rgba_pixel> &image, string filename) {
	try {
		image.write(filename);
	}
	catch (std::exception E) {
		throw imgException(E.what());
	}
	return;
}

void readData(string filename, std::vector<unsigned char> &data) {
	std::ifstream inp(filename, std::ios::binary|std::ios::in);
	data.assign(std::istreambuf_iterator <char>(inp), {});
	return;
}

void writeData(string filename, std::vector<unsigned char> &data) {
	std::ofstream out(filename, std::ios::binary|std::ios::out);
	std::copy(data.begin(), data.end(), std::ostreambuf_iterator <char>(out));
	return;
}

uint getDataSize(stegoImage &image, seedgen &locGen) {
	uint dataSize = 0;
	for (int i = 0; i < 32; i++) {
		location next = locGen.nextLocDataIndep();
		dataSize ^= ((((uint)image.getByte(next.x, next.y, next.colour) >> (uint)(image.layers[next.layer])) & 1) << (uint)i);
	}
	return dataSize;
}

void writeDataSize(stegoImage &image, seedgen &locGen, uint dataSize) {
	for (int i = 0; i < 32; i++) {
		location next = locGen.nextLocDataIndep();
		unsigned char & pix = image.getByte(next.x, next.y, next.colour);
		if ((dataSize >> i)&1) pix |= (1 << (image.layers[next.layer]));
		else pix &= (~(1 << (image.layers[next.layer])));
	}
	return;
}

void decode(stegoImage &image, seedgen &locGen, std::vector<unsigned char> &data) {
	uint dataSize = getDataSize(image, locGen);
	locGen.initSize(dataSize, image.equality);
	data.assign(dataSize, 0);
	for (int i = 0; i < dataSize; i++) {
		for (int k = 0; k < 8; k++) {
			location next = locGen.nextLoc();
			unsigned char pix = image.getByte(next.x, next.y, next.colour);
			data[i] ^= (((pix >> (uint)(image.layers[next.layer])) & (uint)0x01) << (uint)k);
		}
	}
	return;
}

void encode(stegoImage &image, seedgen &locGen, std::vector<unsigned char> &data) {
	writeDataSize(image, locGen, data.size());
	locGen.initSize(data.size(), image.equality);
	for (int i = 0; i < data.size(); i++) {
		for (int k = 0; k < 8; k++) {
			location next = locGen.nextLoc();
			unsigned char & pix = image.getByte(next.x, next.y, next.colour);
			if ((data[i] >> k)&1) pix |= (1 << (image.layers[next.layer]));
			else pix &= (~(1 << (image.layers[next.layer])));
		}
	}
	return;
}

int main(int argc, char ** argv) {
	initializeIO();
	bool mode;
	string files[3];
	string password;
	stegoImage image;
	image.numLayers = image.numColours = -1;
	bool equality = 0;
	if (!getOptions(argc, argv, mode, files, password, image.numColours, image.colours, image.numLayers, image.layers, equality)) return 1;
	loadImage(image.img, files[0]);
	seedgen locGen(image.img.get_width(), image.img.get_height(), image.numLayers, image.numColours, password.size(), password.c_str());
	std::vector<unsigned char> data;
	if (mode) {
		readData(files[2], data);
		encode(image, locGen, data);
		writeImage(image.img, files[1]);
	} else {
		decode(image, locGen, data);
		writeData(files[1], data);
	}
	return 0;
}