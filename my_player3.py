from sys import maxsize
from time import time
from copy import deepcopy
from write import writeOutput, writeStep
from read import readInput, readStep

N = 5

startTime = -1
max_steps = 24
my_piece = -1

neighbour_indexes = [(1, 0), (0, 1), (-1, 0), (0, -1)]
defense_indexes = [(-1, 1), (1, 1), (-1, -1), (1, -1)]


def in_range(i, j):
    if i < 0 or i >= N:
        return False
    elif j < 0 or j >= N:
        return False

    return True


def get_defense(i, j, board):
    count = 0

    for d in defense_indexes:
        new_i = d[0] + i
        new_j = d[1] + j
        if in_range(new_i, new_j):
            if board[new_i][new_j] == board[i][j]:
                count += 1

    return count


def get_neighbours(i, j):
    neighbours = []

    for n in neighbour_indexes:
        new_i = n[0] + i
        new_j = n[1] + j
        if in_range(new_i, new_j):
            neighbours.append((new_i, new_j))

    return neighbours


def find_immediate_allies(i, j, board):
    allies = []
    neighbours = get_neighbours(i, j)

    for n in neighbours:
        if board[n[0]][n[1]] == board[i][j]:
            allies.append((n[0], n[1]))

    return allies


def find_all_allies(i, j, board):
    if board[i][j] == 0:
        return []

    stack = []
    allies = []
    stack.append((i, j))

    while stack:
        node = stack.pop(0)
        allies.append(node)
        for a in find_immediate_allies(node[0], node[1], board):
            if not a in allies and not a in stack:
                stack.append(a)

    return allies


def check_liberty(i, j, board):
    allies = find_all_allies(i, j, board)

    count = 0
    for a in allies:
        for n in get_neighbours(a[0], a[1]):
            if board[n[0]][n[1]] == 0:
                count += 1

    return count


def get_liberty(i, j, board):
    liberties = []
    allies = find_all_allies(i, j, board)

    for a in allies:
        for n in get_neighbours(a[0], a[1]):
            if board[n[0]][n[1]] == 0:
                liberties.append((n[0], n[1]))

    return liberties


def find_dead(board, piece_type):
    deads = []

    for i in range(N):
        for j in range(N):
            if (board[i][j] == piece_type) and ((i, j) not in deads):
                if not check_liberty(i, j, board):
                    deads.append((i, j))

    return deads


def remove_deads(board, deads):
    for d in deads:
        board[d[0]][d[1]] = 0

    return board


def compare_states(board1, board2):
    for i in range(N):
        for j in range(N):
            if board1[i][j] != board2[i][j]:
                return False

    return True


def valid_move(i, j, previous_board, board, piece_type):
    if not in_range(i, j):
        return False

    if board[i][j] != 0:
        return False

    next_board_state = deepcopy(board)
    next_board_state[i][j] = piece_type

    if check_liberty(i, j, next_board_state):
        return True

    deads = find_dead(next_board_state, 3 - piece_type)

    if not deads:
        return False

    board_after_removing_deads = remove_deads(next_board_state, deads)
    return not compare_states(previous_board, board_after_removing_deads)


def worth_move(i, j, board, piece_type):
    if board[i][j] == 0:
        neighbours = get_neighbours(i, j)
        for n in neighbours:
            if board[n[0]][n[1]] != 0 and board[n[0]][n[1]] == 3 - piece_type:  # can improve here
                return True
    return False


def find_possible_moves(previous_board, board, piece_type):
    possible_moves, worth_moves = [], []

    # for i in range(N):
    #     for j in range(N):
    #         if (i, j) not in possible_moves:
    #             if board[i][j] == 3 - piece_type:
    #                 possible_moves.extend(get_liberty(i, j, board))
    #             else:
    #                 liberties = get_liberty(i, j, board)
    #                 if len(liberties) == 1:
    #                     possible_moves.extend(liberties)

    # for move in possible_moves:
    #     if valid_move(i, j, previous_board, board, piece_type):
    #         worth_moves.append((i, j))

    # if worth_moves:
    #     return worth_moves

    # possible_moves = []

    for i in range(N):
        for j in range(N):
            if worth_move(i, j, board, piece_type):
                if valid_move(i, j, previous_board, board, piece_type):
                    possible_moves.append((i, j))

    return possible_moves


def get_piece_count(board, piece_type):
    count, on_risk = 0, 0

    for i in range(N):
        for j in range(N):
            if board[i][j] == piece_type:
                if check_liberty(i, j, board) <= 1:
                    on_risk += 1
                count += 1

    return count, on_risk


def get_score(board, piece_type):
    score, on_risk = get_piece_count(board, piece_type)

    if piece_type == 2:
        score += 2.5

    return score, on_risk


def heur(piece_type, board, black_deads, white_deads):
    black_stones, black_on_risk = get_score(board, 1)
    white_stones, white_on_risk = get_score(board, 2)

    if piece_type == 1:
        score = black_stones - white_stones
        score += white_on_risk - black_on_risk
        score += white_deads * 10 - black_deads * 16
    else:
        score = white_stones - black_stones
        score += black_on_risk - white_on_risk
        score += black_deads * 10 - white_deads * 16

    return score


def minimax(piece_type, previous_board, board, max_depth, steps,  a, b, black_deads, white_deads):
    if max_depth == 0 or steps > max_steps:
        return heur(piece_type, board, black_deads, white_deads)

    if piece_type == my_piece:
        value = -maxsize - 1
        valid_moves = find_possible_moves(previous_board, board, piece_type)
        for m in valid_moves:
            board_copy = deepcopy(board)
            board_copy[m[0]][m[1]] = piece_type

            deads = find_dead(board_copy, 3 - piece_type)

            if(piece_type == 1):
                white_deads += len(deads)
            else:
                black_deads += len(deads)

            board_copy = remove_deads(board_copy, deads)
            curr_val = minimax(3 - piece_type, board, board_copy,
                               max_depth - 1, steps + 1, a, b, black_deads, white_deads)

            value = max(value, curr_val)
            # beta prunning
            if value >= b:
                return value
            a = max(a, value)
    elif piece_type == 3 - my_piece:
        value = maxsize
        valid_moves = find_possible_moves(previous_board, board, piece_type)
        for m in valid_moves:
            board_copy = deepcopy(board)
            board_copy[m[0]][m[1]] = piece_type

            deads = find_dead(board_copy, 3 - piece_type)

            if(piece_type == 1):
                white_deads += len(deads)
            else:
                black_deads += len(deads)

            board_copy = remove_deads(board_copy, deads)

            curr_val = minimax(3 - piece_type, board, board_copy,
                               max_depth - 1, steps + 1, a, b, black_deads, white_deads)
            value = min(value, curr_val)
            # alpha prunning
            if value <= a:
                return value
            b = min(b, value)

    return value


def alpha_beta(piece_type, previous_board, board, max_depth, steps, a, b):
    valid_moves = find_possible_moves(previous_board, board, piece_type)
    if not valid_moves:
        return ()

    moves = []

    rewards = []
    rewards.append([-10, 0, 0.5, 0, -1])
    rewards.append([0, 0.5, 1, 0.5, 0])
    rewards.append([0.5, 1, 10, 1, 0.5])
    rewards.append([0, 0.5, 1, 0.5, 0])
    rewards.append([-10, 0, 0.5, 0, -10])

    value = -maxsize - 1
    value_map = {}
    for m in valid_moves:
        if(time() - startTime > 9600):
            break

        white_deads = 0
        black_deads = 0

        board_copy = deepcopy(board)
        board_copy[m[0]][m[1]] = piece_type

        deads = find_dead(board_copy, 3 - piece_type)

        if(piece_type == 1):
            white_deads = len(deads)
        else:
            black_deads = len(deads)

        board_copy = remove_deads(board_copy, deads)

        minimax_val = minimax(3 - piece_type, board, board_copy,
                              max_depth - 1, steps + 1, a, b, black_deads, white_deads)

        liberty = check_liberty(m[0], m[1], board_copy)
        defense = get_defense(m[0], m[1], board_copy)

        curr_val = minimax_val + \
            rewards[m[0]][m[1]] + liberty + defense + 10 * len(deads)

        if curr_val > value or not moves:
            value = curr_val
            a = curr_val
            moves = [m]
        value_map[m] = curr_val

    for val in value_map:
        if value_map[val] == value:
            return val

    # Driver program


if __name__ == "__main__":
    startTime = time()

    a = -maxsize - 1
    b = maxsize

    piece_type, previous_board, board = readInput(N)
    my_piece = piece_type
    steps = 0
    max_depth = 4

    # reading steps
    if previous_board == [[0]*5]*5:
        steps = piece_type
        writeStep(piece_type + 2)
    else:
        steps = readStep()
        writeStep(readStep() + 2)

    if (board == [[0]*5]*5) or (piece_type == 2 and board[2][2] == 0):
        writeOutput((2, 2))
    elif steps == 2:
        writeOutput((2, 1))
    else:
        action = alpha_beta(piece_type, previous_board,
                            board, max_depth, steps, a, b)
        if not action:
            writeOutput("PASS")
        else:
            writeOutput(action)

    endTime = time()
    print("time taken to make a move - ", endTime - startTime)
