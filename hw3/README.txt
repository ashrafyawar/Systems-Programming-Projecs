-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM:

- FOR NAMED SEMAPHORE TEST RUN:

vg ./hw3named -i ingredients.txt -n sem

- FOR UNNNAMED SEMAPHORE TEST RUN:

vg ./hw3unnamed -i ingredients.txt

****************************************************************************************************************************************************