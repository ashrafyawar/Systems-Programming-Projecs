
-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM WITH DIFFERENT INPUTS:

-on terminal1 paste below code:

vg ./serverY -s requests -o logs.log –p 5 -r 5 -t 2

-on terminal2 paste below code:
vg ./client -s requests -o data.csv

****************************************************************************************************************************************************