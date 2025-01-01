if not exist output mkdir output

bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j1  > output\naive_01.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j2  > output\naive_02.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j4  > output\naive_04.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j8  > output\naive_08.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j16 > output\naive_16.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j24 > output\naive_24.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j32 > output\naive_32.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe -j48 > output\naive_48.txt
bin\naive_10digits_minimal_sum_zero\x64\Release\naive_10digits_minimal_sum_zero.exe      > output_naive.txt

bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j1  > output\factoradic_01.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j2  > output\factoradic_02.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j4  > output\factoradic_04.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j8  > output\factoradic_08.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j16 > output\factoradic_16.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j24 > output\factoradic_24.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j32 > output\factoradic_32.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe -j48 > output\factoradic_48.txt
bin\factoradic_10digits_minimal_sum_zero\x64\Release\factoradic_10digits_minimal_sum_zero.exe      > output_factoradic.txt
