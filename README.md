# CS 4440: Operating Systems - Project 2 
Table of Contents
------------
1) airline.c
   - Purpose: Emulation of the operations of an unfair, but (hopefully) extermely inexpensive airline using 
threads and semaphores. Passengers each go through airline processing in stages: baggage -> security -> boarding. Semaphores and mutexes are used to prevent their being more passangers than there are workers at a given stage at a given time. After all the passengers have successfuly boarded the aircraft and the airline workers have deboarded, the aircraft will commence takeoff.
   - Compile: gcc -pthread -o airline airline.c
   - Run:     ./airline P B S F (where P, B, S and F are unsigned integers [for example, ./airline 200 22 2 20])

2) bounded_buffer.c
   - Purpose: Decompress files produced by MyCompress, expanding +n+ to n ones and -n- to n zeros; other chars stay the same.
   - Compile: gcc -pthread -o bounded_buffer bounded_buffer.c
   - Run:     ./MyDecompress <compressed_file> <decompressed_file>

3) mother_hubbard.c
   - Purpose: Two-process fork version of the compressor (MyCompress) where the child will do everything (child labor) and the parent will wait until    child process is done and report whether or not it succeeded.
   - Compile: gcc -pthread -o mother_hubbard mother_hubbard.c
   - Run:     ./ForkCompress <source_file> <destination_file>


Other(s)/Notes
--------------------
- test_runs.txt — Typescript file showing the working of all the programs for correct input as well as graceful exit on error input.
- PRJ1README - Glossary of Project 1

