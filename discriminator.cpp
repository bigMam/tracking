#include "discriminator.h"

Discriminator::Discriminator()
{
	for(int i = 0; i < 8; i++)
		weights[i] = 1.0 / 8;

	for(int i = 0; i < 8; i++)
		distance[i] = 0.0;
	targetEHD = cv::Mat(5,1,CV_32F);
	currentEHD = cv::Mat(5,1,CV_32F);//��Щһ����������û�б�Ҫ��γ�ʼ�����趨Ϊ��Ա���������ظ����ü���
}
void Discriminator::setCurrentFeature(blockFeature& feature)
{
	current = feature;
	currentLBP = cv::Mat(current.cs_lbpFeature);
	currentCanny = cv::Mat(current.cannyFeature);
	for(int i = 0; i < 5; i++)
	{
		float* currentPtr = currentEHD.ptr<float>(i);
		currentPtr[0] = current.EHD[i];
	}
}

//����˲ʱ��feature���ϵľ���
void Discriminator::computeDistance(blockFeature& target)
{
	count = 0;//�������������������ۼ�����

	targetLBP = cv::Mat(target.cs_lbpFeature);
	targetCanny = cv::Mat(target.cannyFeature);

	distance[0] = compareHist(target.hueHist,current.hueHist,CV_COMP_BHATTACHARYYA);
	distance[1] = compareHist(target.satHist,current.satHist,CV_COMP_BHATTACHARYYA);
	distance[2] = compareHist(target.valHist,current.valHist,CV_COMP_BHATTACHARYYA);
	distance[3] = compareHist(targetLBP,currentLBP,CV_COMP_BHATTACHARYYA);
	distance[4] = compareHist(targetCanny,currentCanny,CV_COMP_BHATTACHARYYA);
	distance[5] = compareHist(target.horDerHist,current.horDerHist,CV_COMP_BHATTACHARYYA);
	distance[6] = compareHist(target.verDerHist,current.verDerHist,CV_COMP_BHATTACHARYYA);

	for(int i = 0; i < 5; i++)
	{
		float* targetPtr = targetEHD.ptr<float>(i);
		targetPtr[0] = target.EHD[i];
	}
	distance[7] = compareHist(targetEHD,currentEHD,CV_COMP_BHATTACHARYYA);
}

void Discriminator::computeDistanceHold(blockFeature& target)
{
	targetLBP = cv::Mat(target.cs_lbpFeature);
	targetCanny = cv::Mat(target.cannyFeature);

	distance[0] = distance[0] + compareHist(target.hueHist,current.hueHist,CV_COMP_BHATTACHARYYA);
	distance[1] = distance[1] + compareHist(target.satHist,current.satHist,CV_COMP_BHATTACHARYYA);
	distance[2] = distance[2] + compareHist(target.valHist,current.valHist,CV_COMP_BHATTACHARYYA);
	distance[3] = distance[3] + compareHist(targetLBP,currentLBP,CV_COMP_BHATTACHARYYA);
	distance[4] = distance[4] + compareHist(targetCanny,currentCanny,CV_COMP_BHATTACHARYYA);
	distance[5] = distance[5] + compareHist(target.horDerHist,current.horDerHist,CV_COMP_BHATTACHARYYA);
	distance[6] = distance[6] + compareHist(target.verDerHist,current.verDerHist,CV_COMP_BHATTACHARYYA);

	for(int i = 0; i < 5; i++)
	{
		float* targetPtr = targetEHD.ptr<float>(i);
		targetPtr[0] = target.EHD[i];
	}
	distance[7] = distance[7] + compareHist(targetEHD,currentEHD,CV_COMP_BHATTACHARYYA);
	count++;
}

void Discriminator::clearDistance()
{
	count = 0;
	for(int i = 0; i < 8; i++)
	{
		distance[i] = 0.0;
	}
}

//���ݵ�ǰ����feature�ľ���ֵ���������ֵ
double Discriminator::distinguish()
{
	double dissimilarity = 0.0;
	if(count > 1)//���ۼӽ��ȡƽ��ֵ
	{
		for(int i = 0; i < 8; i++)
		{
			distance[i] = distance[i] / count;
		}
	}
	for(int i = 0; i < 8; i++)
	{
		dissimilarity = dissimilarity + weights[i] * distance[i];
	}
	return dissimilarity;
}

void Discriminator::getDistance(double outputDistance[])
{
	for(int i = 0; i < 8; i++)
		outputDistance[i] = distance[i];
}
void Discriminator::setWeights(double inputWeights[])
{
	for(int i = 0; i < 8; i++)
		weights[i] = inputWeights[i];
}

//�����ڶ�tracker���岻��������£����Ժ������ڲ������޸ģ����ֽӿڲ��䣬���ܷ����discriminator�����
//��ɼȶ�Ŀ�꣬˵���ӿ���ƻ��ǻ��������