#include <mpi.h> 
#include <bits/stdc++.h>
#include "../lodepng.h"
#include <time.h>
 
using namespace std;

unsigned char rgb_img[550][550][3];
unsigned char sub_img[512*512*4];
unsigned char output_img[512*512*4];

unsigned char grayscale_image(int y, int x) {

    vector<double> pixels;
    
    //Creating gray value from RGB values
    double grayColor = 0.21 * rgb_img[y][x][0] + 0.72 * rgb_img[y][x][1] + 0.07 * rgb_img[y][x][2];
    
    pixels.push_back(grayColor);
    
    sort(pixels.begin(), pixels.end());
    return pixels[pixels.size()/2];

}

int main()
{
    int size,rank;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Request request;
    
    const char* filename = "../peppers.png";
    unsigned error;
    unsigned char* test_img;
    unsigned width, height;
    int subWidth, sendCount;
    
    clock_t begin, end;

    if(rank==0) 
    {  
        error = lodepng_decode32_file(&test_img, &width, &height, filename);
        if
            (error) printf("Decoder error %u: %s\n", error, lodepng_error_text(error));
        sendCount = (int)(height*width*4)/size;
        subWidth = (int)width;
        
        for(int y=0; y<(int)height; y++) {
            for(int x=0; x<(int) width; x++){
                int index = x*width*4 + y*4;
                
                //Asigning value to each color channel
                for(int c=0; c<3;++c){
                    rgb_img[x][y][c] = test_img[index + c];
                }
                
            }
        }

    }
    
    begin = clock();

    MPI_Bcast(&subWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&sendCount, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    MPI_Bcast(rgb_img, 550*550*3, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(output_img, sendCount*size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    MPI_Scatter(test_img, sendCount, MPI_UNSIGNED_CHAR, sub_img, sendCount, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);  
    
    //Scynchronizing
    MPI_Ibarrier(MPI_COMM_WORLD, &request);

    int skipCount = sendCount * rank;
    for(int k=0; k<50; k++){
        for(int i=0; i<sendCount; i++){
            if
                (i%4==3) sub_img[i] = 255;
            else 
                sub_img[i] = grayscale_image((i+skipCount)/(subWidth*4)+3, ((i+skipCount)/4+3)%subWidth);
        }
    }
    
    MPI_Gather(sub_img, sendCount, MPI_UNSIGNED_CHAR, output_img, sendCount, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    
    end = clock();

    if(rank==0){
        error = lodepng_encode32_file("mpi-output.png", output_img, width, height);
        if
            (error) printf("Encoder error %u: %s\n", error, lodepng_error_text(error));
    }
    
    float cpu_time = (float)(end - begin) / CLOCKS_PER_SEC;

    printf("Node %d Time elapsed : %f\n", rank, cpu_time);

    MPI_Finalize();
    return 0;
}