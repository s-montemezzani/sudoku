OOP based C++ sudoku solver.
Implements the following algorithms (see http://www.sadmansoftware.com/sudoku/solvingtechniques.htm for explanations and examples): naked single, hidden single, block row/column interaction, block block interaction, naked subset and hidden subset.

Usage: 
  Write in sudoku.txt all 81 cells, 9 cells per line, using a period for unknown cells. 
    For example 
    
    4.18..3.2
    ..8.6....
    ......9..
    ..7..5.3.
    .5.974.1.
    .1.3..5..
    ..9......
    ....9.1..
    8.3..17.5
    
  Or, write in sudoku.txt all 81 cells in a single line, left to right, top to bottom, using a period for unknown cells.
    For example
    
    4.18..3.2..8.6..........9....7..5.3..5.974.1..1.3..5....9..........9.1..8.3..17.5

