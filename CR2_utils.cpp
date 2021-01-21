#pragma once
#include "CR2_utils.h"

int dataSizeTag(TIFF_TAG tag)
{
    return elementSizeTag(tag) * tag.values;
}

void getShortPointerDataTag(uint16_t* short_val, TIFF_TAG tag, FILE* f) {
    uint16_t ushortString[255];

    if (dataSizeTag(tag) > 4)
    {
        long currentOffset = ftell(f);
        fseek(f, tag.value, SEEK_SET);
        if (tag.type == 3) {
            fread(&ushortString, sizeof(uint16_t), tag.values, f);
            for (unsigned int i = 0; i < tag.values; i++)
                short_val[i] = ushortString[i];
        }
        fseek(f, currentOffset, SEEK_SET);
    }
    else {
        cout << "That was not a pointer";
    }
}

int getDiffValue(uint16_t code, int len)
{
    return (code >= (1 << len - 1) ? code : (code - (1 << len) + 1));
}

void huffCodes(uint8_t* huffData, uint16_t* table)
{
    // [0,1] then shift << 1 and add 0 , 1 repeat
    int tabCount = 0;
    int numNotUsed = 2;
    int numUsed = 0;
    uint16_t unusedCodes[300];
    uint16_t usedCodes[300];
    uint16_t temp[300];
    unusedCodes[0] = 0;
    unusedCodes[1] = 1;

    for (int i = 0; i < 16; i++)
    {
        // Use a number of unused codes
        int num = huffData[i];
        for (int j = 0; j < num; j++)
            usedCodes[numUsed + j] = unusedCodes[j];

        // Remove those codes from unused codes list
        numUsed += num;
        for (int j = num; j < numNotUsed; j++)
            temp[j - num] = unusedCodes[j];
        numNotUsed -= num;

        // Update new codes
        for (int j = 0; j < numNotUsed; j++)
        {
            unusedCodes[2 * j + 0] = (temp[j] << 1) + 0;
            unusedCodes[2 * j + 1] = (temp[j] << 1) + 1;
        }
        numNotUsed *= 2;
    }

    for (int i = 0; i < numUsed; i++)
    {
        table[i] = usedCodes[i];
    }
}

int elementSizeTag(TIFF_TAG tag)
{
    int elSize;
    switch (tag.type)
    {
    case 1:
        elSize = 1;
        break;
    case 2:
        elSize = 1;
        break;
    case 3:
        elSize = 2;
        break;
    case 4:
        elSize = 4;
        break;
    case 5:
        elSize = 8;
        break;
    case 6:
        elSize = 1;
        break;
    case 7:
        elSize = 1;
        break;
    case 8:
        elSize = 2;
        break;
    case 9:
        elSize = 4;
        break;
    case 10:
        elSize = 8;
        break;
    case 11:
        elSize = 4;
        break;
    case 12:
        elSize = 8;
        break;
    default:
        elSize = 4;
        break;
    }
    return elSize;
}






uint16_t first_n_bits(uint16_t in, int desired_length) {
    uint16_t mask = 0b1000000000000000;
    for (int i = 0; i < desired_length; i++) {
        mask = mask >> 1;
        mask = mask ^ 0b1000000000000000;
    }
    uint16_t inter = in & mask;
    return inter >> 8 * sizeof(uint16_t) - desired_length;
}

string first_n_bits_str(uint16_t in, int desired_length) {
    string in_str = bitset<16>(in).to_string();
    return in_str.substr(0, desired_length);
}



//11010, next 2 bits are 10. We consumed the first 2 (11). New buffer will be 01010. Consumed_bits will be reset to 0.
void RegenerateBuffer(uint16_t* buffer, ByteStream* bstream, int* consumed_bits, long long* remaining_to_read) {
    *buffer = *buffer << *consumed_bits;
    *buffer = *buffer ^ bstream->readBits(*consumed_bits);
    *remaining_to_read -= *consumed_bits;
    *consumed_bits = 0;
}



