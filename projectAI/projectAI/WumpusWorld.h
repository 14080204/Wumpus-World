// WumpusWorld.h

#ifndef WUMPUSWORLD_H
#define WUMPUSWORLD_H

#include "Percept.h"
#include "Action.h"
#include "WorldState.h"

class WumpusWorld
{
public:
	WumpusWorld(int size);//��size��С�����������
	WumpusWorld(char* worldFile);//���߰�hyper-parameters�����ض�����
	void Initialize();//��ʼ���������
	Percept& GetPercept();//����Percept���͵�����,�������Ľӿ�֮һ
	void ExecuteAction(Action action);//ִ��Agent�Ĳ���
	bool GameOver();//�ж���Ϸ����
	int GetScore();//Agent�÷�
	void Print();//�����ǰͼ��
	void Write(const char* worldFile);

private:
	int numActions;
	Percept currentPercept;
	WorldState currentState;
};

#endif // WUMPUSWORLD_H
