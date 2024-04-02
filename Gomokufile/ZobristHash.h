#pragma once

#include "head.h"

class ZobristHash
{
public:
  ZobristHash();
  ~ZobristHash();
  void recordHashItem(int depth, int score, HashItem::Flag flag);
  int getHashItemScore(int depth, int alpha, int beta);
  long long random64();
  void randomZobristValue();
  void initCurrentZobristValue();

  static long long boardZobristValue[2][SIZE][SIZE];
  static long long currentZobristValue; // 当前局面的zobrist值
private:
  HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];
};
