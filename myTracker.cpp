#include "myTracker.h"
//�����õ�pedArea�洢��tracker�У����ﲻ��ֱ��ʹ�����ðɣ�
//������ʹ��ͬһ�ڴ�ռ䣬������ĸı䣬���������Ӱ�졣��ò��û�����⣬�ȳ��������ٽ����޸İ�

#define HAVE_BORDER 1
//ʹ�ñ߽�Ԥ�⣬���Ƕ��䲻���и�ֵ���Խǵ�����Ԥ��Ч������������ʱ��׷����ԭ���Ƚ�����������ͨ����ܴ����

Tracker::Tracker()
{
	//��ɶ�kalman�˲����ĳ�ʼ������������֮���tracklet��Ԥ�����

#if HAVE_BORDER == 1
	stateNum = 8;
	measureNum = 4;
#else
	stateNum = 4;
	measureNum = 2;
#endif

	KF = cv::KalmanFilter(stateNum, measureNum, 0);
	state = cv::Mat(stateNum, 1, CV_32F);//�˲���״̬����
	processNoise = cv::Mat(stateNum, 1, CV_32F);//�˲�����������
	measurement = cv::Mat::zeros(measureNum, 1, CV_32F);//�˲�����������

#if HAVE_BORDER == 1
	KF.transitionMatrix = *( Mat_<float>(8, 8) << 
		1,0,1,0,0,0,0,0,
		0,1,0,1,0,0,0,0,
		0,0,1,0,0,0,0,0,
		0,0,0,1,0,0,0,0,
		0,0,0,0,1,0,1,0,
		0,0,0,0,0,1,0,1,
		0,0,0,0,0,0,1,0,
		0,0,0,0,0,0,0,1);//ת�ƾ���
#else
	KF.transitionMatrix = *(Mat_<float>(4,4) << 1,0,1,0,0,1,0,1,0,0,1,0,0,0,0,1);
#endif

	setIdentity(KF.measurementMatrix);//��������
	setIdentity(KF.processNoiseCov, cv::Scalar::all(1e-5));
	setIdentity(KF.measurementNoiseCov, cv::Scalar::all(1e-1));
	setIdentity(KF.errorCovPost, cv::Scalar::all(1));
	//KF.statePost = *(Mat_<float>(4,1) << 320,240,1,1);
	randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));

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
//���¹��̵�˼·����
//һ��ֱ�Ӹ������˼����ο����tracklet��ȡ
//���Ǹ���kalmanԤ��������tracklet��ȡ������֮ǰtracklet���бȽ�
//�������ڿɽ��ܷ�Χ�ڣ�����и��£�������Ϊ����ʧ�ܣ���Ҫ��detector���ͼ������
//����ֵΪisRequset,��ʾ��ǰ���º��Ƿ���Ҫ���м�⣬����ʧ������Ҫ���м�⣬
//Ӧ�ö��Һ��б�Ҫ���߿���Ϣ���ǽ�����ֱ��ʹ��֮ǰ�ı߿���Ϣ�����ԣ�
//�������Ͻǵ��Ԥ�⻹����һ��Ч����
bool Tracker::update(cv::Mat &sourceImage,bool haveRectBoxing)
{
	if(haveRectBoxing)//��ʾ��ǰ�����ʳ�¯�����˼����ο򣬲���Ҫ����Ԥ����̣��д���ȶ
	{
		if(lockedPedArea == NULL)//��⣬����û�м�⵽���ˡ���Ȼû�м�⵽���˵�����Ȼ���ܹ�ʹ��Ԥ�ⷽ����
			//ԭ��֮ǰ��Ԥ�����Ѿ��������⵱ǰͬ���޷�����Ԥ�����
			return true;
		else//�м�����˿��Ը��ݼ�����˶�tracklet���и��¹��̣�������ζ�tracklet���й���û����ȷ��˼·����ʱ�Ƚ���tracklet���й���
			//��д��һ�����̳�����
		{
			//����lockedPedArea�����µ�tracklet
			int topLeftX = lockedPedArea->topLeftX;
			int topLeftY = lockedPedArea->topLeftY;
			int width = lockedPedArea->width;
			int height = lockedPedArea->height;

			int letWidth = width * 0.4;
			int letHeight = height * 0.18;
			int letTopLeftX = topLeftX + width * 0.3;
			int letTopLeftY = topLeftY + height * 0.25;
			cv::Mat subImage = sourceImage(cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight));
			cv::rectangle(sourceImage,cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight),cv::Scalar(255,0,0),2);

			blockFeature target;
			extractor.computeFeature(subImage,target);

			Trackerlet* trackerlet = new Trackerlet();
			trackerlet->topLeftX = letTopLeftX;
			trackerlet->topLeftY = letTopLeftY;
			trackerlet->width = letWidth;
			trackerlet->Height = letHeight;
			trackerlet->next = NULL;
			trackerlet->setBlockFeature(target);
			trackerlet->trackerletID = 0;//���ID��ʱû��ʲô���壬����ʱ��������
			circle(sourceImage,cv::Point(letTopLeftX,letTopLeftY),5,CV_RGB(255,0,0),3);//����ǰ����ֱֵ����ԭͼ�Ͻ��л���

			//���ݵ�ǰ���ֵ��kalman���������������predict������ӣ��������Ԥ������û�в����ĵģ���Ϊ���ڼ��ֵ
			Mat prediction = KF.predict();

			measurement.at<float>(0) = (float)letTopLeftX;
			measurement.at<float>(1) = (float)letTopLeftY;
#if HAVE_BORDER == 1
			//measurement.at<float>(2) = (float)letWidth;
			//measurement.at<float>(3) = (float)letHeight;//�������ʣ�����Ϊʲô���ܼ�������Ĳ���ֵ�������϶�x��yֵ��û��Ӱ���
			//��Ҫͨ���Ķ�Դ�������н��ͣ����������ɣ����ǻ�����Ҫ��������ȥ
#endif
			KF.correct(measurement);//���õ�ǰ����ֵ�����˲�����������

			if(trackerletHead != NULL)
			{
				delete trackerletHead;
			}
			trackerletHead = trackerlet;
			return false;
		}
	}
	else
	{
		//��ǰtrackerletHead�ǿ�,���Գ��Խ��бȽϣ�������ν���Ԥ���أ�
		//����и��ٹ���
		Mat prediction = KF.predict();//�����˲����Ե�ǰ���tracklet���ν���Ԥ��
		float *data = prediction.ptr<float>(0);
		int predictX = data[0];
		int predictY = data[1];
#if HAVE_BORDER == 1
		int predictW = data[2];//����ı߿�ֵ��ʱ����Ҫ������ѡ��ʹ��tracklet�ı߿����ȡֵ
		int predictH = data[3];
#endif
		std::cout<<predictX<<" "<<predictY<<" "<<std::endl;
		circle(sourceImage,cv::Point(predictX,predictY),5,CV_RGB(0,255,0),3);
		if(trackerletHead == NULL)
			//��ǰ��û��tracklet���ڣ�ֱ�ӷ���true
			return true;
		else
		{
			//���ｫ���ݵ�ǰ�õ�tracklet��֮ǰtracklet����Ԥ��ƥ�䣬������ƶȿ�������������ɾ�����������������
			//������൫Ҫ˼·����
			int letWidth = trackerletHead->width;
			int letHeight = trackerletHead->Height;
			cv::rectangle(sourceImage,cv::Rect(predictX,predictY,letWidth,letHeight),cv::Scalar(255,0,0),2);
			cv::Mat subImage = sourceImage(cv::Rect(predictX,predictY,letWidth,letHeight));

			blockFeature target;
			extractor.computeFeature(subImage,target);
			//����ǰ�õ�blockfeature��֮ǰ�洢���ݽ��бȽ�

			double distinguish = extractor.distinguish(trackerletHead->featureSet,target);
			std::cout<<"����ֵΪ��"<<distinguish<<std::endl;
			if(distinguish > 0.3)
				return true;
			else
				return false;
		}
		//circle(sourceImage,cv::Point(predictX + predictW ,predictY + predictH),5,CV_RGB(0,255,0),3);
		//cv::Mat subImage = sourceImage(cv::Rect(predictX,predictY,predictW,predictH));
		//cv::rectangle(sourceImage,cv::Rect(predictX,predictY,predictW,predictH),cv::Scalar(255,0,0),2);

	}
}