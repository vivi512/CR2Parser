#pragma once
#include "RAW_CR2.h"


//Constructor : sets a file associated with the .CR2
RAW_CR2::RAW_CR2(string path) {
    //"V:\\Temp\\Photos Neige\\J0_J1\\_MG_9532.CR2"
    this->fp_path = path;
}

void RAW_CR2::pprint()
{
	cout << "\t -- Printing CR2 Info --" << endl;
	cout << "Width : " << this->width << endl;
	cout << "Height : " << this->height << endl;
	cout << "Strip Offset : " << this->stripOffset << endl;
	cout << "Strip Byte Count : " << this->stripByteCount << endl;
	cout << "CR2 Slice : [" << this->cr2_slice[0] << ", " << this->cr2_slice[1] << ", " << this->cr2_slice[2] << "]" << endl;
	cout << "sRaw Type : " << this->sRawType << endl;

	cout << "Sensor data (Width/Height) : " << this->sensor_width << " / " << this-> sensor_height << endl;
	cout << "Sensor crop (t/b/l/r) : " << this->sensor_top_border << " / " << this->sensor_bottom_border << " / " << this->sensor_left_border << " / " << this->sensor_right_border << endl;
}


void RAW_CR2::fill_headers_and_diff_values() {
    //Open .CR2 file
	FILE* fp = nullptr;
    fopen_s(&fp, this->fp_path.c_str(), "rb");

    fread(&this->tiff_header, sizeof(this->tiff_header), 1, fp);
    fread(&this->cr2_header, sizeof(this->cr2_header), 1, fp);

    /*
        IFD 0
    */

    uint16_t num_IFD_entries;
    fread(&num_IFD_entries, sizeof(num_IFD_entries), 1, fp);

    //Find EXIF offset
    TIFF_TAG temp_tiff_tag; //This tiff tag is only used to find the exif offset
    for (int i = 0; i < num_IFD_entries; i++)
    {
        fread(&temp_tiff_tag, sizeof(temp_tiff_tag), 1, fp);
        if (temp_tiff_tag.ID == EXIF)
        {
            this->exif_subdir_offset = temp_tiff_tag.value;
            break;
        }
        
    }

    //Goto EXIF offset, find MAKERNOTE offset
    fseek(fp, this->exif_subdir_offset, SEEK_SET);
    fread(&num_IFD_entries, sizeof(num_IFD_entries), 1, fp);
    for (int i = 0; i < num_IFD_entries; i++)
    {
        fread(&temp_tiff_tag, sizeof(temp_tiff_tag), 1, fp);
        if (temp_tiff_tag.ID == MAKERNOTE)
        {
            this->makernote_offset = temp_tiff_tag.value;
            break;
        }
        
    }

    //Goto MAKERNOTE, find camera sensor data
    fseek(fp, this->makernote_offset, SEEK_SET);
    fread(&num_IFD_entries, sizeof(num_IFD_entries), 1, fp);
    for (int i = 0; i < num_IFD_entries; i++)
    {
        fread(&temp_tiff_tag, sizeof(temp_tiff_tag), 1, fp);
        //cout << "ID : " << temp_tiff_tag.ID << endl;

        if (temp_tiff_tag.ID == COLOR_BALANCE) {
            long current_pos = ftell(fp);
            fseek(fp, temp_tiff_tag.value, SEEK_SET);
            uint16_t numEntries;
            fread(&numEntries, sizeof(numEntries), 1, fp);
            fseek(fp, temp_tiff_tag.value + 126, SEEK_SET); //126 is the default skip value, see dcraw extract from part 5.1 of lclevy's post
            
            uint16_t entryVal;
            for (int j = 0; j < 4; j++)
            {
                 fread(&entryVal, sizeof(entryVal), 1, fp);
                 this->color_balances[j] = entryVal;
            }
            fseek(fp, current_pos, SEEK_SET);
        }

        else if (temp_tiff_tag.ID == SENSOR_INFO)
        {
            long current_pos = ftell(fp);
            fseek(fp, temp_tiff_tag.value, SEEK_SET);
            uint16_t numEntries;
            fread(&numEntries, sizeof(numEntries), 1, fp);
            for (int j = 0; j < 8; j++)
            {
                uint16_t entryVal;
                fread(&entryVal, sizeof(entryVal), 1, fp);

                switch (j) {
                    case 0:this->sensor_width = entryVal; break;
                    case 1:this->sensor_height = entryVal; break;
                    case 4:this->sensor_left_border = entryVal; break;
                    case 5:this->sensor_top_border = entryVal; break;
                    case 6:this->sensor_right_border = entryVal; break;
                    case 7:this->sensor_bottom_border = entryVal; break;
                
                    default:break;
                }
            }
            fseek(fp, current_pos, SEEK_SET);
        }
    }

    /*
        IFD 3
    */

    fseek(fp, this->cr2_header.raw_ifd_offset, SEEK_SET);
    //IFD3
    fread(&num_IFD_entries, sizeof(num_IFD_entries), 1, fp);

    for (int i = 0; i < num_IFD_entries; i++)
    {
        fread(&temp_tiff_tag, sizeof(temp_tiff_tag), 1, fp);
        switch (temp_tiff_tag.ID) {
        case IMAGE_WIDTH: this->width = temp_tiff_tag.value; break;
        case IMAGE_LENGTH: this->height = temp_tiff_tag.value; break;
        case STRIP_OFFSET: this->stripOffset = temp_tiff_tag.value; break;
        case STRIP_BYTE_COUNTS: this->stripByteCount = temp_tiff_tag.value; break;
        case CR2_SLICE: getShortPointerDataTag(this->cr2_slice, temp_tiff_tag, fp); break;
        case SRAW_TYPE: this->sRawType = temp_tiff_tag.value; break;

        default: break;
        }
    }

    /*
        RAW Headers
    */
    // Goto "Strip Offset" --> RAW Header (Line 375 on ghub)
    // https://github.com/lclevy/libcraw2/blob/master/docs/cr2_lossless.pdf?raw=true
    fseek(fp, this->stripOffset, SEEK_SET);

    //SOI Marker : marks the beginning of raw info. Should be 0xffd8
    uint16_t soi_marker;
    fread(&soi_marker, sizeof(soi_marker), 1, fp);

    //DHT Header (Huffman table)
    fread(&this->dht_header, sizeof(this->dht_header), 1, fp);

    this->dht_header.swap();  //Swap bytes (big endian) of marker & length

    for (int i = 0; i < 16; i++)
    {
        this->huffData[i] = this->dht_header.huff_data_0[i];
        this->huffValues[i] = this->dht_header.huff_vals_0[i];
    }

    //SOF3 Header
    int currentOffset = ftell(fp);
    
    fread(&this->sof3_header, sizeof(this->sof3_header), 1, fp);

    this->sof3_header.swap();

    int num_lines = this->sof3_header.num_lines;
    int samplesPerLine = this->sof3_header.samp_per_lin;

    //SOS
    this->raw_sos_offset = ftell(fp);
    fread(&this->sos_header, sizeof(this->sos_header), 1, fp);
    this->sos_header.swap();

    this->raw_scan_offset = ftell(fp);
    this->raw_scan_size = this->stripByteCount - (this->raw_scan_offset - this->stripOffset);

    /*
        Computing Huffman Table
    */

    int huff_size = 0;
    for (int i = 0; i < 16; i++) {
        huff_size += this->huffData[i];
    }
    this->real_huffman_size = huff_size;

    this->numDiffValues = this->sensor_width * this->sensor_height;

    /*
        Decode RAW Values
    */
    Huffman_Decoder hdec = Huffman_Decoder(this->huffData, this->huffValues);
    
    uint16_t codes[16];
    huffCodes(this->huffData, codes);

    uint16_t minLen = 0;
    uint16_t maxLen = 0;
    uint16_t numCodes = 0;
    for (int i = 0; i < 16; i++)
    {
        numCodes += this->huffData[i];

        if (minLen == 0 && this->huffData[i] != 0)
            minLen = i + 1;

        if (maxLen < i + 1 && this->huffData[i] != 0)
            maxLen = i + 1;
    }

    std::vector<uint16_t> lenTab(numCodes);
    uint16_t ccounter = 0;
    for (int i = 0; i < 16; i++)
    {
        uint16_t HD = this->huffData[i];   //HD = HuffData = Number of codes of this length
        for (int n = 0; n < HD; n++)
        {
            lenTab[ccounter] = i + 1;
            ccounter++;
        }
    }

    unordered_map<uint16_t, int> hmap;
    for (int i = 0; i < numCodes; i++) {
        uint16_t codelen = lenTab[i];
        uint16_t codecode = codes[i];

        uint16_t pow2 = (1 << (maxLen - codelen));
        uint16_t moved_code = codecode << (maxLen - codelen);
        for (uint16_t j = 0; j < pow2; j++) {
            hmap.insert(pair<uint16_t, int>(moved_code ^ j, i));
        }
    }

    ByteStream bstream;
    fseek(fp, this->raw_scan_offset, SEEK_SET);
    bstream.loadBytes(fp, this->raw_scan_size);   //Scan 22 743 889 bytes (70D CR2)
    fseek(fp, this->raw_scan_offset, SEEK_SET);

    uint16_t remainder = 0;
    uint16_t remLen = 0;
    uint16_t CodeBitBuffer;
    int last_values[3] = { 8192 ,8192 ,8192 };   //R G B
    int odd_char, odd_line, resultValue = 0;
    vector<int> resultValues(numDiffValues);

    for (int i = 0; i < numDiffValues; i++)
    {
        uint16_t code, codeLen, counter, codeValue, tempBits;
        tempBits = bstream.readBits(maxLen - remLen);
        CodeBitBuffer = (remainder << (maxLen - remLen)) + tempBits;

        int niceindex = hmap.at(CodeBitBuffer);

        codeLen = lenTab[niceindex];
        codeValue = this->huffValues[niceindex];
        code = codes[niceindex];



        uint16_t numNewBits, newBits, diffCode;
        int temp, diffValue;

        //Consume "diff len" len
        remainder = CodeBitBuffer - (code << (maxLen - codeLen));

        //Check if there are new bits to read
        temp = codeValue - (maxLen - codeLen); // WOOOPS be aware of overflow, we correct in line below!
        numNewBits = temp > 0 ? temp : 0;   // Implying code is still in buffer

        if (numNewBits)
        {
            newBits = bstream.readBits(numNewBits);
            diffCode = (remainder << numNewBits) + newBits;
            remainder = 0;
            remLen = 0;

            if (i == numDiffValues - 1)
            {
                printf("newBits = "); printBits(newBits); printf("\n\n");
            }
        }
        else
        {
            diffCode = (remainder >> abs(temp));
            remainder = remainder - (diffCode << abs(temp));
            remLen = maxLen - codeLen - codeValue;
        }

        diffValue = getDiffValue(diffCode, codeValue);
        resultValues[i] = diffValue;
    }

    vec_diffvalues = resultValues;

    cout << "Done !" << endl << endl;

    //Closing stuff
    fclose(fp);
}