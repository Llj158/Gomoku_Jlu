#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

const int SIZE = 15;
const int EMPTY = 0;
const int PLAYER = 1;
const int OPPONENT = -1;
const int WIN_SCORE = 10000;

int board[15][15] = { 0 };

struct Pattern {
    string pattern;
    int score;
};

vector<Pattern> patterns = {
    {"11111", 50000},
    {"011110", 4320},
    {"011100", 720},
    {"001110", 720},
    {"011010", 720},
    {"010110", 720},
    {"11110", 720},
    {"01111", 720},
    {"11011", 720},
    {"10111", 720},
    {"11101", 720},
    {"001100", 120},
    {"001010", 120},
    {"010100", 120},
    {"000100", 20},
    {"001000", 20},
};

int evaluatePattern(const string& line, int player) {
    int score = 0;
    for (auto i : patterns)
    {
        auto pos = line.find(i.pattern);
        if (pos != string::npos)
        {
            score += i.score;
            break;
        }
    }
    return player == PLAYER ? score : -score;
    // 根据玩家角色调整分数正负
}

int evaluate(int x,int y,int player) {
    int totalScore = 0;
    string line[4];
    for (int i = 0; i < SIZE; i++)
    {
        line[0].push_back(board[i][y] == 1 ? '1' : board[i][y] == 0 ? '0'
                                                                          : '2');
    }
    totalScore += evaluatePattern(line[0], player);

    for (int i = 0; i < SIZE; i++)
    {
        line[1].push_back(board[x][i] == 1 ? '1' : board[x][i] == 0 ? '0'
                                                                          : '2');
    }
    totalScore += evaluatePattern(line[1], player);

    for (int i = x + min(y, SIZE - 1 - x), j = y - min(x, SIZE - 1 - y); i >= 0 && j < SIZE; i--, j++)
    {
        line[2].push_back(board[i][j] == 1 ? '1' : board[i][j] == 0 ? '0'
                                                                          : '2');
    }
    totalScore += evaluatePattern(line[2], player);

    for (int i = x - min(x, y), j = y - min(x, y); i < SIZE && j < SIZE; i++, j++)
    {
        line[3].push_back(board[i][j] == 1 ? '1' : board[i][j] == 0 ? '0'
                                                                          : '2');
    }
    totalScore += evaluatePattern(line[3], player);

    return totalScore;
}

int minimax(int depth, int alpha, int beta, bool maximizingPlayer, int player,int x,int y) {
    if (depth == 0) {
        return evaluate(x, y, player);
    }

    if (maximizingPlayer) {
        int maxEval = INT32_MIN;
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = player;
                    int eval = minimax(depth - 1, alpha, beta, false, player, i, j);
                    maxEval = max(maxEval, eval);
                    alpha = max(alpha, eval);
                    board[i][j] = EMPTY;
                    if (beta <= alpha) {
                        return maxEval;
                    }
                }
            }
        }
        return maxEval;
    }
    else {
        int minEval = INT32_MAX;
        for (int i = 0; i < SIZE; ++i) {
            for (int j = 0; j < SIZE; ++j) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = -player;
                    int eval = minimax(depth - 1, alpha, beta, true, player, i, j);
                    minEval = min(minEval, eval);
                    beta = min(beta, eval);
                    board[i][j] = EMPTY;
                    if (beta <= alpha) {
                        return minEval;
                    }
                }
            }
        }
        return minEval;
    }
}

pair<int, int> nextMove(int player) {
    int bestScore = INT32_MIN;
    int bestMoveX = -1;
    int bestMoveY = -1;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == EMPTY) {
                board[i][j] = player;
                int score = minimax(2, INT32_MIN, INT32_MAX, false, player, i, j);
                board[i][j] = EMPTY;
                if (score > bestScore) {
                    bestScore = score;
                    bestMoveX = i;
                    bestMoveY = j;
                }
            }
        }
    }
    return make_pair(bestMoveX, bestMoveY);
}


int main()
{
    int x, y, n;
    // 恢复目前的棋盘信息
    cin >> n;
    for (int i = 0; i < n - 1; i++)
    {
        cin >> x >> y;
        if (x != -1)
            board[x][y] = -1; // 对方
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 1; // 我方
    }
    cin >> x >> y;
    if (x != -1)
        board[x][y] = -1; // 对方

    // 此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

    /************************************************************************************/
    /***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
    int new_x, new_y;

    if (n == 1)
    {
        if (x == -1)
        {
            // 第一回合我方先手，下在棋盘中央
            new_x = 7;
            new_y = 7;
        }
        else
        {
            new_x = x + 1;
            new_y = y + 1;
        }
    }
    else
    {
        // 执行最优的下一步
        pair<int, int> move = nextMove(1);
        new_x = move.first;
        new_y = move.second;
    }
    /***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
    /************************************************************************************/

    // 向平台输出决策结果
    printf("%d %d\n", new_x, new_y);
    return 0;
}
