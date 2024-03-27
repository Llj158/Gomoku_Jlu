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
int max_height = 50;//������ �����ٵ���
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


int RandomGame(int height,int& max_height,int player)// 0��1 ��Ӧѡ�ֻ�ʤ 2 ƽ ���ģ��ʱѡ���������Ž��� player��һ����˭�µ�
{
    if (height >= max_height) return 2;//�������� ��Ϊƽ��
    int i, j, if_win = 0;
    if (player == -1)//1 �ҷ� -1�Է� ע��������һ���������
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
        board[i][j] = 1;//�ҷ�����

        if (if_win == 1)
        {
            board[i][j] = 0;//��ԭ
            return 0;//�������
        }
        else {
            int win_player = RandomGame(height + 1, max_height, 1);
            board[i][j] = 0;//��ԭ
            return win_player;
        }
    }
    else if(player == 1)//��ʱ��һ����������ҷ� Ҳ������һ��������ǶԷ�
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
        board[i][j] = -1;//�Է�����

        if (if_win == 1)
        {
            board[i][j] = 0;//��ԭ
            return 1;//�������
        }
        else {
            int win_player = RandomGame(height + 1, max_height, -1);
            board[i][j] = 0;//��ԭ
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
                if (min_heap.size() > 10) min_heap.pop();//ѡ��ǰn�������ϴ��λ����Ϊ�ӽڵ㣨������10)
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
    if (root->children.size() == 0 && root->total_num != 0)//����δ�����ʵ�Ҷ�ӽ�� ������չ
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
    if (win_player == 2)// ƽ
    {
        root->total_num += 2;
        root->victory_num += 1;
    }
    else if (win_player == root->player)//ʤ
    {
        root->total_num++;
        root->victory_num++;
    }
    else if (win_player!= root->player)//��
    {
        root->total_num++;
    }

    TreeNode* p = root->children.rbegin()->second;
    auto it = root->children.rbegin();
    auto forwardIt = it.base();
    root->children.erase(--forwardIt);//gpt����ɾ�����������ָ��Ԫ�صķ��� ��֪���в���
    double new_ucb_value = CalculateUCB(p);
    root->children.emplace(new_ucb_value, p);
    return win_player;
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
    10 �޴�����ʱ��Ϊƽ��
    11 Rave�㷨�Ż� ����UCB�㷨
    12 ����UCB���� �ں��ڽ׶Σ������𽥼�С̽��������ʹ���㷨����������ѡ������ͳ����Ϣ��Ϊ׼ȷ�Ľڵ������չ�����ı�Cֵ
    13 �Գ��Լ�֦���ֲ���֦����״̬ѹ���ȷ���
    14 ���ѧϰ
    15 ����� ��Ҫ����չ�� λ��Ϊ-1 -1 playerΪ���һ���µ��ˣ���ʵ���Ƕ��֣�
*/


int main()
{
    int x, y, n;
    // �ָ�Ŀǰ��������Ϣ
    cin >> n;
    for (int i = 0; i < n - 1; i++)
    {
        cin >> x >> y;
        if (x != -1)
            board[x][y] = -1; // �Է�
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 1; // �ҷ�
    }
    cin >> x >> y;
    if (x != -1)
        board[x][y] = -1; // �Է�

    // ��ʱboard[][]��洢�ľ��ǵ�ǰ���̵�����������Ϣ,x��y����ǶԷ����һ���µ���

    /************************************************************************************/
    /***********�����������Ĵ��룬���߽�������������ӵ�λ�ã�����new_x��new_y��****************/
    int new_x, new_y;

    if (n == 1)
    {
        if (x == -1)
        {
            // ��һ�غ��ҷ����֣�������������
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
        // ִ�����ŵ���һ��

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
    /***********���Ϸ������Ĵ��룬���߽�������������ӵ�λ�ã�����new_x��new_y��****************/
    /************************************************************************************/

    // ����
    // ��ƽ̨������߽��
    printf("%d %d\n", new_x, new_y);
    return 0;
}