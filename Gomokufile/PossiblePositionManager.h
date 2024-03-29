#pragma once
#include "head.h"

#include <set>
#include <vector>

using namespace std;


struct HistoryItem {
    set<Position> addedPositions;
    Position removedPosition;
};

class PossiblePositionManager
{
public:
    PossiblePositionManager();
    ~PossiblePositionManager();
    void AddPossiblePositions(int board[SIZE][SIZE],const Position& p);
    void Rollback();
    const set<Position>& GetCurrentPossiblePositions();
    void RemoveAll();
    void SetEvaluateFunc(int(*evaluateFunc)(Position p));
private:
    set<Position> currentPossiblePositions;
    vector<HistoryItem> history;
    vector<pair<int, int> > directions;
    int (*evaluateFunc)(Position p);
};



