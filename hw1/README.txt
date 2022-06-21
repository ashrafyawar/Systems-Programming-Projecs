
-HOW TO RUN THE PROGRAM: 
> Run below commands in order:

alias vg='valgrind --leak-check=full -v --track-origins=yes --log-file=vg_logfile.out'

make

****************************************************************************************************************************************************

- HOW TO TEST THE PROGRAM WITH DIFFERENT INPUTS:
> You can paste each of the below test commands and see the resutl:
> NOTE: if we put ';' at the end of the replacement command, it will replace all the pairs of strings otherwise it will replace one less.

vg ./editorcompiled '/Torem/lorem/;' input_file.txt

vg ./editorcompiled '/Torem/lorem/i;' input_file.txt

vg ./editorcompiled '/^do/EEEEE/;' input_file.txt

vg ./editorcompiled '/do$/EEEEE/;' input_file.txt

vg ./editorcompiled '/yes[sz]tttt/BRACES/;' input_file.txt

vg ./editorcompiled '/Torem/lorem/i;/^do/EEEEE/;/do$/EEEEE/;/yes[sz]tttt/BRACES/;' input_file.txt

vg ./editorcompiled '/Torem/lorem/i;/^do/EEEEE/;' input_file.txt

vg ./editorcompiled '/Torem/lorem/i;/^do/EEEEE/;/do$/EEEEE/;' input_file.txt

vg ./editorcompiled '/Torem/lorem/i;/^do/EEEEE/;/do$/EEEEE/;/yes[sz]tttt/BRACES/;' input_file.txt

****************************************************************************************************************************************************

FILES:

- vg_logfile.out is used to log the valgrid results.
- input_file.txt is used as test input file.