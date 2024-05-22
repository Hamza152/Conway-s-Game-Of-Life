# Conway-s-Game-of-Life-
A C++ implementation of John Horton Conway's Game of Life simulation using Win32 API. Big enough grid to explore patterns, includes real-time animation, and gif saving possibilities.  
I used single header libraries "stb_image.h" for readng ".png" images and "gif.h" for gif saving purposes. Win32 API is used mainly for window creation and messages dispatching, but also for basic file opening and saving dialog boxes, customized versions are planned for the latter in future enhancements. 

# Introduction
Game of life is a cellular automaton, which means a mathematical model of computation that consists of a grid of where each cell is one of a finite number of states. The evolution of the grid is done in a discrete step by step manner based on a specific set of rules. It can simulate any Turing machine and that's why it is Turing complete.  
For Conway's Game of Life, the grid is 2D, and each cell can have one of two possible states "dead" or "alive". The specific rules that determine the state of the grid in the next step are as follows :  
&emsp;&emsp;&emsp;&emsp; 1. Any live cell with fewer than two live neighbors dies, as if by underpopulation.  
&emsp;&emsp;&emsp;&emsp; 2. Any live cell with two or three live neighbors lives on to the next generation.  
&emsp;&emsp;&emsp;&emsp; 3. Any live cell with more than three live neighbors dies, as if by overpopulation.  
&emsp;&emsp;&emsp;&emsp; 4. Any dead cell with exactly three live neighbors becomes a live cell, as if by reproduction.  

# Getting started
To start, launch the executable in "build" folder. Or  build it yourself by launching the "build.bat" file in "code" folder.    
  
As of the UI:   
&emsp;&emsp; * Start by creating an initial generation by selecting which cells you want alive, and from there you can:   
&emsp;&emsp;&emsp;&emsp; - Explore the evolution step by step.  
&emsp;&emsp;&emsp;&emsp; - Jump to a specific generation.  
&emsp;&emsp;&emsp;&emsp; - Launch the simulation with the desired speed.   
&emsp;&emsp;* Left click mouse and drag or use the overview map on the bottom right to navigate through grid.   
&emsp;&emsp;* Zoom In and out using the mouse wheel.   
&emsp;&emsp;* Save patterns you find interesting either as:   
&emsp;&emsp;&emsp;&emsp; - ".txt" format which can be opened by the application. Useful for later exploration and for sharing patterns with friends.
&emsp;&emsp;&emsp;&emsp; - ".gif" animation specifying the desired area, number of generations and the simulation speed.    
