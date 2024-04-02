#pragma once
#include "head.h"

#include <set>
#include <vector>

using namespace std;


struct PosHistory {
    set<Position> newPositions;
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
    //void SetEvaluateFunc(int(*evaluateFunc)(Position p));
private:
    set<Position> currentPossiblePositions;
    vector<PosHistory> allHistory;
    vector<pair<int, int> > directions;
    // int (*evaluateFunc)(Position p);
};



