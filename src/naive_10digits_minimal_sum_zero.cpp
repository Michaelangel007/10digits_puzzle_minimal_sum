// https://www.reddit.com/r/askmath/comments/1hqjtej/what_answer_is_closest_to_zero/
// Copyleft (C) 2024 Michaelangel007 / u/mysticreddit
    #define ALL_SOLUTIONS 0
    
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <time.h>
    #include <omp.h>
    
          int gnThreadsMaximum =   0;
          int gnThreadsActive  =   0; // 0 = auto detect; > 0 use manual # of threads
    const int MAX_THREADS      = 256; // Threadripper 3990X
    
    struct DigitCounter
    {
        DigitCounter()
        {
            memset( aDigits, 0, sizeof(aDigits) );
        }
        void CountDigits( int n, int nDigits )
        {
            int x = n;
            for( int iDigit = 0; iDigit < nDigits; iDigit++ )
            {
                aDigits[ x % 10 ]++;
                x /= 10;
            }
        }
        bool IsValid()
        {
            bool valid = true;
            for( int iDigit = 0; iDigit < 10; iDigit++ )
                if (aDigits[ iDigit ] != 1)
                {
                    valid = false;
                    break;
                }
            return valid;
        }
        char aDigits[10];
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
        
        int    nZeroFirst = 0;
        int    zSolutions = 0; //     198 solutions where E == 0
        int    nSolutions = 0; // 1814301 solutions where E != 0
        time_t nStart = clock();
    #pragma omp parallel for
        for( int64_t n = 0; n < N; n++ )
        {
            const int iThread = omp_get_thread_num();
            int64_t rem = n;
            int d = rem %  100; rem /=  100; //
            int c = rem % 1000; rem /= 1000; // -n4n3n2 * n1n0
            int b = rem %  100; rem /=  100; // +n9n8n7 * n6n5
            int a = rem % 1000;
            int AB = a*b;
            int CD =-c*d;
            int E  = AB + CD;
            if (E < 0) continue;
            
            DigitCounter digits; // filter out numbers with duplicate digits
            digits.CountDigits( a, 3 );
            digits.CountDigits( b, 2 );
            digits.CountDigits( c, 3 );
            digits.CountDigits( d, 2 );
            if (!digits.IsValid())
                continue;
            
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
        printf( "Naive...\n" );
    #if ALL_SOLUTIONS
        printf( "N Sum: %d\n", nSolutions );
    #endif // ALL_SOLUTIONS
        printf( "0 Sum: %d\n", zSolutions );
        printf( "Zero First: %d (***)\n", nZeroFirst );
        printf( "Lowest: %zd\n", nLowest );
        printf( "%.f milliseconds\n", nMilliSeconds );
        return 0;
    }
