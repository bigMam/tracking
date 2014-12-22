#include "tracker.h"
//�����õ�pedArea�洢��tracker�У����ﲻ��ֱ��ʹ�����ðɣ�
//������ʹ��ͬһ�ڴ�ռ䣬������ĸı䣬���������Ӱ�졣��ò��û�����⣬�ȳ��������ٽ����޸İ�

#define HAVE_BORDER 1
//ʹ�ñ߽�Ԥ�⣬���Ƕ��䲻���и�ֵ���Խ�����Ԥ��Ч������������ʱ��׷����ԭ���Ƚ�����������ͨ����ܴ����
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
	lockedPedArea = NULL;//����ͷ�ڵ�
	targetTrackerlet = NULL;//��ǰָ���
	//Ȩ�س�ʼ������
	for(int i = 0; i < 8; ++i)
	{
		weights[i] = 1.0 / 8;
	}
	front = 0;//��ʼ���б�����Ϊ��
	rear = 0;
	for(int i = 0; i < capacity; i++)
	{
		distratorList[i] = NULL;
	}
	letNumber = 0;//�Եõ�trackerlet���б�ţ���ſ��Լ��Լ�

	discriminator = Discriminator();//����Ĭ�Ϲ��캯��
}

//�Բ������ݽ���ɾ����������Ҫ��distrator�б�����
Tracker::~Tracker()
{
	int traversal = rear;
	//����б�
	while(traversal != front)
	{
		delete distratorList[traversal];
		traversal = (traversal + 1) % capacity;
	}
	delete targetTrackerlet;
}


void Tracker::setLoackedPedArea(LockedArea* result)
{
	lockedPedArea = result;//ֱ��ָ��ͬһ�ڴ�ռ�ͺ��ˣ���ʱ�����������������ܻ�Ҫ���иı�
}

bool Tracker::update(cv::Mat &sourceImage)
{
	
	if(targetTrackerlet == 0)//��ǰtargetTrackerletΪ��,���������Ŀ��ȷ���׶ε���
	{
		if(lockedPedArea->next != NULL)//������Ҫ�ٶ�����Ƶ�ĳ�ʼ�׶����ҽ���Ŀ�����˳��ֲ��ɼ�⣬��һ�޶�����
		{
			Trackerlet* trackerlet = new Trackerlet();
			extractTracklet(sourceImage,lockedPedArea->next,trackerlet);

			trackerlet->occupied++;
			targetTrackerlet = trackerlet;//������Ϊ֮ǰû�д���traget���������ֱ�Ӹ�ֵ��û������

			//��ԭͼ�Ͻ��л��ƣ���ɫ��ʾ����ֵ
			circle(sourceImage,cv::Point(trackerlet->topLeftX,trackerlet->topLeftY),5,CV_RGB(255,0,0),3);
			cv::rectangle(sourceImage,
					cv::Rect(trackerlet->topLeftX,trackerlet->topLeftY,targetTrackerlet->width,targetTrackerlet->height),
					cv::Scalar(255,0,0),2);
			//���ݵ�ǰ���ֵ��kalman���������������predict������ӣ��������Ԥ������û�в����ĵģ���Ϊ���ڼ��ֵ
			Mat prediction = KF.predict();
			measurement.at<float>(0) = (float)trackerlet->topLeftX;
			measurement.at<float>(1) = (float)trackerlet->topLeftY;
			KF.correct(measurement);//���õ�ǰ����ֵ�����˲�����������
			//����ֱ���϶����õ�������ȷ����Ȼ�Ǵ�������ģ�ֻ��������Ϊ�������޷��Ը���Ŀ�����ָ��

			//����ɾ������
			LockedArea *head = lockedPedArea;
			LockedArea *current = lockedPedArea->next;
			while(current != NULL) 
			{
				LockedArea* tmp = current;
				head->next = current->next;
				delete tmp;
				current = head->next;
			}
			return false;
		}
		else
		{
			return true;//��ʾ��ǰ�����ڼ����ο���Ҫ��һ���ļ����̣�
		}
	}
	else//��ǰtargetTrackerlet���ڣ����Խ��бȽϡ�Ԥ�����
	{
		Trackerlet* newTargetTrackerlet = NULL;//���ڴ洢�µõ�tracklet�������Ǽ��õ�����Ԥ��õ�

		//���ȶ�lockedPedArea���б����жϣ����������һ���Ƿ���targetTrackerlet���������û�з��֣�����ϻ�������
		LockedArea* current = lockedPedArea->next;//�����ж�

		if(current != NULL)//����⵽����ʱ
		{
			while(current != NULL)
			{
				Trackerlet* trackerlet = new Trackerlet();
				extractTracklet(sourceImage,current,trackerlet);
				//�϶�ΪĿ�����ˣ���ֵ�����۲���ٽ��е��ڣ�������Ҫ����λ����Ϣ

				if(isTargetTrackerlet(trackerlet))
				{
					//����
					circle(sourceImage,cv::Point(trackerlet->topLeftX,trackerlet->topLeftY),5,CV_RGB(255,0,0),3);

					//ֻ��ʹ�ò���ֵ���˲����������������ܹ�ʹ��Ԥ��ֵ���������������ǲ��Ե�
					measurement.at<float>(0) = (float)trackerlet->topLeftX;
					measurement.at<float>(1) = (float)trackerlet->topLeftY;
					KF.correct(measurement);

					//��ʱ�洢
					newTargetTrackerlet = trackerlet;//�����ǽ���ǰ����trackerlet������������û�н����滻targetTrackerlet����
					//move2next
					current = current->next;
					break;
				}
				else//����distrator�б�,ͬʱ������뵽targetTarcker�к�
				{
					insertList(trackerlet);
					insertDistrator(trackerlet);
					//���������ˣ����ｫ���ݴ洢��distrator�У�ͬʱ��ŵ�trakerletList����һ�������ǣ���һ�߽��������⣬
					//���������Ҫ��˫������Լ�����ǽ���ɾ�������ǽ��м��Ƴ�����Ҫ�����ж���
					current = current->next;
				}
			}
			//��ʣ��tracklet����distrator
			while(current != NULL)
			{
				Trackerlet* trackerlet = new Trackerlet();
				extractTracklet(sourceImage,current,trackerlet);

				insertList(trackerlet);
				insertDistrator(trackerlet);
				current = current->next;
			}
			//ʹ�ý���֮�����lockedPedArea����,����ɾ��
			LockedArea *head = lockedPedArea;
			current = lockedPedArea->next;
			while(current != NULL)
			{
				LockedArea* tmp = current;
				head->next = current->next;
				delete tmp;
				current = head->next;
			}
		}
		
		if(newTargetTrackerlet == NULL)//֮ǰ����lockedPedArea����û�еõ����õ�trackerlet����Ҫ����kalman�����˲�Ԥ��
		{
			Mat prediction = KF.predict();//�����˲����Ե�ǰ���tracklet���ν���Ԥ��
			float *data = prediction.ptr<float>(0);
			int predictX = data[0];
			int predictY = data[1];
			cv::Mat subImage = sourceImage(cv::Rect(predictX,predictY,targetTrackerlet->width,targetTrackerlet->height));
			blockFeature target;
			extractor.computeFeature(subImage,target);
			//����ǰ�õ�blockfeature��֮ǰ�洢���ݽ��бȽ�
			double distinguishValue = this->distinguish(targetTrackerlet->featureSet,target);
			std::cout<<"Ԥ��trackerlet����ֵΪ��"<<distinguishValue<<std::endl;
			if(distinguishValue > 1.0)//��ǰԤ�����������������ƶ�Ҫ�󣬷����������
				return true;
			else
			{
				//��Ԥ������������
				Trackerlet* trackerlet = new Trackerlet();
				letNumber++;
				trackerlet->trackerletID = letNumber;//���ID��ʱû��ʲô���壬����ʱ��������
				trackerlet->topLeftX = predictX;
				trackerlet->topLeftY = predictY;
				trackerlet->width = targetTrackerlet->width;
				trackerlet->height = targetTrackerlet->height;
				trackerlet->setBlockFeature(target);
				trackerlet->next = NULL;
				trackerlet->occupied = 0;

				newTargetTrackerlet = trackerlet;//��ʱ�洢�½��õ�targetTrackerlet
				//��ԭͼ�Ͻ��л��ƣ���ɫ��ʾԤ��ֵ
				circle(sourceImage,cv::Point(predictX,predictY),5,CV_RGB(0,255,0),3);
				cv::rectangle(sourceImage,
					cv::Rect(predictX,predictY,targetTrackerlet->width,targetTrackerlet->height),
					cv::Scalar(255,0,0),2);

				//������Ҫ��ϸ��һ�£�����������ô���᲻��ָ��ͬһλ�õ����ݱ��ı��أ������
			}
		}
		//���ｫtargetTrackerlet����ʹ��newTargetTrackerlet���������ͬʱ��֤ԭʼ�б����ݲ���
		newTargetTrackerlet->next = targetTrackerlet->next;

		targetTrackerlet->next = NULL;
		Trackerlet* tmp = targetTrackerlet;

		tmp->occupied--;
		if(tmp->occupied == 0)
		{
			delete tmp;//�������ֱ�ӽ���ɾ������ô����û�п��ܱ�����ռ���أ�����ֱ��ɾ��
			letNumber--;
		}

		newTargetTrackerlet->occupied++;
		targetTrackerlet = newTargetTrackerlet;

		//���ڵ������ˣ������������ݽ���Ȩ�ص���
		featureWeighting(targetTrackerlet->featureSet);

		//�ض������ϸ�����������뷨չ�������������㷨��ʱ��Ҫ���㹻��³���Բſ���

		//ÿ���׶ζ�targetTrackerlet����һ�θ��£���������ݻ������ر���ȷ���Ƿ���ÿ���׶ε�ĩβ��list��������أ�
		//��Ҫ������ղ���ô����Ҫ����ô��targettrackerlet��Ҫ�������д����أ���������֮����Ҫ������һ�׶ε����ƶ��ж�
		return false;//��ʾ����Ҫ���м�⣬���Լ���������һ��ѭ��
	}
}

//deprecated
bool Tracker::update(cv::Mat &sourceImage,bool haveRectBoxing)
{
	//��ʾ��ǰ�����ʳ�¯�����˼����ο򣬲���Ҫ����Ԥ����̣��д���ȶ
	if(haveRectBoxing && lockedPedArea != NULL)
	{

		Trackerlet* trackerlet = new Trackerlet();
		extractTracklet(sourceImage,lockedPedArea,trackerlet);
		circle(sourceImage,cv::Point(trackerlet->topLeftX,trackerlet->topLeftY),5,CV_RGB(255,0,0),3);

		//���ݵ�ǰ���ֵ��kalman���������������predict������ӣ��������Ԥ������û�в����ĵģ���Ϊ���ڼ��ֵ
		Mat prediction = KF.predict();

		measurement.at<float>(0) = (float)trackerlet->topLeftX;
		measurement.at<float>(1) = (float)trackerlet->topLeftY;

		KF.correct(measurement);//���õ�ǰ����ֵ�����˲�����������

		targetTrackerlet = trackerlet;//����ֱ���滻Ҳ�ǲ���ȷ�ģ��滻��Ҫ��Ȩ�ؼ������֮��Ž���
		//����ֱ���϶����õ�������ȷ����Ȼ�Ǵ�������ģ�
		return false;

	}
	else
	{
		//��ǰtrackerletHead�ǿ�,���Գ��Խ��бȽϣ�������ν���Ԥ���أ�
		//����и��ٹ���
		Mat prediction = KF.predict();//�����˲����Ե�ǰ���tracklet���ν���Ԥ��
		float *data = prediction.ptr<float>(0);
		int predictX = data[0];
		int predictY = data[1];
		std::cout<<predictX<<" "<<predictY<<" "<<std::endl;

		if (targetTrackerlet == NULL)
			return true;
		else
		{
			//���ｫ���ݵ�ǰ�õ�tracklet��֮ǰtracklet����Ԥ��ƥ�䣬������ƶȿ�������������ɾ�����������������
			//������൫Ҫ˼·����
			int letWidth = targetTrackerlet->width;
			int letHeight = targetTrackerlet->height;
			cv::Mat subImage = sourceImage(cv::Rect(predictX,predictY,letWidth,letHeight));
			blockFeature target;
			extractor.computeFeature(subImage,target);

			//����ǰ�õ�blockfeature��֮ǰ�洢���ݽ��бȽ�
			double distinguish = this->distinguish(targetTrackerlet->featureSet,target);
			std::cout<<"����ֵΪ��"<<distinguish<<std::endl;
			if(distinguish > 0.35)
				return true;
			else
			{
				circle(sourceImage,cv::Point(predictX,predictY),5,CV_RGB(0,255,0),3);
				cv::rectangle(sourceImage,cv::Rect(predictX,predictY,letWidth,letHeight),cv::Scalar(255,0,0),2);
				return false;
			}
		}
	}
}

//�������һ���������ֵ����ݣ�������ȡ��Ϊһ����������ʵ��
//���ݾ��ο���ȡtacklet
void Tracker::extractTracklet(cv::Mat &sourceImage,LockedArea* lockedPedArea,Trackerlet* trackerlet)
{
	int topLeftX = lockedPedArea->topLeftX;
    int topLeftY = lockedPedArea->topLeftY;
	int width = lockedPedArea->width;
	int height = lockedPedArea->height;
	cv::Rect rect(topLeftX,topLeftY,width,height);

	int letWidth = rect.width * 0.4;
	int letHeight = rect.height * 0.18;
	int letTopLeftX = rect.x + rect.width * 0.3;
	int letTopLeftY = rect.y + rect.height * 0.25;
	cv::Mat subImage = sourceImage(cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight));

	blockFeature target;
	extractor.computeFeature(subImage,target);

	letNumber++;
	trackerlet->trackerletID = letNumber;//���ID��ʱû��ʲô���壬����ʱ��������
	trackerlet->topLeftX = letTopLeftX;
	trackerlet->topLeftY = letTopLeftY;
	trackerlet->width = letWidth;
	trackerlet->height = letHeight;
	trackerlet->setBlockFeature(target);
	trackerlet->next = NULL;

	trackerlet->occupied = 0;

	//�Ծ��ο���б궨
	cv::rectangle(sourceImage,cv::Rect(letTopLeftX,letTopLeftY,letWidth,letHeight),cv::Scalar(255,0,0),2);
}

bool Tracker::isTargetTrackerlet(Trackerlet* current)
{
	if(targetTrackerlet == NULL)
		return false;
	//����λ����Ϣ��feature�����жϹ���
	//����λ����Ϣ��λ��������Խ���ֵ�ſ�λ�ò������Ŀ�꣬����Ҫ�нϸߵ����ƶȣ����϶�ΪͬһĿ��
	//λ�����ʱ�趨Ϊ1.0��λ�ò��ϴ�ʱ�趨Ϊ0.5�����Կ���һ��
	double distinguishValue = this->distinguish(targetTrackerlet->featureSet,current->featureSet);
	std::cout<<"����ֵΪ��"<<distinguishValue<<std::endl;
	if(distinguishValue < 0.5)
		return true;
	else
	{
		//�������λ���жϵ�������ʲô���������϶�Ϊ��Ŀ��ӽ���
		//��ԭʼ�ķ���λ�ò�ֵС�ڸ���ֵ
		int diffX = std::abs(targetTrackerlet->topLeftX  - current->topLeftX);
		int diffY = std::abs(targetTrackerlet->topLeftY - current->topLeftY);
		if(diffX < 30 && diffY < 30)//��Ϊ���߽�Ϊ�ӽ�
			if(distinguishValue < 1.0)
				return true;
			else
				return false;
		else
			if(distinguishValue < 0.5)
				return true;
			else
				return false;
	}
}

//��tracket���뵽distrator�б��У�����֤�������������ޣ��ڳ���ʱ���ܹ�ɾ��last one
void Tracker::insertDistrator(Trackerlet* trackerlet)
{
	//����ʹ�ö��е���ʽ��FIFO�����϶��в���һ�����ԣ�
	//���������棬���������������Ԫ�ؽ�ָ��Ԫ�ؽ����滻���ڶ��������������£�Ҳ���ܹ����ٲ����


	//ע��front ָ���ͷ��һ��Ԫ�أ�rearָ���βԪ��
	trackerlet->occupied++;
	if((front + 1)%capacity == rear)//����
	{	
		//����֮ǰ��Ҫ����βԪ�س���	
		Trackerlet* tmp = distratorList[rear];
		
		tmp->occupied--;
		if(tmp->occupied == 0)
		{
			delete tmp;//�б�Ҫ��Ԫ��ɾ��
			letNumber--;
		}
		rear = (rear + 1)%capacity;

		distratorList[front] = trackerlet;
		front = (front + 1)%capacity;
		
		//����ͱ�����Ҫ����һЩ����Ϊ�����Ǵ洢��ָ����Ϣ�����Է���Ľ��и�ֵ����
	}
	else//��δ������ֱ�ӽ��в������
	{
		distratorList[front] = trackerlet;
		front = (front + 1)%capacity;
	}
}

//���㵱ǰ������Ŀ������֮��Ĳ�ֵ
double Tracker::distinguish(blockFeature& target, blockFeature& current)
{
	discriminator.setCurrentFeature(current);
	discriminator.computeDistance(target);
	double dissimilarity = discriminator.distinguish();

	return dissimilarity;
}
//���ݵ�ǰcurrent����ȷĿ�꣩��preTarget����ǰ�洢�ģ���distrator�����������ݣ�
void Tracker::featureWeighting(blockFeature& current)
{
	//���������и����Ĺ�ʽ�ֱ����������Ͼ���
	discriminator.clearDistance();
	discriminator.setCurrentFeature(current);

	double distance[8];
	double meanDistance[8];
	for(int i = 0; i < 8; i++)
	{
		distance[i] = 0;
		meanDistance[i] = 0;
	}
	int traversal = rear;
	while(traversal != front)
	{   
		blockFeature distratorFeature = distratorList[traversal]->featureSet;
		discriminator.computeDistanceHold(distratorFeature);
		traversal = (traversal + 1) % capacity;
	}
	if(rear != front)//��ʾ��ǰdistrator�б�ǿ�
	{
		discriminator.getDistance(meanDistance);
		int count = rear < front ? front - rear : front - rear + capacity;
		for(int i = 0; i < 8; i++)
		{
			meanDistance[i] = meanDistance[i] / count;//�����������ƽ�����Ͼ���
		}
	}
	blockFeature targetFeature = targetTrackerlet->featureSet;
	discriminator.computeDistance(targetFeature);
	discriminator.getDistance(distance);//��ȡ�����Ŀ����Ͼ���
	if(meanDistance[0] != 0)
	{
		for(int i =0; i < 8; i++)
		{
			weights[i] = distance[i] != 0 ? weights[i] + (meanDistance[i] -  distance[i]) : weights[i] + meanDistance[i];
		}
	}
	double sum = 0;
	for(int i = 0; i < 8; ++i)
	{
		sum = sum + weights[i];
	}
	for(int i = 0; i < 8; ++i)
	{ 
		weights[i] = weights[i] / sum;
	}
	discriminator.setWeights(weights);
}

Trackerlet* Tracker::getTrackerlist()
{
	return targetTrackerlet;
}

//���÷��������targetTrackerlet��������
void Tracker::correctTarget(Trackerlet* correctTrackerlet)
{
	targetTrackerlet = correctTrackerlet;//����ֱ�ӽ��������Ǵ��ڷ��յģ�һ��֮ǰ����û��delete��
	//������ָ������ݺ��п��ܻ���֮���ĳ��ʱ��㱻������Ӷ�����targetTrackerletָ��NULL���������
}

//��Ԫ�ز��������������ʹ��STL����ӷ���һЩ����ʵ�֣�������ں��ڵ��Ż����̣�������ô
void Tracker::insertList(Trackerlet* trackerlet)
{
	trackerlet->occupied++;
	//�򵥵�ͷ�巨ʵ�ֲ������
	trackerlet->next = targetTrackerlet->next;
	targetTrackerlet->next = trackerlet;
}

void Tracker::clearList()//��targetTrackerlet�б������ղ������������׸��ڵ�
{
	if(targetTrackerlet == NULL)
		return;
	Trackerlet *curr,*tmp;
	curr = targetTrackerlet->next;
	while(curr != NULL)//���б������ղ���
	{
		tmp = curr;
		targetTrackerlet->next = curr->next;
		tmp->occupied--;
		if(tmp->occupied == 0)
		{
			delete tmp;
			tmp = NULL;
			letNumber--;
		}
		curr = targetTrackerlet->next;
	}
}