
-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM WITH DIFFERENT INPUTS:

vg ./hw02compiled -i input_file.dat -o output_file.dat

****************************************************************************************************************************************************

my make file contains -lm for handle the math.h library issue.