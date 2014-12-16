//��ɹ��ܣ��ɵײ��ṩ���˾��ο�ܣ������ɿ�tracklet�����ύ���ϲ�manager
#include "SymmetryProcess.h"//lockedArea
#include "featureExtractor.h"//blockFeature
#include "opencv2/video/tracking.hpp"

typedef struct _trackerlet
{
	int trackerletID;
	int topLeftX;
	int topLeftY;
	int width;
	int Height;
	blockFeature featureSet;//ÿ��trackerlet���ж�Ӧ��������ȡ������֮�����ǰ������ԶԱ�
	_trackerlet* next;
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
	cv::KalmanFilter KF;//���趨һ��kalman�˲�������һ�£���ν��в���

	LockedArea *lockedPedArea;//���õ����˴�������
	Trackerlet *trackerletHead;//Ҳ���������ʽ,���������ʽ����Ҫ�����м��õ�tracklet���в������᲻���ʱ�أ�
public:
	Tracker();
	void setLoackedPedArea(LockedArea *result);
	//��֮ǰtracklet���и��£��������µ�tracklet�����ڹ���,�������ʧ�ܣ����趨request
	//haveRectBoxing��ʾ��ǰ�Ǹ��ݾ��ο����ݽ��и��£�������ʱ�ִ���һ���µ�����
	//�Ե�ǰͼ��������ֻ��ɲ��ּ�⣬���������haveBoxing�����ĺ���Ͳ���ȷ��
	//�����������΢����һ�£��������뵽���������⣬һʱ���ܹ�ȫ�����������Ҫ��ȷ��ҪĿ�꣬
	//�������ι�ϵ���ſ��Ա�֤�Լ�������ƫ����Щ�����ǿ��Խ����Ӻ�ģ�
	bool update(cv::Mat &souceImage,bool haveRectBoxing);

};