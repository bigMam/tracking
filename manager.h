
//ʵ�ֶ�trackerlet�Ĺ��������ڱ�Ҫ��ʱ����ɸ���Ŀ����ڵ�����
//�ܹ���Ŀ���ٴγ���ʱ�����ٶ�λ
#include "tracker.h"
class Manager
{
private:
	Trackerlet* trackerletList;//���ɼ��㴫�ݹ�����trackerlet�б�
	Trackerlet targetTrackerlet[6];//���ض�Ŀ����д洢�����ڶ��ض�Ŀ������ж����̣�ͬʱ���ڵ��������ܹ�������ʷ��Ϣ
	//��Ŀ���ٴ�ʶ�𣬽���������һ����Ϣ�ǲ��ܹ���Ŀ���ж������ģ����ڵ������󻷾����ϴ��Ǵ��ڽ϶಻ͬ��
public:
	void setTrackerletList(Trackerlet* list);

	bool dicision(){return true;};//���ݴ���trackerlet�ж���ǰ����Ŀ���Ƿ�Ϊָ��Ŀ�꣬ͬʱ��������Ϣ���и���
	//�ڸ���Ŀ����ָ��Ŀ�겻һ�µ������£�return false������tracker��������

	Trackerlet* correct(){return NULL;};//������ɾ��ߣ��϶���ǰtracker����Ŀ����ָ��Ŀ�겻ͬ�������������Ҳ���ǽ���ǰָ��tracklet������tracker

};