#pragma once
#include "featureExtractor.h"

class Discriminator
{
private:
	double weights[8];
	double distance[8];//���ڱ���һ��feature����һ��feature��distance
	blockFeature current;
	cv::MatND targetLBP;
	cv::MatND currentLBP;
	cv::MatND targetCanny;
	cv::MatND currentCanny;
	cv::MatND targetEHD;
	cv::MatND currentEHD;

	int count;//ͳ����������feature�������
public:
	Discriminator();

	void setCurrentFeature(blockFeature& feature);//�趨��ǰfeature��֮��ֻ��Ҫ������Ŀ��feature���бȽϼ���
	void computeDistance(blockFeature& targe);//������feature���ֶ�
	void clearDistance();//�Ծ�����Ϣ�������
	void computeDistanceHold(blockFeature& target);//�������㵱ǰfeature��targetFeatureƽ������
	double distinguish();

	void getDistance(double outputDistance[]);//��������������������������
	void setWeights(double inputWeights[]);//�����������Ȩ�ؽ��е���
};




