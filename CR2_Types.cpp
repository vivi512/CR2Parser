#include "CR2_Types.h"

uint16_t swapBytes(uint16_t input)
{
    return (input << 8) | (input >> 8);
}

void printBits(uint16_t integer)
{
    for (int i = 0; i < 16; i++)
    {
        printf("%d", bool(integer & (1 << 15)));
        integer = integer << 1;
    }
}

void printBits(uint8_t integer)
{
    for (int i = 0; i < 8; i++)
    {
        printf("%d", bool(integer & (1 << 7)));
        integer = integer << 1;
    }
}


void ByteStream::loadBytes(FILE* fp, long s)
{

    size = s;
    bytes = new uint8_t[size];
    long floc = ftell(fp);

    int fileErrorCode = ferror(fp);
    if (fileErrorCode != 0)
    {
        printf("fileErrorCode = %d\n", fileErrorCode);
    }

    long numReadBytes = fread(bytes, sizeof(uint8_t), size, fp);
    if (numReadBytes != size)
    {
        printf("Warning in loadBytes(): Attempted to load %d bytes from file location %d \nManaged to read %d bytes\n\n", size, floc, numReadBytes);
    }
    size = numReadBytes;
}

uint16_t ByteStream::readBits(int N)
{
    int count = 0;
    while ((32 - bitStart < 24) && byteLoc < size)
    {
        bitBuffer = bitBuffer << 8;
        uint8_t byte = bytes[byteLoc];
        if (byte == 0xFF && (byteLoc + 1 < size))
        {
            if (bytes[byteLoc + 1] != 0x00)
            {
                printf("Byteloc  = %d\n", byteLoc);
                printf("Bitstart = %d\n", bitStart);
                printBits(uint16_t(bitBuffer >> 16)); printf(" ");
                printBits(uint16_t((bitBuffer << 16) >> 16));
                printf("\n\n");
                printBits(bytes[byteLoc]); printf(" ");
                printBits(bytes[byteLoc + 1]);
                printf("\n");

                printf("\n");

                // printf("byteLoc = %d\nSize = %d\n", byteLoc, size);
                if (byteLoc != size - 2)
                {
                    printf("We encountered an unexpected marker! byteLoc = %d\n", byteLoc + 1);
                    printf("Marker : %x%x \n", bytes[byteLoc], bytes[byteLoc + 1]);
                }
            }
            else
                byteLoc++;
        }
        bitBuffer += uint32_t(byte);
        byteLoc++;
        bitStart -= 8;
        count++;
    }
    uint32_t temp = bitBuffer << bitStart;
    bitStart += N;
    return uint16_t(temp >> (32 - N));
}

void ByteStream::print(int N)
{
    for (long i = 0; i < N; i++)
    {
        printBits(bytes[i]);
        printf(" ");
    }
    printf("\n");
}