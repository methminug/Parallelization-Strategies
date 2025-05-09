#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

int matchWord(char searchWord[], char string[]){
    int matches = 0;
    if (string != NULL) {
        if(strcmp(searchWord, string) == 0){
            ++matches;
        }
        string = strtok(NULL, " ");
        #pragma omp task
        matches += matchWord(searchWord, string);
    }
    
    #pragma omp taskwait
    return matches;
}

int main (void)
{
    double tstart, tcalc, tstop;
    
    char searchWord[] = "apple"; 
    //char text[] = "I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree";
    char text[] = "I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree";
    int txtlen = strlen(text);
    int totMatches = 0;
    int sendCount = txtlen/4;
        
    tstart = omp_get_wtime();
    
    int start = 0;
    #pragma omp parallel for collapse(2)
    for(int p = 0; p<4; ++p){
        int stop = ((p+1) * sendCount) - 1;
        if(text[stop] == ' ' || text[stop-1] == ' ' || text[stop+1] == ' ' || stop+1 >= txtlen){
            char subText[100] = {};
            int c = 0;
            while(c <= (stop-start)){
                subText[c] = text[start+c];
                ++c;
            }
            start = stop+1;
            // TESTING STRING SPLIT
            // printf("-> %s\n", subText);
            // printf("=====================================\n");

            char* token = strtok(text, " ");

            int matches = matchWord(searchWord, token);
            #pragma omp critical
            totMatches += matches;
        }
        else{
            for(int j = stop+1; j<txtlen-1; ++j){
                if(text[j] == ' ' || text[j-1] == ' ' || text[j+1] == ' ' || j+1 >= txtlen){
                char subText[100] = {};
                int c = 0;
                while(c <= (j-start)){
                    subText[c] = text[start+c];
                    ++c;
                }
                    start = j+1;
                    // TESTING STRING SPLIT
                    // printf("-> %s\n", subText);
                    // printf("=====================================\n");

                    char* token = strtok(text, " ");

                    int matches = matchWord(searchWord, token);
                    totMatches += matches;

                    break;
                }
            }
        }
    }
    
    printf("Number of times '%s' was found : %d\n", searchWord, totMatches);
    
    tstop  = omp_get_wtime();
    tcalc = tstop - tstart;
    printf("Time elapsed : %f\n", tcalc);

    return 0;
}