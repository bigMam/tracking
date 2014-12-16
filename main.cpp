#include <iostream>
#include "mySVM.h"
#include "SymmetryProcess.h"
#include "myTracker.h"
using namespace cv;
int main()
{
	const char* filename =  "D:\\ImageDataSets\\TestSamples\\image1202.jpg";
	const char* videoname = "D:\\ImageDataSets\\trackingSamples\\MVI_2702_100.avi";
	SVMDetector detector;
	detector.loadDetectorVector("mydetectorNew.xml");
	detector.initSymmetryParam(527,531,310,248,530,1.05);

	Tracker tracker = Tracker();//distinguish����tracker����ɵģ�

	VideoCapture cap(videoname);
	if(!cap.isOpened())
		return -1;
	cv::Mat sourceImage;
	cv::Mat gray;
	int interval = 10;//detector�����ü��
	int k = 0;//ͳ�Ƶ��ü��
	bool isRequest = true;//����������
	LockedArea* current,*tmp;//��¼��ǰ�Ѿ����õ�������

	while(cap.read(sourceImage))
	{
		//�����ù��̣�������˼�����
		if( k > interval || isRequest )//����������ã����ڵ��ã���Ӧ����
		{
			cv::cvtColor(sourceImage,gray,CV_BGR2GRAY);
			//cv::imshow("frame",sourceImage);
			detector.detectBaseOnSymmetry(gray);
			tmp = detector.getResultRect();//������˼������
			current = tmp;
			//�Լ�����ݽ��л��ƣ�ǰ�����ܹ����õ�����
			while(tmp != NULL)
			{
				Rect rect = Rect(tmp->topLeftX,tmp->topLeftY,tmp->width,tmp->height);
				cv::rectangle(sourceImage,rect,Scalar(0,0,0),1);
				tmp= tmp->next;
			}
			//����ֱ�ӽ�ָ����д��ݣ������Ƿ���У������Ļ���tracker��ʼ�������µļ�����������μ���ڼ�
			//�򱣴��ϴμ������������ʲô����
			tracker.setLoackedPedArea(current);//����ǰ�õ��Ľ���洢��tracker�У����������µ�tracklet
			k = 0;
			isRequest = tracker.update(sourceImage,true);//���и��£����ݸ��½���ж��Ƿ���Ҫ��һ���Ľ��м��
		}
		else
		{
			//����Ԥ����и��¹���
			isRequest = tracker.update(sourceImage,false);
		}

		imshow("sourceImage",sourceImage);
		if(!isRequest)//��ǰtracklet���³ɹ������Խ���tracklet�������
			//������³ɹ�����д���tracklet��
		{
			//�����µõ�tracklet���ں�����manager����������ټ������а�
			//git���Դ���
			//std::cout<<"github test"<<std::endl;
		}

		char key = cv::waitKey(3);
		if(key == 27)
			break;
		if(key == 32)
		{
			while(cv::waitKey(3) != 32);
		}
		
		k++;
	}
	cv::waitKey(0);
	return 0;
}