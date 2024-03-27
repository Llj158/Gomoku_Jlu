#include<iostream>
#include<vector>
#include<map>
#include<algorithm>
#include<ctime>
#include<random>
#include<queue>
using namespace std;

int total_simulation = 0;
int C = 2;
int max_height = 50;//最大深度 后面再调整
const int SIZE = 15;
int board[SIZE][SIZE] = { 0 };
struct TreeNode
{
	unsigned int victory_num;
	unsigned int total_num;
	pair<int, int> pos;
	multimap<double, TreeNode*> children;
	int player;

	TreeNode(unsigned int victory_num,unsigned int total_num, pair<int, int> pos, int player)
	{
		this->victory_num = victory_num;
		this->total_num = total_num;
		this->pos = pos;
		this->player = player;
	}
};

struct Pattern
{
    string pattern;
    int score;
};

vector<Pattern> patterns1 = {
    {"11111", 70000},
    {"011110", 4800},
    {"011100", 750},
    {"001110", 750},
    {"011010", 750},
    {"010110", 750},
    {"11110", 750},
    {"01111", 750},
    {"11011", 750},
    {"10111", 750},
    {"11101", 750},
    {"001100", 125},
    {"001010", 125},
    {"010100", 125},
    {"000100", 21},
    {"001000", 21},
};
vector<Pattern> patterns2 = {
    {"22222", 50000},
    {"022220", 4500},
    {"022200", 720},
    {"002220", 720},
    {"022020", 720},
    {"020220", 720},
    {"22220", 720},
    {"02222", 720},
    {"22022", 720},
    {"20222", 720},
    {"22202", 720},
    {"002200", 120},
    {"002020", 120},
    {"020200", 120},
    {"000200", 20},
    {"002000", 20},
};

int evaluatePattern(const string& line,int& if_win)
{
    int score = 0;
    for (auto i : patterns1)
    {
        auto pos = line.find(i.pattern);
        if (pos != string::npos)
        {
            if (i.pattern == "11111") if_win = 1;
            score += i.score;
            break;
        }
    }
    for (auto i : patterns2)
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

int evaluate(int x, int y, int player,int& if_win)
{
    int totalScore = 0;
    string line[4];
    for (int i = max(0, x - 4); i < min(15, x + 5); i++)
    {
        line[0].push_back(board[i][y] == player ? '1' : board[i][y] == 0 ? '0'
            : '2');
    }
    totalScore += evaluatePattern(line[0],if_win);

    for (int i = max(0, y - 4); i < min(15, y + 5); i++)
    {
        line[1].push_back(board[x][i] == player ? '1' : board[x][i] == 0 ? '0'
            : '2');
    }
    totalScore += evaluatePattern(line[1], if_win);

    for (int i = x + min(y, min(SIZE - 1 - x, 5)), j = x + y - i; i >= max(0, x - 4) && j < min(15, y + 5); i--, j++)
    {
        line[2].push_back(board[i][j] == player ? '1' : board[i][j] == 0 ? '0'
            : '2');
    }
    totalScore += evaluatePattern(line[2], if_win);

    for (int i = x - min(x, min(4, y)), j = i + y - x; i < min(15, x + 5) && j < min(15, y + 5); i++, j++)
    {
        line[3].push_back(board[i][j] == player ? '1' : board[i][j] == 0 ? '0'
            : '2');
    }
    totalScore += evaluatePattern(line[3], if_win);

    return totalScore;
}


int RandomGame(int height,int& max_height,int player)// 0、1 对应选手获胜 2 平 随机模拟时选择评估最优进行 player上一步是谁下的
{
    if (height >= max_height) return 2;//层数过多 认为平局
    int i, j, if_win = 0;
    if (player == -1)//1 我方 -1对方 注意这是上一次行棋的人
    {
        int max_score = INT32_MIN;
        pair<int, int> max_pos;
        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 14; j++)
            {
                if (board[i][j] != 0) continue;
                int temp_score = evaluate(i, j, 1, if_win);
                if (temp_score > max_score)
                {
                    max_score = temp_score;
                    max_pos = { i,j };
                }
            }
        }
        i = max_pos.first, j = max_pos.second;
        board[i][j] = 1;//我方落子

        if (if_win == 1)
        {
            board[i][j] = 0;//复原
            return 0;//结果返回
        }
        else {
            int win_player = RandomGame(height + 1, max_height, 1);
            board[i][j] = 0;//复原
            return win_player;
        }
    }
    else if(player == 1)//此时上一步走棋的是我方 也就是这一步行棋的是对方
    {
        int min_score = INT32_MAX;
        pair<int, int>min_pos;
        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 14; j++)
            {
                if (board[i][j] != 0) continue;
                int temp_score = evaluate(i, j, -1, if_win);
                if (temp_score < min_score)
                {
                    min_score = temp_score;
                    min_pos = { i,j };
                }
            }
        }
        i = min_pos.first, j = min_pos.second;
        board[i][j] = -1;//对方落子

        if (if_win == 1)
        {
            board[i][j] = 0;//复原
            return 1;//结果返回
        }
        else {
            int win_player = RandomGame(height + 1, max_height, -1);
            board[i][j] = 0;//复原
            return win_player;
        }
    }
}

void ExpandLayer(TreeNode* root)
{
    int if_win = 0;
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<pair<int, pair<int, int>>>>min_heap;
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			if (board[i][j] == 0)
			{
                int value = evaluate(i, j, -1 * root->player, if_win);
                min_heap.emplace(pair<int, pair<int, int>>{value, pair<int, int>{i, j}});
                if (min_heap.size() > 10) min_heap.pop();//选择前n个评估较大的位置作为子节点（这里是10)
			}
		}
	}
    TreeNode* p;
    while (!min_heap.empty())
    {
        pair<int, int> temp_pos = min_heap.top().second;
        min_heap.pop();
        if (root->player == 1) p = new TreeNode(0, 0, temp_pos, 1);
        else if (root->player == -1) p = new TreeNode(0, 0, temp_pos, -1);
        root->children.emplace(INT32_MAX, p);
    }
}

double CalculateUCB(TreeNode* root)
{
	double temp1 = root->victory_num * 1.0 / root->total_num;
	double temp2 = C*sqrt(log2(total_simulation) / root->total_num);
	return temp1 + temp2;
}

int TreeSearch(TreeNode* root)
{
    int win_player = 0;
    if (root->children.size() == 0 && root->total_num != 0)//不是未被访问的叶子结点 进行扩展
    {
        ExpandLayer(root);
        if (root->children.size() == 0) return 2;
        board[root->pos.first][root->pos.second] = root->player;
        win_player = TreeSearch(root->children.rbegin()->second);
        board[root->pos.first][root->pos.second] = 0;
    }
    else if (root->children.size() != 0)
    {
        if (root->pos != pair<int, int>{-1, -1}) board[root->pos.first][root->pos.second] = root->player + 1;
        win_player = TreeSearch(root->children.rbegin()->second);
        if (root->pos != pair<int, int>{-1, -1}) board[root->pos.first][root->pos.second] = 0;
    }
    else if (root->children.size() == 0 && root->total_num == 0)
    {
        board[root->pos.first][root->pos.second] = root->player;
        win_player = RandomGame(0, max_height, root->player);
        board[root->pos.first][root->pos.second] = 0;
        return win_player;
    }
    if (win_player == 2)// 平
    {
        root->total_num += 2;
        root->victory_num += 1;
    }
    else if (win_player == root->player)//胜
    {
        root->total_num++;
        root->victory_num++;
    }
    else if (win_player!= root->player)//负
    {
        root->total_num++;
    }

    TreeNode* p = root->children.rbegin()->second;
    auto it = root->children.rbegin();
    auto forwardIt = it.base();
    root->children.erase(--forwardIt);//gpt给的删除反向迭代器指向元素的方法 不知道行不行
    double new_ucb_value = CalculateUCB(p);
    root->children.emplace(new_ucb_value, p);
    return win_player;
}
/*
    问题 
    1 暂未用total_simulation限制总层数 最大深度需要调整
    2 游戏结束判断代码还没写 评价函数还没有
    3 扩展结点时选择前n个评估较大的结点 n需要确定
    4 平局怎么处理
    5 考虑层数衰减 离结点越远产生的结果影响越小
    6 出现相同的局面 能否用哈希表储存后直接调用
    7 逻辑问题 这里我不是很理解 所有节点都是选择对我方最有利的方向评价 如果按照决策树的话对方应该选择对我方最不有利的方向 不是很懂
    8 Chrono头文件 计算程序运行时长
    9 并行 学习中
    10 无处可下时认为平局
    11 Rave算法优化 代替UCB算法
    12 调整UCB参数 在后期阶段，可以逐渐减小探索常数，使得算法更加倾向于选择已有统计信息更为准确的节点进行扩展。即改变C值
    13 对称性剪枝、局部剪枝或者状态压缩等方法
    14 深度学习
    15 虚拟根 需要先扩展好 位置为-1 -1 player为最后一步下的人（其实就是对手）
*/


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

        TreeNode* root = new TreeNode(0, 0, pair<int, int>{-1, -1}, -1);
        ExpandLayer(root);
        for (int i = 0; i < 100; i++)
        {
            TreeSearch(root);
            total_simulation++;
        }
        TreeNode* res = root->children.rbegin()->second;
        new_x = res->pos.first;
        new_y = res->pos.second;
    }
    /***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
    /************************************************************************************/

    // 棋盘
    // 向平台输出决策结果
    printf("%d %d\n", new_x, new_y);
    return 0;
}