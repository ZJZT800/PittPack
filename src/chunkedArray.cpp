#include "chunkedArray.h"
#include "params.h"
#include <stdio.h>
#include <stdlib.h>
// N,Ny,Nx denote the number of complex variables at directions 0, 1 and 2
//#pragma acc routine
void ChunkedArray::moveHostToDevice()
{
#if ( OPENACC )
#pragma acc update device( P [0:arraySize] )
#endif
}

PittPackResult ChunkedArray::allocate( int *n, int nbl )
{
    Nx = n[0];
    Ny = n[1];
    Nz = n[2];

    arraySize = 2 * Nx * Ny * Nz;

    // cout << " allocated P with size " << arraySize << endl;
    P = new ( std::nothrow ) double[arraySize];

    //      P = (double*)malloc( arraySize*sizeof(double));

    //    P = new double[arraySize];
    //    coords = new double[6];
    //    dxyz = new double[3];

    if ( nbl <= 0 )
    {
        cout << " Exit Code " << BLOCK_NUMBER_FAIL << endl;
        cout << BLUE << PittPackGetErrorEnum( BLOCK_NUMBER_FAIL ) << RESET << endl;
        exit( 1 );
    }

    cout << " allocated P with size " << arraySize << endl;

    for ( int i = 0; i < arraySize; i++ )
    {
        P[i] = 0.0;
    }
    nChunk    = nbl;
    chunkSize = arraySize / nbl; /*! chunk size refers to the size of the chunks, that is the whole
                                    array divided by nchunks, this is to get rid of multiplication by two */

#if ( OPENACC )
#pragma acc enter data create( this [0:1] )
#pragma acc update device( this )
#pragma acc enter data create( P [0:arraySize] )
#endif

    if ( P == NULL )
    {
        return ( ALLOCATION_FAIL );
    }
    else
    {
        return ( SUCCESS );
    }
}

int  ChunkedArray::getChunkSize() { return ( chunkSize ); }
void ChunkedArray::setCoords( double *X )
{
    Xa = X[0];
    Xb = X[1];
    Ya = X[2];
    Yb = X[3];
    Za = X[4];
    Zb = X[5];
    /*
       for(int i=0;i<6;i++)
    {
        coords[i]=X[i];
     }
    */
}

void ChunkedArray::setDirection( int dir )
{
    orientation = dir;

    double coeff[3]    = {1.0, 1.0, 1.0};
    coeff[orientation] = 1. / nChunk;

    nx = coeff[0] * Nx;
    ny = coeff[1] * Ny;
    nz = coeff[2] * Nz;
#if ( OPENACC )
#pragma acc update device( nx, ny, nz )
#endif
    cout << "each chunk dims=" << nx << " " << ny << " " << nz << endl;
}

#pragma acc routine seq
int                 ChunkedArray::size() { return ( arraySize ); }

//#pragma acc routine
void ChunkedArray::moveDeviceToHost()
{
#if ( OPENACC )
#pragma acc update self( P [0:arraySize] )
#endif
}

#if ( OPENACC )
#pragma acc routine seq
#endif
// index 0 retrieves real and indez zero retrieves
// inline double &ChunkedArray::operator()( int i, int j, int k, int index )
double &ChunkedArray::operator()( int i, int j, int k, int index ) { return P[2 * ( i + nx * j + nx * ny * k ) + index]; }

#if ( OPENACC )
#pragma acc routine seq
#endif
// inline double &ChunkedArray::operator()( int i, int j, int k )
double &ChunkedArray::operator()( int i, int j, int k ) { return P[2 * ( i + nx * j + nx * ny * k )]; }
#if ( OPENACC )
#pragma acc routine seq
#endif
// inline double &ChunkedArray::operator()( int i, int j, int k, int dir, int index )
double &ChunkedArray::operator()( int i, int j, int k, int dir, int index )
{
    if ( dir == 0 )
    {
        return ( P[2 * ( i + nx * j + nx * ny * k ) + index] );
    }
    else if ( dir == 1 )
    {
        return ( P[2 * ( j + ny * i + nx * ny * k ) + index] );
    }
}
#if ( OPENACC )
#pragma acc routine seq
#endif
// inline double &ChunkedArray::operator()( int chunkId, int dir, int i, int j, int k, int index )
double &ChunkedArray::operator()( int chunkId, int dir, int i, int j, int k, int index )
{
    /*! brief: rearrangement is consistent with the planes where (1,0,0) (0,1,0) and (0,0,1) around which rotation occurs */

    if ( dir == 0 )
    {
        return ( P[2 * ( chunkId * chunkSize / 2 + i + nx * j + nx * ny * k ) + index] );
    }
    else if ( dir == 1 )
    {
        // return( P[2 * ( chunkId*chunkSize/2 + j + ny * k + nx * ny * i ) + index]);
        return ( P[2 * ( chunkId * chunkSize / 2 + j + ny * i + nx * ny * k ) + index] );
    }
    else if ( dir == 2 )
    {
        // return( P[2 * ( chunkId*chunkSize/2 + k + ny * i + ny * nz * j ) + index]);
        return ( P[2 * ( chunkId * chunkSize / 2 + k + nz * j + nz * ny * i ) + index] );
    }
    /* this one is added to help solve the Thomas in place and while in y- major direction, y , x and i and j replace each other*/

    else if ( dir == 3 )
    {
        return ( P[2 * ( chunkId * chunkSize / 2 + i + ny * j + nx * ny * k ) + index] );
    }
    // note that at rotated to y-dir, ny corresponds to i, nx corresponds to j
    else if ( dir == 4 )
    {
        return ( P[2 * ( chunkId * chunkSize / 2 + k + nz * j + nz * ny * i ) + index] );
    }
}
#if ( OPENACC )
#pragma acc routine seq
#endif
// inline double &ChunkedArray::operator()( int i )
double &ChunkedArray::operator()( int i )
{
    /*
       if(i>=arraySize)
    {
      printf("index bigger than array size]n");

    //  exit(0);
    }
    */
    return P[i];
}

/*! the order is not important */
ChunkedArray::~ChunkedArray()
{
    if ( P != NULL )
    {
#if ( OPENACC )
#pragma acc exit data delete ( P [0:arraySize] )
#endif
        delete[] P;
        //        free(P);
    }
#if ( OPENACC )
#pragma acc exit data delete ( this )
#endif
}

void ChunkedArray::getAddress( double *rt )
{
    rt = P;
    cout << "pointer address " << rt << endl;
}