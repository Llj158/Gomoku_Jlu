#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


using namespace std;


#define UNKNOWN_SCORE (10000001)
#define HASH_ITEM_INDEX_MASK (0xffff)
#define MAX_SCORE (10000000)
#define MIN_SCORE (-10000000)
const int PLAYER = 1;
const int DEPTH = 7;
const int POINT_NUM = 9;
const int SIZE = 15;

enum Role { HUMAN = -1, COMPUTOR = 1, EMPTY = 0 };

//位置结构体，行是x，列是y
struct Position {
    int x;
    int y;
    int score;
    Position() {}
    Position(int x, int y) {
        this->x = x;
        this->y = y;
        score = 0;
    }
    Position(int x, int y, int score) {
        this->x = x;
        this->y = y;
        this->score = score;
    }
    bool operator <(const Position &pos) const {
        if (score != pos.score) {
            return score > pos.score;
        }
        if (x != pos.x) {
            return x < pos.x;
        }
        else {
            return y < pos.y;
        }
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
