#include "definitions.h"
#include "pencilDcmp.h"
#if ( OPENACC )
void PoissonGPU::performTransformXdir() /*!< Called on Host and Ran on GPU*/
{
    cufftHandle plan;
    double *    ptr = P.P;
#pragma acc host_data use_device( ptr )
    {
        cufftPlan1d( &plan, nx, CUFFT_Z2Z, nyChunk * nzChunk );
        cufftExecZ2Z( plan, (cufftDoubleComplex *)( ptr ), (cufftDoubleComplex *)( ptr ), CUFFT_FORWARD );
        cufftDestroy( plan );
    }
}

void PoissonGPU::performInverseTransformXdir() /*!< Called on Host and Ran on GPU*/
{
    cufftHandle plan;
    double *    ptr = P.P;
#pragma acc host_data use_device( ptr )
    {
        cufftPlan1d( &plan, nx, CUFFT_Z2Z, nyChunk * nzChunk );
        cufftExecZ2Z( plan, (cufftDoubleComplex *)( ptr ), (cufftDoubleComplex *)( ptr ), CUFFT_INVERSE );
        cufftDestroy( plan );
    }
}
void PoissonGPU::performTransformYdir() /*!< Called on Host and Ran on GPU*/
{
    cufftHandle plan;
    double *    ptr = P.P;
#pragma acc host_data use_device( ptr )
    {
        cufftPlan1d( &plan, ny, CUFFT_Z2Z, nxChunk * nzChunk );
        cufftExecZ2Z( plan, (cufftDoubleComplex *)( ptr ), (cufftDoubleComplex *)( ptr ), CUFFT_FORWARD );
        cufftDestroy( plan );
    }
}

void PoissonGPU::performInverseTransformYdir() /*!< Called on Host and Ran on GPU*/
{
    cufftHandle plan;
    double *    ptr = P.P;
#pragma acc host_data use_device( ptr )
    {
        cufftPlan1d( &plan, ny, CUFFT_Z2Z, nxChunk * nzChunk );
        cufftExecZ2Z( plan, (cufftDoubleComplex *)( ptr ), (cufftDoubleComplex *)( ptr ), CUFFT_INVERSE );
        cufftDestroy( plan );
    }
}
// const std::string currentDateTime() ;

void PoissonGPU::pittPack() /*!<called on CPU runs on GPU */
{
    //  struct timeval start_time, stop_time, elapsed_time;
    //  gettimeofday( &start_time, NULL );
    double t1;
    MPI_Barrier( MPI_COMM_WORLD );
    t1 = MPI_Wtime();

#if ( DEBUG2 )
    ofstream myfile;

    std::string filename = "data";
    filename.append( to_string( myRank ) );
    //  ofstream myfile;
    myfile.open( filename );

    myfile << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" << endl;
    myfile << "     Start" << endl;

    printX( myfile );

#endif

    //   initializeAndBind();

    double err = 0.0;
    finalErr   = 0.0;

//    P.moveHostToDevice();
#if ( 1 )

    int result = SUCCESS;

    for ( int num = 0; num < 1; num++ )
    {
#pragma acc data present( P [0:2 * nxChunk * nyChunk * nzChunk * nChunk], this ) copy( result, err )
        {
#pragma acc parallel
            initializeTrigonometric();

            if ( bc[0] != 'P' && bc[2] != 'P' )
            {
#pragma acc parallel
                modifyRhsDirichlet();
            }

#if ( POSS )
            changeOwnershipPairwiseExchangeZX();

            if ( GPUAW == 0 )
            {
                P.moveHostToDevice();
            }
                // perform FFT in x direction
                // remember, need to rearrange and restore each time

#if ( FFTX )
// step 2) change location of the array such that FFT can be performed on a contegeous array
#pragma acc parallel
            changeLocationX();
            // step 3) perform  FFT

#pragma acc parallel
            preprocessSignalAccordingly( 0, 0 );

            performTransformXdir();

#pragma acc parallel
            postprocessSignalAccordingly( 0, 0 );
// step 4) restore the array to original status before FFT
#pragma acc parallel
            restoreLocationX();

#endif

#if ( FFTY )
            // step 5) pencils with n(1,0,0) is converted to pencil with n(0,1,0)
            if ( GPUAW2 == 0 )
            {
                P.moveDeviceToHost();
            }
            changeOwnershipPairwiseExchangeXY();
            if ( GPUAW2 == 0 )
            {
                P.moveHostToDevice();
            }
                // step 6) swaps X and Y coordinates, now X is Y and nx is ny

#pragma acc parallel
            rearrangeX2Y();

            // step 7) change location of the array such that FFT can be performed on a contegeous array in the transverse direction

#pragma acc parallel
            changeLocationY();

#pragma acc parallel
            preprocessSignalAccordingly( 1, 1 );

            // step 8) perform  FFT transform can be performed
            performTransformYdir();
#pragma acc parallel
            postprocessSignalAccordingly( 1, 1 );
            // M.printX( myfile );

            // step 9) restore the array to original status before FFT

#pragma acc parallel
            restoreLocationY();

#endif

#if ( SOLVE )
            // step 10) pencils with n(0,1,0) is converted to pencil with n(0,0,1)

            //   M.changeOwnershipPairwiseExchangeYZ();
            if ( GPUAW == 0 )
            {
                P.moveDeviceToHost();
            }

            changeOwnershipPairwiseExchangeZX();

            if ( GPUAW == 0 )
            {
                P.moveHostToDevice();
            }
            // step 11) Customized multiBlock Thomas and periodic Thomas (with Sherman-Morrison modification)
            //#pragma acc parallel firstprivate( result ) reduction( + : result )
            //            result = solve();


            if ( MULTIGRID == 1 )
            {
#pragma acc parallel num_gangs(1)
                solveMG( 0 );

// #pragma acc parallel num_gangs(1)
//              solveMG(1);
            }
            else
            {
/*
//#pragma acc parallel async(123)
#pragma acc parallel
                solveThm( 0 );
#pragma acc parallel
                solveThm( 1 );
*/
            }

            //#pragma acc wait(123)
            //            #pragma acc parallel
            //                        solveThm(1);

            /*
            #pragma acc update self (result)
                     // M.printX( myfile );
            */

            if ( result != SUCCESS )
            {
                cout << "Exit Code: " << THOMAS_FAIL << endl;
                cout << BLUE << PittPackGetErrorEnum( THOMAS_FAIL ) << RESET << endl;
                exit( 1 );
            }

            // step 12) pencils with n(0,0,1) is converted to pencil with n(0,1,0)
            // M.changeOwnershipPairwiseExchangeYZ();
            if ( GPUAW == 0 )
            {
                P.moveDeviceToHost();
            }
            changeOwnershipPairwiseExchangeZX();

            if ( GPUAW == 0 )
            {
                P.moveHostToDevice();
            }
#endif

#if ( IFFTY )
// step 13) prepre for contigeuous FFT
#pragma acc parallel
            changeLocationY();

            // step 14) perform IFFT

#pragma acc parallel
            preprocessSignalAccordinglyReverse( 1, 1 );

            performInverseTransformYdir();
#pragma acc parallel
            postprocessSignalAccordinglyReverse( 1, 1 );
            // step 15) restore the array to original status before IFFT

#pragma acc parallel
            restoreLocationY();

// step 16) swaps X and Y coordinates, now X is Y and nx is ny
// need to redefine the function ??????????????????????????????????????????????????????????????????????????
// now again switching to x-direction arrangement so printX is fine
#pragma acc parallel
            rearrangeX2YInverse();

            // step 17) pencils with n(0,1,0) is converted to pencil with n(1,0,0)

            if ( GPUAW2 == 0 )
            {
                P.moveDeviceToHost();
            }
            changeOwnershipPairwiseExchangeXY();

            if ( GPUAW2 == 0 )
            {
                P.moveHostToDevice();
            }
#endif

#if ( IFFTX )
                // step 18) change location of the array such that IFFT can be performed on a contegeous array

#pragma acc parallel
            changeLocationX();

            // step 19) perform  IFFT

#pragma acc parallel
            preprocessSignalAccordinglyReverse( 0, 0 );

            performInverseTransformXdir();

#pragma acc parallel
            postprocessSignalAccordinglyReverse( 0, 0 );

// step 20) restore the array to original status before FFT
#pragma acc parallel
            restoreLocationX();

#pragma acc parallel
            rescale();

            // return back to the original set-up
            changeOwnershipPairwiseExchangeZX();

            //        setCoords( Xbox, 2 );
            //#pragma acc update device()

#endif

            if ( INCLUDE_ERROE_CAL_IN_TIMING == 1 )
            {
#pragma acc parallel // reduction(max:err)
                err = getError();

                // cout<<" ****** "<<err<<endl;
            }

#endif
        }
    }

    //#pragma acc update self(err)

    MPI_Allreduce( &err, &finalErr, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

    // cout << YELLOW << "Error  (" << myRank << ") =" << err << " " << finalErr << RESET << endl;
    cout << YELLOW << "Error Per processor"
         << " " << finalErr / p0 / p0 << RESET << endl;

    finalErr = finalErr / p0 / p0;

    //  gettimeofday( &stop_time, NULL );
    //  timersub( &stop_time, &start_time, &elapsed_time ); // Unix time subtract routine

    double t2;
    MPI_Barrier( MPI_COMM_WORLD );
    t2 = MPI_Wtime();

    if ( myRank == 0 )
    {
        runTime = t2 - t1;
    }

    runInfo();
    if ( JIC )
    {
#if ( OPENACC )
#pragma acc data present( P, R )
#endif
        {
#if ( OPENACC )
#pragma acc parallel
#endif
            debug();
        }
    }

    P.moveDeviceToHost();

#if ( DEBUG2 )
    printX( myfile );
    myfile.close();
#endif
    cout << " *********************************" << endl;
#if ( DEBUG2 )
    //#if (1 )
    for ( int k = 0; k < 1; k++ )
    {
        for ( int j = 0; j < nyChunk; j++ )
        {
            for ( int i = 0; i < nxChunk; i++ )
            {
                cout << "\t " << P( i, j, k );
            }
            cout << endl;
        }
    }
#endif

#endif
}

#endif