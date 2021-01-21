#pragma once
#include "CR2_pprint.h"

void pprint_sos(SOS_HEADER sos_header) {
    printf("\n\n\n");
    printf("SOS_HEADER\n\n");
    printf("%6s = %04x\n", "Marker", sos_header.marker);
    printf("%6s = %d\n", "Length", sos_header.length);
    printf("%6s = %d\n", "# comp", sos_header.numComp);
    printf("\n");
    printf("%s", "Scan component selector : \n");

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (j != 1)
                printf("%20s %x", " ", sos_header.scanCompSel[j + 2 * i]);
            else
                printf("%20s %02x\n", " ", sos_header.scanCompSel[j + 2 * i]);
        }
    }
    printf("Last three bytes: \n");
    for (int i = 0; i < 3; i++)
    {
        printf("%20s %02x\n", " ", sos_header.remBytes[i]);
    }
}

void pprint_sof3(SOF3_HEADER sof3_header) {
    cout << "\n\tSOF3 Header : " << endl;

    printf("%15s = %04x\n", "Marker", sof3_header.marker);
    printf("%15s = %04x\n", "Length", sof3_header.length);
    printf("%15s = %d\n", "SampleP", sof3_header.sampleP);
    printf("%15s = %d\n", "Num Lines", sof3_header.num_lines);
    printf("%15s = %d\n", "Samp per line", sof3_header.samp_per_lin);
    printf("%15s = %d\n", "# comp per frame", sof3_header.comp_per_frame);
    printf("\n");

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            printf("%02x", sof3_header.sFactors[j + 3 * i]);
            if (j != 2)
                printf(" : ");
            else
                printf("\n");
        }
    }
}

void print_tiff_header(TIFF_HEADER th)
{
    cout << "Endians : \t\t" << th.id[0] << th.id[1] << endl;
    cout << "TIFF Magic Number : \t" << th.magic_number << " (Should be 0x002a or 42, in decimal)" << endl;
    cout << "TIFF Offset : \t\t" << th.offset << " (Should be 0x0000 0010 or 16, in decimal)" << endl;
}

void print_cr2_header(CR2_HEADER ch)
{
    cout << "CR2 Magic Word : \t" << ch.magic_word[0] << ch.magic_word[1] << " (Should be \"CR\")" << endl;
    cout << "CR2 Version :    \t" << int(ch.major_version) << "." << int(ch.minor_version) << endl;
    cout << "RAW IFD Offset : \t" << ch.raw_ifd_offset << endl;
}

//Functions defined after printTiffTag
void printTagInfo(TIFF_TAG tag);
void printTagType(TIFF_TAG tag);
void printPointerDataTag(TIFF_TAG tag, FILE* f);

void printTiffTag(TIFF_TAG tiff_tag, FILE* fp)
{
    printTagInfo(tiff_tag);
    printTagType(tiff_tag);
    printf("\nData size   : %d", dataSizeTag(tiff_tag));
    if (dataSizeTag(tiff_tag) > 4)
    {
        printf(" : Pointer type");
        long currentOffset = ftell(fp);
        fseek(fp, tiff_tag.value, SEEK_SET);
        printPointerDataTag(tiff_tag, fp);
        fseek(fp, currentOffset, SEEK_SET);
    }
    cout << endl;
}

void printPointerDataTag(TIFF_TAG tag, FILE* f)
{
    char charString[255];
    uint16_t ushortString[255];
    uint32_t ulongString[255];
    long longString[255];
    double dIEEE;

    switch (tag.type)
    {
    case 1:
        // printf("Type        : %s", "unsigned char");

        break;
    case 2: // string (with an ending zero)
        fread(&charString, sizeof(char), tag.values, f);
        printf("\nValue (Ptr) : %0.*s", tag.values, charString);
        break;
    case 3:  // unsigned short (2 bytes)
        fread(&ushortString, sizeof(uint16_t), tag.values, f);
        printf("\nValue (Ptr) : [");
        for (unsigned int i = 0; i < tag.values - 1; i++)
            printf("%d, ", ushortString[i]);
        printf("%d]", ushortString[tag.values - 1]);
        break;
    case 4: // unsigned long (4 bytes)
        break;
    case 5:
        // printf("Type        : %s", "unsigned rationnal (2 unsigned long)");
        fread(&ulongString, sizeof(uint32_t), 2, f);
        printf("\nValue (Ptr) : %d / %d", ulongString[0], ulongString[1]);
        break;
    case 6:
        // printf("Type        : %s", "signed char");

        break;
    case 7:
        // printf("Type        : %s", "byte sequence");

        break;
    case 8:
        // printf("Type        : %s", "signed short");

        break;
    case 9:
        // printf("Type        : %s", "signed long");

        break;
    case 10: // signed rationnal (2 signed long)
        fread(&longString, sizeof(uint32_t), 2, f);
        printf("\nValue (Ptr) : %d / %d", longString[0], longString[1]);
        break;
    case 11:
        // printf("Type        : %s", " float, 4 bytes, IEEE format");

        break;
    case 12: // float, 8 bytes, IEEE format
        fread(&dIEEE, sizeof(double), 1, f);
        printf("\nValue (Ptr) : %f", dIEEE);
        break;
    default:
        // printf("Type        : %s", " unknown type");
        break;
    }
}

void pprint_DHT(DHT_HEADER dh) {
    cout << "\n\tDHT Header : " << endl;
    cout << "Marker : " << dh.marker << " (Should be 65476), length : " << dh.length << endl;
    cout << "Table class / Huffman table index : ";
    cout << hex << unsigned(dh.tc_index0) << dec << endl;
    cout << "Huff_data_0 : [";
    for (int i = 0; i < 15; i++) {
        cout << hex << unsigned(dh.huff_data_0[i]) << dec;
        cout << ", ";
    } cout << hex << unsigned(dh.huff_data_0[15]) << dec; cout << "]" << endl;
    cout << "Huff_vals_0 : [";
    for (int i = 0; i < 14; i++) {
        cout << hex << unsigned(dh.huff_vals_0[i]) << dec;
        cout << ", ";
    } cout << hex << unsigned(dh.huff_vals_0[14]) << dec; cout << "]" << endl;

    cout << "Table class / Huffman table index : ";
    cout << hex << unsigned(dh.tc_index1) << dec << endl;
    cout << "Huff_data_1 : [";
    for (int i = 0; i < 15; i++) {
        cout << hex << unsigned(dh.huff_data_1[i]) << dec;
        cout << ", ";
    } cout << hex << unsigned(dh.huff_data_1[15]) << dec; cout << "]" << endl;
    cout << "Huff_vals_1 : [";
    for (int i = 0; i < 14; i++) {
        cout << hex << unsigned(dh.huff_vals_1[i]) << dec;
        cout << ", ";
    } cout << hex << unsigned(dh.huff_vals_1[14]) << dec; cout << "]" << endl;

}

void printTagType(TIFF_TAG tag)
{
    switch (tag.type)
    {
    case 1:
        printf("Type        : %s", "unsigned char");
        break;
    case 2:
        printf("Type        : %s", "string (with an ending zero)");
        break;
    case 3:
        printf("Type        : %s", "unsigned short (2 bytes)");
        break;
    case 4:
        printf("Type        : %s", "unsigned long (4 bytes)");
        break;
    case 5:
        printf("Type        : %s", "unsigned rationnal (2 unsigned long)");
        break;
    case 6:
        printf("Type        : %s", "signed char");
        break;
    case 7:
        printf("Type        : %s", "byte sequence");
        break;
    case 8:
        printf("Type        : %s", "signed short");
        break;
    case 9:
        printf("Type        : %s", "signed long");
        break;
    case 10:
        printf("Type        : %s", " signed rationnal (2 signed long)");
        break;
    case 11:
        printf("Type        : %s", " float, 4 bytes, IEEE format");
        break;
    case 12:
        printf("Type        : %s", " float, 8 bytes, IEEE format");
        break;
    default:
        printf("Type        : %s", " unknown type");
        break;
    }
}


void printTagInfo(TIFF_TAG tag)
{
    switch (tag.ID)
    {
    case 256:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "ImageWidth", tag.values, tag.value);
        break;
    case 257:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "ImageLength", tag.values, tag.value);
        break;
    case 258:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "BitsPerSample", tag.values, tag.value);
        break;
    case 259:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Compression", tag.values, tag.value);
        break;
    case 262:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "PhotometricInterpretation", tag.values, tag.value);
        break;
    case 271:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Make", tag.values, tag.value);
        break;
    case 272:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Model", tag.values, tag.value);
        break;
    case 273:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "StripOffsets", tag.values, tag.value);
        break;
    case 274:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Orientation", tag.values, tag.value);
        break;
    case 277:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "SamplesPerPixel", tag.values, tag.value);
        break;
    case 278:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "RowsPerStrip", tag.values, tag.value);
        break;
    case 279:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "StripByteCounts", tag.values, tag.value);
        break;
    case 282:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "XResolution", tag.values, tag.value);
        break;
    case 283:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "YResolution", tag.values, tag.value);
        break;
    case 284:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "PlanarConfiguration", tag.values, tag.value);
        break;
    case 296:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "ResolutionUnit", tag.values, tag.value);
        break;
    case 306:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "DateTime", tag.values, tag.value);
        break;
    case 315:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Artist", tag.values, tag.value);
        break;
    case 513:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "JPEGInterchangeFormat", tag.values, tag.value);
        break;
    case 514:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "JPEGInterchangeFormatLength", tag.values, tag.value);
        break;
    case 33432:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Copyright", tag.values, tag.value);
        break;
    case 34665:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "EXIF", tag.values, tag.value);
        break;
    case 34853:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "GPS", tag.values, tag.value);
        break;
    case 50648:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "??", tag.values, tag.value);
        break;
    case 50649:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "??", tag.values, tag.value);
        break;
    case 50656:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "??", tag.values, tag.value);
        break;
    case 50752:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "CR2 slice", tag.values, tag.value);
        break;
    case 50885:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "sRawType", tag.values, tag.value);
        break;
    case 50908:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "??", tag.values, tag.value);
        break;
    case 33434:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Exposure Time", tag.values, tag.value);
        break;
    case 33437:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "f Number", tag.values, tag.value);
        break;
    case 37500:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "Makernote", tag.values, tag.value);
        break;
    default:
        printf("Tag ID      : %d\nDescription : %s\nValues      : %d\nValue       : %d\n", tag.ID, "??", tag.values, tag.value);
        break;
    }
}