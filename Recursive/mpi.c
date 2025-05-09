#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int matchWord(char searchWord[], char string[]){
    int matches = 0;
    if (string != NULL) {
        if(strcmp(searchWord, string) == 0){
            ++matches;
        }
        string = strtok(NULL, " ");
        matches += matchWord(searchWord, string);
    }
    return matches;
}

int main (void)
{
    int rank, size;

    MPI_Init(NULL, NULL);
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    clock_t begin, end;

    char searchWord[] = "apple"; 
    char text[] = "I have an apple tree I have an apple tree I have an apple tree I have an apple tree I have an apple tree";
    //char text[] = "I have an apple tree";
    int txtlen = strlen(text);
    int totMatches;
    int sendCount = txtlen/size;
    char recvString[100]={};

    if(rank == 0){
        int start = 0;
        for(int p = 0; p<size; ++p){
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

                MPI_Bsend(&subText, 100, MPI_CHAR, p, 0, MPI_COMM_WORLD);
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

                        MPI_Bsend(&subText, 100, MPI_CHAR, p, 0, MPI_COMM_WORLD);

                        break;
                    }
                }
            }
        }
    }

    

    MPI_Recv(&recvString, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

    //printf("Rank %d -> %s\n", rank, recvString);

    begin = clock();

    char* token = strtok(recvString, " ");

    int matches = matchWord(searchWord, token);

    end = clock();

    //printf("%d matches for >%s< found by rank %d\n", result, searchWord, rank);

    MPI_Reduce(&matches, &totMatches, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    

    if(rank == 0){
        printf("Number of times '%s' was found : %d\n", searchWord, totMatches);
    }

    float cpu_time = (float)(end - begin) / CLOCKS_PER_SEC;

    printf("Node %d Time elapsed : %f\n", rank, cpu_time);

    MPI_Finalize();

    return 0;
}