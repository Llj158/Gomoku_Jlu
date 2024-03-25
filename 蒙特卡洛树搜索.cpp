#include<iostream>
#include<vector>
#include<map>
#include<algorithm>
#include<ctime>
#include<random>
#include<queue>
using namespace std;

int total_simulation = 0, C = 2;
const int SIZE = 15;
int board[SIZE][SIZE] = { 0 };
struct TreeNode
{
	unsigned int victory_num;
	unsigned int total_num;
	pair<int, int> pos;
	map<double, TreeNode*> children;
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

// 模式
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

int evaluate(int x, int y)
{
    //
}



int RandomGame(int height,int& max_height,int player)// 0 负 1 胜 2 平 随机模拟时选择评估最优进行 player上一步是谁下的
{
    if (JudgeGameOver) return ~player;//取反
    if (height >= max_height) return 2;//层数过多 认为平局
    int i, j;
    if (player == 0)//0 我方 1对方
    {
        int max_score = INT32_MIN;
        pair<int, int> max_pos;
        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 14; j++)
            {
                if (board[i][j] != 0) continue;
                int temp_score = evaluate(i, j);
                if (temp_score > max_score)
                {
                    max_score = temp_score;
                    max_pos = { i,j };
                }
            }
        }
        i = max_pos.first, j = max_pos.second;
        board[i][j] = 1;//我方落子
        int result = RandomGame(height + 1, max_height, 1);
        board[i][j] = 0;//复原
        return result;
    }
    else if(player == 1)
    {
        int min_score = INT32_MAX;
        pair<int, int>min_pos;
        for (i = 0; i < 14; i++)
        {
            for (j = 0; j < 14; j++)
            {
                if (board[i][j] != 0) continue;
                int temp_score = evaluate(i, j);
                if (temp_score < min_score)
                {
                    min_score = temp_score;
                    min_pos = { i,j };
                }
            }
        }
        i = min_pos.first, j = min_pos.second;
        board[i][j] = 2;//对方落子
        int result = RandomGame(height + 1, max_height, 0);
        board[i][j] = 0;//复原
        return result;
    }
}

bool JudgeGameOver()
{
    //判断游戏结束 还没写
}

void ExpandLayer(TreeNode* root)
{
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<pair<int, pair<int, int>>>>min_heap;
    int temp_value;
	for (int i = 0; i < 15; i++)
	{
		for (int j = 0; j < 15; j++)
		{
			if (board[i][j] == 0)
			{
                temp_value = evaluate(i, j);
                min_heap.emplace(pair<int, pair<int, int>>{temp_value, pair<int, int>{i, j}});
                if (min_heap.size() > 10) min_heap.pop();//选择前n个评估较大的位置作为子节点（这里是10)
			}
		}
	}
    TreeNode* p;
    while (!min_heap.empty())
    {
        pair<int, int> temp_pos = min_heap.top().second;
        min_heap.pop();
        if (root->player == 1) p = new TreeNode(0, 0, temp_pos, 2);
        else if (root->player == 2) p = new TreeNode(0, 0, temp_pos, 1);
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
    int max_height = 50;//最大深度 后面再调整
    int result;
    if (root->children.size() == 0 && root->total_num != 0)//不是未被访问的叶子结点 进行扩展
    {
        ExpandLayer(root);
        result = TreeSearch(root->children.rbegin()->second);
    }
    else if (root->children.size() != 0)
    {
        result = TreeSearch(root->children.rbegin()->second);
    }
    else if (root->children.size() == 0 && root->total_num == 0)
    {
        result = RandomGame(0, max_height, root->player);
        return result;
    }
    if (result == 0)//负
    {
        root->total_num++;
    }
    else if (result == 1)//胜
    {
        root->total_num++;
        root->victory_num ++;
    }
    else if (result == 2)//平
    {
        //没想好怎么处理
    }
    TreeNode* p = root->children.rbegin()->second;
    auto it = root->children.rbegin();
    auto forwardIt = it.base();
    root->children.erase(--forwardIt);//gpt给的删除反向迭代器指向元素的方法 不知道行不行
    double new_ucb_value = CalculateUCB(p);
    root->children.emplace(new_ucb_value, p);
    return result;
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
*/