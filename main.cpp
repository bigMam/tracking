#include <iostream>
#include "mySVM.h"
#include "SymmetryProcess.h"
#include "myTracker.h"
using namespace cv;
int main()
{
	const char* filename =  "D:\\ImageDataSets\\TestSamples\\image1202.jpg";
	const char* videoname = "D:\\ImageDataSets\\trackingSamples\\MVI_2683_08_save.avi";
	SVMDetector detector;
	detector.loadDetectorVector("mydetectorNew.xml");
	detector.initSymmetryParam(527,531,310,248,530,1.2);

	Tracker tracker = Tracker();

	VideoCapture cap(videoname);
	if(!cap.isOpened())
		return -1;
	cv::Mat sourceImage;
	cv::Mat gray;
	int interval = 10;//detector�����ü��
	int k = 0;//ͳ�Ƶ��ü��
	bool isRequest = false;
	LockedArea* current,*tmp;//��¼��ǰ�Ѿ����õ�������

	while(cap.read(sourceImage))
	{
		//���ü�����
		if(k > interval || isRequest)
		{
			cv::cvtColor(sourceImage,gray,CV_BGR2GRAY);
			cv::imshow("frame",sourceImage);
			detector.detectBaseOnSymmetry(gray);
			tmp = detector.getResultRect();
			current = tmp;
			while(tmp != NULL)
			{
				Rect rect = Rect(tmp->topLeftX,tmp->topLeftY,tmp->width,tmp->height);
				cv::rectangle(sourceImage,rect,Scalar(0,0,0),1);
				tmp= tmp->next;
			}
			tracker.setLoackedPedArea(current);//����ǰ�õ��Ľ���洢��tracker�У����������µ�tracklet
			k = 0;
			isRequest = false;
		}
		imshow("sourceImage",sourceImage);

		//��tracker�н��и��٣���֪sourceImage���ݣ�
		//���ｫ����µ�sourceImage�����µ�tarcklet��ͬʱ�����е��������и���
		if(!tracker.update(sourceImage))
		{
			isRequest = true;//������ǰδ�ܹ���ʱ����tracklet����Ҫ���¼������
		}
		else
		{
			//�����µõ�tracklet���ں�����manager����������ټ������а�
			//git���Դ���
			//std::cout<<"github test"<<std::endl;
		}
		imshow("sourceImage",sourceImage);
		if(cv::waitKey(1) == 27)
			break;
		k++;
	}
	cv::waitKey(0);
	return 0;
}