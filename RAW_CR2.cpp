#pragma once
#include "RAW_CR2.h"
#include <numeric>
#include <algorithm>
#include <thread>


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
    cout << "Black Mask (t/l) : " << this->top_black_border << " / " << this->left_black_border << endl;

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

                    case 8:this->black_mask_left_border = entryVal; break;
                    case 9:this->black_mask_top_border = entryVal; break;
                    case 10:this->black_mask_right_border = entryVal; break;
                    case 11:this->black_mask_bottom_border = entryVal; break;
                
                    default:break;
                }
            }
            fseek(fp, current_pos, SEEK_SET);
        }
    }

    this->left_black_border = (sensor_left_border - (sensor_width - sensor_right_border));     // = 71 on a 70D
    this->top_black_border = (sensor_top_border - (sensor_height - sensor_bottom_border)) / 2; // = 19 on a 70D

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
    //vector<int> resultValues(numDiffValues);
    this->diff_values = vector<uint16_t>(numDiffValues);    //Allocate diff_values to the right size (numDiffValues)

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
        this->diff_values[i] = diffValue;
    }

    //Allocate r,g,b
    this->r = vector<float>(numDiffValues/4);
    this->g1 = vector<float>(numDiffValues/4);
    this->g2 = vector<float>(numDiffValues/4);
    this->b = vector<float>(numDiffValues/4);

    bayer_values = vector<uint16_t>(numDiffValues);

    //Closing file
    fclose(fp);
}

/*
    TODO : make it better
*/
uint16_t RAW_CR2::consume_upper_black_border(int number_of_lines_to_consume, int start) {
    int pow2 = 8192;
    uint16_t last_values[2] = { pow2,pow2 };
    vector<uint16_t> result;
    vector<uint16_t> Black_levels;
    vector<int> first_cropped, second_cropped;

    //Initialization : Decode first two values
    for (int i = 0+start; i < 2+start; i++) {
        uint16_t newval = last_values[i % 2] + this->diff_values[i];
        result.push_back(newval);
        last_values[i % 2] = newval;
    }
    //Measure Black value
    // * 18
    for (int i = 2+start; i < start + this->sensor_width * number_of_lines_to_consume; i++) {
        uint16_t newval = last_values[i % 2] + this->diff_values[i];
        result.push_back(newval);
        Black_levels.push_back(newval);
        last_values[i % 2] = newval;
    }

    long long Black_level_long = std::accumulate(Black_levels.begin(), Black_levels.end(), 0.0) / Black_levels.size();
    this->black_level= (uint16_t)Black_level_long;
    return (uint16_t)Black_level_long;
}


void RAW_CR2::decode_diff_image_line(int line_num, int col_start=0) {
    int start_idx = line_num * sensor_width;
    uint16_t last_values[2] = { 0,0 };

    if (col_start == 0) {
        if (line_num == 0) {
            last_values[0] = 8192;
            last_values[1] = 8192;
        }
        else {
            last_values[0] = bayer_values[(line_num - 1) * sensor_width];
            last_values[1] = bayer_values[(line_num - 1) * sensor_width + 1];
        }
    }
    else {
        last_values[0] = bayer_values[(line_num) * sensor_width];
        last_values[1] = bayer_values[(line_num) * sensor_width + 1];
    }
    uint16_t newval=0;
    int diff_list_idx=0;

    for (int i = col_start; i < sensor_width; i++) {
        int odd = i % 2;                                    //Odd index ?

        int diff_list_idx = i + start_idx;

        newval = last_values[odd] + this->diff_values[diff_list_idx];

        bayer_values[line_num * this->sensor_width + i] = newval;

        last_values[odd] = newval;

    }
}


void RAW_CR2::raw_values2rgb() {
    //Upper part
    for (int line = 0; line < this->sensor_height / 2; line++) 
    {
        for (int col = 0; col < this->sensor_width / 4; col++) 
        {
            this->r[line * this->sensor_width / 2 + col] = (correct_value(this->bayer_values[line*this->sensor_width + 2*col],this->color_balances[0])); 
            this->g1[line * this->sensor_width / 2 + col] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col+1], this->color_balances[1]));
            this->g2[line * this->sensor_width / 2 + col] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col + this->sensor_width/2], this->color_balances[2]));
            this->b[line * this->sensor_width / 2 + col] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col + this->sensor_width / 2 + 1], this->color_balances[3]));

        }
    }
    
    //Lower part
    for (int line = this->sensor_height / 2; line < this->sensor_height-1; line++)
    {
        for (int col = 0; col < this->sensor_width / 4; col++)
        {
            this->r[(line-sensor_height/2) * this->sensor_width / 2 + col + this->sensor_width/4] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col], this->color_balances[0]));
            this->g1[(line - sensor_height / 2) * this->sensor_width / 2 + col + this->sensor_width / 4] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col + 1], this->color_balances[1]));
            this->g2[(line - sensor_height / 2) * this->sensor_width / 2 + col + this->sensor_width / 4] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col + this->sensor_width / 2], this->color_balances[2]));
            this->b[(line - sensor_height / 2) * this->sensor_width / 2 + col + this->sensor_width / 4] = (correct_value(this->bayer_values[line * this->sensor_width + 2 * col + this->sensor_width / 2 + 1], this->color_balances[3]));
        }
    }
}

void RAW_CR2::decode_slice(int start, int end, int top_black_border, int left_black_border) {
    for (int i = 0; i < this->sensor_height; i++) {
        decode_diff_image_line(i);
    }
    //raw_values2rgb();
}

// 2 Components, first values = 8192,8192
void RAW_CR2::preprocess_linestarts() {
    int start_idx = 0;
    uint16_t last_values[2] = { 8192,8192 };


    uint16_t newval = 0;
    int diff_list_idx = 0;
    for (int line = 0; line < sensor_height; line++) {
        start_idx = line * sensor_width;
        for (int col = 0; col < 2; col++) { // < 2 because there are 2 components
            int odd = col % 2;

            int diff_list_idx = col + start_idx;

            newval = last_values[odd] + this->diff_values[diff_list_idx];

            bayer_values[line * this->sensor_width + col] = newval;

            last_values[odd] = newval;
        }
    }
}

void RAW_CR2::mt_decode_lines(int start, int end) {
    for (int i = start; i < end; i++) {
        this->decode_diff_image_line(i, 2);
    }
}

void RAW_CR2::mt_decode_diff_image_lines() {
    /*preprocess_linestarts();
    const int processor_count = std::thread::hardware_concurrency();
    vector<thread> threads;
    for (int i = 0; i < processor_count; i++) {
        //std::thread t(mt_decode_lines, i*sensor_height/processor_count, (i+1) * sensor_height / processor_count);
        std::thread t(&RAW_CR2::mt_decode_lines, this, i * sensor_height / processor_count, (i + 1) * sensor_height / processor_count);
        threads.push_back(t);
    }
    for (int i = 0; i < threads.size(); i++) {
        threads[i].join();
    }*/
}



unsigned char RAW_CR2::correct_value(uint16_t in, uint16_t col_bal) {
    uint16_t black_clamped = (in - this->black_level) < 0 ? 0 : in - this->black_level;
    return unsigned char((float(black_clamped) * float(col_bal) / 1024.)/30.);
}

vector<unsigned char> RAW_CR2::correct_rgb_and_get_sdr_array() {
    /*vector<int> maxes;
    maxes.push_back(*max_element(r.begin(), r.end()));
    maxes.push_back(*max_element(g1.begin(), g1.end()));
    maxes.push_back(*max_element(g2.begin(), g2.end()));
    maxes.push_back(*max_element(b.begin(), b.end()));
    int max = *max_element(maxes.begin(), maxes.end());*/

    const auto width = this->sensor_width / 2;
    const auto height = this->sensor_height / 2;
    const auto bytesPerPixel = 3;

    vector<unsigned char> image = vector<unsigned char>(width * height * bytesPerPixel);
    // create a nice color transition (replace with your code)
    for (auto y = 0; y < height; y++)
        for (auto x = 0; x < width; x++)
        {
            // memory location of current pixel
            auto offset = (y * width + x);
            image[offset * bytesPerPixel] = r[offset];        // float(max) * 255*40;
            image[offset * bytesPerPixel + 1] = g1[offset]; // float(max) * 255*40;
            image[offset * bytesPerPixel + 2] = b[offset];    // float(max) * 255*40;
        }
    return image;
}

vector<unsigned char> RAW_CR2::get_sdr_array() {
    vector<int> maxes;
    maxes.push_back(*max_element(r.begin(), r.end()));
    maxes.push_back(*max_element(g1.begin(), g1.end()));
    maxes.push_back(*max_element(g2.begin(), g2.end()));
    maxes.push_back(*max_element(b.begin(), b.end()));
    int max = *max_element(maxes.begin(), maxes.end());

    const auto width = this->sensor_width / 2;
    const auto height = this->sensor_height / 2;
    const auto bytesPerPixel = 3;

    vector<unsigned char> image = vector<unsigned char>(width * height * bytesPerPixel);
    // create a nice color transition (replace with your code)
    for (auto y = 0; y < height; y++)
        for (auto x = 0; x < width; x++)
        {
            // memory location of current pixel
            auto offset = (y * width + x);
            image[offset * bytesPerPixel] = float(this->r[offset] < 0 ? 0 : this->r[offset]) / 30.;        // float(max) * 255*40;
            image[offset * bytesPerPixel + 1] = float(this->g1[offset] < 0 ? 0 : this->g1[offset]) / 30.; // float(max) * 255*40;
            image[offset * bytesPerPixel + 2] = float(this->b[offset] < 0 ? 0 : this->b[offset]) / 30.;    // float(max) * 255*40;
        }
    return image;
}