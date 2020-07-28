#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <getopt.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

enum class eGame {
	LegoRacers,
	Paperboy,
	NBA2000,
};

enum class ePlatform {
	PC,
	N64,
	PSX
};

eGame g_game = eGame::LegoRacers;
ePlatform g_platform = ePlatform::PC;

void assertWithMessage(bool condition, const std::string& message)
{
	if (!condition)
	{
		eprintf("%s\n", message.c_str());
		exit(1);
	}
}

bool strEqual(char const* a, char const* b)
{
	return strcmp(a,b) == 0;
}

typedef uint8_t Token;

template<class T> T read(FILE* fh)
{
	T t;
	fread(&t, 1, sizeof(T), fh);
	return t;
}

Token    readToken (FILE* fh) { return read<Token>(fh); }
int8_t   readSByte (FILE* fh) { return read<int8_t>(fh); }
uint8_t  readByte  (FILE* fh) { return read<uint8_t>(fh); }
int16_t  readShort (FILE* fh) { return read<int16_t>(fh); }
uint16_t readUShort(FILE* fh) { return read<uint16_t>(fh); }
int32_t  readInt   (FILE* fh) { return read<int32_t>(fh); }
float    readFloat (FILE* fh) { return read<float>(fh); }

std::string readString(FILE* fh)
{
	std::string buf("");
	uint8_t b;
	while (true)
	{
		fread(&b, 1, 1, fh);
		if (b == 0) { break; }
		buf += (char)b;
	}
	return buf;
}

void print(const std::string& message, int indent, int sqBracketStack, int sqBracketCount)
{
	int newIndent = indent;
	if ((sqBracketStack > 0 && message[0] != '[') || (sqBracketStack == 0 && message[0] == ']'))
	{
		newIndent = 0;
	}

	for (int i = 0; i < newIndent; ++i)
	{
		printf("\t");
	}

	if (sqBracketStack > 0 && sqBracketCount > 0)
	{
		printf(", ");
	}

	printf("%s", message.c_str());

	if (sqBracketStack <= 0)
	{
		printf("\n");
	}
}

void recursivePrint(
	FILE* fh,
	Token token,
	std::map<Token, std::vector<Token>>& structTable,
	int& indent,
	int& sqBracketStack,
	int& sqBracketCount
);

void handleString(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	print("\e[31m\"" + readString(fh) + "\"\e[39m", indent, sqBracketStack, sqBracketCount++);
}

void handleFloat(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readFloat(fh);
	print("(\e[36mfloat\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleQ_20_12(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readInt(fh) / (float)0x1000;
	print("(\e[36mQ20.12\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleInt(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	print(/*"(int) " + */std::to_string(readInt(fh)), indent, sqBracketStack, sqBracketCount++);
}

void handleLeftCurly(int& indent, int& sqBracketStack, int& sqBracketCount)
{
	print("{", indent++, sqBracketStack, sqBracketCount++);
}

void handleRightCurly(int& indent, int& sqBracketStack, int& sqBracketCount)
{
	print("}", --indent, sqBracketStack, sqBracketCount++);
}

void handleLeftBracket(int& indent, int& sqBracketStack, int& sqBracketCount)
{
	sqBracketCount = -1;
	print("[", indent, ++sqBracketStack, sqBracketCount++);
}

void handleRightBracket(int& indent, int& sqBracketStack, int& sqBracketCount)
{
	sqBracketCount = -1;
	print("]", indent, --sqBracketStack, sqBracketCount++);
}

void handleQ_4_4(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readByte(fh) / 16.0f;
	print("(\e[36mQ4.4\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleByte(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	uint8_t value = readByte(fh);
	print("(\e[36mbyte\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleQ_8_8(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readShort(fh) / 256.0f;
	print("(\e[36mQ8.8\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleUShort(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	uint16_t value = readUShort(fh);
	print("(\e[36mushort\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleQ_4_12(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readShort(fh) / (float)0x1000;
	print("(\e[36mQ4.12\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleQ_11_5(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readShort(fh) / (float)0x20;
	print("(\e[36mQ11.5\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleUnknown11(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	// suspect this is Q6.10

	uint16_t rawValue = readShort(fh);
	char str[32];
	sprintf(str, "(\e[36mUNKNOWN_11\e[39m) 0x%04x", rawValue);
	print(str, indent, sqBracketStack, sqBracketCount++);

	//print("(\e[36mQ6.10\e[39m) " + std::to_string(readShort(fh) / (float)0x400), indent, sqBracketStack, sqBracketCount++);
}

void handleByteN(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	float value = readSByte(fh) / (float)0x7f;
	print("(\e[36mByteN\e[39m) " + std::to_string(value), indent, sqBracketStack, sqBracketCount++);
}

void handleUnknown13(FILE* fh, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	// presumably some sort of 16bit fixed-point

	uint16_t rawValue = readShort(fh);
	char str[32];
	sprintf(str, "(\e[36mUNKNOWN_13\e[39m) 0x%04x", rawValue);
	print(str, indent, sqBracketStack, sqBracketCount++);
}

void handleArray(FILE* fh, std::map<Token, std::vector<Token>>& structTable, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	uint16_t arrayLen = readUShort(fh);
	Token arrayType = readToken(fh);
	for (int i = 0; i < arrayLen; ++i)
	{
		recursivePrint(fh, arrayType, structTable, indent, sqBracketStack, sqBracketCount);
	}
}

void handleStructDef(FILE* fh, std::map<Token, std::vector<Token>>& structTable)
{
	Token structId = readToken(fh);
	uint8_t structLen = readByte(fh);
	std::vector<Token> structDef;
	for (int i = 0; i < structLen; ++i)
	{
		structDef.push_back(readToken(fh));
	}
	structTable[structId] = structDef;
}

void handleStructInstance(Token structId, FILE* fh, std::map<Token, std::vector<Token>>& structTable, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	const std::vector<Token> structDef = structTable[structId];
	for (auto& structToken : structDef)
	{
		recursivePrint(fh, structToken, structTable, indent, sqBracketStack, sqBracketCount);
	}
}

void handleBlock(Token keyword, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	char str[32];
	sprintf(str, "\e[32mBlock 0x%02X:\e[39m", keyword);
	print(str, indent, sqBracketStack, sqBracketCount++);
}

void recursivePrint(FILE* fh, Token token, std::map<Token, std::vector<Token>>& structTable, int& indent, int& sqBracketStack, int& sqBracketCount)
{
	// these tokens appear to be universal
	if (token == 0x02) return handleString(fh, indent, sqBracketStack, sqBracketCount);
	if (token == 0x04) return handleInt(fh, indent, sqBracketStack, sqBracketCount);
	if (token == 0x05) return handleLeftCurly(indent, sqBracketStack, sqBracketCount);
	if (token == 0x06) return handleRightCurly(indent, sqBracketStack, sqBracketCount);
	if (token == 0x07) return handleLeftBracket(indent, sqBracketStack, sqBracketCount);
	if (token == 0x08) return handleRightBracket(indent, sqBracketStack, sqBracketCount);
	if (token == 0x0c) return handleByte(fh, indent, sqBracketStack, sqBracketCount);
	if (token == 0x0e) return handleUShort(fh, indent, sqBracketStack, sqBracketCount);

	// for some reason PSX doesn't use floats, it uses Q20.12 fixed-point.
	if (token == 0x03)
	{
		if (g_platform == ePlatform::PSX)
			return handleQ_20_12(fh, indent, sqBracketStack, sqBracketCount);
		else
			return handleFloat(fh, indent, sqBracketStack, sqBracketCount);
	}

	// these tokens are only present in LegoRacers/Paperboy but might be supported elsewhere
	if (g_game == eGame::LegoRacers || g_game == eGame::Paperboy)
	{
		if (token == 0x0b) return handleQ_4_4(fh, indent, sqBracketStack, sqBracketCount);
		if (token == 0x0d) return handleQ_8_8(fh, indent, sqBracketStack, sqBracketCount);
	}

	// these tokens are only present in the console games but might be supported elsewhere
	if ((g_game == eGame::LegoRacers || g_game == eGame::Paperboy) &&
		(g_platform == ePlatform::N64 || g_platform == ePlatform::PSX))
	{
		if (token == 0x0f) return handleQ_4_12(fh, indent, sqBracketStack, sqBracketCount);
		if (token == 0x10) return handleQ_11_5(fh, indent, sqBracketStack, sqBracketCount);
		if (token == 0x12) return handleByteN(fh, indent, sqBracketStack, sqBracketCount);
	}

	// these tokens are only present in Paperboy but might be supported elsewhere
	if (g_game == eGame::Paperboy)
	{
		if (token == 0x11) return handleUnknown11(fh, indent, sqBracketStack, sqBracketCount);
		if (token == 0x13) return handleUnknown13(fh, indent, sqBracketStack, sqBracketCount);
	}

	// these token values are specific to LegoRacers/Paperboy
	if (g_game == eGame::LegoRacers || g_game == eGame::Paperboy)
	{
		if (token == 0x14) return handleArray(fh, structTable, indent, sqBracketStack, sqBracketCount);
		if (token == 0x16) return handleStructDef(fh, structTable);
		if (token >= 0x27) return handleBlock(token, indent, sqBracketStack, sqBracketCount);
	}

	// these token values are specific to NBA2000
	if (g_game == eGame::NBA2000)
	{
		if (token == 0x12) return handleArray(fh, structTable, indent, sqBracketStack, sqBracketCount);
		if (token == 0x14) return handleStructDef(fh, structTable);
		if (token >= 0x25) return handleBlock(token, indent, sqBracketStack, sqBracketCount);
	}

	if (structTable.find(token) != structTable.end())
	{
		return handleStructInstance(token, fh, structTable, indent, sqBracketStack, sqBracketCount);
	}

	eprintf("Unknown token type 0x%02x, did you specify the game and platform?\n", token);
	exit(1);
}

void parseCommandLine(int argc, char** argv)
{
	while (1)
	{
		static struct option long_options[] = {
			{ "game",     required_argument, 0, 'g' },
			{ "platform", required_argument, 0, 'p' },
			{ 0, 0, 0, 0 }
		};

		int option_index = 0;
		int c = getopt_long(argc, argv, "g:p:", long_options, &option_index);
		
		if (c == -1) {
			break;
		}

		switch (c)
		{
			case 'g':
			{
				if (strEqual(optarg,"legoracers"))
				{
					g_game = eGame::LegoRacers;
				}
				else if (strEqual(optarg,"paperboy"))
				{
					g_game = eGame::Paperboy;
				}
				else if (strEqual(optarg,"nba2000"))
				{
					g_game = eGame::NBA2000;
				}
				else
				{
					eprintf("Unknown game '%s'\n", optarg);
					exit(1);
				}
				break;
			}
			case 'p':
			{
				if (strEqual(optarg,"pc"))
				{
					g_platform = ePlatform::PC;
				}
				else if (strEqual(optarg,"n64"))
				{
					g_platform = ePlatform::N64;
				}
				else if (strEqual(optarg,"psx"))
				{
					g_platform = ePlatform::PSX;
				}
				else
				{
					eprintf("Unknown game '%s'\n", optarg);
					exit(1);
				}
				break;
			}
		}
	}
}

int main(int argc, char** argv)
{
	parseCommandLine(argc, argv);

	if (optind >= argc)
	{
		eprintf("Expected a filename.\n");
		return 1;
	}

	int const numFiles = (argc - optind);
	for (int i = 0; i < numFiles; ++i)
	{
		char const* filepath = argv[optind+i];

		if (i > 0)
		{
			printf("\n");
		}

		eprintf("%s\n", filepath);
		//printf("%s\n", filepath);

		FILE* fh = fopen(filepath, "rb");
		if (!fh)
		{
			eprintf("Failed to open %s\n", filepath);
			return 1;
		}

		int indent = 0;
		int sqBracketStack = 0;
		int sqBracketCount = 0;
		std::map<Token, std::vector<Token>> structTable;
		while (true)
		{
			char dummy;
			fread(&dummy,1,1,fh);
			if(feof(fh))break;
			fseek(fh,-1,SEEK_CUR);

			Token token = readToken(fh);
			recursivePrint(fh, token, structTable, indent, sqBracketStack, sqBracketCount);
		}

		fclose(fh);
	}
	return 0;
}
