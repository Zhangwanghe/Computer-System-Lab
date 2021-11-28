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

struct block
{
    unsigned long tag;
    bool invalid;
    int time;
};

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

bool parse_line(char *buffer, unsigned long *address, long *size)
{
    if (buffer[0] != ' ') /* instruction */
    {
        return false;
    }

    // remove \n
    *strchr(buffer, '\n') = 0;

    char *comma = strchr(buffer, ',');

    // size
    *size = atol(comma + 1);

    //
    char *start = buffer + 3;
    *address = 0;
    while (start != comma)
    {
        if (*start < 58 && *start >= 48)
        {
            // number
            *address = *address * 16 + *start - 48;
        }
        else if (*start < 71 && *start >= 65)
        {
            // captial letter
            *address = *address * 16 + *start - 55;
        }
        else
        {
            // Lowercase letter
            *address = *address * 16 + *start - 87;
        }

        ++start;
    }

    return true;
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
    int cur_time = 0;

    // mem init
    struct block **blocks = (struct block **)malloc(set_num * sizeof(struct block *));
    for (int i = 0; i < set_num; ++i)
    {
        blocks[i] = (struct block *)malloc(associativity * sizeof(struct block));
        memset(blocks[i], 0, associativity * (sizeof(struct block) / sizeof(int)));
    }

    char buffer[BUFFER_LENGTH];
    unsigned long address;
    long size;
    while (fgets(buffer, BUFFER_LENGTH, f) != NULL)
    {
        cur_time++;

        if (!parse_line(buffer, &address, &size))
        {
            continue;
        }

        bool hit = false;
        int set_index = ((address + block_size) / block_size) % set_num;
        for (int i = 0; i < associativity; ++i)
        {
            if (blocks[set_index][i].invalid &&
                blocks[set_index][i].tag <= address &&
                blocks[set_index][i].tag + block_size > address + size - 1)
            {
                hit = true;

                // modify visit time
                blocks[set_index][i].time = cur_time;

                break;
            }
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

            int associativity_index = 0;
            int least_time = blocks[set_index][0].time;
            for (int i = 0; i < associativity; ++i)
            {
                if (blocks[set_index][i].time < least_time)
                {
                    least_time = blocks[set_index][i].time;
                    associativity_index = i;
                }
            }

            if (blocks[set_index][associativity_index].invalid)
            {
                ++*eviction_num;
                strcat(buffer, EVCITION);
            }

            blocks[set_index][associativity_index].tag = address - address % block_size;
            blocks[set_index][associativity_index].invalid = true;
            blocks[set_index][associativity_index].time = cur_time;
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