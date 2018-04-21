# INtecplot_to_STL
A quick and dirty conversion utility code

This code is meant to perform a simple conversion from quadrilateral elements
given in an unstructured format readable by AMTEC's TecPlot and convert them to
an STL file that can be used by other pieces of software. In order to operate
properly it needs to be modified to parse the header of the input file, or the
file given as input can be formatted such that this code (main.c) can read it.

The code can be modified very easily to read and convert triangular elements
and act like a simple converter.

Do not be scared by having to modify the code; it is easily readable and it is
easy to know where modifications are needd. I hope people find this useful.

IN 2018/04/21 (Record Store Day!)
