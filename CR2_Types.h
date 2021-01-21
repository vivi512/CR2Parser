#pragma once
#include <iostream>

using namespace std;

/*
    Types
*/

typedef unsigned char uchar;

#pragma pack(push,1)    //Avoids bad stuff when reading uint8_t

typedef struct TIFF_HEADER {
    char     id[2];
    uint16_t magic_number;   //Portable unsigned 16 bit integer --> almost unsigned short
    uint32_t offset;    //Portable unsigned 32 bit integer --> almost unsigned long
} TIFF_HEADER;

typedef struct CR2_HEADER {
    char     magic_word[2];      //short
    uint8_t  major_version;      //Stored as char but it's actually a number. Use int(xxx_version)
    uint8_t  minor_version;      //Stored as char but it's actually a number. Use int(xxx_version)
    uint32_t raw_ifd_offset;    //long
} CR2_HEADER;

typedef struct TIFF_TAG {
    uint16_t ID;        //ID of tag
    uint16_t type;      //type of "value"
    uint32_t values;    //Number of values
    uint32_t value;     //The actual value
} TIFF_TAG;

// THE FOLLOWING ARE RAW IFD HEADERS AND THEIR ENDIANNESS IS DIFFERENT
uint16_t swapBytes(uint16_t input);

typedef struct DHT_HEADER
{
    uint16_t marker, length;
    uint8_t  tc_index0;
    uint8_t  huff_data_0[16];
    uint8_t  huff_vals_0[15];
    uint8_t  tc_index1;
    uint8_t  huff_data_1[16];
    uint8_t  huff_vals_1[15];
    void swap() {
        marker = swapBytes(marker);
        length = swapBytes(length);
    }
} DHT_HEADER;

typedef struct SOF3_HEADER {
    uint16_t marker, length;
    uint8_t  sampleP;
    uint16_t num_lines, samp_per_lin;
    uint8_t  comp_per_frame;
    uint8_t  sFactors[6];   //Changed from 12
    void swap() {
        marker = swapBytes(marker);
        length = swapBytes(length);
        num_lines = swapBytes(num_lines);
        samp_per_lin = swapBytes(samp_per_lin);
    }
} SOF3_HEADER;

typedef struct SOS_HEADER {
    uint16_t marker, length;
    uint8_t  numComp;
    uint8_t  scanCompSel[4]; // Changed from 8
    uint8_t  remBytes[3];
    void swap() {
        marker = swapBytes(marker);
        length = swapBytes(length);
    }
} SOS_HEADER;

#pragma pack(pop) //End of #pragma pack(push,1) (line 12)

/*
    ByteStream + functions associated
*/

struct ByteStream
{
    uint8_t* bytes;
    long size;
    long byteLoc;

    uint32_t bitBuffer;
    int bitStart;

    // Initialize bitStart to 32 so that the bitBuffer is "initialized" as entirely empty
    ByteStream() : size(0), byteLoc(0), bitStart(32), bitBuffer(0) {};

    void loadBytes(FILE* fp, long s);
    uint16_t readBits(int N);
    void print(int N);
};

void printBits(uint16_t integer);
void printBits(uint8_t integer);
//void ByteStream::loadBytes(FILE* fp, long s);
//uint16_t ByteStream::readBits(int N);
//void ByteStream::print(int N);

/*
    ENUMS
*/

enum TAG_ID_TYPE {
    SENSOR_INFO = 224,
    IMAGE_WIDTH = 256,
    IMAGE_LENGTH = 257,
    BITS_PER_SAMPLE = 258,
    COMPRESSION = 259,
    PHOTOMETRIC_INTERPRETATION = 262,
    MAKE = 271,
    MODEL = 272,
    STRIP_OFFSET = 273,
    ORIENTATION = 274,
    SAMPLES_PER_PIXEL = 277,
    ROWS_PER_STRIP = 278,
    STRIP_BYTE_COUNTS = 279,
    X_RESOLUTION = 282,
    Y_RESOLUTION = 283,
    PLANAR_CONFIGURATION = 284,
    RESOLUTION_UNIT = 296,
    DATE_TIME = 306,
    ARTIST = 315,
    JPEG_INTERCHANGE_FORMAT = 513,
    JPEG_INTERCHANGE_FORMAT_LENGTH = 514,
    COLOR_BALANCE = 16385,
    COPYRIGHT = 33432,
    EXIF = 34665,
    GPS = 34853,
    CR2_SLICE = 50752,
    SRAW_TYPE = 50885,
    EXPOSURE_TIME = 33434,
    F_NUMBER = 33437,
    MAKERNOTE = 37500,
};

