#pragma once

#ifdef GENERATELEVEL_EXPORTS
#define GENERATELEVEL_API __declspec(dllexport)
#else
#define GENERATELEVEL_API __declspec(dllimport)
#endif

#include<memory>
#include<random>

enum CellType
{
	FREESPACE, WALL, ITEM, MONSTER, ENTRANCE_OF_LEVEL, EXIT_OF_LEVEL
};

enum ValidMove
{
	NSEW, NEW, SEW, ENS, WNS, NE, NW, SE, SW, NONE
};

class Node
{
private:
	std::pair<int, int> coordinate;
	ValidMove valid_moves;
	CellType cell_type;
	int path_cost;
	int heuristic;
	std::pair<int, int> parent;
public:
	void init(int x_coordinate, int y_coordinate);
	const bool cmp(const Node& a, const Node& b);
	bool is_equal(const Node& a);
	int get_x_coordinate();
	int get_y_coordinate();
	void set_cell_type(CellType cell_type);
	std::pair<int, int> get_coordinates();
	const ValidMove get_valid_moves();
	CellType get_cell_type();
	void set_parent(std::pair<int, int> parent);
	std::pair<int, int> get_parent();
	void set_path_cost(int path_cost);
	int get_path_cost();
	void set_heuristic(int heuristic);
};

class LevelGen
{
	Node generated_Level[50][50];
	Node entrance_of_level;
	Node exit_of_level;
	int path_length;
public:
	LevelGen();
	void set_entrance(std::pair<int, int> entrance);
	void set_exit(std::pair<int, int> exit_of_level);
	void set_wall(int x_coordinate, int y_coordinate);
	void set_monster(int x_coordinate, int y_coordinate);
	void set_item(int x_coordinate, int y_coordinate);
	void calculate_path_length();
	int get_path_length();
	int* generate_level_ptr();
};

bool operator<(const Node& a, const Node& b); //NOTE:

//LB= Lower Bound, UB= Upper Bound
const int RNG_LB = 1;
const int RNG_UB = 6;
const int BORDER_LB = 0;
const int BORDER_UB = 49;
const int NO_OF_BORDERS_LB = 0;
const int NO_OF_BORDERS_UB = 3;

extern "C" GENERATELEVEL_API int test_dll();

extern "C" GENERATELEVEL_API int* gen_level();