#pragma once
#include "TooJPEG.h"
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>
#include <numeric>  //For accumulate
#include <map> 
#include <cmath>
#include <unordered_map>
#include <algorithm>

#include "RAW_CR2.h"


using namespace std;

vector<uint16_t> decodeSlice(vector<int> slice_diff,RAW_CR2* raw) {
    int pow2 = 8192;
    uint16_t last_values[2] = { pow2,pow2 };
    vector<uint16_t> result;
    vector<uint16_t> Black_levels;
    vector<int> first_cropped, second_cropped;

    //Initialization
    for (int i = 0; i < 2; i++) {
        uint16_t newval = last_values[i % 2] + slice_diff[i];
        result.push_back(newval);
        last_values[i % 2] = newval;
    }
    //Measure Black value
    for (int i = 2; i < raw->sensor_width*18; i++) {
        uint16_t newval = last_values[i % 2] + slice_diff[i];
        result.push_back(newval);
        Black_levels.push_back(newval);
        last_values[i % 2] = newval;
    }

    long long Black_level_long = accumulate(Black_levels.begin(), Black_levels.end(), 0.0) / Black_levels.size();
    uint16_t Black_level = (uint16_t)Black_level_long;
    cout << "BLLONG = " << Black_level_long << " BLUINT = " << Black_level;

    uint16_t newval, clamped_nv;
    //Measure Black value
    for (int i = raw->sensor_width * 18; i < slice_diff.size(); i++) {
        if (i % raw->sensor_width < 72) {
            newval = Black_level;
        }
        else {
            //Actually append the data
            newval = last_values[i % 2] + slice_diff[i];
            clamped_nv = newval - Black_level < 0 ? 0 : newval - Black_level;
        }
        int rightside = (i / (raw->sensor_width / 2));
        rightside = rightside % 2;
        int odd = i % 2;

        switch (rightside * 2 + odd) {
        case 0:raw->r.push_back(float(clamped_nv) * float(raw->color_balances[0])/1024.); break;
            case 1:break;//raw->g.push_back(newval); break;
            case 2:raw->g.push_back(clamped_nv *float(raw->color_balances[2])/1024.); break;//cout << raw->color_balances[2] << " "<< float(raw->color_balances[2]) / 1024. << endl; break;
            case 3:raw->b.push_back(clamped_nv *float(raw->color_balances[3])/1024.); break;
            default: cout << "oh shit : " << rightside * 2 + odd << endl;
        }

        result.push_back(newval);
        last_values[i % 2] = newval;
    }

    return result;
}

// output file
std::ofstream myFile("cr2boi.jpg", std::ios_base::out | std::ios_base::binary);

// write a single byte compressed by tooJpeg
void myOutput(unsigned char byte)
{
    myFile << byte;
}

int main()
{
    string myPhotoPath = "V:\\Temp\\Photos Neige\\J0_J1\\_MG_9532.CR2";

    cout << "Headers start" << endl;
    RAW_CR2* raw = new RAW_CR2(myPhotoPath);
    raw->fill_headers_and_diff_values();
    cout << "Headers end" << endl;
    
    vector<int> vec_diffvalues = raw->vec_diffvalues;
    raw->pprint();

    //vector<int>::const_iterator first = vec_diffvalues.begin() + vec_diffvalues.size() / 2;
    //vector<int>::const_iterator last = vec_diffvalues.end();
    vector<int>::const_iterator first = vec_diffvalues.begin();
    vector<int>::const_iterator last = vec_diffvalues.begin() + vec_diffvalues.size() / 2;
    vector<int> newVec(first, last);

    vector<uint16_t> s1_dec = decodeSlice(newVec,raw);

    /*cout << "First few pixels values : " << endl;
    for (int i = 0; i < 20; i++) {
        cout << vec_diffvalues[i] << endl;
    }*/

    const auto width = raw->sensor_width/4;
    const auto height = raw->sensor_height/2;
    // RGB: one byte each for red, green, blue
    const auto bytesPerPixel = 3;

    int min_pixels, max_pixels = 0;
    min_pixels = *min_element(s1_dec.begin(), s1_dec.end());
    max_pixels = *max_element(s1_dec.begin(), s1_dec.end());
    //int maxAbs = abs(max_pixels) < abs(min_pixels) ? abs(min_pixels) : abs(max_pixels);

    //(unsigned char) ((diffValueList[i] - minInt)*(255/float(maxInt - minInt)));
    //return (unsigned char)((dVal + maxAbs) * (128 / float(maxAbs)));

    // allocate memory
    auto image = new unsigned char[width * height * bytesPerPixel];
    // create a nice color transition (replace with your code)
    for (auto y = 0; y < height; y++)
        for (auto x = 0; x < width; x++)
        {
            // memory location of current pixel
            auto offset = (y * width + x);

            //int pixelvalue = (abs(newVec[offset])) * 1;
            //int pixelvalue = (s1_dec[offset] + abs(min_pixels))*255/( abs(min_pixels) + abs(max_pixels) );

            // red and green fade from 0 to 255, blue is always 127
            image[offset* bytesPerPixel]     = ( raw->r[offset] + abs(min_pixels)) * 255 / (abs(min_pixels) + abs(max_pixels));        //Red
            image[offset* bytesPerPixel + 1] = ( raw->g[offset] + abs(min_pixels)) * 255 / (abs(min_pixels) + abs(max_pixels));   //Green
            image[offset* bytesPerPixel + 2] = ( raw->b[offset] + abs(min_pixels)) * 255 / (abs(min_pixels) + abs(max_pixels));                //Blue
            
            //unsigned char normalized_pixel = (unsigned char)((pixels[divoffset]+ abs(min_pixels)) * (255 / float(abs(min_pixels) + abs(max_pixels))));
            //image[offset] = pixelvalue > 255 ? 255 : pixelvalue;
            
        }
    
    const bool isRGB = true;  // true = RGB image, else false = grayscale
    const auto quality = 95;    // compression quality: 0 = worst, 100 = best, 80 to 90 are most often used
    const bool downsample = false; // false = save as YCbCr444 JPEG (better quality), true = YCbCr420 (smaller file)
    const char* comment = "TooJpeg example image"; // arbitrary JPEG comment
    auto ok = TooJpeg::writeJpeg(myOutput, image, width, height, isRGB, quality, downsample, comment);
    delete[] image;

    cout << "done!" << endl;
}



/*
// 800x600 image
    const auto width = 800;
    const auto height = 600;
    // RGB: one byte each for red, green, blue
    const auto bytesPerPixel = 3;
    // allocate memory
    auto image = new unsigned char[width * height * bytesPerPixel];
    // create a nice color transition (replace with your code)
    for (auto y = 0; y < height; y++)
        for (auto x = 0; x < width; x++)
        {
            // memory location of current pixel
            auto offset = (y * width + x) * bytesPerPixel;
            // red and green fade from 0 to 255, blue is always 127
            image[offset] = 255 * x / width;        //Red
            image[offset + 1] = 255 * y / height;   //Green
            image[offset + 2] = 127;                //Blue
        }
    // start JPEG compression
    // note: myOutput is the function defined in line 18, it saves the output in example.jpg
    // optional parameters:
    const bool isRGB = true;  // true = RGB image, else false = grayscale
    const auto quality = 90;    // compression quality: 0 = worst, 100 = best, 80 to 90 are most often used
    const bool downsample = false; // false = save as YCbCr444 JPEG (better quality), true = YCbCr420 (smaller file)
    const char* comment = "TooJpeg example image"; // arbitrary JPEG comment
    auto ok = TooJpeg::writeJpeg(myOutput, image, width, height, isRGB, quality, downsample, comment);
    delete[] image;
    // error => exit code 1
    return ok ? 0 : 1;
*/
