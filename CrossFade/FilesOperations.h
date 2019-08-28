#ifndef FILES_OPERATIONS
#define FILES_OPERATIONS

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#define ALWAYS_INLINE static inline

#define FILE_HEADER_SIZE 44


typedef enum {
	binaryRead = 0,
	binaryWrite = 1,
	read = 2,
	write = 3
} fileOpenModes;


ALWAYS_INLINE FILE * openFile(const char *fileName, const int8_t mode)
{
	// opens file in a certain mode
	// if mode = 0 - read as a binary, if mode = 1 - write as a binary
	// if mode = 2 - read, 			   if mode = 3 - write

	FILE *filePtr = NULL;

	switch (mode)
	{
	case binaryRead:
		if ((filePtr = fopen(fileName, "rb")) == NULL)
		{
			printf("Error opening file for reading as a binary\n");
			system("pause");
			exit(0);
		}
		break;

	case binaryWrite:
		if ((filePtr = fopen(fileName, "wb")) == NULL)
		{
			printf("Error opening file for writing as a binary\n");
			system("pause");
			exit(0);
		}
		break;

	case read:
		if ((filePtr = fopen(fileName, "r")) == NULL)
		{
			printf("Error opening file for reading\n");
			system("pause");
			exit(0);
		}
		break;

	case write:
		if ((filePtr = fopen(fileName, "w")) == NULL)
		{
			printf("Error opening file for writing\n");
			system("pause");
			exit(0);
		}
		break;

	default:
		exit(0);
		break;
	}

	return filePtr;
}

ALWAYS_INLINE void readHeader(uint8_t *headerBuff, FILE *inputFilePtr)
{
	if (fread(headerBuff, FILE_HEADER_SIZE, 1, inputFilePtr) != 1)
	{
		printf("Error reading input file (header)\n");
		system("pause");
		exit(0);
	}
}

ALWAYS_INLINE void writeHeader(uint8_t *headerBuff, FILE *outputFilePtr)
{
	if (fwrite(headerBuff, FILE_HEADER_SIZE, 1, outputFilePtr) != 1)
	{
		printf("Error writing output file (header)\n");
		system("pause");
		exit(0);
	}
}

#endif /* FILES_OPERATIONS */
