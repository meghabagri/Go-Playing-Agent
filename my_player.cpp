#include <vector>
#include <stack>
#include <string>
#include <limits.h>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace std;

typedef vector<vector<int>> D2VECTOR;
int board_side = 5;
int max_steps = 24;
int my_piece = -1;

bool in_range(int i, int j)
{
    if (i < 0 || i >= board_side || j < 0 || j >= board_side)
        return false;
    return true;
}

int get_defense(D2VECTOR &board, int i, int j)
{
    int count = 0;
    int defense_indexes[][2] = {{1, 1}, {-1, -1}, {-1, 1}, {1, -1}};

    for (auto n : defense_indexes)
    {
        int new_i = n[0] + i;
        int new_j = n[1] + j;
        if (in_range(new_i, new_j) && board[new_i][new_j] == board[i][j])
            count++;
    }

    return count;
}

D2VECTOR get_neighbours(int i, int j)
{
    D2VECTOR neighbours;
    int neighbour_indexes[][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

    for (auto n : neighbour_indexes)
        if (in_range(n[0] + i, n[1] + j))
            neighbours.push_back({n[0] + i, n[1] + j});

    return neighbours;
}

D2VECTOR get_immedaite_allies(D2VECTOR &board, int i, int j)
{
    D2VECTOR allies;

    for (auto n : get_neighbours(i, j))
        if (board[n[0]][n[1]] == board[i][j])
            allies.push_back({n[0], n[1]});

    return allies;
}

D2VECTOR find_all_allies(D2VECTOR &board, int i, int j)
{
    if (board[i][j] == 0)
        return {};

    D2VECTOR allies;
    stack<vector<int>> stk;
    stk.push({i, j});

    while (stk.size())
    {
        vector<int> node = stk.top();
        stk.pop();
        allies.push_back(node);

        for (auto a : get_immedaite_allies(board, node[0], node[1]))
            if (find(allies.begin(), allies.end(), a) == allies.end())
                stk.push(a);
    }

    return allies;
}

int get_liberties(D2VECTOR &board, int i, int j)
{
    int count = 0;

    for (auto a : find_all_allies(board, i, j))
        for (auto n : get_neighbours(a[0], a[1]))
            if (board[n[0]][n[1]] == 0)
                count++;

    return count;
}

D2VECTOR get_deads(D2VECTOR &board, int piece_type)
{
    D2VECTOR deads;

    for (int i = 0; i < board_side; i++)
        for (int j = 0; j < board_side; j++)
            if (board[i][j] == piece_type && find(deads.begin(), deads.end(), vector<int>{i, j}) == deads.end())
                if (!get_liberties(board, i, j))
                    deads.push_back({i, j});
    return deads;
}

void remove_deads(D2VECTOR &board, D2VECTOR &deads)
{
    for (auto d : deads)
        board[d[0]][d[1]] = 0;
}

bool compare_states(D2VECTOR &board1, D2VECTOR &board2)
{
    for (int i = 0; i < board_side; i++)
        for (int j = 0; j < board_side; j++)
            if (board1[i][j] != board2[i][j])
                return false;

    return true;
}

bool valid_move(D2VECTOR &prev_board, D2VECTOR &board, int piece_type, int i, int j)
{
    if (!in_range(i, j))
        return false;

    if (board[i][j] != 0)
        return false;

    D2VECTOR next_board_state = board;
    next_board_state[i][j] = piece_type;
    if (get_liberties(next_board_state, i, j))
        return true;

    D2VECTOR deads = get_deads(next_board_state, 3 - piece_type);

    if (!deads.size())
        return false;

    remove_deads(next_board_state, deads);
    return !compare_states(prev_board, next_board_state);
}

bool worth_move(D2VECTOR &board, int piece_type, int i, int j)
{
    if (board[i][j] == 0)
        for (auto n : get_neighbours(i, j))
            if (board[n[0]][n[0]] != 0)
                return true;
    return false;
}

D2VECTOR find_possible_moves(D2VECTOR &prev_board, D2VECTOR &board, int piece_type)
{
    D2VECTOR possible_moves;

    for (int i = 0; i < board_side; i++)
        for (int j = 0; j < board_side; j++)
            if (worth_move(board, piece_type, i, j))
                if (valid_move(prev_board, board, piece_type, i, j))
                    possible_moves.push_back({i, j});

    return possible_moves;
}

int get_piece_count(D2VECTOR &board, int piece_type)
{
    int count = 0;
    for (int i = 0; i < board_side; i++)
        for (int j = 0; j < board_side; j++)
            if (board[i][j] == piece_type)
                count += get_liberties(board, i, j) + 1;

    return count;
}

float get_score(D2VECTOR &board, int piece_type)
{
    float score = get_piece_count(board, piece_type);

    if (piece_type == 2)
        score += 2.5;

    return score;
}

float heur(int piece_type, D2VECTOR &board)
{
    float black_stones = get_score(board, 1);
    float white_stones = get_score(board, 2);
    float score = 0;

    if (piece_type == 1)
    {
        score = black_stones - white_stones;
    }
    else
    {
        score = white_stones - black_stones;
    }
    return score;
}

float minimax(int piece_type, D2VECTOR previous_board, D2VECTOR board, int max_depth, int steps, float a, float b)
{
    if (max_depth == 0 || steps > max_steps)
        return heur(piece_type, board);

    D2VECTOR valid_moves = find_possible_moves(previous_board, board, piece_type);

    if (valid_moves.size() == 0)
        return heur(piece_type, board);

    float value;
    if (piece_type == my_piece)
    {
        value = INT_MIN;
        for (auto m : valid_moves)
        {
            D2VECTOR board_copy = board;
            board_copy[m[0]][m[1]] = piece_type;

            D2VECTOR deads = get_deads(board_copy, 3 - piece_type);

            remove_deads(board_copy, deads);

            float curr_val = minimax(3 - piece_type, board, board_copy, max_depth - 1, steps + 1, a, b);

            value = max(value, curr_val);

            if (value >= b)
                return value;
            a = max(a, value);
        }
    }
    else
    {
        value = INT_MAX;
        for (auto m : valid_moves)
        {
            D2VECTOR board_copy = board;
            board_copy[m[0]][m[1]] = piece_type;

            D2VECTOR deads = get_deads(board_copy, 3 - piece_type);

            remove_deads(board_copy, deads);

            float curr_val = minimax(3 - piece_type, board, board_copy, max_depth - 1, steps + 1, a, b);
            value = min(value, curr_val);
            if (value <= a)
                return value;
            b = min(b, value);
        }
    }
    return value;
}

vector<int> alpha_beta(int piece_type, D2VECTOR &previous_board, D2VECTOR &board, int max_depth, int steps, float a, float b)
{
    D2VECTOR valid_moves = find_possible_moves(previous_board, board, piece_type);
    if (!valid_moves.size())
        return {};

    D2VECTOR moves;

    float value = INT_MIN;
    vector<pair<vector<int>, float>> value_map;

    D2VECTOR reward = {
        {-100, 0, 5, 0, -100},
        {
            0,
            5,
            10,
            5,
            0,
        },
        {5, 10, 100, 10, 5},
        {0, 5, 10, 5, 0},
        {-100, 0, 5, 0, -100}};

    for (auto m : valid_moves)
    {

        D2VECTOR board_copy = board;
        board_copy[m[0]][m[1]] = piece_type;

        D2VECTOR deads = get_deads(board_copy, 3 - piece_type);
        remove_deads(board_copy, deads);

        float minimax_val = minimax(3 - piece_type, board, board_copy, max_depth - 1, steps + 1, a, b);
        int liberty = get_liberties(board_copy, m[0], m[1]);
        int defense = get_defense(board_copy, m[0], m[1]);

        float curr_val = minimax_val + deads.size() + 0.01 * reward[m[0]][m[1]];

        if (curr_val > value || moves.size() == 0)
        {
            value = curr_val;
            a = curr_val;
            moves.push_back(m);
        }
        value_map.push_back({{m[0], m[1]}, curr_val});
    }
    for (auto val : value_map)
        if (val.second == value)
            return {val.first[0], val.first[1]};

    return {};
}

bool check_if_board_is_empty(D2VECTOR &prev_board)
{
    for (int i = 0; i < board_side; i++)
        for (int j = 0; j < board_side; j++)
            if (prev_board[i][j] != 0)
                return false;

    return true;
}
int main()
{
    fstream input_file, step_file, output_file;
    D2VECTOR prev_board, board;
    int piece_type, max_depth = 4, steps = 0;

    input_file.open("input.txt", ios::in);
    output_file.open("output.txt", ios::out | ios::trunc);

    if (!input_file)
        cout << "input file not detected";
    else
    {
        input_file >> piece_type;
        my_piece = piece_type;

        for (int i = 0; i < board_side; i++)
        {
            string line;
            vector<int> temp;
            input_file >> line;
            for (auto i : line)
                temp.push_back(i - '0');
            prev_board.push_back(temp);
        }

        for (int i = 0; i < board_side; i++)
        {
            string line;
            vector<int> temp;
            input_file >> line;
            for (auto i : line)
                temp.push_back(i - '0');
            board.push_back(temp);
        }
    }
    bool empty_prev_board = check_if_board_is_empty(prev_board);

    if (empty_prev_board)
    {
        steps = piece_type;
        step_file.open("step.txt", ios::out | ios::trunc);
        step_file << steps;
        step_file.close();
    }
    else
    {
        step_file.open("step.txt", ios::in);
        step_file >> steps;
        step_file.close();

        step_file.open("step.txt", ios::out | ios::trunc);
        step_file << steps + 2;
        step_file.close();
    }

    if (check_if_board_is_empty(board) || (piece_type == 2 && board[2][2] == 0))
    {
        string output_string = "2,2";
        output_file << output_string;
        output_file.close();
    }
    else if (steps == 2 && board[2][1] == 0)
    {
        string output_string = "2,1";
        output_file << output_string;
        output_file.close();
    }
    else
    {
        vector<int> action = alpha_beta(piece_type, prev_board, board, max_depth, steps, INT_MIN, INT_MAX);
        string action_string = "";
        for (int i = 0; i < action.size(); i++)
        {
            action_string += to_string(action[i]);
            if (i == 0)
                action_string += ',';
        }

        if (!action_string.size())
        {
            output_file << "PASS";
            output_file.close();
        }
        else
        {
            output_file << action_string;
            output_file.close();
        }
    }

    return 0;
}