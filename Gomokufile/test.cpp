#include <iostream>
#include <queue>
#include <cassert>
#include <set>
#include <vector>
#include <string>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <time.h>

using namespace std;

/*---------------------------head-----------------------------*/

#define UNKNOWN_SCORE (10000001)
#define HASH_ITEM_INDEX_MASK (0xffff)
#define MAX_SCORE 10000000
#define MIN_SCORE -10000000
const int PLAYER = 1;
const int DEPTH = 8;
const int POINT_NUM = 5;
const int SIZE = 15;
const double timeLimit = 0.99;
const double iterLimit = 0.1;
const int firstPOINT_NUM = 8;
const int midPOINT_NUM = 85;
const int initDepth = 8;

int addflag = 2;
int doubleadd = 1;

enum Role
{
    HUMAN = 1,
    COMPUTOR = 2,
    EMPTY = 0
};

class Position
{
public:
    int x;
    int y;
    int score;
    int lineScore[2][4] = {0};
    Position(int x = 0, int y = 0, int score = 0)
    {
        this->x = x;
        this->y = y;
        this->score = score;
    }

    bool operator==(const Position &pos) const
    {
        return x == pos.x && y == pos.y;
    }
};

struct compare
{
    bool operator()(const Position &a, const Position &b)
    {
        if (a.score != b.score)
        {
            return a.score < b.score;
        }
        if (a.x != b.x)
        {
            return a.x > b.x;
        }
        else
        {
            return a.y > b.y;
        }
    }
};

struct PointHash
{
    std::size_t operator()(const Position &p) const
    {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
};

struct PointEqual
{
    bool operator()(const Position &lhs, const Position &rhs) const
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

// 保存棋局的哈希表条目
struct HashItem
{
    long long checksum; // 校验和
    int depth;          // 搜索深度
    int score;          // 分数
    enum Flag
    {
        ALPHA = 0,
        BETA = 1,
        EXACT = 2,
        EMPTY = 3
    } flag; // ALPHA剪枝，BETA剪枝，确切值，空
};

struct Pattern
{
    string pattern;
    int score;
};

/*---------------------------head-----------------------------*/

/*------------------------全局变量-------------------------*/
int board[15][15] = {0};
int scores[4][21][2]; // 保存棋局分数
int allScore[2];      // 局面总评分（2个角色）
vector<Pattern> patterns = {
    {"11111", 50000},
    {"011110", 10000},
    {"11110", 720},
    {"01111", 720},
    {"11011", 720},
    {"10111", 720},
    {"11101", 720},
    {"001110", 720},
    {"011100", 720},
    {"010110", 720},
    {"011010", 720},
    {"110011", 120},
    {"00111", 120},
    {"11100", 120},
    {"11010", 120},
    {"01011", 120},
    {"01101", 120},
    {"10110", 120},
    {"01110", 120},
    {"10101", 120},
    {"10011", 120},
    {"11001", 120},
    {"001100", 124},
    {"010100", 124},
    {"001010", 124},
    {"010010", 124},
    {"11000", 20},
    {"00011", 20},
    {"00101", 20},
    {"10100", 20},
    {"10010", 20},
    {"01001", 20},
    {"10001", 20}};

// priority_queue<Position, vector<Position>, compare> topPossiblePositions;
//  存储搜索结果，即下一步棋子的位置
Position searchResult;
clock_t startTime;
/*------------------------全局变量-------------------------*/

/*---------------------------ZobristHash-----------------------------*/

class ZobristHash
{
public:
    ZobristHash();
    void recordHashItem(int depth, int score, HashItem::Flag flag);
    int getHashItemScore(int depth, int alpha, int beta);
    long long getrandom64();
    void randomZobristValue();
    void initValue();

    long long boardZobristValue[2][SIZE][SIZE];
    long long currentZobristValue; // 当前局面的zobrist值
private:
    HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];
};

ZobristHash::ZobristHash()
{
    randomZobristValue();
    initValue();
}

void ZobristHash::recordHashItem(int depth, int score, HashItem::Flag flag)
{
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);

    if (hashItems[index].flag != HashItem::EMPTY && hashItems[index].depth > depth) // 如果当前条目已经有数据，且深度小于当前深度，则不覆盖
        return;
    else
    {
        hashItems[index].checksum = currentZobristValue;
        hashItems[index].depth = depth;
        hashItems[index].score = score;
        hashItems[index].flag = flag;
    }
}

// 在哈希表中取得计算好的局面的分数
int ZobristHash::getHashItemScore(int depth, int alpha, int beta)
{
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);

    if (hashItems[index].flag == HashItem::EMPTY)
        return UNKNOWN_SCORE;

    if (hashItems[index].checksum == currentZobristValue) // 校验和相同,如不相同则说明这个局面的数据已经被覆盖了
    {
        if (hashItems[index].depth >= depth)
        {
            if (hashItems[index].flag == HashItem::EXACT)
            {
                return hashItems[index].score;
            }
            if (hashItems[index].flag == HashItem::ALPHA && hashItems[index].score <= alpha)
            {
                return alpha;
            }
            if (hashItems[index].flag == HashItem::BETA && hashItems[index].score >= beta)
            {
                return beta;
            }
        }
    }

    return UNKNOWN_SCORE;
}

// 生成64位随机数
long long ZobristHash::getrandom64()
{
    return (long long)rand() | ((long long)rand() << 15) | ((long long)rand() << 30) | ((long long)rand() << 45) | ((long long)rand() << 60);
}

// 生成zobrist键值
void ZobristHash::randomZobristValue()
{
    for (int j = 0; j < SIZE; j++)
    {
        for (int k = 0; k < SIZE; k++)
        {
            for (int i = 0; i < 2; i++)
            {
                boardZobristValue[i][j][k] = getrandom64();
            }
        }
    }
}

// 初始化初始局面的zobrist值
void ZobristHash::initValue()
{
    currentZobristValue = getrandom64();
}

ZobristHash zh;

/*---------------------------ZobristHash-----------------------------*/

/*---------------------------ACSearcher-----------------------------*/

using namespace std;

// trie树节点
struct ACNode
{
    ACNode(int p, char c)
    {
        this->parent = p;
        this->ch = c;
        this->fail = -1;
    }

    char ch;
    unordered_map<char, int> sons;
    int fail;
    vector<int> output;
    int parent;
};

// AC算法类
class ACSearcher
{
public:
    ACSearcher();

    void LoadPattern(const vector<string> &paterns);
    void BuildGotoTable();
    void BuildFailTable();
    vector<int> ACSearch(const string &text); // 返回匹配到的模式的索引

    int maxState;           // 最大状态数
    vector<ACNode> nodes;   // trie树
    vector<string> paterns; // 需要匹配的模式

    void AddState(int parent, char ch); // 初始化新状态
};

ACSearcher::ACSearcher()
{
    this->maxState = 0;
    // 初始化根节点
    AddState(-1, 'a');
    ACNode *temp = &nodes[0];
    temp->fail = -1;
}

void ACSearcher::LoadPattern(const vector<string> &paterns)
{
    this->paterns = paterns;
}

void ACSearcher::BuildGotoTable()
{
    assert(nodes.size());

    for (int i = 0; i < paterns.size(); i++)
    {
        // 从根节点开始
        int currentIndex = 0;
        // for (int j = 0; j < paterns[i].size(); j++) {
        //     if (nodes[currentIndex].sons.find(paterns[i][j]) == nodes[currentIndex].sons.end()) {
        //         maxState++;
        //         nodes[currentIndex].sons[paterns[i][j]] = maxState;

        //        //生成新节点
        //        AddState(currentIndex, paterns[i][j]);
        //        currentIndex = maxState;
        //    }
        //    else {
        //        currentIndex = nodes[currentIndex].sons[paterns[i][j]];
        //    }
        //}
        ACNode *node_ptr = &nodes[currentIndex];
        for (auto &x : paterns[i])
        {
            if (node_ptr->sons.find(x) == nodes[currentIndex].sons.end())
            {
                maxState++;
                node_ptr->sons[x] = maxState;

                AddState(currentIndex, x);
                currentIndex = maxState;
            }
            else
            {
                currentIndex = node_ptr->sons[x];
            }
            node_ptr = &nodes[currentIndex];
        }

        node_ptr->output.push_back(i);
    }
}

void ACSearcher::BuildFailTable()
{
    assert(nodes.size());

    // 中间节点收集器
    vector<int> midNodesIndex;

    // 给第一层的节点设置fail为0，并把第二层节点加入到midState里
    ACNode &root = nodes[0];

    for (auto &iter : root.sons)
    {
        nodes[iter.second].fail = 0;
        for (auto &iter2 : nodes[iter.second].sons)
        {
            midNodesIndex.push_back(iter2.second);
        }
    }

    // 广度优先遍历
    while (!midNodesIndex.empty())
    {
        vector<int> newMidNodesIndex;

        int i;
        for (i = 0; i < midNodesIndex.size(); i++)
        {
            ACNode *currentNode = &nodes[midNodesIndex[i]];

            // 以下循环为寻找当前节点的fail值
            int currentFail = nodes[currentNode->parent].fail;
            while (true)
            {
                ACNode &currentFailNode = nodes[currentFail];

                if (currentFailNode.sons.find(currentNode->ch) != currentFailNode.sons.end())
                {
                    // 成功找到该节点的fail值
                    currentNode->fail = currentFailNode.sons.find(currentNode->ch)->second;

                    // 后缀包含
                    if (nodes[currentNode->fail].output.size())
                    {
                        currentNode->output.insert(currentNode->output.end(), nodes[currentNode->fail].output.begin(), nodes[currentNode->fail].output.end());
                    }
                    break;
                }
                else
                {
                    currentFail = currentFailNode.fail;
                }

                // 如果是根节点
                if (currentFail == -1)
                {
                    currentNode->fail = 0;
                    break;
                }
            }

            // 收集下一层节点
            for (auto &iter : currentNode->sons)
            {
                newMidNodesIndex.push_back(iter.second);
            }
        }
        midNodesIndex = newMidNodesIndex;
    }
}

vector<int> ACSearcher::ACSearch(const string &text)
{
    vector<int> result;
    // 初始化为根节点
    int currentIndex = 0;
    ACNode *node_ptr = &nodes[currentIndex];
    int i;
    // unordered_map<char, int>::iterator tmpIter;

    for (i = 0; i < text.size();)
    {
        // 顺着trie树查找
        auto tmpIter = node_ptr->sons.find(text[i]);
        if (tmpIter != node_ptr->sons.end())
        {
            currentIndex = tmpIter->second;
            i++;
        }
        else
        {
            // 失配的情况
            while (node_ptr->fail != -1 && node_ptr->sons.find(text[i]) == node_ptr->sons.end())
            {
                currentIndex = node_ptr->fail;
                node_ptr = &nodes[currentIndex];
            }

            // 如果没有成功找到合适的fail
            if (node_ptr->sons.find(text[i]) == node_ptr->sons.end())
            {
                i++;
            }
        }
        node_ptr = &nodes[currentIndex];

        if (!node_ptr->output.empty())
        {
            result.insert(result.end(), node_ptr->output.begin(), node_ptr->output.end());
        }
    }

    return result;
}

void ACSearcher::AddState(int parent, char ch)
{
    this->nodes.push_back(ACNode(parent, ch));
    assert(nodes.size() - 1 == maxState);
}

ACSearcher acs;

/*---------------------------ACSearcher-----------------------------*/

/*-----------------------------启发评估函数--------------------------------*/
// 根据位置评分，其中board是当前棋盘，p是位置，role是评分角色，比如role是Human则是相对人类评分，比如role是computer则是对于电脑评分
int evaluatePoint(Position &p)
{
    int result;
    int i, j;

    result = 0;
    int role = HUMAN; // 两个角色都要评分，所以这里先设为HUMAN

    string lines[4];
    string lines1[4];
    for (i = max(0, p.x - 5); i < min(SIZE, p.x + 6); i++)
    {
        if (i != p.x)
        {
            if (board[i][p.y] == role)
            {
                lines[0].push_back('1');
                lines1[0].push_back('2');
            }
            else if (board[i][p.y] == 0)
            {
                lines[0].push_back('0');
                lines1[0].push_back('0');
            }
            else
            {
                lines[0].push_back('2');
                lines1[0].push_back('1');
            }
        }
        else
        {
            lines[0].push_back('1');
            lines1[0].push_back('1');
        }
    }
    for (i = max(0, p.y - 5); i < min(SIZE, p.y + 6); i++)
    {
        if (i != p.y)
        {
            if (board[p.x][i] == role)
            {
                lines[1].push_back('1');
                lines1[1].push_back('2');
            }
            else if (board[p.x][i] == 0)
            {
                lines[1].push_back('0');
                lines1[1].push_back('0');
            }
            else
            {
                lines[1].push_back('2');
                lines1[1].push_back('1');
            }
        }
        else
        {
            lines[1].push_back('1');
            lines1[1].push_back('1');
        }
    }
    for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5); i < min(SIZE, p.x + 6) && j < min(SIZE, p.y + 6); i++, j++)
    {
        if (i != p.x)
        {
            if (board[i][j] == role)
            {
                lines[2].push_back('1');
                lines1[2].push_back('2');
            }
            else if (board[i][j] == 0)
            {
                lines[2].push_back('0');
                lines1[2].push_back('0');
            }
            else
            {
                lines[2].push_back('2');
                lines1[2].push_back('1');
            }
        }
        else
        {
            lines[2].push_back('1');
            lines1[2].push_back('1');
        }
    }
    for (i = p.x + min(min(p.y, SIZE - 1 - p.x), 5), j = p.y - min(min(p.y, SIZE - 1 - p.x), 5); i >= max(0, p.x - 5) && j < min(SIZE, p.y + 6); i--, j++)
    {
        if (i != p.x)
        {
            if (board[i][j] == role)
            {
                lines[3].push_back('1');
                lines1[3].push_back('2');
            }
            else if (board[i][j] == 0)
            {
                lines[3].push_back('0');
                lines1[3].push_back('0');
            }
            else
            {
                lines[3].push_back('2');
                lines1[3].push_back('1');
            }
        }
        else
        {
            lines[3].push_back('1');
            lines1[3].push_back('1');
        }
    }

    int score_human = 0;
    int score_ai = 0;

    for (i = 0; i < 4; i++)
    {
        vector<int> tmp = acs.ACSearch(lines[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            Pattern *temp = &patterns[tmp[j]];
            p.lineScore[0][i] += temp->score;
        }
        tmp = acs.ACSearch(lines1[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            Pattern *temp = &patterns[tmp[j]];
            p.lineScore[1][i] += temp->score;
        }
    }

    int w_1 = 0;
    int w_2 = 0;

    for (int i = 0; i < 4; i++)
    {
        if (p.lineScore[0][i] >= 720)
        {
            w_1++;
        }

        score_human += p.lineScore[0][i];
        if (p.lineScore[1][i] == 720)
        {
            w_2++;
        }
        score_ai += p.lineScore[1][i];
    }

    if (w_1 >= 2)
    {
        score_human *= w_1;
    }
    if (w_2 >= 2)
    {
        score_ai *= w_2;
    }

    result = score_ai + score_human;

    return result;
}

int evaluatePointline(Position &p, int x) // 横
{
    int result;
    int i, j;

    result = 0;
    int role = HUMAN; // 两个角色都要评分，所以这里先设为HUMAN

    string lines;
    string lines1;
    switch (x)
    {
    case 0:
        for (i = max(0, p.x - 5); i < min(SIZE, p.x + 6); i++)
        {
            if (i != p.x)
            {
                if (board[i][p.y] == role)
                {
                    lines.push_back('1');
                    lines1.push_back('2');
                }
                else if (board[i][p.y] == 0)
                {
                    lines.push_back('0');
                    lines1.push_back('0');
                }
                else
                {
                    lines.push_back('2');
                    lines1.push_back('1');
                }
            }
            else
            {
                lines.push_back('1');
                lines1.push_back('1');
            }
        }
        break;
    case 1:
        for (i = max(0, p.y - 5); i < min(SIZE, p.y + 6); i++)
        {
            if (i != p.y)
            {
                if (board[p.x][i] == role)
                {
                    lines.push_back('1');
                    lines1.push_back('2');
                }
                else if (board[p.x][i] == 0)
                {
                    lines.push_back('0');
                    lines1.push_back('0');
                }
                else
                {
                    lines.push_back('2');
                    lines1.push_back('1');
                }
            }
            else
            {
                lines.push_back('1');
                lines1.push_back('1');
            }
        }
        break;
    case 2:
        for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5); i < min(SIZE, p.x + 6) && j < min(SIZE, p.y + 6); i++, j++)
        {
            if (i != p.x)
            {
                if (board[i][j] == role)
                {
                    lines.push_back('1');
                    lines1.push_back('2');
                }
                else if (board[i][j] == 0)
                {
                    lines.push_back('0');
                    lines1.push_back('0');
                }
                else
                {
                    lines.push_back('2');
                    lines1.push_back('1');
                }
            }
            else
            {
                lines.push_back('1');
                lines1.push_back('1');
            }
        }
        break;
    case 3:
        for (i = p.x + min(min(p.y, SIZE - 1 - p.x), 5), j = p.y - min(min(p.y, SIZE - 1 - p.x), 5); i >= max(0, p.x - 5) && j < min(SIZE, p.y + 6); i--, j++)
        {
            if (i != p.x)
            {
                if (board[i][j] == role)
                {
                    lines.push_back('1');
                    lines1.push_back('2');
                }
                else if (board[i][j] == 0)
                {
                    lines.push_back('0');
                    lines1.push_back('0');
                }
                else
                {
                    lines.push_back('2');
                    lines1.push_back('1');
                }
            }
            else
            {
                lines.push_back('1');
                lines1.push_back('1');
            }
        }
        break;
    }

    int score_human = 0;
    int score_ai = 0;

    vector<int> tmp = acs.ACSearch(lines);
    vector<int> tmp1 = acs.ACSearch(lines1);
    p.lineScore[0][x] = 0;
    p.lineScore[1][x] = 0;

    for (j = 0; j < tmp.size(); j++)
    {
        p.lineScore[0][x] += patterns[tmp[j]].score;
    }

    for (j = 0; j < tmp1.size(); j++)
    {
        p.lineScore[1][x] += patterns[tmp1[j]].score;
    }

    int w_1 = 0;
    int w_2 = 0;

    for (int i = 0; i < 4; i++)
    {
        if (p.lineScore[0][i] >= 720)
        {
            w_1++;
        }

        score_human += p.lineScore[0][i];
        if (p.lineScore[1][i] == 720)
        {
            w_2++;
        }
        score_ai += p.lineScore[1][i];
    }

    if (w_1 >= 2)
    {
        score_human *= w_1;
    }
    if (w_2 >= 2)
    {
        score_ai *= w_2;
    }

    result = score_ai + score_human;

    return result;
}

/*-----------------------------启发评估函数--------------------------------*/

/*---------------------------PossiblePositionManager-----------------------------*/

using namespace std;

struct PosHistory
{
    unordered_set<Position, PointHash, PointEqual> newPositions;
    Position removedPosition;
    unordered_set<Position, PointHash, PointEqual> UpdataPositions;
};

class PossiblePositionManager
{
public:
    PossiblePositionManager();
    void AddPossiblePositions(int board[SIZE][SIZE], const Position &p);
    void AddPossiblePositions2(int board[SIZE][SIZE], const Position &p);
    void InitPossiblePositions(int board[SIZE][SIZE]);
    void ScoringAddPossiblePositions(int board[SIZE][SIZE], const Position &p);
    void Rollback();
    const unordered_set<Position, PointHash, PointEqual> &GetCurrentPossiblePositions();
    // set<Position> pruningPositions;
private:
    unordered_set<Position, PointHash, PointEqual> currentPossiblePositions;
    vector<PosHistory> allHistory;
    vector<pair<int, int>> directions;
};

bool isValidPosition(const Position &pos)
{
    bool temp = pos.x >= 0 && pos.x < SIZE && pos.y >= 0 && pos.y < SIZE;
    return temp;
}

PossiblePositionManager::PossiblePositionManager()
{
    directions.emplace_back(pair<int, int>(1, 1));
    directions.emplace_back(pair<int, int>(1, -1));
    directions.emplace_back(pair<int, int>(-1, 1));
    directions.emplace_back(pair<int, int>(-1, -1));
    directions.emplace_back(pair<int, int>(1, 0));
    directions.emplace_back(pair<int, int>(0, 1));
    directions.emplace_back(pair<int, int>(-1, 0));
    directions.emplace_back(pair<int, int>(0, -1));
}

void PossiblePositionManager::AddPossiblePositions(int board[SIZE][SIZE], const Position &p)
{

    unordered_set<Position, PointHash, PointEqual> newPositions;
    for (const auto &direct : directions)
    {
        Position newPos(p.x + direct.first, p.y + direct.second);
        if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
        {
            auto insertResult = currentPossiblePositions.insert(newPos);

            // 如果插入成功
            if (insertResult.second)
                newPositions.insert(newPos);
        }
    }

    PosHistory *ph = new PosHistory(); // 新增历史记录
    ph->newPositions = newPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) // 当前选中位置原本为可选位置，现选择后不可能选择，删除
    {
        currentPossiblePositions.erase(p);
        ph->removedPosition = p;
    }
    else
        ph->removedPosition.x = -1; // 给回溯提供标志

    allHistory.push_back(*ph); // 保存历史记录
}
void PossiblePositionManager::AddPossiblePositions2(int board[SIZE][SIZE], const Position &p)
{

    // 加入下了的棋外围两格的点
    unordered_set<Position, PointHash, PointEqual> newPositions;
    for (const auto &direct : directions)
    {
        Position newPos(p.x + direct.first, p.y + direct.second);
        if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
        {

            auto insertResult = currentPossiblePositions.insert(newPos);

            // 如果插入成功
            if (insertResult.second)
                newPositions.insert(newPos);
        }
    }
    for (const auto &direct : directions)
    {
        Position newPos(p.x + 2 * direct.first, p.y + 2 * direct.second);
        if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
        {
            auto insertResult = currentPossiblePositions.insert(newPos);

            // 如果插入成功
            if (insertResult.second)
                newPositions.insert(newPos);
        }
    }

    PosHistory *ph = new PosHistory(); // 新增历史记录
    ph->newPositions = newPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) // 当前选中位置原本为可选位置，现选择后不可能选择，删除
    {
        currentPossiblePositions.erase(p);
        ph->removedPosition = p;
    }
    else
        ph->removedPosition.x = -1; // 给回溯提供标志

    allHistory.emplace_back(*ph); // 保存历史记录
}

void PossiblePositionManager::InitPossiblePositions(int board[SIZE][SIZE])
{
    unordered_set<Position, PointHash, PointEqual> tempPositions;
    for (auto &it : currentPossiblePositions)
    {
        Position p = it;
        p.score = evaluatePoint(p);
        tempPositions.insert(p);
    }
    currentPossiblePositions = tempPositions;
}

void PossiblePositionManager::ScoringAddPossiblePositions(int board[SIZE][SIZE], const Position &p)
{
    int i, j;
    unordered_set<Position, PointHash, PointEqual> newPositions;
    unordered_set<Position, PointHash, PointEqual> UpdataPositions;
    PosHistory ph; // 新增历史记录
    // 更新已有可能落子点的分数————p的四个方向十个内的可能落子点

    for (i = max(0, p.x - 5); i < min(SIZE, p.x + 6); i++)
    {
        if (i != p.x)
        {
            auto newPos = Position(i, p.y);
            auto it = currentPossiblePositions.find(newPos);
            if (it != currentPossiblePositions.end())
            {
                UpdataPositions.insert(*it); // 记录需要更新的点
                newPos = *it;
                newPos.score = evaluatePointline(newPos, 0);
                currentPossiblePositions.erase(it); // 替换
                currentPossiblePositions.insert(newPos);
                newPositions.insert(newPos);
            }
        }
    }
    for (i = max(0, p.y - 5); i < min(SIZE, p.y + 6); i++)
    {
        if (i != p.y)
        {
            auto newPos = Position(p.x, i);
            auto it = currentPossiblePositions.find(newPos);
            if (it != currentPossiblePositions.end())
            {
                UpdataPositions.insert(*it); // 记录需要更新的点
                newPos = *it;
                newPos.score = evaluatePointline(newPos, 1);
                currentPossiblePositions.erase(it); // 替换
                currentPossiblePositions.insert(newPos);
                newPositions.insert(newPos);
            }
        }
    }
    for (i = p.x - min(min(p.x, p.y), 5), j = p.y - min(min(p.x, p.y), 5); i < min(SIZE, p.x + 6) && j < min(SIZE, p.y + 6); i++, j++)
    {
        if (i != p.x)
        {
            auto newPos = Position(i, j);
            auto it = currentPossiblePositions.find(newPos);
            if (it != currentPossiblePositions.end())
            {
                UpdataPositions.insert(*it); // 记录需要更新的点
                newPos = *it;
                newPos.score = evaluatePointline(newPos, 2);
                currentPossiblePositions.erase(it); // 替换
                currentPossiblePositions.insert(newPos);
                newPositions.insert(newPos);
            }
        }
    }
    for (i = p.x + min(min(p.y, SIZE - 1 - p.x), 5), j = p.y - min(min(p.y, SIZE - 1 - p.x), 5); i >= max(0, p.x - 5) && j < min(SIZE, p.y + 6); i--, j++)
    {
        if (i != p.x)
        {
            auto newPos = Position(i, j);
            auto it = currentPossiblePositions.find(newPos);
            if (it != currentPossiblePositions.end())
            {
                UpdataPositions.insert(*it); // 记录需要更新的点
                newPos = *it;
                newPos.score = evaluatePointline(newPos, 3);
                currentPossiblePositions.erase(it); // 替换
                currentPossiblePositions.insert(newPos);
                newPositions.insert(newPos);
            }
        }
    }

    for (const auto &direct : directions) // 增加新可能落子点
    {
        Position newPos(p.x + direct.first, p.y + direct.second);

        if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
        {
            if (currentPossiblePositions.find(newPos) != currentPossiblePositions.end())
                continue;
            newPos.score = evaluatePoint(newPos);
            auto insertResult = currentPossiblePositions.insert(newPos);

            // 如果插入成功
            if (insertResult.second)
                newPositions.insert(newPos);
        }
    }

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end())
    {
        currentPossiblePositions.erase(p);
        ph.removedPosition = p;
    }
    else
        ph.removedPosition.x = -1;

    ph.newPositions = newPositions;
    ph.UpdataPositions = UpdataPositions;
    allHistory.push_back(ph); // 保存历史记录
}

void PossiblePositionManager::Rollback()
{
    if (currentPossiblePositions.empty())
        return;

    PosHistory hi = allHistory.back();
    allHistory.pop_back();

    // 回溯

    // 清除掉前一步加入的点
    for (auto &pos : hi.newPositions)
        currentPossiblePositions.erase(pos);

    // 加回前一步删除的点
    if (hi.removedPosition.x != -1)
        currentPossiblePositions.insert(hi.removedPosition);

    // 更新前一步更新的点
    for (auto &pos : hi.UpdataPositions)
    {
        currentPossiblePositions.insert(pos); // 为了更新被删除的点
    }
}

const unordered_set<Position, PointHash, PointEqual> &PossiblePositionManager::GetCurrentPossiblePositions()
{
    return currentPossiblePositions;
}

PossiblePositionManager pp_manager;

/*---------------------------PossiblePositionManager-----------------------------*/

void updateScore(Position p)
{

    string lines[4];
    string lines1[4];

    int i, j;
    int role = HUMAN;

    // 竖
    for (i = 0; i < SIZE; i++)
    {

        if (board[i][p.y] == role)
        {
            lines[0].push_back('1');
            lines1[0].push_back('2');
        }
        else if (board[i][p.y] == 0)
        {
            lines[0].push_back('0');
            lines1[0].push_back('0');
        }
        else
        {
            lines[0].push_back('2');
            lines1[0].push_back('1');
        }
    }
    // 横
    for (i = 0; i < SIZE; i++)
    {

        if (board[p.x][i] == role)
        {
            lines[1].push_back('1');
            lines1[1].push_back('2');
        }
        else if (board[p.x][i] == 0)
        {
            lines[1].push_back('0');
            lines1[1].push_back('0');
        }
        else
        {
            lines[1].push_back('2');
            lines1[1].push_back('1');
        }
    }
    // 反斜杠
    for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < SIZE && j < SIZE; i++, j++)
    {
        if (board[i][j] == role)
        {
            lines[2].push_back('1');
            lines1[2].push_back('2');
        }
        else if (board[i][j] == 0)
        {
            lines[2].push_back('0');
            lines1[2].push_back('0');
        }
        else
        {
            lines[2].push_back('2');
            lines1[2].push_back('1');
        }
    }
    // 斜杠
    for (i = p.x + min(p.y, SIZE - 1 - p.x), j = p.y - min(p.y, SIZE - 1 - p.x); i >= 0 && j < SIZE; i--, j++)
    {
        if (board[i][j] == role)
        {
            lines[3].push_back('1');
            lines1[3].push_back('2');
        }
        else if (board[i][j] == 0)
        {
            lines[3].push_back('0');
            lines1[3].push_back('0');
        }
        else
        {
            lines[3].push_back('2');
            lines1[3].push_back('1');
        }
    }

    int lineScore[4] = {0};
    int line1Score[4] = {0};

    // 计算分数
    for (i = 0; i < 4; i++)
    {
        vector<int> result = acs.ACSearch(lines[i]);
        for (j = 0; j < result.size(); j++)
        {
            Pattern *temp = &patterns[result[j]];
            lineScore[i] += temp->score;
        }

        result = acs.ACSearch(lines1[i]);
        for (j = 0; j < result.size(); j++)
        {
            Pattern *temp = &patterns[result[j]];
            line1Score[i] += temp->score;
        }
    }

    int a = p.y;
    int b = p.x;
    int c = p.y - p.x + 10;
    int d = p.y + p.x - 4;
    // 减去以前的记录
    allScore[0] -= scores[0][a][0];
    allScore[0] -= scores[1][b][0];
    allScore[1] -= scores[0][a][1];
    allScore[1] -= scores[1][b][1];

    // scores顺序 竖、横、\、/
    scores[0][a][0] = lineScore[0];
    scores[0][a][1] = line1Score[0];
    scores[1][b][0] = lineScore[1];
    scores[1][b][1] = line1Score[1];

    // 加上新的记录
    allScore[0] += scores[0][a][0];
    allScore[0] += scores[1][b][0];
    allScore[1] += scores[0][a][1];
    allScore[1] += scores[1][b][1];

    if (p.y >= p.x - 10 && p.y <= p.x + 10) // 当棋子在两个对角线上围成的区域内时
    {

        allScore[0] -= scores[2][c][0];
        allScore[1] -= scores[2][c][1];

        scores[2][c][0] = lineScore[2];
        scores[2][c][1] = line1Score[2];

        allScore[0] += scores[2][c][0];
        allScore[1] += scores[2][c][1];
    }

    if (p.x + p.y >= 4 && p.x + p.y <= 24)
    {
        allScore[0] -= scores[3][d][0];
        allScore[1] -= scores[3][d][1];

        scores[3][d][0] = lineScore[3];
        scores[3][d][1] = line1Score[3];

        allScore[0] += scores[3][d][0];
        allScore[1] += scores[3][d][1];
    }
}

void rollbackScore(int x, int y)
{
    int a = y;
    int b = x;
    int c = y - x + 10;
    int d = x + y - 4;
    for (int i = 0; i < 2; i++)
    {
        allScore[i] -= scores[0][a][i];
        allScore[i] -= scores[1][b][i];
        scores[0][a][i] = 0; // 将该位置的分数清零
        scores[1][b][i] = 0;
    }

    if (y - x >= -10 && y - x <= 10)
    {
        for (int i = 0; i < 2; i++)
        {
            allScore[i] -= scores[2][c][i];
            scores[2][c][i] = 0;
        }
    }
    if (x + y >= 4 && x + y <= 24)
    {
        for (int i = 0; i < 2; i++)
        {
            allScore[i] -= scores[3][d][i];
            scores[3][d][i] = 0;
        }
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
    return 0;
}
int depth4time = 0;

// alpha-beta剪枝
int abSearch(int depth, int alpha, int beta, Role currentSearchRole, int limitDepth)
{
    HashItem::Flag flag = HashItem::ALPHA;
    int score = zh.getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != limitDepth)
    {
        return score;
    }
    // 评估当前局面
    int score1 = evaluateSituation(currentSearchRole);
    int score2 = evaluateSituation(currentSearchRole == HUMAN ? COMPUTOR : HUMAN);

    if (score1 >= 50000)
    {
        int res = MAX_SCORE - 1000 - (limitDepth - depth);
        return res; // 如果当前局面已经胜利，返回最大分数
    }
    if (score2 >= 50000)
    {
        int res = MIN_SCORE + 1000 + (limitDepth - depth);
        return res;
    }

    if (depth == 0)
    {
        zh.recordHashItem(depth, score1 - score2, HashItem::EXACT);
        return score1 - score2;
    }
    clock_t endTime = clock();
    if (endTime - startTime > timeLimit * (double)CLOCKS_PER_SEC)
    {
        return score1 - score2;
    }
    int cnt = 0;
    int pointnum = POINT_NUM;
    if (!addflag)
    {
        pointnum -= 1;
    }

    if (depth == limitDepth)
    {
        pointnum = firstPOINT_NUM;
    }

    priority_queue<Position, vector<Position>, compare> possiblePositions; // 优先队列，用于对可能出现的位置进行评分排序

    const unordered_set<Position, PointHash, PointEqual> &tmpPossiblePositions = pp_manager.GetCurrentPossiblePositions(); // 当前可能出现的位置

    for (auto iter = tmpPossiblePositions.begin(); iter != tmpPossiblePositions.end(); iter++)
    {
        possiblePositions.push(*iter);
    }

    if (possiblePositions.empty())
    {
        return score1 - score2;
    }

    // 对可能出现的位置进行评分排序
    while (!possiblePositions.empty())
    {
        Position p = possiblePositions.top();

        possiblePositions.pop();
        // 放置棋子
        board[p.x][p.y] = currentSearchRole;
        zh.currentZobristValue ^= zh.boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(p);

        // 增加可能出现的位置

        pp_manager.ScoringAddPossiblePositions(board, p); // 加入新的可能落子点,并更新已有可能落子点的分数
        int val;
        val = -abSearch(depth - 1, -beta, -alpha, currentSearchRole == HUMAN ? COMPUTOR : HUMAN, limitDepth);

        // 取消上一次增加的可能出现的位置
        pp_manager.Rollback();
        // 取消放置
        board[p.x][p.y] = 0;
        zh.currentZobristValue ^= zh.boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(p);

        if (val >= beta) // 当val >= beta时，当前节点不会被选择，所以直接返回beta
        {
            zh.recordHashItem(depth, beta, HashItem::BETA);
            return beta;
        }

        if (val > alpha)
        {
            flag = HashItem::EXACT;
            alpha = val;
            if (depth == limitDepth)
            {
                searchResult = p;
                // clock_t endTime = clock(); // 记录结束时间
                // cout << p.x << " " << p.y << " " << val << "    ";
                // cout << "time:" << (endTime - startTime) / (double)CLOCKS_PER_SEC << "s" << endl;
            }
        }

        cnt++;
        if (cnt >= pointnum)
        {
            break;
        }
    }

    zh.recordHashItem(depth, alpha, flag);
    return alpha;
}

// 获得下一步的走法
Position getAGoodMove()
{
    int i = initDepth;

    int alpha = MIN_SCORE;
    int beta = MAX_SCORE;
    int valWINDOW = 100;
    while (i <= DEPTH)
    {
        int threshold = iterLimit * (double)CLOCKS_PER_SEC;

        int score = abSearch(i, alpha, beta, COMPUTOR, i);

        if ((score <= alpha) || (score >= beta))
        {
            alpha = MIN_SCORE;
            beta = MAX_SCORE;
            continue;
        }
        alpha = score - valWINDOW;
        beta = score + valWINDOW;
        i++;

        clock_t endTime = clock(); // 记录结束时间
        if (endTime - startTime > threshold)
        {
            break;
        }
    }
    return searchResult;
}

// 初始化函数，插入特征和分值
void init()
{
    vector<string> patternStrs;
    for (int i = 0; i < patterns.size(); i++)
    {
        Pattern *temp = &patterns[i];
        patternStrs.emplace_back(temp->pattern);
    }

    // 初始化ACSearcher
    acs.LoadPattern(patternStrs);
    acs.BuildGotoTable();
    acs.BuildFailTable();
}
void updataSituation(int x, int y, int role)
{
    board[x][y] = role;
    zh.currentZobristValue ^= zh.boardZobristValue[role - 1][x][y];
    updateScore(Position(x, y));
    if (addflag && doubleadd)
        pp_manager.AddPossiblePositions2(board, Position(x, y));

    else
        pp_manager.AddPossiblePositions(board, Position(x, y));
}
void rollbackSituation(int x, int y, int role)
{
    board[x][y] = EMPTY;
    zh.currentZobristValue ^= zh.boardZobristValue[role - 1][x][y];
    rollbackScore(x, y);
    pp_manager.Rollback();
}
// 人类下棋，传给界面
Position nextStep(int x, int y)
{
    updataSituation(x, y, HUMAN);

    pp_manager.InitPossiblePositions(board);

    // 增加可能出现的位置

    Position result = getAGoodMove();

    return result;
}

int needSwapboard[15][15] =
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
        {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
int isSwap(int x, int y)
{
    return !needSwapboard[x][y];
}
void edgestart(int &new_x, int &new_y, int x, int y)
{
    if (x <= 4)
    {
        if (y <= 4)
        {
            new_x = x + 3;
            new_y = y + 3;
        }
        else if (y >= 10)
        {
            new_x = x + 3;
            new_y = y - 3;
        }
        else
        {
            new_x = x + 3;
            new_y = y;
        }
    }
    else if (x >= 10)
    {
        if (y >= 10)
        {
            new_x = x - 3;
            new_y = y - 3;
        }
        else if (y <= 4)
        {
            new_x = x - 3;
            new_y = y + 3;
        }
        else
        {
            new_x = x - 3;
            new_y = y;
        }
    }
    else
    {
        if (y <= 4)
        {
            new_x = x;
            new_y = y + 3;
        }
        else if (y >= 10)
        {
            new_x = x;
            new_y = y - 3;
        }
        else
        {
            new_x = x;
            new_y = y;
        }
    }
}

int main()
{
    init();
    int new_x, new_y;
    int x, y, n;
    // 恢复目前的棋盘信息
    cin >> n;
    if (n == 1)
    {
        cin >> x >> y;
        if (x != -1)
        {
            if (isSwap(x, y))
            {
                new_x = -1;
                new_y = -1;
            }
            else
            {
                edgestart(new_x, new_y, x, y);
            }
        }
        else // 我方先手下在平衡点
        {
            new_x = 6;
            new_y = 1;
        }
    }
    else
    {
        /*
        3
        -1 -1
        2 3
        -1 -1
        3 4
        3 3

        3
        2 3
        -1 -1
        5 5
        3 4
        3 3

        3
        2 3
        2 2
        5 5
        3 4
        3 3

        3
        -1 -1
        2 3
        2 2
        3 4
        3 3


        */

        int lastx = 0, lasty = 0;
        int i = 0;
        for (i = 0; i < n - 1; i++)
        {
            cin >> x >> y;
            if (x != -1)
            {
                if (addflag == 2)
                    addflag = 0;
                updataSituation(x, y, 1);
            }
            else if (i != 0) // 人类交换
            {
                addflag = 0;
                rollbackSituation(lastx, lasty, 2);
                updataSituation(lastx, lasty, 1);
            }
            else
            {
                addflag = 1;
            }
            lastx = x;
            lasty = y;
            cin >> x >> y;
            if (x != -1)
            {
                updataSituation(x, y, 2);
            }
            else if (i == 0) // ai交换
            {

                addflag = 1;
                rollbackSituation(lastx, lasty, HUMAN);
                updataSituation(lastx, lasty, 2);
            }
            lastx = x;
            lasty = y;
        }
        cin >> x >> y;
        if (x != -1)
        {
            board[x][y] = 1; // 对方
        }
        else if (i != 0) // 人类交换
        {
            new_x = 7;
            new_y = 4;
            printf("%d %d\n", new_x, new_y);
            return 0;
        }

        // 此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

        /************************************************************************************/
        /***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
        startTime = clock(); // 记录开始时间
        // 执行最优的下一步
        /*clock_t start = clock();*/
        Position p = nextStep(x, y);
        new_x = p.x;
        new_y = p.y;
        /* clock_t end = clock();
         double duration = (double)(end - start) / CLOCKS_PER_SEC;
         cout << "time: " << duration << endl;*/
        /***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
        /************************************************************************************/

        // 棋盘
        // 向平台输出决策结果
    }
    printf("%d %d\n", new_x, new_y);
    return 0;
}