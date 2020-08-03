#include "pch.h"
#include "GenerateLevel.h"
#include <random>
#include<unordered_set>
#include <queue>

std::unique_ptr<LevelGen> generate_Level();
void generate_entrance(int generated_border_loc, int selected_border, std::pair<int, int>& entrance);
void walk_along_boundary(std::pair<int, int> start, std::pair<int, int>& end, int distance, int border_no);
void generate_items_and_monsters(std::default_random_engine& prng, std::uniform_int_distribution<int>& grid, LevelGen& level_generated);
ValidMove calc_valid_move(std::pair<int, int> coordinates);
int calc_manhattan_distance(std::pair<int, int> point1, std::pair<int, int> point2);
bool operator<(const Node& a, const Node& b);
std::vector<std::pair<int, int>> calc_navigable_coordinates(Node& node);
void generate_walls(std::default_random_engine& prng, std::uniform_int_distribution<int>& grid, std::unique_ptr<LevelGen> &level_generated);
void generate_items_and_monsters(std::default_random_engine& prng, std::uniform_int_distribution<int>& grid, std::unique_ptr<LevelGen> &level_generated);



std::unique_ptr<LevelGen> generate_Level()
{
    std::unique_ptr<LevelGen> lg(new LevelGen);

    //boilerplate for random
    std::random_device rando_d;
    std::default_random_engine prng(rando_d());
    std::uniform_int_distribution<int> grid(RNG_LB, RNG_UB);

    //generation of location of entrance
    std::uniform_int_distribution<int> select_border(NO_OF_BORDERS_LB, NO_OF_BORDERS_UB); //chooses which border the entrance is placed at
    std::uniform_int_distribution<int> border_loc(BORDER_LB, BORDER_UB); // chooses the displacement of the entrance at the border
    int generated_border_loc = border_loc(prng);
    int selected_border = select_border(prng);
    std::pair<int, int> entrance;
    generate_entrance(generated_border_loc, selected_border, entrance);
    lg->set_entrance(entrance);
    

    //generation of location of exit
    //add distance to location of entrance
    //constrained distance to ensure that entrance and exit are at least 75 units away from each other along the border
    std::uniform_int_distribution<int> generate_distance(75, 120);
    int distance = generate_distance(prng);
    std::pair<int, int> exit_of_level;
    walk_along_boundary(entrance, exit_of_level, distance, selected_border); //generates exit
    lg->set_exit(exit_of_level);

    int count = 15;
    do {
        //add walls to the level
        generate_walls(prng, grid, lg);

        count--;

        lg->calculate_path_length();
        //check if path exists from entrance to exit
    } while ((count > 0) && (lg->get_path_length()!=(-1)));

    count = 15;
    do {
        //generate blue noise for placement of items, monsters
        generate_items_and_monsters(prng, grid, lg);

        count--;

        lg->calculate_path_length();
        //check if path exists from entrance to exit
    } while ((count > 0) && (lg->get_path_length() != (-1)));

    return lg;
}

void generate_entrance(int generated_border_loc, int selected_border, std::pair<int, int> &entrance)
{
    switch (selected_border)
    {
    case 0: //top boundary
        entrance.first = 0;
        entrance.second = generated_border_loc;
        break;
    case 1: //bottom boundary
        entrance.first = 49;
        entrance.second = generated_border_loc;
        break;
    case 2: //left boundary
        entrance.second = 0;
        entrance.first = generated_border_loc;
        break;
    case 3: //right boundary
        entrance.second = 49;
        entrance.first = generated_border_loc;
        break;
    }
}

//recursive function for calculating exit
//might have bugs, check conditions
void walk_along_boundary(std::pair<int, int> start, std::pair<int, int>& end, int distance, int border_no)
{
    switch (border_no)
    {
    case 0:
        if (start.second + distance > 49)
        {
            walk_along_boundary(std::make_pair(0, 49), end, distance - (49 - start.second), 3);
        }
        else
        {
            end.first = start.first;
            end.second = start.second + distance;
        }
        break;
    case 1:
        if (start.second - distance < 0)
        {
            walk_along_boundary(std::make_pair(49, 0), end, distance - (start.second), 2);
        }
        else
        {
            end.first = start.first;
            end.second = start.second - distance;
        }
        break;
    case 2:
        if (start.first - distance < 0)
        {
            walk_along_boundary(std::make_pair(0, 0), end, distance - (start.first), 0);
        }
        else
        {
            end.first = start.first-distance;
            end.second = start.second;
        }
        break;
    case 3:
        if (start.first + distance > 49)
        {
            walk_along_boundary(std::make_pair(49, 49), end, distance - (49-start.first), 1);
        }
        else
        {
            end.first = start.first+distance;
            end.second = start.second;
        }
        break;
    }
}

ValidMove calc_valid_move(std::pair<int, int> coordinates)
{
    if (coordinates.first == 0)
    {
        if (coordinates.second == 0)
        {
            return SE;
        }
        else if (coordinates.second == 49)
        {
            return SW;
        }
        else
        {
            return SEW;
        }
    }
    else if (coordinates.first == 49)
    {
        if (coordinates.second == 0)
        {
            return NE;
        }
        else if (coordinates.second == 49)
        {
            return NW;
        }
        else
        {
            return NEW;
        }
    }
    else if (coordinates.second == 0)
    {
        return ENS;
    }
    else if (coordinates.second == 49)
    {
        return WNS;
    }
    else
    {
        return NSEW;
    }
}

int calc_manhattan_distance(std::pair<int, int> point1, std::pair<int, int> point2)
{
    return std::abs(point1.first - point2.first) + std::abs(point1.second - point2.second);
}

void Node::init(int x_coordinate, int y_coordinate)
{
    this->coordinate.first = x_coordinate;
    this->coordinate.second = y_coordinate;
    this->cell_type = FREESPACE;
    this->path_cost = 0;
    this->heuristic = 0;
    this->valid_moves = calc_valid_move(this->coordinate);
    this->parent.first = -1;
    this->parent.second = -1;
}

void Node::set_cell_type(CellType cell_type)
{
    this->cell_type = cell_type;
}

std::pair<int, int> Node::get_coordinates()
{
    return coordinate;
}

const ValidMove Node::get_valid_moves()
{
    return this->valid_moves;
}

CellType Node::get_cell_type()
{
    return this->cell_type;
}

void Node::set_parent(std::pair<int, int> parent)
{
    this->parent = parent;
}

std::pair<int, int> Node::get_parent()
{
    return this->parent;
}

void Node::set_path_cost(int path_cost)
{
    this->path_cost = path_cost;
}

int Node::get_path_cost()
{
    return this->path_cost;
}

void Node::set_heuristic(int heuristic)
{
    this->heuristic = heuristic;
}

bool operator<(const Node& a, const Node& b) //IMP: overridden to create min priority queue
{
    Node temp;
    return temp.cmp(a, b);
}

int test_dll()
{
    return 10;
}

int* gen_level()
{
    return generate_Level()->generate_level_ptr();
}

const bool Node::cmp(const Node& a, const Node& b)
{
    int f1 = a.heuristic + a.path_cost;
    int f2 = b.heuristic + b.path_cost;
    if (f1 == f2)
    {
        if (a.path_cost == b.path_cost)
        {
            return a.heuristic > b.heuristic;
        }
        else
        {
            return a.path_cost > b.path_cost;
        }
    }
    else
    {
        return f1 > f2;
    }
}

bool Node::is_equal(const Node& a)
{
    return (coordinate.first==a.coordinate.first)&&(coordinate.second==a.coordinate.second);
}

int Node::get_x_coordinate()
{
    return coordinate.first;
}

int Node::get_y_coordinate()
{
    return coordinate.second;
}

std::vector<std::pair<int, int>> calc_navigable_coordinates(Node& node)
{
    std::vector<std::pair<int, int>> collection_of_coordinates;
    std::pair<int, int> coordinates = node.get_coordinates();
    std::pair<int, int> north, south, east, west;
    north.first = coordinates.first - 1;
    north.second = coordinates.second;
    south.first = coordinates.first + 1;
    south.second = coordinates.second;
    east.first = coordinates.first;
    east.second = coordinates.second + 1;
    west.first = coordinates.first;
    west.second = coordinates.second - 1;
    switch (node.get_valid_moves())
    {
    case NSEW:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(east);
        collection_of_coordinates.push_back(west);
        break;
    case NEW:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(east);
        collection_of_coordinates.push_back(west);
        break;
    case SEW:
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(east);
        collection_of_coordinates.push_back(west);
        break;
    case ENS:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(east);
        break;
    case WNS:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(west);
        break;
    case NE:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(east);
        break;
    case NW:
        collection_of_coordinates.push_back(north);
        collection_of_coordinates.push_back(west);
        break;
    case SE:
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(east);
        break;
    case SW:
        collection_of_coordinates.push_back(south);
        collection_of_coordinates.push_back(west);
        break;
    }
    return collection_of_coordinates;
}

LevelGen::LevelGen()
{
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            generated_Level[i][j].init(i, j);
        }
    }
    this->path_length = -1;
}

void LevelGen::set_entrance(std::pair<int, int> entrance_of_level)
{
    generated_Level[entrance_of_level.first][entrance_of_level.second].set_cell_type(ENTRANCE_OF_LEVEL);
    this->entrance_of_level = generated_Level[entrance_of_level.first][entrance_of_level.second];
}

void LevelGen::set_exit(std::pair<int, int> exit_of_level)
{
    generated_Level[exit_of_level.first][exit_of_level.second].set_cell_type(EXIT_OF_LEVEL);
    this->exit_of_level = generated_Level[exit_of_level.first][exit_of_level.second];
}

void LevelGen::set_wall(int x_coordinate, int y_coordinate)
{
    generated_Level[x_coordinate][y_coordinate].set_cell_type(WALL);
}

void LevelGen::set_monster(int x_coordinate, int y_coordinate)
{
    generated_Level[x_coordinate][y_coordinate].set_cell_type(MONSTER);
}

void LevelGen::set_item(int x_coordinate, int y_coordinate)
{
    generated_Level[x_coordinate][y_coordinate].set_cell_type(ITEM);
}

void LevelGen::calculate_path_length()
{
    std::priority_queue<Node> open_set;
    bool closed_set[50][50];
    bool open_list[50][50];
    for (int i = 0; i < 50; i++)
    {
        for (int j = 0; j < 50; j++)
        {
            closed_set[i][j] = false;
            open_list[i][j] = true;
        }
    }

    Node current, temp;

    open_set.push(entrance_of_level);

    while (!open_set.empty())
    {
        current = open_set.top();
        open_set.pop();
        open_list[current.get_x_coordinate()][current.get_y_coordinate()] = false;

        closed_set[current.get_x_coordinate()][current.get_y_coordinate()] = true;

        std::vector<std::pair<int, int>> navigable_coordinates = calc_navigable_coordinates(current);

        for (auto navigable_node : navigable_coordinates)
        {
            if ((generated_Level[navigable_node.first][navigable_node.second].get_cell_type() == FREESPACE) &&
                (closed_set[navigable_node.first][navigable_node.second] == false))
            {
                temp = generated_Level[navigable_node.first][navigable_node.second];

                if (temp.is_equal(exit_of_level))
                {
                    temp.set_parent(current.get_coordinates());
                    temp.set_path_cost(current.get_path_cost() + 1);
                    break;
                }

                if (open_list[navigable_node.first][navigable_node.second])
                {
                    if (temp.get_path_cost() > (current.get_path_cost() + 1))
                    {
                        temp.set_parent(current.get_coordinates());
                        temp.set_path_cost(current.get_path_cost() + 1);
                    }
                }
                else
                {
                    open_list[navigable_node.first][navigable_node.second] = true;
                    open_set.push(temp);
                    temp.set_parent(current.get_coordinates());
                    temp.set_path_cost(current.get_path_cost() + 1);
                    temp.set_heuristic(calc_manhattan_distance(temp.get_coordinates(), exit_of_level.get_coordinates()));
                }
            }
        }
    }

    if (generated_Level[exit_of_level.get_x_coordinate()][exit_of_level.get_y_coordinate()].get_parent().first == (-1))
    {
        path_length = -1;
    }
    else 
    {
        path_length = generated_Level[exit_of_level.get_x_coordinate()][exit_of_level.get_y_coordinate()].get_path_cost();
    }
}

int LevelGen::get_path_length()
{
    return this->path_length;
}

int* LevelGen::generate_level_ptr()
{
    int* level_generated_ptr = new int[2500];
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            level_generated_ptr[i * 50 + j] = generated_Level[i][j].get_cell_type();
        }
    }
    return level_generated_ptr;
}

void generate_walls(std::default_random_engine& prng, std::uniform_int_distribution<int>& grid, std::unique_ptr<LevelGen> &level_generated)
{
    int aux_random[50][50]; //aux array containing random values

    for (int i = 1; i < 49; i++)
    {
        for (int j = 1; j < 49; j++)
        {
            aux_random[i][j] = grid(prng);
        }
    }

    //smoothen the noise in the aux array (try to convert it into pink noise)
    for (int i = 1; i < 49; i++) //try eliminating corners to see what will happen
    {
        for (int j = 1; j < 50; j++)
        {
            int avg = aux_random[i - 1][j - 1] + aux_random[i - 1][j] + aux_random[i - 1][j + 1];
            avg += (aux_random[i][j - 1] + aux_random[i][j] + aux_random[i][j + 1]);
            avg += (aux_random[i + 1][j - 1] + aux_random[i + 1][j] + aux_random[i + 1][j + 1]);
            avg /= 9;
        }
    }

    //place walls based on the noise generated in the aux array
    for (int i = 1; i < 49; i++)
    {
        for (int j = 1; j < 49; j++)
        {
            if (aux_random[i][j] < 3)
            {
                level_generated->set_wall(i, j);
            }
        }
    }
}

void generate_items_and_monsters(std::default_random_engine& prng, std::uniform_int_distribution<int>& grid, std::unique_ptr<LevelGen> &level_generated)
{
    int aux_random[50][50];

    for (int i = 1; i < 49; i++)
    {
        for (int j = 1; j < 49; j++)
        {
            aux_random[i][j] = grid(prng);
        }
    }
    for (int i = 1; i < 49; i++)
    {
        for (int j = 1; j < 49; j++)
        {
            if ((aux_random[i][j] == 1) || (aux_random[i][j] == 2))
            {
                if (aux_random[i - 1][j - 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i - 1][j] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i - 1][j + 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i][j - 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i][j + 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i + 1][j - 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i + 1][j] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
                else if (aux_random[i + 1][j + 1] == aux_random[i][j])
                {
                    aux_random[i][j] = 6;
                }
            }
        }
    }

    //place items based on the noise generated
    for (int i = 1; i < 49; i++)
    {
        for (int j = 1; j < 49; j++)
        {
            if (aux_random[i][j] == 1)
            {
                level_generated->set_monster(i, j);
            }
            else if (aux_random[i][j] == 2)
            {
                level_generated->set_item(i, j);
            }
        }
    }
}