-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM:

vg ./hw5 -i inputFile1.txt -j inputFile2.txt -o outputFile.csv -n 4 -m 4

****************************************************************************************************************************************************