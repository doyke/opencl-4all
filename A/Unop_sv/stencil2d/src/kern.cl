/**
 * Copyright (c) 2018 Andre Bannwart Perina and others
 *
 * Adapted from
 * shoc/src/opencl/level1/stencil2d/stencil2d.cl
 * Different licensing may apply, please check SHOC documentation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

inline
int
ToGlobalRow( int gidRow, int lszRow, int lidRow )
{
    // assumes coordinates and dimensions are logical (without halo)
    // returns logical global row (without halo)
    return gidRow*lszRow + lidRow;
}

inline
int
ToGlobalCol( int gidCol, int lszCol, int lidCol )
{
    // assumes coordinates and dimensions are logical (without halo)
    // returns logical global column (without halo)
    return gidCol*lszCol + lidCol;
}


inline
int
ToFlatHaloedIdx( int row, int col, int rowPitch )
{
    // assumes input coordinates and dimensions are logical (without halo)
    // and a halo of width 1
    return (row + 1)*(rowPitch + 2) + (col + 1);
}


inline
int
ToFlatIdx( int row, int col, int pitch )
{
    return row * pitch + col;
}

__attribute__((reqd_work_group_size(1,256,1)))
__kernel
void
StencilKernel( __global float* data,
                __global float* newData,
                const int alignment,
                float wCenter,
                float wCardinal,
                float wDiagonal,
                __local float* sh )
{
    // determine our location in the OpenCL coordinate system
    // To match with the row-major ordering used to store the 2D
    // array in both the host and on the device, we use:
    //   dimension 0 == rows,
    //   dimension 1 == columns
    int gidRow = get_group_id(0);
    int gidCol = get_group_id(1);
    int gszRow = get_num_groups(0);
    int gszCol = get_num_groups(1);
    int lidRow = get_local_id(0);
    int lidCol = get_local_id(1);
    int lszRow = 8;
    int lszCol = get_local_size(1);

    // determine our logical global data coordinates (without halo)
    int gRow = ToGlobalRow( gidRow, lszRow, lidRow );
    int gCol = ToGlobalCol( gidCol, lszCol, lidCol );

    // determine pitch of rows (without halo)
    int nCols = gszCol * lszCol + 2;     // num columns including halo
    int nPaddedCols = nCols + (((nCols % alignment) == 0) ? 0 : (alignment - (nCols % alignment)));
    int gRowWidth = nPaddedCols - 2;    // remove the halo

    // Copy my global data item to a shared local buffer.
    // That local buffer is passed to us as a parameter.
    // We assume it is large enough to hold all the data computed by
    // our block, plus a halo of width 1.
    int lRowWidth = lszCol;          // logical, not haloed
    for( int i = 0; i < (lszRow + 2); i++ )
    {
        int lidx = ToFlatHaloedIdx( lidRow - 1 + i, lidCol, lRowWidth );
        int gidx = ToFlatHaloedIdx( gRow - 1 + i, gCol, gRowWidth );
        sh[lidx] = data[gidx];
    }

    // Copy the "left" and "right" halo rows into our local memory buffer.
    // Only two threads are involved (first column and last column).
    if( lidCol == 0 )
    {
        for( int i = 0; i < (lszRow + 2); i++ )
        {
            int lidx = ToFlatHaloedIdx(lidRow - 1 + i, lidCol - 1, lRowWidth );
            int gidx = ToFlatHaloedIdx(gRow - 1 + i, gCol - 1, gRowWidth );
            sh[lidx] = data[gidx];
        }
    }
    else if( lidCol == (lszCol - 1) )
    {
        for( int i = 0; i < (lszRow + 2); i++ )
        {
            int lidx = ToFlatHaloedIdx(lidRow - 1 + i, lidCol + 1, lRowWidth );
            int gidx = ToFlatHaloedIdx(gRow - 1 + i, gCol + 1, gRowWidth );
            sh[lidx] = data[gidx];
        }
    }

    // let all those loads finish
    barrier( CLK_LOCAL_MEM_FENCE );

    // do my part of the smoothing operation
    for( int i = 0; i < lszRow; i++ )
    {
        int cidx  = ToFlatHaloedIdx( lidRow     + i, lidCol    , lRowWidth );
        int nidx  = ToFlatHaloedIdx( lidRow - 1 + i, lidCol    , lRowWidth );
        int sidx  = ToFlatHaloedIdx( lidRow + 1 + i, lidCol    , lRowWidth );
        int eidx  = ToFlatHaloedIdx( lidRow     + i, lidCol + 1, lRowWidth );
        int widx  = ToFlatHaloedIdx( lidRow     + i, lidCol - 1, lRowWidth );
        int neidx = ToFlatHaloedIdx( lidRow - 1 + i, lidCol + 1, lRowWidth );
        int seidx = ToFlatHaloedIdx( lidRow + 1 + i, lidCol + 1, lRowWidth );
        int nwidx = ToFlatHaloedIdx( lidRow - 1 + i, lidCol - 1, lRowWidth );
        int swidx = ToFlatHaloedIdx( lidRow + 1 + i, lidCol - 1, lRowWidth );

        float centerValue = sh[cidx];
        float cardinalValueSum = sh[nidx] + sh[sidx] + sh[eidx] + sh[widx];
        float diagonalValueSum = sh[neidx] + sh[seidx] + sh[nwidx] + sh[swidx];

        newData[ToFlatHaloedIdx(gRow + i, gCol, gRowWidth)] = wCenter * centerValue +
                wCardinal * cardinalValueSum +
                wDiagonal * diagonalValueSum;
    }
}
