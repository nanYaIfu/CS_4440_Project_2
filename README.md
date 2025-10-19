# CS 4440: Operating Systems - Project 2 
Table of Contents
------------
1) airline.c
   - Purpose: Emulation of the operations of an unfair, but (hopefully) extermely inexpensive airline using 
threads and semaphores. Passengers each go through airline processing in stages: baggage -> security -> boarding. Semaphores and mutexes are used to prevent their being more passangers than there are workers at a given stage at a given time. After all the passengers have successfuly boarded the aircraft and the airline workers have deboarded, the aircraft will commence takeoff.
   - Compile: gcc -pthread -o airline airline.c
   - Run:     ./airline P B S F (where P, B, S and F are some unsigned integer [for example, ./airline 200 22 2 20])

2) bounded_buffer.c
   - Purpose: Implementation of Producer-Consumer synchronization problem (bounded-buffer version); contains a control system that
initializes a buffer and the synchronization variables, then creates and terminates the threads running for the producer/consumer
   - Compile: gcc -pthread -o bounded_buffer bounded_buffer.c
   - Run:     ./bounded_buffer

3) mother_hubbard.c
   - Purpose: Emulation of a very strict, inconvenient household workflow for taking care of children. The program takes an input N for the number of cycles the program will run. The mother and father alternate between taking car of their 12 children and resting using mutexes and semaphores.
   - Compile: gcc -pthread -o mother_hubbard mother_hubbard.c
   - Run:     ./mother_hubbard N (where N is some unsigned integer [for example, ./mother_hubbard 2])


Other(s)/Notes
--------------------
- type_script.sh — Typescript file showing the working of all the programs for correct input as well as graceful exit on error input.
- makefile - Text file for project compilation purposes [using make()]
- README.md - Glossary of Project 2

