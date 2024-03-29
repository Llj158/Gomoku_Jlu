#include <iostream>
#include "head.h"
#include "PossiblePositionManager.h"
#include "ACSearcher.h"

using namespace std;


/*------------------------全局变量-------------------------*/

int board[15][15] = { 0 };
long long boardZobristValue[2][SIZE][SIZE];
long long currentZobristValue; // 当前局面的zobrist值
HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];

int scores[2][72];  //保存棋局分数（2个角色72行，包括横竖撇捺）
int allScore[2];    //局面总评分（2个角色）

ACSearcher acs;

PossiblePositionManager ppm;


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

/*------------------------全局变量-------------------------*/


void recordHashItem(int depth, int score, HashItem::Flag flag)
{
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth)
    { // 如果当前条目已经有数据，且深度比当前深度大，直接返回
        return;
    }

    phashItem->checksum = currentZobristValue;
    phashItem->score = score;
    phashItem->flag = flag;
    phashItem->depth = depth;
}

// 在哈希表中取得计算好的局面的分数
int getHashItemScore(int depth, int alpha, int beta)
{
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag == HashItem::EMPTY)
        return UNKNOWN_SCORE;

    if (phashItem->checksum == currentZobristValue) // 校验和相同,如不相同则说明这个局面的数据已经被覆盖了
    {
        if (phashItem->depth >= depth)
        {
            if (phashItem->flag == HashItem::EXACT)
            {
                return phashItem->score;
            }
            if (phashItem->flag == HashItem::ALPHA && phashItem->score <= alpha)
            {
                return alpha;
            }
            if (phashItem->flag == HashItem::BETA && phashItem->score >= beta)
            {
                return beta;
            }
        }
    }

    return UNKNOWN_SCORE;
}

// 生成64位随机数
long long random64()
{
    return (long long)rand() | ((long long)rand() << 15) | ((long long)rand() << 30) | ((long long)rand() << 45) | ((long long)rand() << 60);
}
// 生成zobrist键值
void randomBoardZobristValue()
{
    int i, j, k;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < SIZE; j++)
        {
            for (k = 0; k < SIZE; k++)
            {
                boardZobristValue[i][j][k] = random64();
            }
        }
    }
}

// 初始化初始局面的zobrist值
void initCurrentZobristValue()
{
    currentZobristValue = random64();
}

// 存储搜索结果，即下一步棋子的位置
Position searchResult;

// 根据位置评分，其中board是当前棋盘，p是位置，role是评分角色，比如role是Human则是相对人类评分，比如role是computer则是对于电脑评分
int evaluatePoint(Position p)
{
    int result;
    int i, j;

    result = 0;
    int role = HUMAN; // 两个角色都要评分，所以这里先设为HUMAN

    string lines[4];
    string lines1[4];
    for (i = max(0, p.x - 5); i < (unsigned int)min(SIZE, p.x + 6); i++)
    {
        if (i != p.x)
        {
            lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0'
                                                                                : '2');
            lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0'
                                                                                 : '1');
        }
        else
        {
            lines[0].push_back('1');
            lines1[0].push_back('1');
        }
    }
    for (i = max(0, p.y - 5); i < (unsigned int)min(SIZE, p.y + 6); i++)
    {
        if (i != p.y)
        {
            lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0'
                                                                                : '2');
            lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0'
                                                                                 : '1');
        }
        else
        {
            lines[1].push_back('1');
            lines1[1].push_back('1');
        }
    }
    for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5); i < (unsigned int)min(SIZE, p.x + 6) && j < (unsigned int)min(SIZE, p.y + 6); i++, j++)
    {
        if (i != p.x)
        {
            lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                            : '2');
            lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0'
                                                                             : '1');
        }
        else
        {
            lines[2].push_back('1');
            lines1[2].push_back('1');
        }
    }
    for (i = p.x + min(min(p.y, SIZE - 1 - p.x), 5), j = p.y - min(min(p.y, SIZE - 1 - p.x), 5); i >= (unsigned int)max(0, p.x - 5) && j < (unsigned int)min(SIZE, p.y + 6); i--, j++)
    {
        if (i != p.x)
        {
            lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                            : '2');
            lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0'
                                                                             : '1');
        }
        else
        {
            lines[3].push_back('1');
            lines1[3].push_back('1');
        }
    }

    for (i = 0; i < 4; i++)
    {
        vector<int> tmp = acs.ACSearch(lines[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            result += patterns[tmp[j]].score;
        }

        tmp = acs.ACSearch(lines1[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            result += patterns[tmp[j]].score;
        }
    }

    return result;
}

void updateScore(Position p)
{

    string lines[4];
    string lines1[4];
    int i, j;
    int role = HUMAN;

    // 竖
    for (i = 0; i < SIZE; i++)
    {

        lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0'
                                                                            : '2');
        lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0'
                                                                             : '1');
    }
    // 横
    for (i = 0; i < SIZE; i++)
    {

        lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0'
                                                                            : '2');
        lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0'
                                                                             : '1');
    }
    // 反斜杠
    for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < SIZE && j < SIZE; i++, j++)
    {

        lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                        : '2');
        lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0'
                                                                         : '1');
    }
    // 斜杠
    for (i = p.x + min(p.y, SIZE - 1 - p.x), j = p.y - min(p.y, SIZE - 1 - p.x); i >= 0 && j < SIZE; i--, j++)
    {

        lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                        : '2');
        lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0'
                                                                         : '1');
    }

    int lineScore[4] = {0};
    int line1Score[4] = {0};

    // 计算分数
    for (i = 0; i < 4; i++)
    {
        vector<int> result = acs.ACSearch(lines[i]);
        for (j = 0; j < result.size(); j++)
        {
            lineScore[i] += patterns[result[j]].score;
        }

        result = acs.ACSearch(lines1[i]);
        for (j = 0; j < result.size(); j++)
        {
            line1Score[i] += patterns[result[j]].score;
        }
    }

    int a = p.y;
    int b = SIZE + p.x;
    int c = 2 * SIZE + (p.y - p.x + 10);
    int d = 2 * SIZE + 21 + (p.x + p.y - 4);
    // 减去以前的记录
    for (i = 0; i < 2; i++)
    {
        allScore[i] -= scores[i][a];
        allScore[i] -= scores[i][b];
    }

    // scores顺序 竖、横、\、/
    scores[0][a] = lineScore[0];
    scores[1][a] = line1Score[0];
    scores[0][b] = lineScore[1];
    scores[1][b] = line1Score[1];

    // 加上新的记录
    for (i = 0; i < 2; i++)
    {
        allScore[i] += scores[i][a];
        allScore[i] += scores[i][b];
    }

    if (p.y - p.x >= -10 && p.y - p.x <= 10) // 当棋子在两个对角线上围成的区域内时
    {

        for (i = 0; i < 2; i++)
            allScore[i] -= scores[i][c];

        scores[0][c] = lineScore[2];
        scores[1][c] = line1Score[2];

        for (i = 0; i < 2; i++)
            allScore[i] += scores[i][c];
    }

    if (p.x + p.y >= 4 && p.x + p.y <= 24)
    {

        for (i = 0; i < 2; i++)
            allScore[i] -= scores[i][d];

        scores[0][d] = lineScore[3];
        scores[1][d] = line1Score[3];

        for (i = 0; i < 2; i++)
            allScore[i] += scores[i][d];
    }
}

// 局面评估函数，给一个局面评分
int evaluateSituation(Role role)
{

    if (role == COMPUTOR)
    {
        return allScore[1];
    }
    else if (role == HUMAN)
    {
        return allScore[0];
    }

    cout << "error" << endl;

    return 0;
}

// alpha-beta剪枝
int abSearch(int depth, int alpha, int beta, Role currentSearchRole)
{
    HashItem::Flag flag = HashItem::ALPHA;
    int score = getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != DEPTH)
    {
        return score;
    }
    // 评估当前局面
    int score1 = evaluateSituation(currentSearchRole);
    int score2 = evaluateSituation(currentSearchRole == HUMAN ? COMPUTOR : HUMAN);

    if (score1 >= 50000)
    {
        return MAX_SCORE - 1000 - (DEPTH - depth); // 如果当前局面已经胜利，返回最大分数
    }
    if (score2 >= 50000)
    {
        return MIN_SCORE + 1000 + (DEPTH - depth);
    }

    if (depth == 0)
    {
        recordHashItem(depth, score1 - score2, HashItem::EXACT);
        return score1 - score2;
    }

    // set<Position> possiblePossitions = createPossiblePosition(board);

    int count = 0;
    set<Position> possiblePositions;                                               // 存储可能出现的位置
    const set<Position> &tmpPossiblePositions = ppm.GetCurrentPossiblePositions(); // 当前可能出现的位置

    // 对当前可能出现的位置进行粗略评分
    set<Position>::iterator iter;
    for (iter = tmpPossiblePositions.begin(); iter != tmpPossiblePositions.end(); iter++)
    {
        possiblePositions.insert(Position(iter->x, iter->y, evaluatePoint(*iter)));
    }

    // 对可能出现的位置进行评分排序
    while (!possiblePositions.empty())
    {
        Position p = *possiblePositions.begin();

        possiblePositions.erase(possiblePositions.begin());

        // 放置棋子
        board[p.x][p.y] = currentSearchRole;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(p);

        // 增加可能出现的位置
        p.score = 0;
        ppm.AddPossiblePositions(board, p);

        int val = -abSearch(depth - 1, -beta, -alpha, currentSearchRole == HUMAN ? COMPUTOR : HUMAN);


        // 取消上一次增加的可能出现的位置
        ppm.Rollback();

        // 取消放置
        board[p.x][p.y] = 0;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(p);

        if (val >= beta) // 当val >= beta时，当前节点不会被选择，所以直接返回beta
        {
            recordHashItem(depth, beta, HashItem::BETA);
            return beta;
        }
        if (val > alpha)
        {
            flag = HashItem::EXACT;
            alpha = val;
            if (depth == DEPTH)
            {
                searchResult = p;
            }
        }

        count++;
        if (count >= POINT_NUM) // 
        {
            break;
        }
    }

    recordHashItem(depth, alpha, flag);
    return alpha;
}

// 获得下一步的走法
Position getAGoodMove()
{
    int score = abSearch(DEPTH, MIN_SCORE, MAX_SCORE, COMPUTOR);
    if (score >= MAX_SCORE - 1000 - 1)
    {
        // winner = COMPUTOR;
    }
    else if (score <= MIN_SCORE + 1000 + 1)
    {
        // winner = HUMAN;
    }
    return searchResult;
}

// 初始化函数，插入特征和分值
void init()
{
    vector<string> patternStrs;
    for (size_t i = 0; i < patterns.size(); i++)
    {
        patternStrs.push_back(patterns[i].pattern);
    }

    // 初始化ACSearcher
    acs.LoadPattern(patternStrs);
    acs.BuildGotoTable();
    acs.BuildFailTable();

    randomBoardZobristValue();
    currentZobristValue = random64();
}


//人类下棋，传给界面
Position nextStep(int x, int y) {


    board[x][y] = HUMAN;
    currentZobristValue ^= boardZobristValue[HUMAN - 1][x][y];
    updateScore(Position(x, y));

    //增加可能出现的位置
    ppm.AddPossiblePositions(board, Position(x, y));

    Position result = getAGoodMove();

    board[result.x][result.y] = COMPUTOR;
    currentZobristValue ^= boardZobristValue[COMPUTOR - 1][result.x][result.y];
    updateScore(result);

    //增加可能出现的位置
    ppm.AddPossiblePositions(board, result);

    return result;
}
void updataSituation(int x, int y, int role)
{
	board[x][y] = role;
	currentZobristValue ^= boardZobristValue[role - 1][x][y];
	updateScore(Position(x, y));
    ppm.AddPossiblePositions(board, Position(x, y));
}


int evaluatePattern(const string &line)
{
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
    return score;
}
int evaluate(int x, int y, int player)
{
    int totalScore = 0;
    string line[4];
    for (int i = 0; i < SIZE; i++)
    {
        line[0].push_back(board[i][y] == player ? '1' : board[i][y] == 0 ? '0'
                                                                         : '2');
    }
    totalScore += evaluatePattern(line[0]);

    for (int i = 0; i < SIZE; i++)
    {
        line[1].push_back(board[x][i] == player ? '1' : board[x][i] == 0 ? '0'
                                                                         : '2');
    }
    totalScore += evaluatePattern(line[1]);

    for (int i = x + min(y, SIZE - 1 - x), j = y - min(x, SIZE - 1 - y); i >= 0 && j < SIZE; i--, j++)
    {
        line[2].push_back(board[i][j] == player ? '1' : board[i][j] == 0 ? '0'
                                                                         : '2');
    }
    totalScore += evaluatePattern(line[2]);

    for (int i = x - min(x, y), j = y - min(x, y); i < SIZE && j < SIZE; i++, j++)
    {
        line[3].push_back(board[i][j] == player ? '1' : board[i][j] == 0 ? '0'
                                                                         : '2');
    }
    totalScore += evaluatePattern(line[3]);

    return totalScore;
}
vector<int> s;
int minimax(int depth, int alpha, int beta, bool maximizingPlayer, int player, int x, int y, int lastx, int lasty)
{

    if (depth == 0)
    {
        int curscore;
        if (!maximizingPlayer)
            curscore = evaluate(x, y, -player) - evaluate(lastx, lasty, player); // 从a来，检测的为ai，player = -1
        else
            curscore = evaluate(lastx, lasty, player) - evaluate(x, y, -player);
        return curscore;
    }
    if (maximizingPlayer)
    {
        int maxEval = INT32_MIN;
        for (int i = 0; i < SIZE; ++i)
        {
            for (int j = 0; j < SIZE; ++j)
            {
                if (board[i][j] == EMPTY)
                {
                    board[i][j] = player;
                    int eval = minimax(depth - 1, alpha, beta, false, -player, i, j, x, y);
                    maxEval = max(maxEval, eval);
                    alpha = max(alpha, eval);
                    board[i][j] = EMPTY;
                    if (beta <= alpha)
                    {
                        return maxEval;
                    }
                }
            }
        }
        return maxEval;
    }
    else
    {

        int minEval = INT32_MAX;
        for (int i = 0; i < SIZE; ++i)
        {
            for (int j = 0; j < SIZE; ++j)
            {
                if (board[i][j] == EMPTY)
                {
                    board[i][j] = player;
                    int eval = minimax(depth - 1, alpha, beta, true, -player, i, j, x, y);
                    minEval = min(minEval, eval);
                    beta = min(beta, eval);
                    board[i][j] = EMPTY;
                    if (beta <= alpha)
                    {
                        return minEval;
                    }
                }
            }
        }

        return minEval;
    }
}

void nextMove(int player, int &new_x, int &new_y)
{
    int bestScore = INT32_MIN;
    int bestMoveX = -1;
    int bestMoveY = -1;
    for (int i = 0; i < SIZE; ++i)
    {
        for (int j = 0; j < SIZE; ++j)
        {
            if (board[i][j] == EMPTY)
            {
                board[i][j] = player;
                int score = minimax(1, INT32_MIN, INT32_MAX, false, -player, i, j, -1, -1);
                board[i][j] = EMPTY;
                if (score > bestScore)
                {
                    bestScore = score;
                    bestMoveX = i;
                    bestMoveY = j;
                }
            }
        }
    }
    new_x = bestMoveX;
    new_y = bestMoveY;
}

int main()
{
    init();
    int x, y, n;
    // 恢复目前的棋盘信息
    cin >> n;
    for (int i = 0; i < n - 1; i++)
    {
        cin >> x >> y;
        if (x != -1)
        {
            updataSituation(x, y, -1);
        }
        cin >> x >> y;
        if (x != -1)
        {
            updataSituation(x, y, 1);
        }
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
            // 执行最优的下一步
            Position p = nextStep(x, y);
            new_x = p.x;
            new_y = p.y;
        }
    }
    else
    {
        // 执行最优的下一步
        Position p = nextStep(x, y);
        new_x = p.x;
        new_y = p.y;
    }
    /***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
    /************************************************************************************/

    // 棋盘
    // 向平台输出决策结果
    printf("%d %d\n", new_x, new_y);
    return 0;
}
