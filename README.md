# Grid-Based-Route-Planning
A project to practice dynamic memory and linked lists in C. This program takes stdin input (available via txt files) that specifies the size of a grid, a beginning point, a desired end point, and a route that an autonomous agent will take. Inside the grid, there may be blocks to the route. In such a situtaion, my program reconstructs the given route so that the agent can reach the end.

## Input
The ﬁrst line of the input encodes the dimensions of the grid, for
example 10x20 speciﬁes that the grid has 10 rows and 20 columns. 

The second line of the input encodes the initial cell of the agent in the grid, while the third line encodes the goal cell. A cell is encoded using the format [r,c], where r and c are numbers that stand for, respectively, the row and column of the cell. 

Subsequent input lines specify positions of blocks in the grid, one block cell per line. 

An input line with a single character $ denotes the end of block lines. The input lines that follow encode a route of the agent in the grid. A route is encoded by alternating cells and ->, for example [0,0]->[0,1]->[0,2] encodes the route that starts at cell [0,0], and then proceeds via cell [0,1]to cell [0,2]; no blanks or tabs are used in encodings of routes, while a single newline character may follow ->.

