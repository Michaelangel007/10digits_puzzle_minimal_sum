// https://www.reddit.com/r/askmath/comments/1hqjtej/what_answer_is_closest_to_zero/
// Copyleft (C) 2024 Michaelangel007 / u/mysticreddit
    #define ALL_SOLUTIONS 0
    
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <time.h>
    #include <assert.h>
    #include <omp.h>
    
          int gnThreadsMaximum =   0;
          int gnThreadsActive  =   0; // 0 = auto detect; > 0 use manual # o f threads
    const int MAX_THREADS      = 256; // Threadripper 3990X
    
    const int  SET_SIZE                    = 10;
    const int  SET_PERMUTATIONS            = 10*9*8*7*6*5*4*3*2*1; // 10! = [0 .. 3,628,800)
    const int  SET_FACTORS  [ SET_SIZE   ] = { 362880, 40320, 5040, 720, 120, 24, 6, 2, 1, 1 };
    const char SET_ELEMENTS [ SET_SIZE+1 ] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09";
    struct FactoradicConverter
    {
        FactoradicConverter( const int factoradic )
        {
            assert( factoradic < SET_PERMUTATIONS);
            int  n = factoradic;
            int  f;
            int  q;
            memcpy( aRemaining, SET_ELEMENTS, SET_SIZE+1 );
            
            for (int iDigit = 0; iDigit < SET_SIZE; iDigit++)
            {
                f = SET_FACTORS[ iDigit ];
                q = n / f;
                if (q > 0)
                    n -= q*f;
                aPermutation[ iDigit ] = aRemaining[ q ];
                int nLen = SET_SIZE - q;
                assert( nLen > 0 );
                memcpy( aRemaining + q, aRemaining + q + 1, nLen );
            }
            
            nPermutation = 0;
            for (int iDigit = 0; iDigit < SET_SIZE; iDigit++)
            {
                nPermutation *= 10;
                nPermutation += aPermutation[ iDigit ]; // (aPermutation[ iDigit ] - '0');
            }
        }
        char    aRemaining  [ SET_SIZE+1 ];
        char    aPermutation[ SET_SIZE+1 ];
        int64_t nPermutation;
    };
    
    int main(int nArg, char *aArg[])
    {
        gnThreadsMaximum = omp_get_num_procs();
        if( gnThreadsMaximum > MAX_THREADS )
            gnThreadsMaximum = MAX_THREADS;
        
        if((nArg > 1) && (aArg[1][0]) == '-' && (aArg[1][1] == 'j'))
        {
            int i = atoi( aArg[ 1 ] + 2 );
            if( i               > 0 )
                gnThreadsActive = i;
            if( gnThreadsActive > MAX_THREADS )
                gnThreadsActive = MAX_THREADS;
        }
        
        if(!gnThreadsActive) // user didn't specify how many threads to use, default to all of them
            gnThreadsActive = gnThreadsMaximum;
        omp_set_num_threads( gnThreadsActive );
        
        const int64_t N = 10000000000ll; // 10 unique digits
              int64_t aLowest[ MAX_THREADS ];
        for( int iThread = 0; iThread < MAX_THREADS; iThread++ )
            aLowest[ iThread ] = N;
        
        int    nZeroFirst = 0; // is zero first digit in ANY a, b, c, d ?
        int    zSolutions = 0; //     198 solutions where E == 0
        int    nSolutions = 0; // 1814301 solutions where E != 0
        time_t nStart = clock();
    #pragma omp parallel for
        for( int f = 0; f < SET_PERMUTATIONS; f++ )
        {
            const int iThread = omp_get_thread_num();
            FactoradicConverter factoradic( f );
            int64_t rem = factoradic.nPermutation;
            int d = rem %  100; rem /=  100; //
            int c = rem % 1000; rem /= 1000; // -n4n3n2 * n1n0
            int b = rem %  100; rem /=  100; // +n9n8n7 * n6n5
            int a = rem % 1000;
            int AB = a*b;
            int CD =-c*d;
            int E  = AB + CD;
            if (E < 0) continue;
            
            if (aLowest[ iThread ] > E)
                aLowest[ iThread ] = E;
            
            if (!E)
            {
                if(((a / 100) == 0)
                || ((b /  10) == 0)
                || ((c / 100) == 0)
                || ((d /  10) == 0))
                {
    #pragma omp atomic
                    nZeroFirst++;
                    printf( "%3d * %2d = %5d   -%3d * %2d = %+6d   = %5d  [#%2d/%2d] 0###\n", a, b, AB, c, d, CD, E, iThread, gnThreadsActive );
                }
                else
                    printf( "%3d * %2d = %5d   -%3d * %2d = %+6d   = %5d  [#%2d/%2d] == 0\n", a, b, AB, c, d, CD, E, iThread, gnThreadsActive );
    #pragma omp atomic
                zSolutions++;
            }
    #if ALL_SOLUTIONS
            else
            {
                if (!nSolutions)
                    printf( "%3d * %2d = %5d   -%3d * %2d = %+6d   = %5d  [#%2d/%2d] != 0\n", a, b, AB, c, d, CD, E, iThread, gnThreadsActive );
    #pragma omp atomic
                nSolutions++;
            }
    #endif // ALL_SOLUTIONS
        }
        
        int64_t nLowest = N;
        for( int iThread = 0; iThread < MAX_THREADS; iThread++ )
            if (nLowest > aLowest[ iThread ])
                nLowest = aLowest[ iThread ];
        
        clock_t nEnd = clock();
        double nMilliSeconds = 1000.0 * (double)(nEnd - nStart) / CLOCKS_PER_SEC;
    #if ALL_SOLUTIONS
        printf( "N Sum: %d\n", nSolutions );
    #endif // ALL_SOLUTIONS
        printf( "0 Sum: %d (== 0)\n", zSolutions );
        printf( "Zero First: %d (0###)\n", nZeroFirst );
        printf( "Lowest: %zd\n", nLowest );
        printf( "%.f milliseconds, %d threads, Factoradic\n", nMilliSeconds, gnThreadsActive );
        return 0;
    }
