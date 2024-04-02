#include "ZobristHash.h"


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


