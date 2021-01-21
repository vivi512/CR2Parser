#pragma once
#ifndef CR2_TYPES
#include "CR2_Types.h"
#endif // !CR2_TYPES

#ifndef CR2_UTILS
#include "CR2_utils.h"	//Also include CR2_Types
#endif // !CR2_UTILS

void pprint_sos(SOS_HEADER sos_header);
void pprint_sof3(SOF3_HEADER sof3_header);
void print_tiff_header(TIFF_HEADER th);
void print_cr2_header(CR2_HEADER ch);

void printTagInfo(TIFF_TAG tag);
void printTagType(TIFF_TAG tag);
void printPointerDataTag(TIFF_TAG tag, FILE* f);

void printTiffTag(TIFF_TAG tiff_tag, FILE* fp);

void printPointerDataTag(TIFF_TAG tag, FILE* f);

void pprint_DHT(DHT_HEADER dh);