#pragma once
#include <iostream>
#include <vector>
#include "Huffman_Decoder.h"
#ifndef CR2_UTILS
#include "CR2_utils.h"	//Also include CR2_Types
#endif // !CR2_UTILS



using namespace std;

//var1
//var2
//void function(args) <-- will fill vars above it.

class RAW_CR2
{
public:
	string fp_path;
	RAW_CR2(string path);

	//IFD0
	TIFF_HEADER tiff_header;
	CR2_HEADER cr2_header;
	uint16_t exif_subdir_offset, makernote_offset;
	uint16_t sensor_width, sensor_height, sensor_left_border, sensor_top_border, sensor_right_border, sensor_bottom_border, 
					black_mask_top_border, black_mask_bottom_border, black_mask_left_border, black_mask_right_border = 0;
	uint16_t left_black_border, top_black_border=0;
	uint16_t color_balances[4];

	//IFD3
	int width, height = 0;
	unsigned long stripOffset, stripByteCount = 0;
	uint16_t cr2_slice[3] = { 0,0,0 };
	unsigned long sRawType = 0;

	//Huffman, Diff and RAW
	DHT_HEADER dht_header;
	int real_huffman_size = 0;
	uint8_t huffData[16];
	int huffValues[16];
	SOF3_HEADER sof3_header;
	SOS_HEADER sos_header;
	unsigned long raw_sos_offset, raw_scan_offset, raw_scan_size = 0;
	int numDiffValues = 0;
	vector<uint16_t> diff_values;
	void fill_headers_and_diff_values();

	uint16_t black_level = 0;
	uint16_t consume_upper_black_border(int number_of_lines_to_consume, int start = 0);

	vector<float> r, g1, g2, b;	//Allocated in fill_headers_and_diff_values, filled in decode_slice()
	void decode_slice(int start, int end, int top_black_border, int left_black_border);
	//void decode_bayer_line(int line_num, int left_black_border);
	vector<uint16_t> bayer_values;	//Allocated in fill_headers_and_diff_values, filled in decode_slice()
	void decode_diff_image_line(int line_num);

	void raw_values2rgb();

	vector<unsigned char> get_sdr_array();
	unsigned char correct_value(uint16_t in, uint16_t col_bal);
	vector<unsigned char> correct_rgb_and_get_sdr_array();


	

	//vector<int> vec_diffvalues;
	//int diffValueList[53];
	//vector<uint16_t> diff_values;
	

	void pprint();
};