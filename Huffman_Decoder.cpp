#include "Huffman_Decoder.h"
#include <iostream>
#include <bitset>
#include <vector>

using namespace std;

//Used in the constructor, used later
void huffCodess(uint8_t* huffData, uint16_t* table);

Huffman_Decoder::Huffman_Decoder(uint8_t hlen[16], int hvalues[16])
{
    //cout << "Creating tables" << endl;
    uint16_t hcodes[16];
    huffCodess(hlen, hcodes);
    
    
    int num_values_8 = 0;
    vector<uint8_t> hval_lens[16];
    for (int i = 0; i < 16; i++) {
        this->real_num_values += hlen[i];
        if (i < 8) {
            num_values_8 += hlen[i];
        }
        for (int j = 0; j < hlen[i]; j++) {
            hval_lens->push_back(i+1);
        }
        if (hlen[i] != 0) {
            this->maxlen = i + 1;
        }
    }

    //Fill 8 Table
    for (int i = 0; i < num_values_8; i++) {
        uint8_t val_len = hval_lens->at(i);
        int value = hvalues[i];
        uint16_t value_code = hcodes[i];

        value_code = value_code << 8 - val_len; // 0000 0101 of len 3 --> 1010 000

        uint16_t twopow = (1 << 8 - val_len);

        for (uint8_t j = 0; j < twopow; j++) {
            HUFF_ENTRY entry = { value, val_len, value_code };
            hmap_8.insert(pair<uint8_t, HUFF_ENTRY>(value_code ^ j, entry));
            //cout << "Val : " << value << " Len : " << unsigned(val_len) << " Code : " << bitset<9>(unsigned((uint8_t)(value_code ^ j))) << endl;
        }
    }

    //cout << endl << "=================================================================" << endl << endl;

    //Fill 16 Table
    for (int i = num_values_8; i < this->real_num_values; i++) {
        uint8_t val_len = hval_lens->at(i);
        int value = hvalues[i];
        uint16_t value_code = hcodes[i];

        value_code = value_code << 16 - val_len; // 0000 0101 of len 3 --> 1010 000

        uint16_t twopow = (1 << 16 - val_len);

        for (uint8_t j = 0; j < twopow; j++) {
            HUFF_ENTRY entry = { value, val_len, value_code };
            hmap_16.insert(pair<uint16_t, HUFF_ENTRY>(value_code ^ j, entry));
            //cout << "Val : " << value << " Len : " << unsigned(val_len) << " Code : " << bitset<17>(value_code ^ j) << endl;
        }
    }

    //cout << "Done generating hmaps !" << endl;

}

HUFF_ENTRY Huffman_Decoder::getEntry(uint16_t in)
{
    uint16_t in_16 = in >> 8;
    uint16_t in_8 = in_16;
    // If entry is in hmap 8
    if (hmap_8.find(in_8) != hmap_8.end()) {
        return hmap_8.at(in_8);
    }
    //Entry has to be in hmap16
    else {
        return hmap_16.at(in);
    }
}

//Generate huff codes from depth array
void huffCodess(uint8_t* huffData, uint16_t* table)
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