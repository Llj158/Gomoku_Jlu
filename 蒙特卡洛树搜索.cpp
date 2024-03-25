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

// ģʽ
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



int RandomGame(int height,int& max_height,int player)// 0 �� 1 ʤ 2 ƽ ���ģ��ʱѡ���������Ž��� player��һ����˭�µ�
{
    if (JudgeGameOver) return ~player;//ȡ��
    if (height >= max_height) return 2;//�������� ��Ϊƽ��
    int i, j;
    if (player == 0)//0 �ҷ� 1�Է�
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
        board[i][j] = 1;//�ҷ�����
        int result = RandomGame(height + 1, max_height, 1);
        board[i][j] = 0;//��ԭ
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
        board[i][j] = 2;//�Է�����
        int result = RandomGame(height + 1, max_height, 0);
        board[i][j] = 0;//��ԭ
        return result;
    }
}

bool JudgeGameOver()
{
    //�ж���Ϸ���� ��ûд
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
                if (min_heap.size() > 10) min_heap.pop();//ѡ��ǰn�������ϴ��λ����Ϊ�ӽڵ㣨������10)
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
    int max_height = 50;//������ �����ٵ���
    int result;
    if (root->children.size() == 0 && root->total_num != 0)//����δ�����ʵ�Ҷ�ӽ�� ������չ
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
    if (result == 0)//��
    {
        root->total_num++;
    }
    else if (result == 1)//ʤ
    {
        root->total_num++;
        root->victory_num ++;
    }
    else if (result == 2)//ƽ
    {
        //û�����ô����
    }
    TreeNode* p = root->children.rbegin()->second;
    auto it = root->children.rbegin();
    auto forwardIt = it.base();
    root->children.erase(--forwardIt);//gpt����ɾ�����������ָ��Ԫ�صķ��� ��֪���в���
    double new_ucb_value = CalculateUCB(p);
    root->children.emplace(new_ucb_value, p);
    return result;
}
/*
    ���� 
    1 ��δ��total_simulation�����ܲ��� ��������Ҫ����
    2 ��Ϸ�����жϴ��뻹ûд ���ۺ�����û��
    3 ��չ���ʱѡ��ǰn�������ϴ�Ľ�� n��Ҫȷ��
    4 ƽ����ô����
    5 ���ǲ���˥�� ����ԽԶ�����Ľ��Ӱ��ԽС
    6 ������ͬ�ľ��� �ܷ��ù�ϣ�����ֱ�ӵ���
    7 �߼����� �����Ҳ��Ǻ���� ���нڵ㶼��ѡ����ҷ��������ķ������� ������վ������Ļ��Է�Ӧ��ѡ����ҷ�������ķ��� ���Ǻܶ�
    8 Chronoͷ�ļ� �����������ʱ��
    9 ���� ѧϰ��
*/