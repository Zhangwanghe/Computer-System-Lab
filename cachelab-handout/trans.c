/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 * 
 * Wanghe Zhang
 */
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

char transpose_save_diagonal32_desc[] = "Transpose copy and self trans 32";
void transpose_save_diagonal32(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, diagonal;
    int msize = 8;
    int nsize = 8;

    for (ii = 0; ii < N; ii += nsize)
    {
        for (jj = 0; jj < M; jj += msize)
        {
            if (ii != jj)
            {
                for (i = ii; i < ii + nsize; i++)
                {
                    for (j = jj; j < jj + msize; j++)
                    {

                        tmp = A[i][j];
                        B[j][i] = tmp;
                    }
                }

                continue;
            }

            for (i = ii; i < ii + nsize; i++)
            {
                diagonal = A[i][i];
                for (j = jj; j < jj + msize; j++)
                {
                    if (i == j)
                    {
                        continue;
                    }

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                B[i][i] = diagonal;
            }
        }
    }
}

char transpose_save_diagonal64_desc[] = "Transpose 64 ori";
void transpose_save_diagonal64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp, diagonal, offset, line[8];

    for (jj = 0; jj < M; jj += 8)
    {
        for (ii = 0; ii < N; ii += 8)
        {
            offset = ii - jj;
            for (i = ii; i < ii + 4; i++)
            {
                diagonal = A[i][i - offset];
                for (j = jj; j < jj + 4; j++)
                {
                    if (i == ii)
                    {
                        line[j - jj] = A[i][j + 4];
                    }

                    if (i == ii + 1)
                    {
                        line[j - jj + 4] = A[i][j + 4];
                    }

                    if (i - j == offset)
                    {
                        continue;
                    }

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                B[i - offset][i] = diagonal;
            }

            offset = ii - jj + 4;
            for (i = ii + 4; i < ii + 4 * 2; i++)
            {
                diagonal = A[i][i - offset];
                for (j = jj; j < jj + 4; j++)
                {
                    if (i - j == offset)
                    {
                        continue;
                    }

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                B[i - offset][i] = diagonal;
            }

            offset = ii - jj;
            for (i = ii + 4; i < ii + 8; i++)
            {
                diagonal = A[i][i - offset];
                for (j = jj + 4; j < jj + 8; j++)
                {
                    if (i - j == offset)
                    {
                        continue;
                    }

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                B[i - offset][i] = diagonal;
            }

            offset = ii - jj - 4;
            for (i = ii; i < ii + 4; i++)
            {
                if (i == ii)
                {
                    B[jj + 4][i] = line[0];
                    B[jj + 5][i] = line[1];
                    B[jj + 6][i] = line[2];
                    B[jj + 7][i] = line[3];
                    continue;
                }

                if (i == ii + 1)
                {
                    B[jj + 4][i] = line[4];
                    B[jj + 5][i] = line[5];
                    B[jj + 6][i] = line[6];
                    B[jj + 7][i] = line[7];
                    continue;
                }

                diagonal = A[i][i - offset];
                for (j = jj + 4; j < jj + 8; j++)
                {
                    if (i - j == offset)
                    {
                        continue;
                    }

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
                B[i - offset][i] = diagonal;
            }
        }
    }
}

char transpose_save_diagonal64_rm_variable_desc[] = "Transpose 64 remove variable";
void transpose_save_diagonal64_rm_variable(int M, int N, int A[N][M], int B[M][N])
{
    int i, k, tmp, diagonal, line[8];

    for (k = 0; k < M * N / 8; k += 8)
    {
        for (i = 0; i < 16; i++)
        {
            if (i % 4 == 0)
            {
                diagonal = A[i / 4 + k % 64][i / 4 + (k / 64) * 8];
            }

            if (i / 4 == 0)
            {
                line[i % 4] = A[i / 4 + k % 64][i % 4 + (k / 64) * 8 + 4];
            }

            if (i / 4 == 1)
            {
                line[i % 4 + 4] = A[i / 4 + k % 64][i % 4 + (k / 64) * 8 + 4];
            }

            if (i / 4 != i % 4)
            {
                tmp = A[i / 4 + k % 64][i % 4 + (k / 64) * 8];
                B[i % 4 + (k / 64) * 8][i / 4 + k % 64] = tmp;
            }

            if (i % 4 == 3)
            {
                B[i / 4 + (k / 64) * 8][i / 4 + k % 64] = diagonal;
            }
        }

        for (i = 0; i < 16; i++)
        {
            if (i % 4 == 0)
            {
                diagonal = A[i / 4 + k % 64 + 4][i / 4 + (k / 64) * 8];
            }

            if (i / 4 != i % 4)
            {
                tmp = A[i / 4 + k % 64 + 4][i % 4 + (k / 64) * 8];
                B[i % 4 + (k / 64) * 8][i / 4 + k % 64 + 4] = tmp;
            }

            if (i % 4 == 3)
            {
                B[i / 4 + (k / 64) * 8][i / 4 + k % 64 + 4] = diagonal;
            }
        }

        for (i = 0; i < 16; i++)
        {
            if (i % 4 == 0)
            {
                diagonal = A[i / 4 + k % 64 + 4][i / 4 + (k / 64) * 8 + 4];
            }

            if (i / 4 != i % 4)
            {
                tmp = A[i / 4 + k % 64 + 4][i % 4 + (k / 64) * 8 + 4];
                B[i % 4 + (k / 64) * 8 + 4][i / 4 + k % 64 + 4] = tmp;
            }

            if (i % 4 == 3)
            {
                B[i / 4 + (k / 64) * 8 + 4][i / 4 + k % 64 + 4] = diagonal;
            }
        }

        for (i = 0; i < 16; i++)
        {
            if (i / 4 == 0)
            {
                B[(k / 64) * 8 + 4][i / 4 + k % 64] = line[0];
                B[(k / 64) * 8 + 5][i / 4 + k % 64] = line[1];
                B[(k / 64) * 8 + 6][i / 4 + k % 64] = line[2];
                B[(k / 64) * 8 + 7][i / 4 + k % 64] = line[3];
                continue;
            }

            if (i / 4 == 1)
            {
                B[(k / 64) * 8 + 4][i / 4 + k % 64] = line[4];
                B[(k / 64) * 8 + 5][i / 4 + k % 64] = line[5];
                B[(k / 64) * 8 + 6][i / 4 + k % 64] = line[6];
                B[(k / 64) * 8 + 7][i / 4 + k % 64] = line[7];
                continue;
            }

            if (i % 4 == 0)
            {
                diagonal = A[i / 4 + k % 64][i / 4 + (k / 64) * 8 + 4];
            }

            if (i / 4 != i % 4)
            {
                tmp = A[i / 4 + k % 64][i % 4 + (k / 64) * 8 + 4];
                B[i % 4 + (k / 64) * 8 + 4][i / 4 + k % 64] = tmp;
            }

            if (i % 4 == 3)
            {
                B[i / 4 + (k / 64) * 8 + 4][i / 4 + k % 64] = diagonal;
            }
        }
    }
}

char transpose_save_diagonal61_desc[] = "Transpose 61 remove variable";
void transpose_save_diagonal61(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii, jj, tmp;
    int msize = 8;
    int nsize = 16;

    for (ii = 0; ii < N; ii += nsize)
    {
        if (ii == 64)
        {
            nsize = 3;
        }

        for (jj = 0; jj < M; jj += msize)
        {
            if (jj == 56)
            {
                msize = 5;
            }

            for (i = ii; i < ii + nsize; i++)
            {
                for (j = jj; j < jj + msize; j++)
                {

                    tmp = A[i][j];
                    B[j][i] = tmp;
                }
            }
        }

        msize = 8;
    }
}

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32)
    {
        transpose_save_diagonal32(M, N, A, B);
    }
    else if (M == 64 && N == 64)
    {
        transpose_save_diagonal64_rm_variable(M, N, A, B);
    }
    else if (M == 61 && N == 67)
    {
        transpose_save_diagonal61(M, N, A, B);
    }
    else 
    {
        int i, j, ii, jj, tmp;
        int msize = 8;
        int nsize = 8;
        // int en = bsize * (N / bsize); /* Amount that fits evenly into blocks */

        for (ii = 0; ii < N; ii += nsize)
        {
            for (jj = 0; jj < M; jj += msize)
            {
                for (i = ii; i < ii + nsize; i++)
                {
                    for (j = jj; j < jj + msize; j++)
                    {
                        tmp = A[i][j];
                        B[j][i] = tmp;
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    //registerTransFunction(trans, trans_desc);
    //registerTransFunction(transpose_save_diagonal61, transpose_save_diagonal61_desc);

    // registerTransFunction(transpose_save_diagonal32, transpose_save_diagonal32_desc);
    // registerTransFunction(transpose_save_diagonal64, transpose_save_diagonal64_desc);
    // registerTransFunction(transpose_save_diagonal64_rm_variable, transpose_save_diagonal64_rm_variable_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
