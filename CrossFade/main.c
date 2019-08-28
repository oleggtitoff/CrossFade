#define _CRT_SECURE_NO_WARNINGS

#include <math.h>

#include "FilesOperations.h"

#define INPUT_FILE_NAME "Input.wav"
#define OUTPUT_FILE_NAME "Output.wav"

#define BYTES_PER_SAMPLE 4
#define DATA_BUFF_SIZE 1000
#define SAMPLE_RATE 48000
#define CHANNELS 2

#define FADE_TIME 0.01
#define TARGET_GAIN 0


typedef struct {
	double fadeTime;
	double targetGain;
} FadeParams;

typedef struct {
	double fadeAlpha;
	double targetGain;
} FadeCoeffs;

typedef struct {
	double fadeGain;
} FadeStates;


void init(FadeParams *params, FadeCoeffs *coeffs, FadeStates *states);
void setParams(FadeParams *params, FadeCoeffs *coeffs);
void run(FILE *inputFilePtr, FILE *outputFilePtr, FadeCoeffs *coeffs, FadeStates *states);


int main()
{
	FILE *inputFilePtr = openFile(INPUT_FILE_NAME, 0);
	FILE *outputFilePtr = openFile(OUTPUT_FILE_NAME, 1);
	uint8_t headerBuff[FILE_HEADER_SIZE];
	FadeParams fadeParams;
	FadeCoeffs fadeCoeffs;
	FadeStates fadeStates;

	init(&fadeParams, &fadeCoeffs, &fadeStates);
	setParams(&fadeParams, &fadeCoeffs);

	readHeader(headerBuff, inputFilePtr);
	writeHeader(headerBuff, outputFilePtr);
	run(inputFilePtr, outputFilePtr, &fadeCoeffs, &fadeStates);

	fclose(inputFilePtr);
	fclose(outputFilePtr);
	return 0;
}


int32_t doubleToFixed31(double x)
{
	if (x >= 1)
	{
		return INT32_MAX;
	}
	else if (x < -1)
	{
		return INT32_MIN;
	}

	return (int32_t)(x * (double)(1LL << 31));
}

int32_t	Saturation(int64_t x)
{
	if (x < (int64_t)INT32_MIN)
	{
		return INT32_MIN;
	}
	else if (x >(int64_t)INT32_MAX)
	{
		return INT32_MAX;
	}

	return (int32_t)x;
}

int32_t roundFixed63To31(int64_t x)
{
	return (int32_t)((x + (1LL << 30)) >> 31);
}

int32_t Add(int32_t x, int32_t y)
{
	return Saturation((int64_t)x + y);
}

int32_t Mul(int32_t x, int32_t y)
{
	if (x == INT32_MIN && y == INT32_MIN)
	{
		return INT32_MAX;
	}

	return roundFixed63To31((int64_t)x * y);
}


void initFadeParams(FadeParams *params)
{
	params->fadeTime = 0;
	params->targetGain = 0;
}

void initFadeCoeffs(FadeCoeffs *coeffs)
{
	coeffs->fadeAlpha = 0;
	coeffs->targetGain = 0;
}

void initFadeStates(FadeStates *states)
{
	states->fadeGain = 1;
}

void init(FadeParams *params, FadeCoeffs *coeffs, FadeStates *states)
{
	initFadeParams(params);
	initFadeCoeffs(coeffs);
	initFadeStates(states);
}


void calcCoeffs(FadeParams *params, FadeCoeffs *coeffs)
{
	coeffs->fadeAlpha  = 1.0 - exp(-1.0 / (SAMPLE_RATE * params->fadeTime));
	coeffs->targetGain = params->targetGain;
}

void setParams(FadeParams *params, FadeCoeffs *coeffs)
{
	params->fadeTime   = FADE_TIME;
	params->targetGain = TARGET_GAIN;

	calcCoeffs(params, coeffs);
}


double fadeGain(FadeCoeffs *coeffs, FadeStates *states)
{
	if (fabs(coeffs->targetGain - states->fadeGain) < 0.000001)
	{
		states->fadeGain = coeffs->targetGain;
	}

	states->fadeGain = coeffs->targetGain * coeffs->fadeAlpha + (1.0 - coeffs->fadeAlpha) * states->fadeGain;

	return states->fadeGain;
}

int32_t crossFade(FadeCoeffs *coeffs, FadeStates *states, int32_t x, int32_t y)
{
	double gain = fadeGain(coeffs, states);
	int32_t gain1 = doubleToFixed31(gain);
	int32_t gain2 = doubleToFixed31(1.0 - gain);

	return Add(Mul(gain1, x), Mul(gain2, y));
}

void run(FILE *inputFilePtr, FILE *outputFilePtr, FadeCoeffs *coeffs, FadeStates *states)
{
	int32_t dataBuff[DATA_BUFF_SIZE * CHANNELS];
	size_t samplesRead;
	uint32_t i;
	uint32_t counter = 0;

	while (1)
	{
		samplesRead = fread(dataBuff, BYTES_PER_SAMPLE, DATA_BUFF_SIZE, inputFilePtr);

		if (!samplesRead)
		{
			break;
		}
		if (counter > 50)
		{
			for (i = 0; i < samplesRead / CHANNELS; i++)
			{
				int32_t sample1 = dataBuff[i * CHANNELS];
				int32_t sample2 = dataBuff[i * CHANNELS + 1];

				dataBuff[i * CHANNELS] = crossFade(coeffs, states, sample1, sample2);
			}
		}

		fwrite(dataBuff, BYTES_PER_SAMPLE, samplesRead, outputFilePtr);
		counter++;
	}
}