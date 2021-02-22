#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#define BLOCK '#'				/* broad character in grid*/
#define INITIAL 'I'				/* initial character in grid*/
#define GOAL 'G'				/* goal character in grid*/
#define MAX_POS_IN_LINE 5		/*max number of positions that can be printed */

/* 2 integers representing position */
typedef struct {			
	int row;
	int col;
} pos_t;			

/* initialising a node for doubly linked list*/
typedef struct node node_t;	

struct node {					
	pos_t* pos;
	node_t* next;
	node_t* prev;
};

typedef struct {
	node_t* head;
	node_t* foot;
} list_t;

typedef struct repair_node repair_node_t;

/*node for repair lists (like a normal node but with weight and a prev 
position*/ 
struct repair_node {		 
	pos_t* pos;				
	int weight;
	pos_t* prev_pos;
	repair_node_t* next;
	repair_node_t* prev;
};

/* list for repair nodes */
typedef struct {
	repair_node_t* head;
	repair_node_t* foot;
} repair_list_t;

/* set of two positions for important grid positions e.g initial and goal 
coords */
typedef struct {
	pos_t* initial;
	pos_t* goal;
} grid_pos_t;

/* information about the grid. Holds the actual grid itself and its number of
rows and columns */
typedef struct {
	char** grid;
	int nrows;
	int ncols;
} grid_info_t;

/* information about the route. Holds the route itself and its status. */
typedef struct {
	list_t* route;
	int status;
} route_info_t;

/******************************************************************************/
int check_block(node_t* node, char** grid);
void run_stage_1(route_info_t* route_info, grid_info_t* grid_info);
list_t* delete_blocks(list_t* route, grid_pos_t* block_pos);
void repair_grid(route_info_t* route_info, grid_info_t* grid_info);
repair_list_t* make_repair_chain(grid_pos_t* block_pos, grid_info_t* grid_info);
list_t* final_repair_route(grid_pos_t* block_pos, repair_list_t* repair_chain,
	list_t* route, char** grid);
list_t* insertBefore(list_t* route, pos_t* next_pos, pos_t* new_pos);
int validate_repair_pos(grid_info_t* grid_info, pos_t* adjacent_pos,
	int weight, pos_t* prev_pos, repair_list_t* repair_chain, pos_t* goal);
int pos_in_list(repair_node_t* node, pos_t* pos);
repair_list_t* insert_foot_repair_node(repair_list_t* repair_list, pos_t* pos,
	int weight, pos_t* prev_pos);
grid_pos_t* find_blocked_route(char** grid, node_t* node);
void print_grid(grid_info_t* grid_info);
route_info_t* print_info(grid_info_t* grid_info);
void check_status1_2(route_info_t* route_info, grid_pos_t* grid_pos);
grid_pos_t* read_initial_and_goal_pos(char** grid);
int print_status(int current_status);
void print_route(node_t* node);
void print_coord(pos_t* pos);
int array_to_num(char array[], int array_len);
grid_info_t* grid_to_memory();
int process_blocks(char** grid);
int mygetchar();
int read_pos(pos_t* pos);
list_t* make_empty_route(void);
list_t* insert_route_foot_pos(list_t* route, pos_t* route_pos);
int route_reader(char** grid, list_t* route, grid_pos_t* grid_pos);
void print_route_repair(repair_node_t* node);

/******************************************************************************/

int
main(int argc, char* argv[]) {
	
	grid_info_t* grid_info = grid_to_memory();
	route_info_t* route_info = print_info(grid_info);

	run_stage_1(route_info, grid_info);
}
/******************************************************************************/
/* checks if there's a block in the route by going through every node after the
given node. Returns 1 if there is a block.
*/

int
check_block(node_t* node, char** grid) {
	while (node) {
		if (grid[node->pos->row][node->pos->col] == BLOCK) {
			return 1;
		}
		node = node->next;
	}
	return 0;
}

/******************************************************************************/
/* runs stage 1. Prints grid and if the status is 4 (a block in the route),
repairs grid.
*/

void
run_stage_1(route_info_t* route_info, grid_info_t* grid_info) {
	printf("==STAGE 1=======================================\n");
	print_grid(grid_info);
	
	if (route_info->status == 4) {
		printf("------------------------------------------------\n");
		repair_grid(route_info, grid_info);
		print_route(route_info->route->head);
		if (check_block(route_info->route->head, grid_info->grid)) {
			print_status(route_info->status);
		} else {
			route_info->status = 5;
			print_status(route_info->status);
		}
	}
	
	printf("================================================");
}

/******************************************************************************/
/* function driving the grid repair. It first finds the block, then it makes
a list (repair chain) of positions adjacent to the block. After that, it 
deletes blocked positions in the original route and then inserts the new route
positions.

It then prints the new grid out and lined border underneath.
*/

void
repair_grid(route_info_t* route_info, grid_info_t* grid_info) {
	
	grid_pos_t* block_pos = find_blocked_route(grid_info->grid, 
		route_info->route->head);
	
	repair_list_t* repair_chain = make_repair_chain(block_pos, grid_info);
	
	route_info->route = delete_blocks(route_info->route, block_pos);
	
	route_info->route = final_repair_route(block_pos, repair_chain, 
		route_info->route, grid_info->grid);
	
	print_grid(grid_info);
	printf("------------------------------------------------\n");
}

/******************************************************************************/
/* deletes the next set of blocks in the route.
*/

list_t*
delete_blocks(list_t* route, grid_pos_t* block_pos) {
	node_t* del_node = malloc(sizeof(*del_node));
	assert(del_node);
	del_node->pos = malloc(sizeof(*del_node->pos));
	assert(del_node->pos);
	del_node = route->head;
	del_node->pos->row = route->head->pos->row;
	del_node->pos->col = route->head->pos->col;

	node_t* temp_node = malloc(sizeof(*del_node));
	assert(temp_node);
	
	while (del_node) {
		if (del_node->pos->row == block_pos->initial->row
			&& del_node->pos->col == block_pos->initial->col) {
			del_node = del_node->next;
			break;
		} del_node = del_node->next;
	}

	/*covers case of only 1 block deletion, returning early*/
	if (block_pos->initial->row == block_pos->goal->row
		&& block_pos->initial->col == block_pos->goal->col) {
		if (route->head == del_node) {
			route->head = del_node->next;
		}
		if (del_node->prev != NULL) {
			del_node->prev->next = del_node->next;
		}
		if (del_node->next != NULL) {
			del_node->next->prev = del_node->prev;
		}
		return route;
	}

	while (!(del_node->pos->row == block_pos->goal->row
		&& del_node->pos->col == block_pos->goal->col)) {
		if (route->head == del_node) {
			route->head = del_node->next;
		}
		if (del_node->prev != NULL) {
			del_node->prev->next = del_node->next;
		}
		if (del_node->next != NULL) {
			del_node->next->prev = del_node->prev;
		}
		else {
			break;
		}
		temp_node = del_node->next;
		free(del_node);
		del_node = temp_node;
	}

	return route;
}

/******************************************************************************/
/* creates a list of positions adjacent to the position where the route was
first blocked. Adjacent positions are added in the order: going above, below,
to the left, then to the right. The weight (number of positions away from
beginning of the block) and the prev_pos (where the adjacent position came from)
is also added to the node which is then added to the foot of the repair list.

Once an adjacent position has reached the end of the route block, it stops
adding adjacent blocks and returns the list of adjacent blocks (i.e the repair
list).
*/

repair_list_t*
make_repair_chain(grid_pos_t* block_pos, grid_info_t* grid_info) {
	repair_list_t* repair_chain = malloc(sizeof(*repair_chain));
	assert(repair_chain);
	repair_chain->head = repair_chain->foot = NULL;

	int weight = 0;
	
	if (validate_repair_pos(grid_info, block_pos->initial,  weight, block_pos->goal, repair_chain, block_pos->goal)) {
		return repair_chain;
	}

	pos_t* adjacent_pos = malloc(sizeof(*adjacent_pos));
	assert(adjacent_pos);

	repair_node_t* node = malloc(sizeof(*node));
	assert(node);
	node = repair_chain->head;
	
	while (node->pos) {
		weight = node->weight + 1;

		adjacent_pos->row = node->pos->row - 1;
		adjacent_pos->col = node->pos->col;
		if (validate_repair_pos(grid_info, adjacent_pos, weight, node->pos,
			repair_chain, block_pos->goal)) {
			break;
		}

		adjacent_pos->row = node->pos->row + 1;
		adjacent_pos->col = node->pos->col;
		if (validate_repair_pos(grid_info, adjacent_pos, weight, node->pos,
			repair_chain, block_pos->goal)) {
			break;
		}

		adjacent_pos->row = node->pos->row;
		adjacent_pos->col = node->pos->col - 1;
		if (validate_repair_pos(grid_info, adjacent_pos, weight, node->pos,
			repair_chain, block_pos->goal)) {
			break;
		}
		adjacent_pos->row = node->pos->row;
		adjacent_pos->col = node->pos->col + 1;
		if (validate_repair_pos(grid_info, adjacent_pos, weight, node->pos,
			repair_chain, block_pos->goal)) {
			break;
		}
		node = node->next;
	}

	free(adjacent_pos);

	return repair_chain;

}

/******************************************************************************/
/* repairs the route and returns it. From the repair chain of adjacent positions
to the block. It takes the foot of the repair chain and works backwards from it,
tracing through the previous positions from the foot.

As it works backwards, it inserts every position from the repair chain into the
route. Working backwards to insert from the end of the block to the beginning
and updating the grid with '*' to signify where the new route is.

Stops inserting backwards once it's reached the beginning of the block and
returns the route.
*/

list_t*
final_repair_route(grid_pos_t* block_pos, repair_list_t* repair_chain,
	list_t* route, char** grid) {
	repair_node_t* node = malloc(sizeof(*node));
	assert(node);
	node->pos = malloc(sizeof(pos_t));
	assert(node->pos);
	node->prev_pos = malloc(sizeof(pos_t));
	assert(node->prev_pos);

	node->pos->row = repair_chain->foot->pos->row;
	node->pos->col = repair_chain->foot->pos->col;

	node->prev_pos->row = repair_chain->foot->prev_pos->row;
	node->prev_pos->col = repair_chain->foot->prev_pos->col;

	repair_node_t* temp_node = malloc(sizeof(*node));
	assert(temp_node);

		/* keeps adding items until the beginning of the blocked route is reached */
	while (!(node->prev_pos->row == block_pos->initial->row
		&& node->prev_pos->col == block_pos->initial->col)) {
		route = insertBefore(route, node->pos, node->prev_pos);

		grid[node->prev_pos->row][node->prev_pos->col] = '*';

		/* goes through repair list until the prev position to the current
		repair position is found */
		temp_node = repair_chain->head;
		while (temp_node) {
			if (temp_node->pos->row == node->prev_pos->row
				&& temp_node->pos->col == node->prev_pos->col) {
				node = temp_node;
				break;
			} temp_node = temp_node->next;
		}
	}

	free(repair_chain);
	free(node);

	return route;
}


/******************************************************************************/
/* Given a position, the function will search for the node with that position.
The function will then insert a new node before the node with the given
position.

adapted from: https://www.geeksforgeeks.org/doubly-linked-list/
*/
list_t*
insertBefore(list_t* route, pos_t* next_pos, pos_t* new_pos) {

	node_t* next_node = malloc(sizeof(*next_node));
	assert(next_node);
	next_node->pos = malloc(sizeof(pos_t));
	assert(next_node->pos);

	next_node = route->head;
	next_node->pos->row = route->head->pos->row;
	next_node->pos->col = route->head->pos->col;

	while (next_node) {
		if (next_node->pos->row == next_pos->row
			&& next_node->pos->col == next_pos->col) {
			break;
		} next_node = next_node->next;
	}

	node_t* new_node = malloc(sizeof(*new_node));
	assert(new_node);
	new_node->pos = malloc(sizeof(pos_t));
	assert(new_node->pos);

	new_node->pos->row = new_pos->row;
	new_node->pos->col = new_pos->col;

	new_node->prev = next_node->prev;
	next_node->prev = new_node;
	new_node->next = next_node;


	if (new_node->prev != NULL) {
		new_node->prev->next = new_node;
	}
	else {
		route->head = new_node;
	}
	//	print_route(route->head);
	return route;
}

/******************************************************************************/
/* validates that the given repair position for the repair list is not outside
of the grid bounds. It also validates that the repair position isn't already
in the repair list.

If those checks are passed, the position and its info (weight and prev_pos) is
inserted in the foot of the repair list.
*/

int
validate_repair_pos(grid_info_t* grid_info, pos_t* adjacent_pos,
	int weight, pos_t* prev_pos, repair_list_t* repair_chain, pos_t* goal) {

	if (adjacent_pos->row > grid_info->nrows - 1
		|| adjacent_pos->col > grid_info->ncols - 1
		|| adjacent_pos->row < 0 || adjacent_pos->col < 0) {
		return 0;
	}

	if (!pos_in_list(repair_chain->head, adjacent_pos)
		&& (grid_info->grid[adjacent_pos->row][adjacent_pos->col]) != BLOCK) {
		repair_chain = insert_foot_repair_node(repair_chain, adjacent_pos,
			weight, prev_pos);
	}

	if (repair_chain->foot->pos->row == goal->row
		&& repair_chain->foot->pos->col == goal->col) {
		return 1;
	}

	return 0;

}

/******************************************************************************/
/* verifies if a given position is in the linked list of repairs positions.
returns 1 if item is in the list and 0 if not
*/

int
pos_in_list(repair_node_t* node, pos_t* pos) {

	while (node) {
		if (node->pos->row == pos->row && node->pos->col == pos->col) {
			return 1;
		}
		node = node->next;
	}

	return 0;
}

/******************************************************************************/
/* makes new repair node at the foot of the repair list, assigns weight and the
previous position the cell was adjacent to.
*/

repair_list_t*
insert_foot_repair_node(repair_list_t* repair_list, pos_t* pos, int weight,
	pos_t* prev_pos) {

	repair_node_t* new;
	new = (repair_node_t*)malloc(sizeof(*new));
	assert(new);
	new->pos = malloc(sizeof(pos_t));
	assert(new->pos);
	new->prev_pos = malloc(sizeof(pos_t));
	assert(new->prev_pos);
	assert(repair_list);

	new->weight = weight;

	new->pos->row = pos->row;
	new->pos->col = pos->col;
	new->prev_pos->row = prev_pos->row;
	new->prev_pos->col = prev_pos->col;

	new->next = NULL;

	if (repair_list->foot == NULL) {
		new->prev = NULL;
		repair_list->head = repair_list->foot = new;
	}
	else {
		new->prev = repair_list->foot;
		repair_list->foot->next = new;
		repair_list->foot = new;
	}

	return repair_list;
}

/******************************************************************************/
/* finds position of next blocked route. Returns the position of the start of
the route blockage and the end of the block.
*/

grid_pos_t*
find_blocked_route(char** grid, node_t* node) {
	int initial_block_found = 0;

	grid_pos_t* block_pos = malloc(sizeof(*block_pos));
	assert(block_pos);
	block_pos->initial = malloc(sizeof(pos_t));
	assert(block_pos->initial);
	block_pos->goal = malloc(sizeof(pos_t));
	assert(block_pos->goal);


	while (node) {
		if (initial_block_found == 0) {
			if (grid[node->pos->row][node->pos->col] == BLOCK) {
				block_pos->initial->row = node->prev->pos->row;
				block_pos->initial->col = node->prev->pos->col;
				initial_block_found = 1;
			}
			else if (grid[node->pos->row][node->pos->col] == GOAL) {
				return 0;
			}
		}
		else if (initial_block_found == 1) {
			if (grid[node->pos->row][node->pos->col] == '*'
				|| grid[node->pos->row][node->pos->col] == GOAL) {
				block_pos->goal->row = node->pos->row;
				block_pos->goal->col = node->pos->col;
				return block_pos;
			}
		}
		node = node->next;
	}

	return 0;
}

/******************************************************************************/
/* prints grid that's given as a 2d array. Also places borders outside the row
and columns to denote the row and column number
*/

void
print_grid(grid_info_t* grid_info) {
	int i, j;
	printf(" ");

	for (i = 0; i < grid_info->ncols; i++) {
		printf("%d", i % 10);
	}

	printf("\n");
	for (i = 0; i < grid_info->nrows; i++) {
		printf("%d", i % 10);
		for (j = 0; j < grid_info->ncols; j++) {
			printf("%c", grid_info->grid[i][j]);
		}
		printf("\n");
	}
}

/******************************************************************************/
/*reads in then prints stage 1 info about the route. i.e. grid size, no. of
blocks, initial  and goal cells, proposed route, and status. Returns route and
status
*/

route_info_t*
print_info(grid_info_t* grid_info) {
	printf("==STAGE 0=======================================\n");
	route_info_t* route_info = malloc(sizeof(*route_info));

	printf("The grid has %d rows and %d columns.\n", grid_info->nrows,
		grid_info->ncols);

	grid_pos_t* grid_pos = read_initial_and_goal_pos(grid_info->grid);

	int block_count = process_blocks(grid_info->grid);
	printf("The grid has %d block(s).\n", block_count);

	printf("The initial cell in the grid is [%d,%d].\n",
		grid_pos->initial->row, grid_pos->initial->col);
	printf("The goal cell in the grid is [%d,%d].\n",
		grid_pos->goal->row, grid_pos->goal->col);

	route_info->route = make_empty_route();
	route_info->status = route_reader(grid_info->grid,
		route_info->route, grid_pos);
	printf("The proposed route in the grid is:\n");
	print_route(route_info->route->head);

	check_status1_2(route_info, grid_pos);
	print_status(route_info->status);

	return route_info;
}

/******************************************************************************/
/* checks if status 1 and 2 are true. i.e. checks if the beginning of the route
is the same as the initial grid cell (status 1) and checks if the end of the
route is the same as the goal cell (status 2).
*/

void
check_status1_2(route_info_t* route_info, grid_pos_t* grid_pos) {

	if (route_info->route->head->pos->row != grid_pos->initial->row
		|| route_info->route->head->pos->col != grid_pos->initial->col) {
		route_info->status = 1;
	}
	else if (route_info->route->foot->pos->row != grid_pos->goal->row
		|| route_info->route->foot->pos->col != grid_pos->goal->col) {
		route_info->status = 2;
	}
}

/******************************************************************************/
/* the following function reads initial and goal coordinates into structure
grid_pos_t. Then it stores the initial and goal positions into the grid.

The function returns the grid positions of the initial and goal coordinates.
*/

grid_pos_t*
read_initial_and_goal_pos(char** grid) {
	grid_pos_t* grid_pos = malloc(sizeof(*grid_pos));
	assert(grid_pos);
	grid_pos->initial = malloc(sizeof(pos_t));
	assert(grid_pos->initial);
	grid_pos->goal = malloc(sizeof(pos_t));
	assert(grid_pos->goal);

	read_pos(grid_pos->initial);
	grid[grid_pos->initial->row][grid_pos->initial->col] = INITIAL;

	read_pos(grid_pos->goal);
	grid[grid_pos->goal->row][grid_pos->goal->col] = GOAL;

	return grid_pos;
}

/******************************************************************************/
/* Prints status depending on which status it is (from status 1 to 5).
*/

int
print_status(int current_status) {

	if (current_status == 1) {
		printf("Initial cell in the route is wrong!");
	}
	else if (current_status == 2) {
		printf("Goal cell in the route is wrong!");
	}
	else if (current_status == 3) {
		printf("There is an illegal move in this route!");
	}
	else if (current_status == 4) {
		printf("There is a block on this route!");
	}
	else if (current_status == 5) {
		printf("The route is valid!");
	}

	printf("\n");

	return current_status;
}

/******************************************************************************/
/* prints repair routes with no more than five cells per line, each separated
by ->. This is for debugging purposes.
*/

void
print_route_repair(repair_node_t* node) {
	int i = 0;
	while (node) {
		if (i == MAX_POS_IN_LINE - 1) {
			printf("[%d,%d]", node->pos->row, node->pos->col);
			i = 0;
		}
		else {
			printf("[%d,%d]", node->pos->row, node->pos->col);
			i++;
		}

		node = node->next;
		if (node) {
			if (i > 0 && i <= 5) {
				printf("->");
			}
			else {
				printf("\n");
			}
		}
		else {
			printf(".\n");
		}
	}
}

/******************************************************************************/
/* prints routes with no more than five cells per line, each separated by ->
*/

void
print_route(node_t* node) {
	int i = 0;
	while (node) {
		if (i == MAX_POS_IN_LINE - 1) {
			printf("[%d,%d]", node->pos->row, node->pos->col);
			i = 0;
		}
		else {
			printf("[%d,%d]", node->pos->row, node->pos->col);
			i++;
		}

		node = node->next;
		if (node) {
			if (i > 0 && i <= 5) {
				printf("->");
			}
			else {
				printf("->\n");
			}
		}
		else {
			printf(".\n");
		}
	}
}


/******************************************************************************/
/* makes a 2D array to represent the grid. Fills in each element with a space.
Fills out grid info with the 2d array and also the number of rows and columns
the array has.
*/

grid_info_t*
grid_to_memory(void) {
	int i, j, rows, cols;
	grid_info_t* new_grid = malloc(sizeof(grid_info_t));

	scanf("%dx%d", &rows, &cols);

	new_grid->grid = malloc(rows * sizeof(char*));
	assert(new_grid->grid);

	for (i = 0; i < rows; i++) {
		new_grid->grid[i] = malloc(cols * sizeof(char));
		assert(new_grid->grid[i]);
		for (j = 0; j < cols; j++) {
			new_grid->grid[i][j] = ' ';
		}
	}

	new_grid->nrows = rows;
	new_grid->ncols = cols;

	return new_grid;
}

/******************************************************************************/
int
mygetchar() {
	int c;
	while ((c = getchar()) == '\r') {
	}
	return c;
}

/******************************************************************************/
/* makes an empty route.
*/

list_t*
make_empty_route(void) {
	list_t* route = (list_t*)malloc(sizeof(*route));
	assert(route != NULL);
	route->head = route->foot = NULL;
	return route;
}

/******************************************************************************/
/* reads each new route position into the route. As each position is read, it
checks if an illegal move has been made or if the position is on a block. If
the check is true, the status of the route is updated. The function returns
status 5 if no block or illegal moves are detected.
*/
int
route_reader(char** grid, list_t* route, grid_pos_t* grid_pos) {
	int status = 5;

	pos_t* route_pos = malloc(sizeof(pos_t));
	assert(route_pos);

	while (read_pos(route_pos)) {
		route = insert_route_foot_pos(route, route_pos);
		if (grid[route_pos->row][route_pos->col] == BLOCK) {
			status = 4;
		}
		else if ((route_pos->row == grid_pos->initial->row &&
			route_pos->col == grid_pos->initial->col)
			|| (route_pos->row == grid_pos->goal->row &&
				route_pos->col == grid_pos->goal->col)) {
		}
		else {
			grid[route_pos->row][route_pos->col] = '*';
		}
		if (route->foot->prev) {
			/* checks if the route position has made an illegal move */
			if (((route->foot->prev->pos->row + 1 == route_pos->row
				|| route->foot->prev->pos->row - 1 == route_pos->row)
				&& route->foot->prev->pos->col == route_pos->col)

				|| ((route->foot->prev->pos->col + 1 == route_pos->col
					|| route->foot->prev->pos->col - 1 == route_pos->col)
					&& route->foot->prev->pos->row == route_pos->row)) {
				continue;
			}
			else {
				status = 3;
			}
		}
	}

	free(route_pos);
	return status;

}

/******************************************************************************/
/* makes a doubly linked list of the path coordinates, inserting every new
position at the foot
*/

list_t*
insert_route_foot_pos(list_t* list, pos_t* route_pos) {
	node_t* new;
	new = (node_t*)malloc(sizeof(*new));
	new->pos = malloc(2 * sizeof(pos_t));
	assert(new);
	assert(list);

	new->pos->row = route_pos->row;
	new->pos->col = route_pos->col;
	new->next = NULL;

	if (list->foot == NULL) {
		new->prev = NULL;
		list->head = list->foot = new;
	}
	else {
		new->prev = list->foot;
		list->foot->next = new;
		list->foot = new;
	}

	return list;

}

/******************************************************************************/
/* Reads blocks, then places blocks into grid. Returns the number of blocks
there are.
*/

int
process_blocks(char** grid) {
	int block_count = 0;

	pos_t* block_pos = malloc(sizeof(pos_t));
	assert(block_pos);

	while (read_pos(block_pos)) {
		grid[block_pos->row][block_pos->col] = BLOCK;
		block_count += 1;
	}
	free(block_pos);

	return block_count;
}

/*****************************************************************************/
/* read coordinates of format [n, n] where n is an integer. Returns 0 if EOF or
$, otherwise, returns a 1.
*/

int
read_pos(pos_t* pos) {
	int nrow, ncol;
	char ch;

	while ((ch = mygetchar()) && (isspace(ch) || ch == '-'
		|| ch == '>')) {
	}

	if (ch == EOF || ch == '$') {
		return 0;
	}

	scanf("%d,%d]", &nrow, &ncol);

	pos->row = nrow;
	pos->col = ncol;
	return 1;
}

/******************************************************************************/
