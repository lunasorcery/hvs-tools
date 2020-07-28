#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

enum class eVersion {
	Jam = 0,
	LJam = 1,
};

size_t getNameLengthForVersion(eVersion version)
{
	switch (version)
	{
		case eVersion::Jam:  return 15;
		case eVersion::LJam: return 12;
	}
}

template<class T> T read(FILE* fh)
{
	T t;
	fread(&t, 1, sizeof(T), fh);
	return t;
}

uint8_t readU8(FILE* fh) { return read<uint8_t>(fh); }
uint32_t readU32(FILE* fh) { return read<uint32_t>(fh); }

std::string readFilename(FILE* fh, eVersion version)
{
	size_t const nameLength = getNameLengthForVersion(version);

	char buffer[16];
	fread(buffer, 1, nameLength, fh);
	buffer[nameLength] = 0;

	return std::string(buffer);
}

void recurse(std::string const& path, FILE* fh, eVersion version)
{
	mkdir(path.c_str(), 0755);

	uint32_t numFiles = readU32(fh);
	for (uint32_t i = 0; i < numFiles; ++i)
	{
		std::string const filename = readFilename(fh, version);
		uint32_t const offset = readU32(fh);
		uint32_t const size = readU32(fh);
		
		std::string const fullpath = path + "/" + filename;
		printf("%s\n", fullpath.c_str());
		
		size_t const pos = ftell(fh);
		fseek(fh, offset, SEEK_SET);
		{
			FILE* fh2 = fopen(fullpath.c_str(), "wb");
			for(uint32_t j=0;j<size;++j)
			{
				char b;
				fread(&b,1,1,fh);
				fwrite(&b,1,1,fh2);
			}
			fclose(fh2);
		}
		fseek(fh, pos, SEEK_SET);
	}

	uint32_t numDirs = readU32(fh);
	for (uint32_t i = 0; i < numDirs; ++i)
	{
		std::string const dirname = readFilename(fh, version);
		uint32_t const offset = readU32(fh);

		size_t const pos = ftell(fh);
		fseek(fh, offset, SEEK_SET);
		{
			std::string const fullpath = path + "/" + dirname;
			recurse(fullpath, fh, version);
		}
		fseek(fh, pos, SEEK_SET);
	}
}

void unjam(std::string const& path)
{
	FILE* fh = fopen(path.c_str(), "rb");
	if(!fh)
	{
		exit(1);
	}

	eVersion version;

	char magic[4];
	fread(magic, 1, 4, fh);
	if (memcmp(magic,"JAM",3)==0)
	{
		version = eVersion::Jam;
		fseek(fh, -1, SEEK_CUR);
	}
	else if (memcmp(magic,"LJAM",4)==0)
	{
		version = eVersion::LJam;
	}
	else
	{
		exit(2);
	}

	std::string targetPath = path + ".dump";
	recurse(targetPath, fh, version);
}

int main(int argc, char** argv)
{
	if (argc <= 1)
	{
		eprintf("Expected a filename.\n");
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		unjam(argv[i]);
	}
}
