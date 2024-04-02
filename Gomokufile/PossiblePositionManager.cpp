/*管理搜索范围，每选中一点落子，该点周围格子变为可选中状态*/

#include "PossiblePositionManager.h"

#include <cassert>

bool isValidPosition(const Position &pos)
{
    return pos.x >= 0 && pos.x < SIZE && pos.y >= 0 && pos.y < SIZE;
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

void PossiblePositionManager::AddPossiblePositions(int board[SIZE][SIZE], const Position &p)
{

    set<Position> newPositions;
    for (const auto& direct : directions)
    {
		Position newPos(p.x + direct.first, p.y + direct.second);
        if (isValidPosition(newPos) && board[newPos.x][newPos.y] == EMPTY)
        {
			auto insertResult = currentPossiblePositions.insert(newPos);

			// 如果插入成功
			if (insertResult.second)
                newPositions.insert(newPos);
		}
	}
    
    

    PosHistory ph;//新增历史记录
    ph.newPositions = newPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end())//当前选中位置原本为可选位置，现选择后不可能选择，删除
    {
        currentPossiblePositions.erase(p);
        ph.removedPosition = p;
    }
    else
        ph.removedPosition.x = -1;//给回溯提供标志

    allHistory.push_back(ph);//保存历史记录
}

void PossiblePositionManager::Rollback()
{
    if (currentPossiblePositions.empty())
        return;

    PosHistory hi = allHistory.back();
    allHistory.pop_back();

    //回溯

    // 清除掉前一步加入的点
    for (auto &pos : hi.newPositions)
        currentPossiblePositions.erase(pos);

    // 加回前一步删除的点
    if (hi.removedPosition.x != -1)
        currentPossiblePositions.insert(hi.removedPosition);
}

const set<Position> &PossiblePositionManager::GetCurrentPossiblePositions()
{
	return currentPossiblePositions;
}