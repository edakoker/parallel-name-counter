 Author names: Naya Singhania, Gonul Koker
 Author emails: naya.singhania@sjsu.edu, gonul.koker@sjsu.edu
 Last modified date: October 13, 2025

 Purpose of code:This program, countnames_parallel, reads one or more text files specified as command-line arguments. It counts the occurrences of each name (one name per line) across all provided files.
 The program uses multi-processing to handle files in parallel. It creates a new child process using fork() for each input file. Child processes send their results back to the parent process through a pipe(). The parent process aggregates the counts from all children and prints the final summary to standard output.

HOW TO RUN
compilation:
gcc -o countnames_parallel countnames_parallel.c -Wall -Werror

general usage of test cases:
./countnames_parallel test/file1.txt test/file2.txt ...

Test Cases and Expected Outputs:

1.
./countnames_parallel test/names1.txt test/names2.txt

Warning - file test/names1.txt line 3 is empty.
Tom Wu: 4
Jenn Xu: 2

2.
./countnames_parallel test/names1.txt test/names2.txt test/names2.txt

Warning - file test/names1.txt line 3 is empty.
Tom Wu: 5
Jenn Xu: 4

3.
./countnames_parallel test/names1.txt non_existent_file.txt test/namesB.txt

Warning - file test/names1.txt line 3 is empty.
error: cannot open file non_existent_file.txt
Warning - file test/namesB.txt line 2 is empty.
Warning - file test/namesB.txt line 5 is empty.
Tom Wu: 6
Nicky: 1
Dave Joe: 2
Yuan Cheng Chang: 3
John Smith: 1

4.
./countnames_parallel
(If no files are given, the program reads from stdin in here)

5.
time ./countnames_parallel test/names_long_redundant.txt test/names_long_redundant1.txt
(to measure the execution time on large files)

Warning - file test/names_long_redundant.txt line 97 is empty.
Warning - file test/names_long_redundant1.txt line 2 is empty.
... (additional warnings may appear on stderr) ...

John Smith: 1
Emily Davis: 1
Michael Johnson: 1
... (many more lines of name counts) ...
MARGARET MOORE: 1

real    0m0.003s
user    0m0.001s
sys     0m0.002s
