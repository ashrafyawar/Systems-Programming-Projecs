-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM:

valgrind ./hw4 -C 10 -N 3 -F inputFile.txt

OR
vg ./hw4 -C 10 -N 3 -F inputFile.txt

****************************************************************************************************************************************************