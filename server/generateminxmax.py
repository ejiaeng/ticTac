from typing import Optional, List, Dict, Tuple
import math
import functools
import json

def check_winner(board) -> Optional[int]:
    win = [
        (0, 1, 2), (3, 4, 5), (6, 7, 8),  
        (0, 3, 6), (1, 4, 7), (2, 5, 8),  
        (0, 4, 8), (2, 4, 6)             
    ]
    
    for r1, r2, r3 in win:
        line_sum = board[r1] + board[r2] + board[r3]
        
        if line_sum == 3:
            return 1   
        if line_sum == -3:
            return -1  
            
    if 0 not in board:
        return 0  
        
    return None 


def main() -> dict:
    board = [0] * 9 
    d = dict()
    p = 0
    
    def solve(board, player, penalty) -> int:
        bt = tuple(board)
        if bt in d:
            return d[bt]

        winner = check_winner(board)
        if winner is not None:
            if winner == 1: return 10 - penalty  
            if winner == -1: return -10 + penalty 
            return 0                              

        if player == 1:
            best_score = -math.inf
            for i in range(9):
                if board[i] == 0:  
                    board[i] = 1
                    score = solve(board, -1, penalty + 1)
                    board[i] = 0
                    best_score = max(score, best_score)
            
            d[bt] = best_score
            return best_score
        else:
            best_score = math.inf
            for i in range(9):
                if board[i] == 0:  
                    board[i] = -1
                    score = solve(board, 1, penalty + 1)
                    board[i] = 0
                    best_score = min(score, best_score)
            
            d[bt] = best_score
            return best_score

    solve(board, 1, p)
    return d


if __name__ == "__main__":
    memo_table = main()
    print(f"Total board states evaluated and cached: {len(memo_table)}")
    
    json_table = {",".join(map(str, k)): v for k, v in memo_table.items()}
    
    with open("minimax.json", "w") as f:
        json.dump(json_table, f)