/* 
    name : Wanghe Zhang
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define FILENAME_LENGTH 100
#define BUFFER_LENGTH 50
#define REQUIRED_ARG 4
#define REQUIRED_ARG_S 's'
#define REQUIRED_ARG_E 'E'
#define REQUIRED_ARG_B 'b'
#define REQUIRED_ARG_T 't'
#define REQUIRED_ARG_S_INDEX 0
#define REQUIRED_ARG_E_INDEX 1
#define REQUIRED_ARG_B_INDEX 2
#define REQUIRED_ARG_T_INDEX 3
#define OPTIONAL_ARG_V 'v'
#define OPTIONAL_ARG_H 'h'
#define HIT " hit"
#define MISS " miss"
#define EVCITION " eviction"

void usage();
void parse_command(int argc, char **argv, bool *verbose, int *set_bits,
                   int *associativity, int *block_bits, char *filename);
void parse_file(char *filename, bool verbose, int set_bits, int associativity,
                int block_bits, int *hit_num, int *miss_num, int *eviction_num);

int main(int argc, char **argv)
{
    bool verbose = false;
    int set_bits = 0;
    int associativity = 0;
    int block_bits = 0;
    char filename[FILENAME_LENGTH];
    parse_command(argc, argv, &verbose, &set_bits, &associativity, &block_bits, filename);

    int hit_num = 0;
    int miss_num = 0;
    int eviction_num = 0;
    parse_file(filename, verbose, set_bits, associativity, block_bits,
               &hit_num, &miss_num, &eviction_num);

    printSummary(hit_num, miss_num, eviction_num);
    return 0;
}

void usage()
{
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
           "Options:\n"
           " -h         Print this help message.\n"
           " -v         Optional verbose flag.\n"
           " -s <num>   Number of set index bits.\n"
           " -E <num>   Number of lines per set.\n"
           " -b <num>   Number of block offset bits.\n"
           " -t <file>  Trace file.\n");
}

void parse_command(int argc, char **argv, bool *verbose, int *set_bits,
                   int *associativity, int *block_bits, char *filename)
{
    int opt;
    bool args_flag[REQUIRED_ARG] = {false, false, false, false};
    while ((opt = getopt(argc, argv, "s:E:b:t:hv")) != -1)
    {
        switch (opt)
        {
        case REQUIRED_ARG_S:
        {
            *set_bits = atoi(optarg);
            args_flag[REQUIRED_ARG_S_INDEX] = true;
            break;
        }
        case REQUIRED_ARG_E:
        {
            *associativity = atoi(optarg);
            args_flag[REQUIRED_ARG_E_INDEX] = true;
            break;
        }
        case REQUIRED_ARG_B:
        {
            *block_bits = atoi(optarg);
            args_flag[REQUIRED_ARG_B_INDEX] = true;
            break;
        }
        case REQUIRED_ARG_T:
        {
            snprintf(filename, FILENAME_LENGTH, "%s", optarg);
            args_flag[REQUIRED_ARG_T_INDEX] = true;
            break;
        }
        case OPTIONAL_ARG_V:
        {
            *verbose = true;
            break;
        }
        case OPTIONAL_ARG_H:
        {
            usage();
            exit(0);
        }
        default:
        {
            usage();
            exit(0);
        }
        }
    }

    for (int i = 0; i < REQUIRED_ARG; ++i)
    {
        if (args_flag[i] == false)
        {
            fprintf(stderr, "%s%s", argv[0], ": Missing required command line argument\n");
            usage();
            exit(0);
        }
    }
}

void parse_file(char *filename, bool verbose, int set_bits, int associativity,
                int block_bits, int *hit_num, int *miss_num, int *eviction_num)
{
    FILE *f = fopen(filename, "r");
    if (f == NULL)
    {
        fprintf(stderr, "%s%s%s\n", "Can't open ", filename, " file\n");
        exit(-1);
    }

    unsigned long set_num = 1 << set_bits;
    unsigned long block_size = 1 << block_bits;
    unsigned long set_size = associativity * block_size;
    //int total_block = set_num * set_size;

    // mem
    unsigned long **blocks = (unsigned long **)malloc(set_num * sizeof(unsigned long *));
    for (int i = 0; i < set_num; ++i)
    {
        blocks[i] = (unsigned long *)malloc(associativity * sizeof(unsigned long));
        memset(blocks[i], 0, associativity * (sizeof(unsigned long) / sizeof(int)));
    }

    // used block num and flag
    // int used_block_num = 0;
    bool **used_blocks = (bool **)malloc(set_num * sizeof(bool *));
    for (int i = 0; i < set_num; ++i)
    {
        used_blocks[i] = (bool *)malloc(associativity * sizeof(bool));
        memset(used_blocks[i], false, associativity);
    }

    // time index
    // int cur_time = 0;
    // int **time_index = (int **)malloc(set_size * sizeof(int *));
    // for (int i = 0; i < set_size; ++i)
    // {
    //     time_index[i] = (int *)malloc(associativity * sizeof(int));
    //     memset(time_index[i], cur_time, associativity);
    // }

    char buffer[BUFFER_LENGTH];
    unsigned long address;
    long size;
    while (fgets(buffer, BUFFER_LENGTH, f) != NULL)
    {
        if (buffer[0] != ' ') /* instruction */
        {
            continue;
        }

        // remove \n
        *strchr(buffer, '\n') = 0;

        char *comma = strchr(buffer, ',');
        size = atol(comma + 1);
        address = 0;
        char *start = buffer + 3;
        while (start != comma)
        {
            if (*start < 58 && *start >= 48)
            {
                // number
                address = address * 16 + *start - 48;
            }
            else if (*start < 71 && *start >= 65)
            {
                // captial letter
                address = address * 16 + *start - 55;
            }
            else
            {
                // Lowercase letter
                address = address * 16 + *start - 87;
            }

            ++start;
        }
       
        bool hit = false;
        int set_index = (address / set_size) % set_num;
        int associativity_index = (address % set_size) / block_size;
        //printf("%d %d %d %ld %ld %ld\n", set_index, associativity_index, used_blocks[set_index][associativity_index], blocks[set_index][associativity_index], block_size, address);
        if (used_blocks[set_index][associativity_index] &&
            blocks[set_index][associativity_index] <= address &&
            blocks[set_index][associativity_index] + block_size > address + size - 1)
        {
            hit = true;

            // modify visit time
            // time_index[i][j] = cur_time;
        }

        if (hit)
        {
            ++*hit_num;
            strcat(buffer, HIT);
        }
        else
        {
            ++*miss_num;
            strcat(buffer, MISS);

            if (used_blocks[set_index][associativity_index])
            {
                ++*eviction_num;
                strcat(buffer, EVCITION);
            }

            blocks[set_index][associativity_index] = address - address % block_size;
            used_blocks[set_index][associativity_index] = true;
        }

        if (buffer[1] == 'M')
        {
            ++*hit_num;
            strcat(buffer, HIT);
        }

        if (verbose)
        {

            printf("%s\n", buffer);
        }
    }

    fclose(f);
}