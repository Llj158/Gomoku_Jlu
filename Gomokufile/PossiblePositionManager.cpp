
#include "PossiblePositionManager.h"

#include <cassert>


bool IsInBoard(int x, int y) {
    if (x >= 0 && x < 15 && y >= 0 && y < 15)
        return true;
    return false;
}

PossiblePositionManager::PossiblePositionManager()
{
    directions.push_back(pair<int, int>(1, 1));
    directions.push_back(pair<int, int>(1, -1));
    directions.push_back(pair<int, int>(-1, 1));
    directions.push_back(pair<int, int>(-1, -1));
    directions.push_back(pair<int, int>(1, 0));
    directions.push_back(pair<int, int>(0, 1));
    directions.push_back(pair<int, int>(-1, 0));
    directions.push_back(pair<int, int>(0, -1));
}


PossiblePositionManager::~PossiblePositionManager()
{
}

void  PossiblePositionManager::AddPossiblePositions(int board[SIZE][SIZE],const Position& p) {
    unsigned int i;
    set<Position> addedPositions;

    for (i = 0; i < directions.size(); i++) {
        //�жϷ�Χ
        if (!IsInBoard(p.x + directions[i].first, p.y + directions[i].second))
            continue;

        if (board[p.x + directions[i].first][p.y + directions[i].second] == EMPTY) {
            Position pos(p.x + directions[i].first, p.y + directions[i].second);
            pair<set<Position>::iterator, bool> insertResult = currentPossiblePositions.insert(pos);

            //�������ɹ�
            if(insertResult.second)
                addedPositions.insert(pos);
        }
    }

    HistoryItem hi;
    hi.addedPositions = addedPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) {
        currentPossiblePositions.erase(p);
        hi.removedPosition = p;
    }
    else {
        hi.removedPosition.x = -1;
    }

    history.push_back(hi);
}

void PossiblePositionManager::Rollback() {
    if (currentPossiblePositions.empty())
        return;

    HistoryItem hi = history[history.size() - 1];
    history.pop_back();

    set<Position>::iterator iter;

    //�����ǰһ������ĵ�
    for (iter = hi.addedPositions.begin(); iter != hi.addedPositions.end(); iter++) {
        currentPossiblePositions.erase(*iter);
    }

    //����ǰһ��ɾ���ĵ�
    if(hi.removedPosition.x != -1)
        currentPossiblePositions.insert(hi.removedPosition);
}

const set<Position>& PossiblePositionManager::GetCurrentPossiblePositions() {
    return currentPossiblePositions;
}

void PossiblePositionManager::RemoveAll() {
    currentPossiblePositions.clear();
    history.clear();
}
