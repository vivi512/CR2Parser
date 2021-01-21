#pragma once
#include <unordered_map>
using namespace std;

//Find the min between two values
#define MIN(a,b) ((a) <= (b) ? (a) : (b))

typedef struct HUFF_ENTRY {
    int value;
    uint8_t length;
    uint16_t code;
} HUFF_ENTRY;

class Huffman_Decoder
{
public:
    //Maximum length of Huffman codes
    #define MAX_BITLEN 16
    unordered_map<uint8_t, HUFF_ENTRY> hmap_8;
    unordered_map<uint16_t, HUFF_ENTRY> hmap_16;
    uint16_t hlen[16];
    int hvalues[16];
    //uint16_t hcodes[16];

    int maxlen;
    int real_num_values = 0;

    int size = 0;

    Huffman_Decoder(uint8_t hlen[16], int hvalues[16]);
    HUFF_ENTRY getEntry(uint16_t in);
};