#include "manager.h"

Manager::Manager()
{
	front = 0;//��ʼ���б�����Ϊ��
	rear = 0;
	for(int i = 0; i < capacity; i++)
	{
		targetPool[i] = NULL;
	}
}

void Manager::setTrackerletList(Trackerlet* list)
{
	trackerletList = list;
}
//���ݴ���trackerlet�ж���ǰ����Ŀ���Ƿ�Ϊָ��Ŀ�꣬ͬʱ��������Ϣ���и���
//�ڸ���Ŀ����ָ��Ŀ�겻һ�µ������£�return false������tracker��������
//Ĭ�������һ���ڵ�Ϊ����Ŀ��
bool Manager::dicision()
{
	//��������������ж�ͷ����Ƿ�Ϊָ��Ŀ�ꣻ����Ŀ��أ���ǰ�趨Ŀ�������Ϊ5���������ӣ�

	//�жϵ�ǰtrackerletListͷ����Ƿ�Ϊ���ٽڵ�
	if(trackerletList == NULL)
		return true;//�����������������£���Ȼ�޷������������̣���Ȼ����true������֪����û�д���ô�ģ�
	if(front == rear)//��ǰĿ���Ϊ�գ�ֱ�Ӽ�������ͷ���
	{
		trackerletList->occupied++;
		targetPool[front++] = trackerletList;//��ͷ����������
	}
	else//Ŀ��طǿյ������£���Ҫ��ͷ�������ж���
	{
		

	}




	//��Ŀ��ؽ��и��²���



	return true;
}