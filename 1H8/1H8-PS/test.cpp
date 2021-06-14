#include <iostream>
#include <stdio.h>

using namespace std;

int main()
{
	int queue[10] = {0};
    int front = 0;
	int rear = 0;
	int select, num;
    while(true)
	{
		printf("请选择出队还是入队, 入队0, 出队1:\n");
    	cin >> select;

		if(select == 0)
		{
			printf("请输入入队的数字:\n");
    	    cin >> num;
			if((rear + 1) % 10 == front)
			{
				printf("队列已满！！！\n");
				continue;
			}

		    queue[rear] = num;
			rear = (rear + 1) % 10; //队尾前进一;
		}else if(select == 1)
		{
			if(rear == front)
			{
				printf("队列为空！！！\n");
				continue;
			}

			printf("%d出队....\n", queue[front]);
			queue[front] = NULL; //删除出队元素
			front = ( front + 1 ) % 10; //队首前进一;
		}

		printf("当前队列:\n");
		printf("        %4d\n", queue[2]);
		printf("    %4d    %4d\n", queue[1], queue[3]);
		printf("%4d            %4d\n", queue[0], queue[4]);
		printf("\n");
		printf("%4d            %4d\n", queue[9], queue[5]);
		printf("    %4d    %4d\n", queue[8], queue[6]);
		printf("        %4d\n", queue[7]);
		/*
		for(int i = 0; i < 10; i++)
		{
			cout << queue[i] << " ";
		}
		printf("\n");
		*/
    }

	return 0;
}

