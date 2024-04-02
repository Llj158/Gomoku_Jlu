/*����������Χ��ÿѡ��һ�����ӣ��õ���Χ���ӱ�Ϊ��ѡ��״̬*/

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

			// �������ɹ�
			if (insertResult.second)
                newPositions.insert(newPos);
		}
	}
    
    

    PosHistory ph;//������ʷ��¼
    ph.newPositions = newPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end())//��ǰѡ��λ��ԭ��Ϊ��ѡλ�ã���ѡ��󲻿���ѡ��ɾ��
    {
        currentPossiblePositions.erase(p);
        ph.removedPosition = p;
    }
    else
        ph.removedPosition.x = -1;//�������ṩ��־

    allHistory.push_back(ph);//������ʷ��¼
}

void PossiblePositionManager::Rollback()
{
    if (currentPossiblePositions.empty())
        return;

    PosHistory hi = allHistory.back();
    allHistory.pop_back();

    //����

    // �����ǰһ������ĵ�
    for (auto &pos : hi.newPositions)
        currentPossiblePositions.erase(pos);

    // �ӻ�ǰһ��ɾ���ĵ�
    if (hi.removedPosition.x != -1)
        currentPossiblePositions.insert(hi.removedPosition);
}

const set<Position> &PossiblePositionManager::GetCurrentPossiblePositions()
{
	return currentPossiblePositions;
}