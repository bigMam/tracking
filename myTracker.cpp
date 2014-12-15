#include "myTracker.h"
//�����õ�pedArea�洢��tracker�У����ﲻ��ֱ��ʹ�����ðɣ�
//������ʹ��ͬһ�ڴ�ռ䣬������ĸı䣬���������Ӱ�졣��ò��û�����⣬�ȳ��������ٽ����޸İ�

Tracker::Tracker()
{
	//��ɶ�kalman�˲����ĳ�ʼ������������֮���tracklet��Ԥ�����
	KF = cv::KalmanFilter(stateNum, measureNum, 0);
	state = cv::Mat(stateNum, 1, CV_32F);//�˲���״̬����
	processNoise = cv::Mat(stateNum, 1, CV_32F);//�˲�����������
	measurement = cv::Mat::zeros(measureNum, 1, CV_32F);//�˲�����������
	KF.transitionMatrix = *( cv::Mat_<float>(4, 4) << 1,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1 );//ת�ƾ���

	setIdentity(KF.measurementMatrix);//��������
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));
	randn(KF.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));

	//�Զ���������ȡģ����г�ʼ������
	extractor = FeatureExtractor();
	extractor.initCache();
	lockedPedArea = NULL;
	trackerletHead = NULL;
}


void Tracker::setLoackedPedArea(LockedArea* result)
{
	lockedPedArea = result;//ֱ��ָ��ͬһ�ڴ�ռ�ͺ��ˣ���ʱ�����������������ܻ�Ҫ���иı�
}
bool Tracker::update(cv::Mat &sourceImage)
{
	if(trackerletHead == NULL)
	{
		if(lockedPedArea == NULL)
			return false;
		else
		{
			//����lockedPedArea�����µ�tracklet
			int topLeftX = lockedPedArea->topLeftX;
			int topLeftY = lockedPedArea->topLeftY;
			int width = lockedPedArea->width;
			int height = lockedPedArea->height;
			int letWidth = width * 0.5;
			int letHeight = height * 0.25;
			int letTopLeftX = topLeftX + width * 0.25;
			int letTopLeftY = topLeftY + height * 0.25;
			cv::Mat subImage = sourceImage(cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight));
			cv::rectangle(sourceImage,cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight),cv::Scalar(255,0,0),2);
			//blockFeature target;
			//extractor.computeFeature(subImage,target);
			//Trackerlet* trackerlet = new Trackerlet();
			//trackerlet->topLeftX = letTopLeftX;
			//trackerlet->topLeftY = letTopLeftY;
			//trackerlet->width = letWidth;
			//trackerlet->Height = letHeight;
			//trackerlet->next = NULL;
			//trackerlet->trackerletID = 0;//���ID��ʱû��ʲô���壬����ʱ��������
			return true;
		}
	}
	else
	{
		return true;
	}
}