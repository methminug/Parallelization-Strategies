#include <CL/sycl.hpp>
#include "../lodepng.h"
#include <bits/stdc++.h>
#define IMGSIZE 512 * 512 * 4
using namespace sycl;

int main(){
  //# define queue which has default device associated for offload
  gpu_selector selector;
  queue q(selector);
  std::cout << "Device: " << q.get_device().get_info<info::device::name>() << "\n";
    
    const char* filename = "../peppers.png";
    unsigned char* test_img = malloc_shared<unsigned char>(IMGSIZE, q);
    unsigned char* rgb_img = malloc_shared<unsigned char>(IMGSIZE, q);
    unsigned width, height;
    
    unsigned error = lodepng_decode32_file(&test_img, &width, &height, filename);
    if(error)
      printf("Decoder error %u: %s\n", error, lodepng_error_text(error));
    
    unsigned char* output_img = malloc_shared<unsigned char>(IMGSIZE, q);

    for(int y=0; y< (int) height; y++) {
        for(int x=0; x< (int) width; x++){
            int index = x*width*4 + y*4;

            //Asigning value to each color channel
            for(int c=0; c<3;++c){
                rgb_img[x*width*3 + y*3 +c] = test_img[index + c];
            }

        }
    }

    q.parallel_for(IMGSIZE, [=] (auto i){
        int index = i%4;
        double grayScalePixel = 0.0;
        switch (index) {
            case 0:
                grayScalePixel = 0.21 * rgb_img[i] + 0.72 * rgb_img[i+1] + 0.07 * rgb_img[i+2];
                break;
            case 1:
                grayScalePixel = 0.21 * rgb_img[i-1] + 0.72 * rgb_img[i] + 0.07 * rgb_img[i+1];
                break;
            case 2:
                grayScalePixel = 0.21 * rgb_img[i-2] + 0.72 * rgb_img[i-1] + 0.07 * rgb_img[i];
                break;
            case 3:
                grayScalePixel = 255.0;
                break;
            default:
                break;
        }
      
       output_img[i] = grayScalePixel;
      
    }).wait();

    
     error = lodepng_encode32_file("lab/oneapi-output.png", output_img, width, height);
     if(error)
       printf("Encoder error %u: %s\n", error, lodepng_error_text(error));
    
    free(output_img, q);
    free(test_img, q);
    free(rgb_img, q);
    return 0;
}
