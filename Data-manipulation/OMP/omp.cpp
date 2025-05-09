#include "../lodepng.h"
#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

unsigned char rgb_img[512*512*3];

unsigned char grayscale_image(int i) {

    vector<double> pixels;
    
    //Creating gray value from RGB values
    double grayColor = 0.21 * rgb_img[i+0] + 0.72 * rgb_img[i+1] + 0.07 * rgb_img[i+2];
    
    pixels.push_back(grayColor);
    
    sort(pixels.begin(), pixels.end());
    return pixels[pixels.size()/2];

}

int main() {

    const char* filename = "../peppers.png";
    double tstart, tstop;

    std::vector<unsigned char> test_img; 
    unsigned width, height;
    unsigned error = lodepng::decode(test_img, width, height, filename);
    if(error)
      printf("Decoder error %u: %s\n", error, lodepng_error_text(error));

    unsigned char* output_img = (unsigned char*) malloc((int)test_img.size());

    // Separate RGB channels
    #pragma omp parallel for collapse(2)
    for(int y=0; y< (int) height; y++) {
        for(int x=0; x< (int) width; x++){
            int index = x*width*4 + y*4;

            //Asigning value to each color channel
            for(int c=0; c<3;++c){
                rgb_img[x*width*3 + y*3 +c] = test_img[index + c];
            }

        }
    }
    
    tstart = omp_get_wtime();
    
    #pragma omp parallel for collapse(2)
    for(int r=0; r<(int)height; r++){
      for(int c=0; c<(int)width; c++) {
          int index = (r)*width*4 + (c)*4;
          
          double grayScalePixel = grayscale_image(r*width*3 + c*3);
          
          output_img[index + 0] = grayScalePixel;
          output_img[index + 1] = grayScalePixel;
          output_img[index + 2] = grayScalePixel;
          
          //Alpha value
          output_img[index + 3] = 255;
      }
    }

    
    tstop = omp_get_wtime();
    printf("Time elapsed: %f\n",tstop-tstart );

    error = lodepng_encode32_file("omp-output.png", output_img, width, height);
    if(error)
      printf("Encoder error %u: %s\n", error, lodepng_error_text(error));

    free(output_img);
    return 0;

}