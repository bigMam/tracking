#include <iostream>
#include "mySVM.h"
#include "SymmetryProcess.h"
#include "myTracker.h"
using namespace cv;
int main()
{
	const char* filename =  "D:\\ImageDataSets\\TestSamples\\image1202.jpg";
	const char* videoname = "D:\\ImageDataSets\\trackingSamples\\MVI_2693_01.avi";
	SVMDetector detector;
	detector.loadDetectorVector("mydetectorNew.xml");
	detector.initSymmetryParam(527,531,310,248,530,0.72);

	Tracker tracker = Tracker();

	VideoCapture cap(videoname);
	if(!cap.isOpened())
		return -1;
	cv::Mat sourceImage;
	cv::Mat gray;
	int interval = 8;//detector�����ü��
	int k = 0;//ͳ�Ƶ��ü��
	bool isRequest = true;
	LockedArea* current,*tmp;//��¼��ǰ�Ѿ����õ�������

	while(cap.read(sourceImage))
	{
		//���ü����̣���Ҫ���� ���˼�����
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
			isRequest = tracker.update(sourceImage,true);
		}
		else
		{
			isRequest = tracker.update(sourceImage,false);
		}

		imshow("sourceImage",sourceImage);
		if(!isRequest)//��ǰtracklet���³ɹ������Խ���tracklet�������
		{
			//�����µõ�tracklet���ں�����manager����������ټ������а�
			//git���Դ���
			//std::cout<<"github test"<<std::endl;
		}
		if(cv::waitKey(1) == 27)
			break;
		k++;
	}
	cv::waitKey(0);
	return 0;
}