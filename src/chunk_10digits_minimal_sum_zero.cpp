// https://www.reddit.com/r/askmath/comments/1hqjtej/what_answer_is_closest_to_zero/
// Copyleft (C) 2024 Michaelangel007 / u/mysticreddit
    #define ALL_SOLUTIONS 0
    
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <chrono>
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

    // Q. Does having eaching thread do more work improve performance?
    // A. No. See results at end-of-file.
    //
    //int nWorkPerThread = 1;
    //int nWorkPerThread = 200;
    //int nWorkPerThread = 256;
    //for( int nWorkPerThread = 100; nWorkPerThread <= 10000; nWorkPerThread += 100 )
    //for( int nWorkPerThread = 1; nWorkPerThread < 129; nWorkPerThread++ )
      for( int nWorkPerThread = 1; nWorkPerThread < 8192; nWorkPerThread *= 2 )
{
        const int nPageSize = SET_PERMUTATIONS / nWorkPerThread;
        const int nLastSize = SET_PERMUTATIONS % nWorkPerThread;
        
        for( int iThread = 0; iThread < MAX_THREADS; iThread++ )
            aLowest[ iThread ] = N;
        
        int    nZeroFirst = 0;
        int    zSolutions = 0; //     198 solutions where E == 0
        int    nSolutions = 0; // 1814301 solutions where E != 0
        auto   nStart = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for
        for (int iPage = 0; iPage < nPageSize; iPage++)
        {
            const int iThread = omp_get_thread_num();
            int  fHead  = iPage * nWorkPerThread;
            bool isLast = (iPage == nPageSize - 1);
            int  nWork  = isLast ? nLastSize : nWorkPerThread;
            int  fTail  = fHead + nWork;
            for( int iFact = fHead; iFact < fTail; iFact++ )
            {
                FactoradicConverter factoradic( iFact );
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
//                        printf( "%3d * %2d = %5d   -%3d * %2d = %+6d   = %5d  [#%2d/%2d] 0###\n", a, b, AB, c, d, CD, E, iThread, gnThreadsActive );
                    }
                    else
                    {
//                        printf( "%3d * %2d = %5d   -%3d * %2d = %+6d   = %5d  [#%2d/%2d] == 0\n", a, b, AB, c, d, CD, E, iThread, gnThreadsActive );
                    }
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
        }
        
        int64_t nLowest = N;
        for( int iThread = 0; iThread < MAX_THREADS; iThread++ )
            if (nLowest > aLowest[ iThread ])
                nLowest = aLowest[ iThread ];
        
        auto nEnd     = std::chrono::high_resolution_clock::now();
        auto nElapsed = (nEnd - nStart);
        std::chrono::nanoseconds  ns = std::chrono::duration_cast<std::chrono::nanoseconds>(nElapsed);
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(nElapsed);
        printf( "%10lld ns / %u = %.f ns/factoradic, ", ns.count(), SET_PERMUTATIONS, (double)ns.count() / (double)SET_PERMUTATIONS );
        printf( "Work/Th:%5d, Pages: %7d, Last Work: %4d, Time: %2lld ms, 0 Sum: %d\n", nWorkPerThread, nPageSize, nLastSize, ms.count(), zSolutions );
}
        return 0;
    }

/*
Chunk Size: 1 .. 128
  11710400 ns / 3628800 = 3 ns/factoradic, Work/Th:    1, Pages: 3628800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11319200 ns / 3628800 = 3 ns/factoradic, Work/Th:    2, Pages: 1814400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11393400 ns / 3628800 = 3 ns/factoradic, Work/Th:    3, Pages: 1209600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12128400 ns / 3628800 = 3 ns/factoradic, Work/Th:    4, Pages:  907200, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11297100 ns / 3628800 = 3 ns/factoradic, Work/Th:    5, Pages:  725760, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11741400 ns / 3628800 = 3 ns/factoradic, Work/Th:    6, Pages:  604800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11660300 ns / 3628800 = 3 ns/factoradic, Work/Th:    7, Pages:  518400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11663200 ns / 3628800 = 3 ns/factoradic, Work/Th:    8, Pages:  453600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11740600 ns / 3628800 = 3 ns/factoradic, Work/Th:    9, Pages:  403200, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11669800 ns / 3628800 = 3 ns/factoradic, Work/Th:   10, Pages:  362880, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11315400 ns / 3628800 = 3 ns/factoradic, Work/Th:   11, Pages:  329890, Last Work:   10, Time: 11 ms, 0 Sum: 198
  10825600 ns / 3628800 = 3 ns/factoradic, Work/Th:   12, Pages:  302400, Last Work:    0, Time: 10 ms, 0 Sum: 198
  11991800 ns / 3628800 = 3 ns/factoradic, Work/Th:   13, Pages:  279138, Last Work:    6, Time: 11 ms, 0 Sum: 198
  11366800 ns / 3628800 = 3 ns/factoradic, Work/Th:   14, Pages:  259200, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11801200 ns / 3628800 = 3 ns/factoradic, Work/Th:   15, Pages:  241920, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11748700 ns / 3628800 = 3 ns/factoradic, Work/Th:   16, Pages:  226800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11189600 ns / 3628800 = 3 ns/factoradic, Work/Th:   17, Pages:  213458, Last Work:   14, Time: 11 ms, 0 Sum: 198
  11845600 ns / 3628800 = 3 ns/factoradic, Work/Th:   18, Pages:  201600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11785500 ns / 3628800 = 3 ns/factoradic, Work/Th:   19, Pages:  190989, Last Work:    9, Time: 11 ms, 0 Sum: 198
  11268100 ns / 3628800 = 3 ns/factoradic, Work/Th:   20, Pages:  181440, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12094100 ns / 3628800 = 3 ns/factoradic, Work/Th:   21, Pages:  172800, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11174600 ns / 3628800 = 3 ns/factoradic, Work/Th:   22, Pages:  164945, Last Work:   10, Time: 11 ms, 0 Sum: 198
  12623200 ns / 3628800 = 3 ns/factoradic, Work/Th:   23, Pages:  157773, Last Work:   21, Time: 12 ms, 0 Sum: 198
  13150300 ns / 3628800 = 4 ns/factoradic, Work/Th:   24, Pages:  151200, Last Work:    0, Time: 13 ms, 0 Sum: 198
  14396600 ns / 3628800 = 4 ns/factoradic, Work/Th:   25, Pages:  145152, Last Work:    0, Time: 14 ms, 0 Sum: 198
  13658500 ns / 3628800 = 4 ns/factoradic, Work/Th:   26, Pages:  139569, Last Work:    6, Time: 13 ms, 0 Sum: 198
  11571000 ns / 3628800 = 3 ns/factoradic, Work/Th:   27, Pages:  134400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12411200 ns / 3628800 = 3 ns/factoradic, Work/Th:   28, Pages:  129600, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11568700 ns / 3628800 = 3 ns/factoradic, Work/Th:   29, Pages:  125131, Last Work:    1, Time: 11 ms, 0 Sum: 198
  11457500 ns / 3628800 = 3 ns/factoradic, Work/Th:   30, Pages:  120960, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11470900 ns / 3628800 = 3 ns/factoradic, Work/Th:   31, Pages:  117058, Last Work:    2, Time: 11 ms, 0 Sum: 198
  11176900 ns / 3628800 = 3 ns/factoradic, Work/Th:   32, Pages:  113400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12225800 ns / 3628800 = 3 ns/factoradic, Work/Th:   33, Pages:  109963, Last Work:   21, Time: 12 ms, 0 Sum: 198
  11445700 ns / 3628800 = 3 ns/factoradic, Work/Th:   34, Pages:  106729, Last Work:   14, Time: 11 ms, 0 Sum: 198
  11322000 ns / 3628800 = 3 ns/factoradic, Work/Th:   35, Pages:  103680, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12280400 ns / 3628800 = 3 ns/factoradic, Work/Th:   36, Pages:  100800, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11666800 ns / 3628800 = 3 ns/factoradic, Work/Th:   37, Pages:   98075, Last Work:   25, Time: 11 ms, 0 Sum: 198
  11269000 ns / 3628800 = 3 ns/factoradic, Work/Th:   38, Pages:   95494, Last Work:   28, Time: 11 ms, 0 Sum: 198
  11326300 ns / 3628800 = 3 ns/factoradic, Work/Th:   39, Pages:   93046, Last Work:    6, Time: 11 ms, 0 Sum: 198
  11182900 ns / 3628800 = 3 ns/factoradic, Work/Th:   40, Pages:   90720, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11176300 ns / 3628800 = 3 ns/factoradic, Work/Th:   41, Pages:   88507, Last Work:   13, Time: 11 ms, 0 Sum: 198
  11270900 ns / 3628800 = 3 ns/factoradic, Work/Th:   42, Pages:   86400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11410700 ns / 3628800 = 3 ns/factoradic, Work/Th:   43, Pages:   84390, Last Work:   30, Time: 11 ms, 0 Sum: 198
  11833800 ns / 3628800 = 3 ns/factoradic, Work/Th:   44, Pages:   82472, Last Work:   32, Time: 11 ms, 0 Sum: 198
  12057100 ns / 3628800 = 3 ns/factoradic, Work/Th:   45, Pages:   80640, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11580300 ns / 3628800 = 3 ns/factoradic, Work/Th:   46, Pages:   78886, Last Work:   44, Time: 11 ms, 0 Sum: 198
  11076000 ns / 3628800 = 3 ns/factoradic, Work/Th:   47, Pages:   77208, Last Work:   24, Time: 11 ms, 0 Sum: 198
  11274100 ns / 3628800 = 3 ns/factoradic, Work/Th:   48, Pages:   75600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11461000 ns / 3628800 = 3 ns/factoradic, Work/Th:   49, Pages:   74057, Last Work:    7, Time: 11 ms, 0 Sum: 198
  11296800 ns / 3628800 = 3 ns/factoradic, Work/Th:   50, Pages:   72576, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11635700 ns / 3628800 = 3 ns/factoradic, Work/Th:   51, Pages:   71152, Last Work:   48, Time: 11 ms, 0 Sum: 198
  11589900 ns / 3628800 = 3 ns/factoradic, Work/Th:   52, Pages:   69784, Last Work:   32, Time: 11 ms, 0 Sum: 198
  11893700 ns / 3628800 = 3 ns/factoradic, Work/Th:   53, Pages:   68467, Last Work:   49, Time: 11 ms, 0 Sum: 198
  12020100 ns / 3628800 = 3 ns/factoradic, Work/Th:   54, Pages:   67200, Last Work:    0, Time: 12 ms, 0 Sum: 198
  12304200 ns / 3628800 = 3 ns/factoradic, Work/Th:   55, Pages:   65978, Last Work:   10, Time: 12 ms, 0 Sum: 198
  11403500 ns / 3628800 = 3 ns/factoradic, Work/Th:   56, Pages:   64800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11746500 ns / 3628800 = 3 ns/factoradic, Work/Th:   57, Pages:   63663, Last Work:    9, Time: 11 ms, 0 Sum: 198
  11271500 ns / 3628800 = 3 ns/factoradic, Work/Th:   58, Pages:   62565, Last Work:   30, Time: 11 ms, 0 Sum: 198
  11059600 ns / 3628800 = 3 ns/factoradic, Work/Th:   59, Pages:   61505, Last Work:    5, Time: 11 ms, 0 Sum: 198
  11840700 ns / 3628800 = 3 ns/factoradic, Work/Th:   60, Pages:   60480, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11409400 ns / 3628800 = 3 ns/factoradic, Work/Th:   61, Pages:   59488, Last Work:   32, Time: 11 ms, 0 Sum: 198
  11347500 ns / 3628800 = 3 ns/factoradic, Work/Th:   62, Pages:   58529, Last Work:    2, Time: 11 ms, 0 Sum: 198
  11451900 ns / 3628800 = 3 ns/factoradic, Work/Th:   63, Pages:   57600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  10815200 ns / 3628800 = 3 ns/factoradic, Work/Th:   64, Pages:   56700, Last Work:    0, Time: 10 ms, 0 Sum: 198
  11507100 ns / 3628800 = 3 ns/factoradic, Work/Th:   65, Pages:   55827, Last Work:   45, Time: 11 ms, 0 Sum: 198
  10416300 ns / 3628800 = 3 ns/factoradic, Work/Th:   66, Pages:   54981, Last Work:   54, Time: 10 ms, 0 Sum: 198
  11100700 ns / 3628800 = 3 ns/factoradic, Work/Th:   67, Pages:   54161, Last Work:   13, Time: 11 ms, 0 Sum: 198
  11739500 ns / 3628800 = 3 ns/factoradic, Work/Th:   68, Pages:   53364, Last Work:   48, Time: 11 ms, 0 Sum: 198
  11005700 ns / 3628800 = 3 ns/factoradic, Work/Th:   69, Pages:   52591, Last Work:   21, Time: 11 ms, 0 Sum: 198
  12102800 ns / 3628800 = 3 ns/factoradic, Work/Th:   70, Pages:   51840, Last Work:    0, Time: 12 ms, 0 Sum: 198
  12172700 ns / 3628800 = 3 ns/factoradic, Work/Th:   71, Pages:   51109, Last Work:   61, Time: 12 ms, 0 Sum: 198
  11150200 ns / 3628800 = 3 ns/factoradic, Work/Th:   72, Pages:   50400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11566200 ns / 3628800 = 3 ns/factoradic, Work/Th:   73, Pages:   49709, Last Work:   43, Time: 11 ms, 0 Sum: 198
  11163500 ns / 3628800 = 3 ns/factoradic, Work/Th:   74, Pages:   49037, Last Work:   62, Time: 11 ms, 0 Sum: 198
  11942800 ns / 3628800 = 3 ns/factoradic, Work/Th:   75, Pages:   48384, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12137300 ns / 3628800 = 3 ns/factoradic, Work/Th:   76, Pages:   47747, Last Work:   28, Time: 12 ms, 0 Sum: 198
  12814900 ns / 3628800 = 4 ns/factoradic, Work/Th:   77, Pages:   47127, Last Work:   21, Time: 12 ms, 0 Sum: 198
  11439000 ns / 3628800 = 3 ns/factoradic, Work/Th:   78, Pages:   46523, Last Work:    6, Time: 11 ms, 0 Sum: 198
  11086800 ns / 3628800 = 3 ns/factoradic, Work/Th:   79, Pages:   45934, Last Work:   14, Time: 11 ms, 0 Sum: 198
  11640100 ns / 3628800 = 3 ns/factoradic, Work/Th:   80, Pages:   45360, Last Work:    0, Time: 11 ms, 0 Sum: 198
   9319700 ns / 3628800 = 3 ns/factoradic, Work/Th:   81, Pages:   44800, Last Work:    0, Time:  9 ms, 0 Sum: 198
  11147600 ns / 3628800 = 3 ns/factoradic, Work/Th:   82, Pages:   44253, Last Work:   54, Time: 11 ms, 0 Sum: 198
  11407700 ns / 3628800 = 3 ns/factoradic, Work/Th:   83, Pages:   43720, Last Work:   40, Time: 11 ms, 0 Sum: 198
  11222300 ns / 3628800 = 3 ns/factoradic, Work/Th:   84, Pages:   43200, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11212100 ns / 3628800 = 3 ns/factoradic, Work/Th:   85, Pages:   42691, Last Work:   65, Time: 11 ms, 0 Sum: 198
  11241900 ns / 3628800 = 3 ns/factoradic, Work/Th:   86, Pages:   42195, Last Work:   30, Time: 11 ms, 0 Sum: 198
  11659800 ns / 3628800 = 3 ns/factoradic, Work/Th:   87, Pages:   41710, Last Work:   30, Time: 11 ms, 0 Sum: 198
  12043400 ns / 3628800 = 3 ns/factoradic, Work/Th:   88, Pages:   41236, Last Work:   32, Time: 12 ms, 0 Sum: 198
  11316000 ns / 3628800 = 3 ns/factoradic, Work/Th:   89, Pages:   40773, Last Work:    3, Time: 11 ms, 0 Sum: 198
  12332500 ns / 3628800 = 3 ns/factoradic, Work/Th:   90, Pages:   40320, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11448000 ns / 3628800 = 3 ns/factoradic, Work/Th:   91, Pages:   39876, Last Work:   84, Time: 11 ms, 0 Sum: 198
  11449200 ns / 3628800 = 3 ns/factoradic, Work/Th:   92, Pages:   39443, Last Work:   44, Time: 11 ms, 0 Sum: 198
  11361900 ns / 3628800 = 3 ns/factoradic, Work/Th:   93, Pages:   39019, Last Work:   33, Time: 11 ms, 0 Sum: 198
  11382600 ns / 3628800 = 3 ns/factoradic, Work/Th:   94, Pages:   38604, Last Work:   24, Time: 11 ms, 0 Sum: 198
  12012600 ns / 3628800 = 3 ns/factoradic, Work/Th:   95, Pages:   38197, Last Work:   85, Time: 12 ms, 0 Sum: 198
  11479900 ns / 3628800 = 3 ns/factoradic, Work/Th:   96, Pages:   37800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12490900 ns / 3628800 = 3 ns/factoradic, Work/Th:   97, Pages:   37410, Last Work:   30, Time: 12 ms, 0 Sum: 198
  11342500 ns / 3628800 = 3 ns/factoradic, Work/Th:   98, Pages:   37028, Last Work:   56, Time: 11 ms, 0 Sum: 198
  11379000 ns / 3628800 = 3 ns/factoradic, Work/Th:   99, Pages:   36654, Last Work:   54, Time: 11 ms, 0 Sum: 198
  11101000 ns / 3628800 = 3 ns/factoradic, Work/Th:  100, Pages:   36288, Last Work:    0, Time: 11 ms, 0 Sum: 198
  10081900 ns / 3628800 = 3 ns/factoradic, Work/Th:  101, Pages:   35928, Last Work:   72, Time: 10 ms, 0 Sum: 198
  11265000 ns / 3628800 = 3 ns/factoradic, Work/Th:  102, Pages:   35576, Last Work:   48, Time: 11 ms, 0 Sum: 198
  11352100 ns / 3628800 = 3 ns/factoradic, Work/Th:  103, Pages:   35231, Last Work:    7, Time: 11 ms, 0 Sum: 198
  11382400 ns / 3628800 = 3 ns/factoradic, Work/Th:  104, Pages:   34892, Last Work:   32, Time: 11 ms, 0 Sum: 198
  11263500 ns / 3628800 = 3 ns/factoradic, Work/Th:  105, Pages:   34560, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11240600 ns / 3628800 = 3 ns/factoradic, Work/Th:  106, Pages:   34233, Last Work:  102, Time: 11 ms, 0 Sum: 198
  11225300 ns / 3628800 = 3 ns/factoradic, Work/Th:  107, Pages:   33914, Last Work:    2, Time: 11 ms, 0 Sum: 198
  11438400 ns / 3628800 = 3 ns/factoradic, Work/Th:  108, Pages:   33600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11390100 ns / 3628800 = 3 ns/factoradic, Work/Th:  109, Pages:   33291, Last Work:   81, Time: 11 ms, 0 Sum: 198
  11502600 ns / 3628800 = 3 ns/factoradic, Work/Th:  110, Pages:   32989, Last Work:   10, Time: 11 ms, 0 Sum: 198
  14385800 ns / 3628800 = 4 ns/factoradic, Work/Th:  111, Pages:   32691, Last Work:   99, Time: 14 ms, 0 Sum: 198
  11614600 ns / 3628800 = 3 ns/factoradic, Work/Th:  112, Pages:   32400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11497400 ns / 3628800 = 3 ns/factoradic, Work/Th:  113, Pages:   32113, Last Work:   31, Time: 11 ms, 0 Sum: 198
  11401100 ns / 3628800 = 3 ns/factoradic, Work/Th:  114, Pages:   31831, Last Work:   66, Time: 11 ms, 0 Sum: 198
  11003500 ns / 3628800 = 3 ns/factoradic, Work/Th:  115, Pages:   31554, Last Work:   90, Time: 11 ms, 0 Sum: 198
  11550900 ns / 3628800 = 3 ns/factoradic, Work/Th:  116, Pages:   31282, Last Work:   88, Time: 11 ms, 0 Sum: 198
  11275400 ns / 3628800 = 3 ns/factoradic, Work/Th:  117, Pages:   31015, Last Work:   45, Time: 11 ms, 0 Sum: 198
  11009900 ns / 3628800 = 3 ns/factoradic, Work/Th:  118, Pages:   30752, Last Work:   64, Time: 11 ms, 0 Sum: 198
  11369000 ns / 3628800 = 3 ns/factoradic, Work/Th:  119, Pages:   30494, Last Work:   14, Time: 11 ms, 0 Sum: 198
  11248100 ns / 3628800 = 3 ns/factoradic, Work/Th:  120, Pages:   30240, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11291200 ns / 3628800 = 3 ns/factoradic, Work/Th:  121, Pages:   29990, Last Work:   10, Time: 11 ms, 0 Sum: 198
  11149100 ns / 3628800 = 3 ns/factoradic, Work/Th:  122, Pages:   29744, Last Work:   32, Time: 11 ms, 0 Sum: 198
  11523400 ns / 3628800 = 3 ns/factoradic, Work/Th:  123, Pages:   29502, Last Work:   54, Time: 11 ms, 0 Sum: 198
  11346900 ns / 3628800 = 3 ns/factoradic, Work/Th:  124, Pages:   29264, Last Work:   64, Time: 11 ms, 0 Sum: 198
  11325900 ns / 3628800 = 3 ns/factoradic, Work/Th:  125, Pages:   29030, Last Work:   50, Time: 11 ms, 0 Sum: 198
  11532800 ns / 3628800 = 3 ns/factoradic, Work/Th:  126, Pages:   28800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11542400 ns / 3628800 = 3 ns/factoradic, Work/Th:  127, Pages:   28573, Last Work:   29, Time: 11 ms, 0 Sum: 198
  11731800 ns / 3628800 = 3 ns/factoradic, Work/Th:  128, Pages:   28350, Last Work:    0, Time: 11 ms, 0 Sum: 198

Chunk Size: 100 .. 1000, step 100
  13017200 ns / 3628800 = 4 ns/factoradic, Work/Th:  100, Pages:   36288, Last Work:    0, Time: 13 ms, 0 Sum: 198
  11698100 ns / 3628800 = 3 ns/factoradic, Work/Th:  200, Pages:   18144, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11767500 ns / 3628800 = 3 ns/factoradic, Work/Th:  300, Pages:   12096, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11284000 ns / 3628800 = 3 ns/factoradic, Work/Th:  400, Pages:    9072, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11730100 ns / 3628800 = 3 ns/factoradic, Work/Th:  500, Pages:    7257, Last Work:  300, Time: 11 ms, 0 Sum: 198
  11592700 ns / 3628800 = 3 ns/factoradic, Work/Th:  600, Pages:    6048, Last Work:    0, Time: 11 ms, 0 Sum: 198
  10988300 ns / 3628800 = 3 ns/factoradic, Work/Th:  700, Pages:    5184, Last Work:    0, Time: 10 ms, 0 Sum: 198
  11057700 ns / 3628800 = 3 ns/factoradic, Work/Th:  800, Pages:    4536, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12170300 ns / 3628800 = 3 ns/factoradic, Work/Th:  900, Pages:    4032, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11533900 ns / 3628800 = 3 ns/factoradic, Work/Th: 1000, Pages:    3628, Last Work:  800, Time: 11 ms, 0 Sum: 198
  11162000 ns / 3628800 = 3 ns/factoradic, Work/Th: 1100, Pages:    3298, Last Work: 1000, Time: 11 ms, 0 Sum: 198
  11885800 ns / 3628800 = 3 ns/factoradic, Work/Th: 1200, Pages:    3024, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11322000 ns / 3628800 = 3 ns/factoradic, Work/Th: 1300, Pages:    2791, Last Work:  500, Time: 11 ms, 0 Sum: 198
  12173200 ns / 3628800 = 3 ns/factoradic, Work/Th: 1400, Pages:    2592, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11985500 ns / 3628800 = 3 ns/factoradic, Work/Th: 1500, Pages:    2419, Last Work:  300, Time: 11 ms, 0 Sum: 198
  11530100 ns / 3628800 = 3 ns/factoradic, Work/Th: 1600, Pages:    2268, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11556600 ns / 3628800 = 3 ns/factoradic, Work/Th: 1700, Pages:    2134, Last Work: 1000, Time: 11 ms, 0 Sum: 198
   9519900 ns / 3628800 = 3 ns/factoradic, Work/Th: 1800, Pages:    2016, Last Work:    0, Time:  9 ms, 0 Sum: 198
  12860800 ns / 3628800 = 4 ns/factoradic, Work/Th: 1900, Pages:    1909, Last Work: 1700, Time: 12 ms, 0 Sum: 198
  12865300 ns / 3628800 = 4 ns/factoradic, Work/Th: 2000, Pages:    1814, Last Work:  800, Time: 12 ms, 0 Sum: 198
  15676300 ns / 3628800 = 4 ns/factoradic, Work/Th: 2100, Pages:    1728, Last Work:    0, Time: 15 ms, 0 Sum: 198
  14001100 ns / 3628800 = 4 ns/factoradic, Work/Th: 2200, Pages:    1649, Last Work: 1000, Time: 14 ms, 0 Sum: 198
  16303200 ns / 3628800 = 4 ns/factoradic, Work/Th: 2300, Pages:    1577, Last Work: 1700, Time: 16 ms, 0 Sum: 198
  11264700 ns / 3628800 = 3 ns/factoradic, Work/Th: 2400, Pages:    1512, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11979200 ns / 3628800 = 3 ns/factoradic, Work/Th: 2500, Pages:    1451, Last Work: 1300, Time: 11 ms, 0 Sum: 198
  10792500 ns / 3628800 = 3 ns/factoradic, Work/Th: 2600, Pages:    1395, Last Work: 1800, Time: 10 ms, 0 Sum: 198
  12171600 ns / 3628800 = 3 ns/factoradic, Work/Th: 2700, Pages:    1344, Last Work:    0, Time: 12 ms, 0 Sum: 198
  12329000 ns / 3628800 = 3 ns/factoradic, Work/Th: 2800, Pages:    1296, Last Work:    0, Time: 12 ms, 0 Sum: 198
  12296900 ns / 3628800 = 3 ns/factoradic, Work/Th: 2900, Pages:    1251, Last Work:  900, Time: 12 ms, 0 Sum: 198
  12872300 ns / 3628800 = 4 ns/factoradic, Work/Th: 3000, Pages:    1209, Last Work: 1800, Time: 12 ms, 0 Sum: 198
  12233800 ns / 3628800 = 3 ns/factoradic, Work/Th: 3100, Pages:    1170, Last Work: 1800, Time: 12 ms, 0 Sum: 198
  12150800 ns / 3628800 = 3 ns/factoradic, Work/Th: 3200, Pages:    1134, Last Work:    0, Time: 12 ms, 0 Sum: 198
  11798500 ns / 3628800 = 3 ns/factoradic, Work/Th: 3300, Pages:    1099, Last Work: 2100, Time: 11 ms, 0 Sum: 198
  11663500 ns / 3628800 = 3 ns/factoradic, Work/Th: 3400, Pages:    1067, Last Work: 1000, Time: 11 ms, 0 Sum: 198
  11031800 ns / 3628800 = 3 ns/factoradic, Work/Th: 3500, Pages:    1036, Last Work: 2800, Time: 11 ms, 0 Sum: 198
  13473000 ns / 3628800 = 4 ns/factoradic, Work/Th: 3600, Pages:    1008, Last Work:    0, Time: 13 ms, 0 Sum: 198
  11000600 ns / 3628800 = 3 ns/factoradic, Work/Th: 3700, Pages:     980, Last Work: 2800, Time: 11 ms, 0 Sum: 198
  11655800 ns / 3628800 = 3 ns/factoradic, Work/Th: 3800, Pages:     954, Last Work: 3600, Time: 11 ms, 0 Sum: 198
  12196100 ns / 3628800 = 3 ns/factoradic, Work/Th: 3900, Pages:     930, Last Work: 1800, Time: 12 ms, 0 Sum: 198
  12200600 ns / 3628800 = 3 ns/factoradic, Work/Th: 4000, Pages:     907, Last Work:  800, Time: 12 ms, 0 Sum: 198
  12146400 ns / 3628800 = 3 ns/factoradic, Work/Th: 4100, Pages:     885, Last Work:  300, Time: 12 ms, 0 Sum: 198
  11290900 ns / 3628800 = 3 ns/factoradic, Work/Th: 4200, Pages:     864, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11907500 ns / 3628800 = 3 ns/factoradic, Work/Th: 4300, Pages:     843, Last Work: 3900, Time: 11 ms, 0 Sum: 198
  12012000 ns / 3628800 = 3 ns/factoradic, Work/Th: 4400, Pages:     824, Last Work: 3200, Time: 12 ms, 0 Sum: 198
  11907800 ns / 3628800 = 3 ns/factoradic, Work/Th: 4500, Pages:     806, Last Work: 1800, Time: 11 ms, 0 Sum: 198
  10902200 ns / 3628800 = 3 ns/factoradic, Work/Th: 4600, Pages:     788, Last Work: 4000, Time: 10 ms, 0 Sum: 198
  10140400 ns / 3628800 = 3 ns/factoradic, Work/Th: 4700, Pages:     772, Last Work:  400, Time: 10 ms, 0 Sum: 198
  12758600 ns / 3628800 = 4 ns/factoradic, Work/Th: 4800, Pages:     756, Last Work:    0, Time: 12 ms, 0 Sum: 198
  10884200 ns / 3628800 = 3 ns/factoradic, Work/Th: 4900, Pages:     740, Last Work: 2800, Time: 10 ms, 0 Sum: 198
  11792300 ns / 3628800 = 3 ns/factoradic, Work/Th: 5000, Pages:     725, Last Work: 3800, Time: 11 ms, 0 Sum: 198
  12582900 ns / 3628800 = 3 ns/factoradic, Work/Th: 5100, Pages:     711, Last Work: 2700, Time: 12 ms, 0 Sum: 198
  11798300 ns / 3628800 = 3 ns/factoradic, Work/Th: 5200, Pages:     697, Last Work: 4400, Time: 11 ms, 0 Sum: 198
  11414000 ns / 3628800 = 3 ns/factoradic, Work/Th: 5300, Pages:     684, Last Work: 3600, Time: 11 ms, 0 Sum: 198
  11791600 ns / 3628800 = 3 ns/factoradic, Work/Th: 5400, Pages:     672, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12650700 ns / 3628800 = 3 ns/factoradic, Work/Th: 5500, Pages:     659, Last Work: 4300, Time: 12 ms, 0 Sum: 198
  11604000 ns / 3628800 = 3 ns/factoradic, Work/Th: 5600, Pages:     648, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11502000 ns / 3628800 = 3 ns/factoradic, Work/Th: 5700, Pages:     636, Last Work: 3600, Time: 11 ms, 0 Sum: 198
  12488600 ns / 3628800 = 3 ns/factoradic, Work/Th: 5800, Pages:     625, Last Work: 3800, Time: 12 ms, 0 Sum: 198
  12248600 ns / 3628800 = 3 ns/factoradic, Work/Th: 5900, Pages:     615, Last Work:  300, Time: 12 ms, 0 Sum: 198
  11147500 ns / 3628800 = 3 ns/factoradic, Work/Th: 6000, Pages:     604, Last Work: 4800, Time: 11 ms, 0 Sum: 198
  10894400 ns / 3628800 = 3 ns/factoradic, Work/Th: 6100, Pages:     594, Last Work: 5400, Time: 10 ms, 0 Sum: 198
  10764500 ns / 3628800 = 3 ns/factoradic, Work/Th: 6200, Pages:     585, Last Work: 1800, Time: 10 ms, 0 Sum: 198
  12138500 ns / 3628800 = 3 ns/factoradic, Work/Th: 6300, Pages:     576, Last Work:    0, Time: 12 ms, 0 Sum: 198
  10681000 ns / 3628800 = 3 ns/factoradic, Work/Th: 6400, Pages:     567, Last Work:    0, Time: 10 ms, 0 Sum: 198
  11879000 ns / 3628800 = 3 ns/factoradic, Work/Th: 6500, Pages:     558, Last Work: 1800, Time: 11 ms, 0 Sum: 198
  11844300 ns / 3628800 = 3 ns/factoradic, Work/Th: 6600, Pages:     549, Last Work: 5400, Time: 11 ms, 0 Sum: 198
  12848800 ns / 3628800 = 4 ns/factoradic, Work/Th: 6700, Pages:     541, Last Work: 4100, Time: 12 ms, 0 Sum: 198
  11779500 ns / 3628800 = 3 ns/factoradic, Work/Th: 6800, Pages:     533, Last Work: 4400, Time: 11 ms, 0 Sum: 198
  11950200 ns / 3628800 = 3 ns/factoradic, Work/Th: 6900, Pages:     525, Last Work: 6300, Time: 11 ms, 0 Sum: 198
  12036100 ns / 3628800 = 3 ns/factoradic, Work/Th: 7000, Pages:     518, Last Work: 2800, Time: 12 ms, 0 Sum: 198
  11298000 ns / 3628800 = 3 ns/factoradic, Work/Th: 7100, Pages:     511, Last Work:  700, Time: 11 ms, 0 Sum: 198
  12257200 ns / 3628800 = 3 ns/factoradic, Work/Th: 7200, Pages:     504, Last Work:    0, Time: 12 ms, 0 Sum: 198
  12221100 ns / 3628800 = 3 ns/factoradic, Work/Th: 7300, Pages:     497, Last Work:  700, Time: 12 ms, 0 Sum: 198
  11277400 ns / 3628800 = 3 ns/factoradic, Work/Th: 7400, Pages:     490, Last Work: 2800, Time: 11 ms, 0 Sum: 198
  11351600 ns / 3628800 = 3 ns/factoradic, Work/Th: 7500, Pages:     483, Last Work: 6300, Time: 11 ms, 0 Sum: 198
  11919900 ns / 3628800 = 3 ns/factoradic, Work/Th: 7600, Pages:     477, Last Work: 3600, Time: 11 ms, 0 Sum: 198
  10780800 ns / 3628800 = 3 ns/factoradic, Work/Th: 7700, Pages:     471, Last Work: 2100, Time: 10 ms, 0 Sum: 198
  11063800 ns / 3628800 = 3 ns/factoradic, Work/Th: 7800, Pages:     465, Last Work: 1800, Time: 11 ms, 0 Sum: 198
  10887600 ns / 3628800 = 3 ns/factoradic, Work/Th: 7900, Pages:     459, Last Work: 2700, Time: 10 ms, 0 Sum: 198
  11264800 ns / 3628800 = 3 ns/factoradic, Work/Th: 8000, Pages:     453, Last Work: 4800, Time: 11 ms, 0 Sum: 198
  11612300 ns / 3628800 = 3 ns/factoradic, Work/Th: 8100, Pages:     448, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11525300 ns / 3628800 = 3 ns/factoradic, Work/Th: 8200, Pages:     442, Last Work: 4400, Time: 11 ms, 0 Sum: 198
  12237000 ns / 3628800 = 3 ns/factoradic, Work/Th: 8300, Pages:     437, Last Work: 1700, Time: 12 ms, 0 Sum: 198
  11897400 ns / 3628800 = 3 ns/factoradic, Work/Th: 8400, Pages:     432, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11960300 ns / 3628800 = 3 ns/factoradic, Work/Th: 8500, Pages:     426, Last Work: 7800, Time: 11 ms, 0 Sum: 198
  12776000 ns / 3628800 = 4 ns/factoradic, Work/Th: 8600, Pages:     421, Last Work: 8200, Time: 12 ms, 0 Sum: 198
  12312900 ns / 3628800 = 3 ns/factoradic, Work/Th: 8700, Pages:     417, Last Work:  900, Time: 12 ms, 0 Sum: 198
  11145600 ns / 3628800 = 3 ns/factoradic, Work/Th: 8800, Pages:     412, Last Work: 3200, Time: 11 ms, 0 Sum: 198
  12239500 ns / 3628800 = 3 ns/factoradic, Work/Th: 8900, Pages:     407, Last Work: 6500, Time: 12 ms, 0 Sum: 198
  11931100 ns / 3628800 = 3 ns/factoradic, Work/Th: 9000, Pages:     403, Last Work: 1800, Time: 11 ms, 0 Sum: 198
  10116300 ns / 3628800 = 3 ns/factoradic, Work/Th: 9100, Pages:     398, Last Work: 7000, Time: 10 ms, 0 Sum: 198
  11246900 ns / 3628800 = 3 ns/factoradic, Work/Th: 9200, Pages:     394, Last Work: 4000, Time: 11 ms, 0 Sum: 198
  10918900 ns / 3628800 = 3 ns/factoradic, Work/Th: 9300, Pages:     390, Last Work: 1800, Time: 10 ms, 0 Sum: 198
  12440100 ns / 3628800 = 3 ns/factoradic, Work/Th: 9400, Pages:     386, Last Work:  400, Time: 12 ms, 0 Sum: 198
  11230300 ns / 3628800 = 3 ns/factoradic, Work/Th: 9500, Pages:     381, Last Work: 9300, Time: 11 ms, 0 Sum: 198
  11796700 ns / 3628800 = 3 ns/factoradic, Work/Th: 9600, Pages:     378, Last Work:    0, Time: 11 ms, 0 Sum: 198
  12145000 ns / 3628800 = 3 ns/factoradic, Work/Th: 9700, Pages:     374, Last Work: 1000, Time: 12 ms, 0 Sum: 198
  12224600 ns / 3628800 = 3 ns/factoradic, Work/Th: 9800, Pages:     370, Last Work: 2800, Time: 12 ms, 0 Sum: 198
  10746100 ns / 3628800 = 3 ns/factoradic, Work/Th: 9900, Pages:     366, Last Work: 5400, Time: 10 ms, 0 Sum: 198
  11621100 ns / 3628800 = 3 ns/factoradic, Work/Th:10000, Pages:     362, Last Work: 8800, Time: 11 ms, 0 Sum: 198

Chunk Size: 256 (worst, avg, best)
  10884200 ns / 3628800 = 3 ns/factoradic, Work/Th:  256, Pages:   14175, Last Work:    0, Time: 10 ms, 0 Sum: 198
  12156000 ns / 3628800 = 3 ns/factoradic, Work/Th:  256, Pages:   14175, Last Work:    0, Time: 12 ms, 0 Sum: 198
  15354100 ns / 3628800 = 4 ns/factoradic, Work/Th:  256, Pages:   14175, Last Work:    0, Time: 15 ms, 0 Sum: 198

Chunk Size: Pow2
  13445800 ns / 3628800 = 4 ns/factoradic, Work/Th:    1, Pages: 3628800, Last Work:    0, Time: 13 ms, 0 Sum: 198
  11514500 ns / 3628800 = 3 ns/factoradic, Work/Th:    2, Pages: 1814400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11166900 ns / 3628800 = 3 ns/factoradic, Work/Th:    4, Pages:  907200, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11168100 ns / 3628800 = 3 ns/factoradic, Work/Th:    8, Pages:  453600, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11618800 ns / 3628800 = 3 ns/factoradic, Work/Th:   16, Pages:  226800, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11556700 ns / 3628800 = 3 ns/factoradic, Work/Th:   32, Pages:  113400, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11215700 ns / 3628800 = 3 ns/factoradic, Work/Th:   64, Pages:   56700, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11151100 ns / 3628800 = 3 ns/factoradic, Work/Th:  128, Pages:   28350, Last Work:    0, Time: 11 ms, 0 Sum: 198
  11154900 ns / 3628800 = 3 ns/factoradic, Work/Th:  256, Pages:   14175, Last Work:    0, Time: 11 ms, 0 Sum: 198
  10383200 ns / 3628800 = 3 ns/factoradic, Work/Th:  512, Pages:    7087, Last Work:  256, Time: 10 ms, 0 Sum: 198
  10951900 ns / 3628800 = 3 ns/factoradic, Work/Th: 1024, Pages:    3543, Last Work:  768, Time: 10 ms, 0 Sum: 198
  10974100 ns / 3628800 = 3 ns/factoradic, Work/Th: 2048, Pages:    1771, Last Work: 1792, Time: 10 ms, 0 Sum: 198
  13108300 ns / 3628800 = 4 ns/factoradic, Work/Th: 4096, Pages:     885, Last Work: 3840, Time: 13 ms, 0 Sum: 198
*/