//��ɹ��ܣ��ɵײ��ṩ���˾��ο�ܣ������ɿ�tracklet�����ύ���ϲ�manager
#pragma once

#include "SymmetryProcess.h"//lockedArea
#include "featureExtractor.h"//blockFeature
#include "discriminator.h"
#include "opencv2/video/tracking.hpp"

typedef struct _trackerlet
{
	int trackerletID;//�����趨��ţ������ڶ���洢��Ԫ�ڶ�ͬһĿ����в��ң�������Ϊ�˱�ʶ֮���
	int topLeftX;
	int topLeftY;
	int width;
	int height;
	int occupied;//��ʾ��ǰtrackerlet������ʹ������ռ�ã����һ��ʹ���ߣ����Խ���ɾ������������ܹ������Ƴ���
	blockFeature featureSet;//ÿ��trackerlet���ж�Ӧ��������ȡ������֮�����ǰ������ԶԱ�
	_trackerlet* next;
	_trackerlet()
	{
		trackerletID = 0;
		topLeftX = 0;
		topLeftY = 0;
		width = 0;
		height = 0;
		occupied = 0;//�����ռ���ߣ���Ҫ��������distrator���飬targetTrackerletList�б�,���о���manager�е�Ŀ���
		next = NULL;
	}
	void setBlockFeature(blockFeature& blockfeatures)
	{
		featureSet = blockfeatures;
	}
}Trackerlet;

class Tracker
{
	int stateNum;//״̬����[x,y,dx,dy,width,height,dw,dh]
	int measureNum;//��������[x,y,width,height]
	cv::Mat state; // (x,y,dX,dY)
	cv::Mat processNoise;
	cv::Mat measurement;
	FeatureExtractor extractor;
	Discriminator discriminator;//�ֱ���
	cv::KalmanFilter KF;//���趨һ��kalman�˲�������һ�£���ν��в���

	LockedArea* lockedPedArea;//���õ����˴�������ͷ���
	Trackerlet* targetTrackerlet;
	//Ҳ���������ʽ,���������ʽ����Ҫ�����м��õ�tracklet���в������᲻���ʱ�أ�
	//�Ƚ�������һ�����ˣ�֮���ٽ��е�����������һ�ν��������ݶ����ǽ���
	//����targetTrackerletΪ�б���ʽ����һ��Ԫ��Ϊ����Ŀ�꣬���Ϊ��ǰ���õ�����Ŀ��

	double weights[8];//����Ȩ�أ��ֱ�Ե�ǰdiscriminator��������manager��discriminator��������Ȩ�ظ���

	Trackerlet* distratorList[6];//�����ǽ�����tracklet���ݱ����������ڸ�������ֵȨ�أ�������Ҫ����һ���ռ��������ж϶ӿ�or����
	static const int capacity = 6;//distrator�б��������ޣ�������oldest oneɾ��
	int front;//��ͷ�±�
	int rear;//��β�±�

	int letNumber;//trackerlet���

public:
	Tracker();
	~Tracker();
	void setLoackedPedArea(LockedArea *result);
	//��֮ǰtracklet���и��£��������µ�tracklet�����ڹ���,�������ʧ�ܣ����趨request
	//haveRectBoxing��ʾ��ǰ�Ǹ��ݾ��ο����ݽ��и��£�������ʱ�ִ���һ���µ�����
	//�Ե�ǰͼ��������ֻ��ɲ��ּ�⣬���������haveBoxing�����ĺ���Ͳ���ȷ��
	//�����������΢����һ�£��������뵽���������⣬һʱ���ܹ�ȫ�����������Ҫ��ȷ��ҪĿ�꣬
	//�������ι�ϵ���ſ��Ա�֤�Լ�������ƫ����Щ�����ǿ��Խ����Ӻ�ģ�
	bool update(cv::Mat &souceImage,bool haveRectBoxing);
	bool update(cv::Mat &souceImage);//�µĸ��¹��̣�������distrator�Ĺ�������̫���ˣ�������Ҫ��һ��ϸ��
	void extractTracklet(cv::Mat &sourceImage,LockedArea* lockedPedArea,Trackerlet* tracklet);//����rect��ȡtracklet
	double distinguish(blockFeature& target, blockFeature& current);//�����������������ֶ�
	void featureWeighting(blockFeature& current);//���߸��ݵ�ǰ�õ����ݶԸ�����������Ȩ�ؽ��е���
	void insertDistrator(Trackerlet* tracklet);//������tracklet����distrator��ͬʱ��֤distrator��������
	bool isTargetTrackerlet(Trackerlet* current);//�жϵ�ǰtrackerlet�Ƿ�ΪĿ��targetTrackerlet


	void correctTarget(Trackerlet* correctTrackerlet);//�Ը���Ŀ�������������
	Trackerlet* getTrackerlist();
	void clearList();
	void insertList(Trackerlet* trackerlet);
};