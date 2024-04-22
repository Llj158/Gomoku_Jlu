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
#include <random>
using namespace std;

/*---------------------------head-----------------------------*/

#define UNKNOWN_SCORE (10000001)
#define HASH_ITEM_INDEX_MASK (0xffff)
#define MAX_SCORE 10000000
#define MIN_SCORE -10000000
const int PLAYER = 1;
const int DEPTH = 7;
const int POINT_NUM = 7;
const int SIZE = 15;
const double timeLimit = 0.99;
const double iterLimit = 0.1;
const int firstPOINT_NUM = 20;
const int midPOINT_NUM = 85;
const int initDepth = 6;

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
    Position() {}
    Position(int x, int y)
    {
        this->x = x;
        this->y = y;
        score = 0;
    }
    Position(int x, int y, int score)
    {
        this->x = x;
        this->y = y;
        this->score = score;
    }
    bool operator<(const Position &pos) const
    {
        if (score != pos.score)
        {
            return score > pos.score;
        }
        if (x != pos.x)
        {
            return x < pos.x;
        }
        else
        {
            return y < pos.y;
        }
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

/*---------------------------ZobristHash-----------------------------*/

class ZobristHash
{
public:
    ZobristHash();
    ~ZobristHash(){};
    void recordHashItem(int depth, int score, HashItem::Flag flag);
    int getHashItemScore(int depth, int alpha, int beta);
    long long random64();
    void randomZobristValue();
    void initCurrentZobristValue();

    long long boardZobristValue[2][SIZE][SIZE];
    long long currentZobristValue; // 当前局面的zobrist值
private:
    HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];
};

ZobristHash::ZobristHash()
{
    randomZobristValue();
    initCurrentZobristValue();
}

void ZobristHash::recordHashItem(int depth, int score, HashItem::Flag flag)
{
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) // 如果当前条目已经有数据，且深度小于当前深度，则不覆盖
        return;

    phashItem->checksum = currentZobristValue;
    phashItem->score = score;
    phashItem->flag = flag;
    phashItem->depth = depth;
}

// 在哈希表中取得计算好的局面的分数
int ZobristHash::getHashItemScore(int depth, int alpha, int beta)
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
long long ZobristHash::random64()
{
    return (long long)rand() | ((long long)rand() << 15) | ((long long)rand() << 30) | ((long long)rand() << 45) | ((long long)rand() << 60);
}

// 生成zobrist键值
void ZobristHash::randomZobristValue()
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
void ZobristHash::initCurrentZobristValue()
{
    currentZobristValue = random64();
}

/*---------------------------ZobristHash-----------------------------*/

/*---------------------------ACSearcher-----------------------------*/

using namespace std;

// trie树节点
struct ACNode
{
    ACNode(int p, char c)
        : parent(p),
          ch(c),
          fail(-1)
    {
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
    ~ACSearcher();

    void LoadPattern(const vector<string> &paterns);
    void BuildGotoTable();
    void BuildFailTable();
    vector<int> ACSearch(const string &text); // 返回匹配到的模式的索引

private:
    int maxState;           // 最大状态数
    vector<ACNode> nodes;   // trie树
    vector<string> paterns; // 需要匹配的模式

    void AddState(int parent, char ch); // 初始化新状态
};

ACSearcher::ACSearcher()
    : maxState(0)
{
    // 初始化根节点
    AddState(-1, 'a');
    nodes[0].fail = -1;
}

ACSearcher::~ACSearcher()
{
}

void ACSearcher::LoadPattern(const vector<string> &paterns)
{
    this->paterns = paterns;
}

void ACSearcher::BuildGotoTable()
{
    assert(nodes.size());

    unsigned int i, j;
    for (i = 0; i < paterns.size(); i++)
    {
        // 从根节点开始
        int currentIndex = 0;
        for (j = 0; j < paterns[i].size(); j++)
        {
            if (nodes[currentIndex].sons.find(paterns[i][j]) == nodes[currentIndex].sons.end())
            {
                nodes[currentIndex].sons[paterns[i][j]] = ++maxState;

                // 生成新节点
                AddState(currentIndex, paterns[i][j]);
                currentIndex = maxState;
            }
            else
            {
                currentIndex = nodes[currentIndex].sons[paterns[i][j]];
            }
        }

        nodes[currentIndex].output.push_back(i);
    }
}

void ACSearcher::BuildFailTable()
{
    assert(nodes.size());

    // 中间节点收集器
    vector<int> midNodesIndex;

    // 给第一层的节点设置fail为0，并把第二层节点加入到midState里
    ACNode root = nodes[0];

    unordered_map<char, int>::iterator iter1, iter2;
    for (iter1 = root.sons.begin(); iter1 != root.sons.end(); iter1++)
    {
        nodes[iter1->second].fail = 0;
        ACNode &currentNode = nodes[iter1->second];

        // 收集第三层节点
        for (iter2 = currentNode.sons.begin(); iter2 != currentNode.sons.end(); iter2++)
        {
            midNodesIndex.push_back(iter2->second);
        }
    }

    // 广度优先遍历
    while (midNodesIndex.size())
    {
        vector<int> newMidNodesIndex;

        unsigned int i;
        for (i = 0; i < midNodesIndex.size(); i++)
        {
            ACNode &currentNode = nodes[midNodesIndex[i]];

            // 以下循环为寻找当前节点的fail值
            int currentFail = nodes[currentNode.parent].fail;
            while (true)
            {
                ACNode &currentFailNode = nodes[currentFail];

                if (currentFailNode.sons.find(currentNode.ch) != currentFailNode.sons.end())
                {
                    // 成功找到该节点的fail值
                    currentNode.fail = currentFailNode.sons.find(currentNode.ch)->second;

                    // 后缀包含
                    if (nodes[currentNode.fail].output.size())
                    {
                        currentNode.output.insert(currentNode.output.end(), nodes[currentNode.fail].output.begin(), nodes[currentNode.fail].output.end());
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
                    currentNode.fail = 0;
                    break;
                }
            }

            // 收集下一层节点
            for (iter1 = currentNode.sons.begin(); iter1 != currentNode.sons.end(); iter1++)
            {
                // 收集下一层节点
                newMidNodesIndex.push_back(iter1->second);
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

    unsigned int i;
    unordered_map<char, int>::iterator tmpIter;
    for (i = 0; i < text.size();)
    {
        // 顺着trie树查找
        if ((tmpIter = nodes[currentIndex].sons.find(text[i])) != nodes[currentIndex].sons.end())
        {
            currentIndex = tmpIter->second;
            i++;
        }
        else
        {
            // 失配的情况
            while (nodes[currentIndex].fail != -1 && nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.end())
            {
                currentIndex = nodes[currentIndex].fail;
            }

            // 如果没有成功找到合适的fail
            if (nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.end())
            {
                i++;
            }
        }

        if (nodes[currentIndex].output.size())
        {
            result.insert(result.end(), nodes[currentIndex].output.begin(), nodes[currentIndex].output.end());
        }
    }

    return result;
}

void ACSearcher::AddState(int parent, char ch)
{
    nodes.push_back(ACNode(parent, ch));
    assert(nodes.size() - 1 == maxState);
}

/*---------------------------ACSearcher-----------------------------*/

/*---------------------------PossiblePositionManager-----------------------------*/

using namespace std;

struct PosHistory
{
    unordered_set<Position, PointHash, PointEqual> newPositions;
    Position removedPosition;
};

class PossiblePositionManager
{
public:
    PossiblePositionManager();
    ~PossiblePositionManager();
    void AddPossiblePositions(int board[SIZE][SIZE], const Position &p);
    void AddPossiblePositions2(int board[SIZE][SIZE], const Position &p);
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
    return pos.x >= 0 && pos.x < SIZE && pos.y >= 0 && pos.y < SIZE;
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

PossiblePositionManager::~PossiblePositionManager()
{
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

    PosHistory ph; // 新增历史记录
    ph.newPositions = newPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) // 当前选中位置原本为可选位置，现选择后不可能选择，删除
    {
        currentPossiblePositions.erase(p);
        ph.removedPosition = p;
    }
    else
        ph.removedPosition.x = -1; // 给回溯提供标志

    allHistory.push_back(ph); // 保存历史记录
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
    // for (const auto& direct : directions)
    //{
    //    Position newPos(p.x + 2 * direct.first, p.y + direct.second);
    //     if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
    //     {
    //         auto insertResult = currentPossiblePositions.insert(newPos);

    //        // 如果插入成功
    //        if (insertResult.second)
    //            newPositions.insert(newPos);
    //    }
    //}
    // for (const auto& direct : directions)
    //{
    //    Position newPos(p.x + direct.first, p.y + 2 * direct.second);
    //    if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
    //    {
    //        auto insertResult = currentPossiblePositions.insert(newPos);

    //        // 如果插入成功
    //        if (insertResult.second)
    //            newPositions.insert(newPos);
    //    }
    //}

    PosHistory ph; // 新增历史记录
    // ph.newPositions = newPositions;

    for (auto pos : newPositions)
        ph.newPositions.insert(pos);

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) // 当前选中位置原本为可选位置，现选择后不可能选择，删除
    {
        currentPossiblePositions.erase(p);
        ph.removedPosition = p;
    }
    else
        ph.removedPosition.x = -1; // 给回溯提供标志

    allHistory.emplace_back(ph); // 保存历史记录
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
}

const unordered_set<Position, PointHash, PointEqual> &PossiblePositionManager::GetCurrentPossiblePositions()
{
    return currentPossiblePositions;
}

/*---------------------------PossiblePositionManager-----------------------------*/

/*------------------------全局变量-------------------------*/

int board[15][15] = {0};

int scores[2][72]; // 保存棋局分数（2个角色72行，包括横竖撇捺）
int allScore[2];   // 局面总评分（2个角色）

ACSearcher acs;
PossiblePositionManager pp_manager;
ZobristHash zh;

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

// 根据位置评分，其中board是当前棋盘，p是位置，role是评分角色，比如role是Human则是相对人类评分，比如role是computer则是对于电脑评分
int evaluatePoint(Position p)
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
    int max_human = 0;
    int max_ai = 0;
    int doubleflag = 0; // 避免同一行分数重复加倍
    int lock = 0;
    for (i = 0; i < 4; i++)
    {
        vector<int> tmp = acs.ACSearch(lines[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            score_human += patterns[tmp[j]].score;
            if (patterns[tmp[j]].score <= max_human && patterns[tmp[j]].score % 720 == 0 && doubleflag && lock)
            {
                score_human *= 2;
                doubleflag = 0;
            }
            max_human = max(max_human, patterns[tmp[j]].score);
        }
        if (max_human >= 720)
        {
            lock = 1;
        }
        if (i != 0)
            doubleflag = 1;
        tmp = acs.ACSearch(lines1[i]);
        for (j = 0; j < tmp.size(); j++)
        {
            score_ai += patterns[tmp[j]].score;
            if (patterns[tmp[j]].score <= max_ai && patterns[tmp[j]].score % 720 == 0 && doubleflag && lock)
            {
                score_ai *= 2;
                doubleflag = 0;
            }
            max_ai = max(max_ai, patterns[tmp[j]].score);
        }
        if (max_ai >= 720)
        {
            lock = 1;
        }
        doubleflag = 1;
    }
    result = score_ai + score_human;

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

void rollbackScore(int x, int y)
{
    int a = y;
    int b = SIZE + x;
    int c = 2 * SIZE + (y - x + 10);
    int d = 2 * SIZE + 21 + (x + y - 4);
    for (int i = 0; i < 2; i++)
    {
        allScore[i] -= scores[i][a];
        allScore[i] -= scores[i][b];
        scores[i][a] = 0; // 将该位置的分数清零
        scores[i][b] = 0;
    }

    if (y - x >= -10 && y - x <= 10)
    {
        for (int i = 0; i < 2; i++)
        {
            allScore[i] -= scores[i][c];
            scores[i][c] = 0;
        }
    }
    if (x + y >= 4 && x + y <= 24)
    {
        for (int i = 0; i < 2; i++)
        {
            allScore[i] -= scores[i][d];
            scores[i][d] = 0;
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
        return MAX_SCORE - 1000 - (limitDepth - depth); // 如果当前局面已经胜利，返回最大分数
    }
    if (score2 >= 50000)
    {
        return MIN_SCORE + 1000 + (limitDepth - depth);
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
    int count = 0;

    priority_queue<Position, vector<Position>, compare> possiblePositions; // 存储可能出现的位置

    const unordered_set<Position, PointHash, PointEqual> &tmpPossiblePositions = pp_manager.GetCurrentPossiblePositions(); // 当前可能出现的位置
    // 对当前可能出现的位置进行粗略评分
    for (auto iter = tmpPossiblePositions.begin(); iter != tmpPossiblePositions.end(); iter++)
    {
        possiblePositions.push(Position(iter->x, iter->y, evaluatePoint(*iter)));
    }

    if (possiblePositions.empty())
    {
        return score1 - score2;
    }

    int pointnum = POINT_NUM;
    if (!addflag)
    {
        pointnum -= 1;
    }

    if (depth == limitDepth)
    {
        pointnum = POINT_NUM;
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
        p.score = 0;

        pp_manager.AddPossiblePositions2(board, p);
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
            }
        }

        count++;
        if (count >= pointnum)
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
    startTime = clock(); // 记录开始时间
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
        patternStrs.emplace_back(patterns[i].pattern);
    }

    // 初始化ACSearcher
    acs.LoadPattern(patternStrs);
    acs.BuildGotoTable();
    acs.BuildFailTable();
}

// 人类下棋，传给界面
Position nextStep(int x, int y)
{

    board[x][y] = HUMAN;
    zh.currentZobristValue ^= zh.boardZobristValue[HUMAN - 1][x][y];
    updateScore(Position(x, y));

    // 增加可能出现的位置
    if (addflag && doubleadd)
        pp_manager.AddPossiblePositions2(board, Position(x, y));

    else
        pp_manager.AddPossiblePositions(board, Position(x, y));

    Position result = getAGoodMove();

    return result;
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
            new_x = 7;
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
                rollbackSituation(lastx, lasty, COMPUTOR);
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

        // 执行最优的下一步
        Position p = nextStep(x, y);
        new_x = p.x;
        new_y = p.y;
        /***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
        /************************************************************************************/

        // 棋盘
        // 向平台输出决策结果
    }
    printf("%d %d\n", new_x, new_y);
    return 0;
}