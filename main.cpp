#include <iostream>
#include "mySVM.h"
#include "SymmetryProcess.h"
#include "tracker.h"
#include "manager.h"
using namespace cv;

extern int videoCut(const char* sourceVideo,const char* targetVideo,int stratSec,int endSec);
int pedTracking(const char* videoname);
int main()
{
	const char* videoname = "D:\\ImageDataSets\\trackingSamples\\MVI_2708_75_2.avi";
	//const char* targetvideo = "D:\\ImageDataSets\\trackingSamples\\MVI_2722_target_2.avi";

	//videoCut(videoname,targetvideo,1,20);

	//const char* targetVideo = "D:\\ImageDataSets\\trackingSamples\\MVI_2708_75_2_target.avi";
	//int ex=static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)); 
	//char EXT[] = {ex & 0XFF , (ex & 0XFF00) >> 8,(ex & 0XFF0000) >> 16,(ex & 0XFF000000) >> 24, 0}; //������ʲô 
	//cv::Size S = cv::Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH),  
	//	(int)cap.get(CV_CAP_PROP_FRAME_HEIGHT) ); 
	//cv::VideoWriter cap_write;
	//cap_write.open(targetVideo,ex, cap.get(CV_CAP_PROP_FPS),S, true); //��д���ļ�����ָ����ʽ
	pedTracking(videoname);
}

int pedTracking(const char* videoname)
{
	cv::VideoCapture cap(videoname);
	if(!cap.isOpened())
		return -1;

	SVMDetector detector;
	detector.loadDetectorVector("mydetectorNew.xml");
	detector.initSymmetryParam(527,531,310,248,530,0.75);

	Tracker tracker = Tracker();//distinguish����tracker����ɵģ�

	Manager manager = Manager();

	cv::Mat sourceImage;
	cv::Mat gray;
	int interval = 5;//detector�����ü��
	int k = 0;//ͳ�Ƶ��ü��
	bool isRequest = true;//����������
	LockedArea* current,*tmp;//��¼��ǰ�Ѿ����õ�������
	Trackerlet* trackerletlist;//tracker��manager�ύtrackerlet�б�
	Trackerlet* correctTrackerlet;//manager��tracker�����������

	while(cap.read(sourceImage))
	{
		std::cout<<std::endl;
		std::cout<<"NEXT PERIOD:"<<std::endl;
		//�����ù��̣�������˼�����
		if( k > interval || isRequest )//����������ã����ڵ��ã���Ӧ����
		{
			cv::cvtColor(sourceImage,gray,CV_BGR2GRAY);
			//cv::imshow("frame",sourceImage);
			detector.detectBaseOnSymmetry(gray);
			current = detector.getResultRect();//������˼������
			tmp = current->next;
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
			isRequest = false;
		}
		//��tracker���ݽ��и���
		isRequest = tracker.update(sourceImage);

		imshow("sourceImage",sourceImage);
		//cap_write<<sourceImage;
		if(!isRequest)//��ǰtracklet���³ɹ������Խ���tracklet�������
		{
			trackerletlist = tracker.getTrackerlist();
			manager.setTrackerletList(trackerletlist);
			//֮��Ӧ���Ǹ��ݴ���trackerlet�����ж����̣��ж��ĸ�trackerlet����Ŀ��trackerlet������˵
			//��trackerȷ��������Ŀ��
			if(!manager.dicision())
			{
				correctTrackerlet = manager.correct();
				tracker.correctTarget(correctTrackerlet);
			}
		} 
		char key = cv::waitKey(3);
		if(key == 27)
			break;
		if(key == 32)
		{
			while(cv::waitKey(3) != 32);
		}
		k++;

		tracker.clearList();
	}
	cap.release();
	cv::waitKey(0);
	return 0;
}