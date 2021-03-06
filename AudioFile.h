/**********************************************************************
 Medea: A Digital Music Player

 @file  AudioFile.h

 @brief:

 manages reading audio files

 @author Eric Woodard
 @date   12/11/2020

 TODO: Rename to AudioFile.h
 **********************************************************************/
#ifndef MUSICFILE_H
#define MUSICFILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using std::string;
using std::ios;
using std::fstream;
using std::ios_base;

struct AudioHeader
{
	AudioHeader(fstream &inFile)
	{
		uint8_t chunkID[4];
		inFile.read(reinterpret_cast<char *>(&chunkID), sizeof(uint8_t) * 4);
		inFile.read(reinterpret_cast<char *>(&chunkSize), sizeof(uint32_t));

		uint8_t format[4];
		inFile.read(reinterpret_cast<char *>(&format), sizeof(uint8_t) * 4);

		uint8_t subChunk1ID[4];
		uint32_t subChunk1Size;
		inFile.read(reinterpret_cast<char *>(&subChunk1ID), sizeof(uint8_t) * 4);
		inFile.read(reinterpret_cast<char *>(&subChunk1Size), sizeof(uint32_t));

		inFile.read(reinterpret_cast<char *>(&audioFormat), sizeof(uint16_t));
		inFile.read(reinterpret_cast<char *>(&numChannels), sizeof(uint16_t));
		inFile.read(reinterpret_cast<char *>(&sampleRate), sizeof(uint32_t));
		inFile.read(reinterpret_cast<char *>(&byteRate), sizeof(uint32_t));

		inFile.read(reinterpret_cast<char *>(&blockAlighn), sizeof(uint16_t));
		inFile.read(reinterpret_cast<char *>(&bitsPerSample), sizeof(uint16_t));
		if (subChunk1Size == 18)
		{
			uint16_t extraSize;
			inFile.read(reinterpret_cast<char *>(&extraSize), sizeof(uint16_t));
			if (extraSize == 22)
			{
				//error can't read WAVE_FORMAT_EXTENSIBLE
			}
		}
		char subChunk2ID[4];
		char data[]{ 'd', 'a', 't', 'a' };
		inFile.read(reinterpret_cast<char *>(&subChunk2ID), sizeof(uint8_t) * 4);
		while (std::strncmp(subChunk2ID, data, 4) != 0)
		{
			inFile.read(reinterpret_cast<char *>(&subChunk2Size), sizeof(uint32_t));
			int junk;
			inFile.read(reinterpret_cast<char *>(&junk), sizeof(uint8_t) * subChunk2Size);
			inFile.read(reinterpret_cast<char *>(&subChunk2ID), sizeof(uint8_t) * 4);
		}

		inFile.read(reinterpret_cast<char *>(&subChunk2Size), sizeof(uint32_t));
		duration = subChunk2Size / (sampleRate * numChannels * bitsPerSample / 8);
		dataPos = inFile.tellg();
	}

	uint32_t chunkSize; //file size

	uint16_t audioFormat; //
	uint16_t numChannels; //number of audio channels usually two

	uint32_t sampleRate; //samples per second (probably 44100 or 48000)
	uint32_t byteRate; //bytes per second
	uint16_t blockAlighn; //bytes per frame
	uint16_t bitsPerSample; //

	uint32_t subChunk2Size; //size of the audio data
	
	int duration; //length of audio data
	std::streamoff dataPos; //loction of audio data in the file
};


class MusicFile
{
public:
	MusicFile(string path) 
		: fileStream(path, ios::in | ios::binary), header(fileStream) {}
	~MusicFile();

	void readSample(void* buffer, int bufferSize);

	uint32_t getSampleRate()  const { return header.sampleRate; }
	int getNumChannels() const { return header.numChannels; }
	uint16_t getBitsPerSample() const { return header.bitsPerSample; }
	uint16_t getBlockAlighn() const { return header.blockAlighn; }
	long getDataSize() const { return header.subChunk2Size; }

	int getCurrTrackTime(const long &numFrames);
	void seek(long bytes);

	std::streampos getDataPos() { return header.dataPos; }
	std::streampos getEndPos() { return header.dataPos + header.subChunk2Size; }
	std::streampos getCurrPos() { return fileStream.tellg(); }

	string timeToString(const long &numFrames);
	string timeToString();
	
private:
	fstream fileStream;
	AudioHeader header;
};

#endif

