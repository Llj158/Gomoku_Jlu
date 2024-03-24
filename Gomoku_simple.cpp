#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>
using namespace std;

const int SIZE = 15;
int board[SIZE][SIZE] = {0}; // ����1���Է�2���հ�0

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

struct Position
{
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
};
struct compare
{
    bool operator()(const Position &a, const Position &b)
    {
        return a.score < b.score;
    }
};

priority_queue<Position, vector<Position>, compare> q;
// int scores[72]; // ������ַ�������������Ʋ�ࣩ
// int allScore;   // ����������

int evaluatePoint(int x, int y)
{
    int score = 0;
    string pattern[4];
    int role = 1;
    // ����
    for (int i = 0; i < SIZE; i++)
    {
        pattern[0].push_back(board[i][y] == role ? '1' : board[i][y] == 0 ? '0'
                                                                          : '2');
    }
    for (auto i : patterns)
    {
        auto pos = pattern[0].find(i.pattern);
        if (pos != string::npos)
        {
            score += i.score;
            break;
        }
    }
    // ����
    for (int i = 0; i < SIZE; i++)
    {
        pattern[1].push_back(board[x][i] == role ? '1' : board[x][i] == 0 ? '0'
                                                                          : '2');
    }
    for (auto i : patterns)
    {
        auto pos = pattern[1].find(i.pattern);
        if (pos != string::npos)
        {
            score += i.score;
            break;
        }
    }
    // Ʋ��
    for (int i = x + min(y, SIZE - 1 - x), j = y - min(x, SIZE - 1 - y); i >= 0 && j < SIZE; i--, j++)
    {
        pattern[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                          : '2');
    }
    for (auto i : patterns)
    {
        auto pos = pattern[2].find(i.pattern);
        if (pos != string::npos)
        {
            score += i.score;
            break;
        }
    }
    // ����
    for (int i = x - min(x, y), j = y - min(x, y); i < SIZE && j < SIZE; i++, j++)
    {
        pattern[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0'
                                                                          : '2');
    }
    for (auto i : patterns)
    {
        auto pos = pattern[3].find(i.pattern);
        if (pos != string::npos)
        {
            score += i.score;
            break;
        }
    }
    return score;
}

int vis[SIZE][SIZE] = {0};
int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};

void onedeep_dfs(int x, int y)
{
    vis[x][y] = 1;
    for (int i = 0; i < 8; i++)
    {
        int nx = x + dx[i], ny = y + dx[7 - i];
        if (nx >= 0 && nx < SIZE && ny >= 0 && ny < SIZE && board[nx][ny] == 0 && !vis[nx][ny])
        {
            q.push(Position(nx, ny, evaluatePoint(nx, ny)));
            vis[nx][ny] = 1;
        }
    }
}

void calScore()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            if (board[i][j] != 0)
            {
                onedeep_dfs(i, j);
            }
        }
    }
}

int main()
{
    int x, y, n;
    // �ָ�Ŀǰ��������Ϣ
    cin >> n;
    for (int i = 0; i < n - 1; i++)
    {
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 2; // �Է�
        cin >> x >> y;
        if (x != -1)
            board[x][y] = 1; // �ҷ�
    }
    cin >> x >> y;
    if (x != -1)
        board[x][y] = 2; // �Է�

    // ��ʱboard[][]��洢�ľ��ǵ�ǰ���̵�����������Ϣ,x��y����ǶԷ����һ���µ���

    /************************************************************************************/
    /***********�����������Ĵ��룬���߽�������������ӵ�λ�ã�����new_x��new_y��****************/
    int new_x, new_y;
    if (x != -1 && n == 1)
    { // ��һ�غϣ��ҷ����֣��ܻ���һ������
        new_x = -1;
        new_y = -1;
    }
    else
    { // �ǵ�һ�غ��ҷ����֣����һ�غ��ҷ����֣����ܻ���
        if (n == 1)
        { // ��һ�غ��ҷ����֣�������������
            new_x = 7;
            new_y = 7;
        }
        else
        {
            calScore();
            new_x = q.top().x;
            new_y = q.top().y;
        }
    }

    /***********���Ϸ������Ĵ��룬���߽�������������ӵ�λ�ã�����new_x��new_y��****************/
    /************************************************************************************/

    // ��ƽ̨������߽��
    printf("%d %d\n", new_x, new_y);
    return 0;
}