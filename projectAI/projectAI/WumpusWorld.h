// WumpusWorld.h

#ifndef WUMPUSWORLD_H
#define WUMPUSWORLD_H

#include "Percept.h"
#include "Action.h"
#include "WorldState.h"

class WumpusWorld
{
public:
	WumpusWorld(int size);//以size大小生成随机世界
	WumpusWorld(char* worldFile);//或者按hyper-parameters生成特定世界
	void Initialize();//初始化世界参数
	Percept& GetPercept();//返回Percept类型的引用,主函数的接口之一
	void ExecuteAction(Action action);//执行Agent的操作
	bool GameOver();//判断游戏结束
	int GetScore();//Agent得分
	void Print();//输出当前图像
	void Write(const char* worldFile);

private:
	int numActions;
	Percept currentPercept;
	WorldState currentState;
};

#endif // WUMPUSWORLD_H
