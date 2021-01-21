#pragma once
#include <iostream>
#include <bitset>
//#ifndef CR2_TYPES
#include "CR2_Types.h"
//#endif // !CR2_TYPES

using namespace std;

/*
    Types and pretty print functions
*/

typedef unsigned char uchar;

int elementSizeTag(TIFF_TAG tag);


int dataSizeTag(TIFF_TAG tag);

void getShortPointerDataTag(uint16_t* short_val, TIFF_TAG tag, FILE* f);

int getDiffValue(uint16_t code, int len);

void huffCodes(uint8_t* huffData, uint16_t* table);

int elementSizeTag(TIFF_TAG tag);

uint16_t first_n_bits(uint16_t in, int desired_length);

string first_n_bits_str(uint16_t in, int desired_length);

void RegenerateBuffer(uint16_t* buffer, ByteStream* bstream, int* consumed_bits, long long* remaining_to_read);



