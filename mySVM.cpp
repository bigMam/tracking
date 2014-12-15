#include "mySVM.h"

using namespace std;
using namespace cv;



SVMDetector::SVMDetector()
{
	svm_classifier = MySVM();
	hog = HOGDescriptor(Size(64,128),Size(16,16),Size(8,8),Size(8,8),9);//��ʼ��HOG�������ӣ�����֮��ļ���
	detectorDim = 3781;
	//hog_cache.init(&hog);//������ɶ�blockData����pixData�ļ��㣬֮��Ĳ�����ֻ��Ҫ�򵥽��е��þͿ�����
	//֮�����ܹ���������������ԭ�����ڣ��Ѿ����������ͼ��ĳߴ�Ϊ64*128
}

void SVMDetector::loadImage(const char* filename)
{
	sourceImage = cv::imread(filename,0);//���ش�����ͼ��
}

void SVMDetector::loadImage(cv::Mat& image)
{
	image.copyTo(sourceImage);
}

int SVMDetector::computeDetectorVector()
{

	/*************************************************************************************************
	����SVMѵ����ɺ�õ���XML�ļ����棬��һ�����飬����support vector������һ�����飬����alpha,��һ��������������rho;
	��alpha����ͬsupport vector��ˣ�ע�⣬alpha*supportVector,���õ�һ����������֮���ٸ���������������һ��Ԫ��rho��
	��ˣ���õ���һ�������������ø÷�������ֱ���滻opencv�����˼��Ĭ�ϵ��Ǹ���������cv::HOGDescriptor::setSVMDetector()����
	�Ϳ����������ѵ������ѵ�������ķ������������˼���ˡ�
	***************************************************************************************************/
	if(svm_classifier.get_support_vector_count() == 0)
	{
		std::cout<<"δ����svm������"<<std::endl;
		return 0;
	}
	int DescriptorDim = svm_classifier.get_var_count();//����������ά������HOG�����ӵ�ά��
	int supportVectorNum = svm_classifier.get_support_vector_count();//֧�������ĸ���
	cout<<"֧������������"<<supportVectorNum<<endl;

	Mat alphaMat = Mat::zeros(1, supportVectorNum, CV_32FC1);//alpha���������ȵ���֧����������
	Mat supportVectorMat = Mat::zeros(supportVectorNum, DescriptorDim, CV_32FC1);//֧����������
	Mat resultMat = Mat::zeros(1, DescriptorDim, CV_32FC1);//alpha��������֧����������Ľ��

	//��֧�����������ݸ��Ƶ�supportVectorMat������
	for(int i=0; i<supportVectorNum; i++)
	{
		const float * pSVData = svm_classifier.get_support_vector(i);//���ص�i��֧������������ָ��
		for(int j=0; j<DescriptorDim; j++)
		{
			//cout<<pData[j]<<" ";
			supportVectorMat.at<float>(i,j) = pSVData[j];
		}
	}

	//��alpha���������ݸ��Ƶ�alphaMat��
	double * pAlphaData = svm_classifier.get_alpha_vector();//����SVM�ľ��ߺ����е�alpha����
	for(int i=0; i<supportVectorNum; i++)
	{
		alphaMat.at<float>(0,i) = pAlphaData[i];
	}

	//����-(alphaMat * supportVectorMat),����ŵ�resultMat��
	//gemm(alphaMat, supportVectorMat, -1, 0, 1, resultMat);//��֪��Ϊʲô�Ӹ��ţ�
	resultMat = -1 * alphaMat * supportVectorMat;

	//�õ����յ�setSVMDetector(const vector<float>& detector)�����п��õļ����

	//��resultMat�е����ݸ��Ƶ�����myDetector��
	for(int i=0; i<DescriptorDim; i++)
	{
		myDetector.push_back(resultMat.at<float>(0,i));
	}
	//������ƫ����rho���õ������
	myDetector.push_back(svm_classifier.get_rho());
	cout<<"�����ά����"<<myDetector.size()<<endl;//�õ���������detectorVector��֮����Ǹ���
	detectorDim = myDetector.size();

	//classifier�����ͣ�չ����Ӧ�ļ������
	//����HOGDescriptor�ļ����
}
void SVMDetector::saveDetectorVector(const char* filename)
{
	FileStorage fs(filename, FileStorage::WRITE);
	cv::Mat mDetector = cv::Mat(myDetector);
	fs<<"detector"<<mDetector;
	fs.release();
}
void SVMDetector::loadDetectorVector(const char* filename)
{
	myDetector.clear();
	FileStorage fs(filename, FileStorage::READ);
	cv::Mat mDetector;
	fs["detector"] >> mDetector;
	float *ptr = mDetector.ptr<float>(0);
	for(int i = 0 ; i < detectorDim; i++)
	{
		myDetector.push_back(ptr[i]);
	}
	fs.release();
}

//��������ͼ�����������ֵ�����������ͼ��ͳһΪ64*128��ʽ��
//����ʱʹ��hog�ṩ�������������㷽�����м��㣬����
void SVMDetector::computeDescriptor(cv::Mat& tmpImage,std::vector<float>& descriptors)
{
	hog.compute(tmpImage,descriptors);//ֱ������hog�Դ��������м�⣬����blockData��pixData���ظ�����
}
//�����Ѿ����صķ�������samples���з���Ԥ��
//�ȼ���ʹ�����Ժ˺������д���������У��ٽ�һ��������չ
//Ŀǰ�����Ǽ򵥴ֱ���һЩ�����ǲ�Ӱ�칦�ܵ�ʵ�ְ�,���ﳢ��ʹ��poly��������Ԥ�⣬��һ��Ч�����
bool SVMDetector::predict(std::vector<float>& samples,int var_count)
{
	double s = 0;
	int k;
	for(k = 0; k <= var_count - 4; k += 4 )
		s += samples[k]*myDetector[k] + samples[k+1]*myDetector[k+1] +
		samples[k+2]*myDetector[k+2] + samples[k+3]*myDetector[k+3];
	for( ; k < var_count; k++ )
		s += samples[k]*myDetector[k];

	s = s + myDetector[k];
	if(s > 0)
		return true;
	else 
		return false;
}

void SVMDetector::initSymmetryParam()
{
	//SymmetryProcess(float ax,float ay,float u0,float v0,float f,float theta,float high,
	//	         int Rx,int Ry,float aspectRatio,float minHigh, float maxHigh);
	//sp = SymmetryProcess(950,954,528,363,948,0,1.4,1024,768,0.4,1.5,1.9);//����Ĳ�����ʱ������������
											//ʵ��Ӧ���п�����Ҫ���ò�������������Ĳ��������ǹ̶������
	sp = SymmetryProcess(527,531,310,248,530,0,1.2,640,480,0.4,1.5,1.9);
	sp.initParam();
}
void SVMDetector::initSymmetryParam(float ax,float ay,float u0,float v0,float f,float h)
{
	sp = SymmetryProcess(ax,ay,u0,v0,f,0.0,h,640,480,0.4,1.5,1.9);
	sp.initParam();
}

void SVMDetector::detectBaseOnSymmetry(cv::Mat &sourceImage)
{
	clock_t start,end;
	start = clock();
	sp.loadImage(sourceImage);//���ش�����ͼ��,��Ҫ�����жϣ�������ǻҶ�ͼ����Ҫ����ת��

	sp.cannyProc();//��ȡ��Ե��Ϣ
	//sp.AddScanLines();//��ָ�������趨ɨ���ߣ��Ǳ�Ҫ
	sp.computeSymmetryCurve();//�ص㣬����ɨ�����ϸ����ضԳ�ֵ
	//sp.plotCurve();  //���ÿ��ɨ���߻��ƶ�Ӧ�ĶԳ�ֵ����ͼ���Ǳ�Ҫ
	sp.eliminate();  //�������������յĺ�ѡ����ȷ����û�����Ե�Ӱ�죬

	sp.statisticNew();//ͳ��ɨ���ߵ׶�λ�ã��������ۼ�ֵ������ȷ����ѡ���� bottomInfo
	sp.extractPeaks();//����ʹ�÷Ǽ���ֵ�����㷨�õ������ۼ�ֵ�ķ�ֵ��Ϣ peakInfo
	sp.lockPedestrianArea();//�õ�Ԥ��������������������֮�����//������Ҫ���иĽ�
							//��ԭʼͼ���ж�Ӧ�������resize������������������ƥ��
	end = clock();
	std::cout<<"���ڶԳ��Եĺ�ѡ����ȷ������ʱ��"<<end - start << std::endl;
	lockedPedArea = sp.getAreaInfo();//����ڱ�Ե�Գ��Եĺ�ѡ�����ȷ�����ü����̻��ܷ�����Ż����д���һ����ȷ��
	//���ԭʼͼ���к�ѡ����֮��Ĳ����ǶԺ�ѡ�������������֤
	LockedArea* post = lockedPedArea;
	LockedArea* current = lockedPedArea->next;

	cv::Mat tmpImage;
	std::vector<float> descriptor;
	start = clock();

	//�����˼·����С�������ν��м�⣬������ɹ������ϵ�к���������Ҫ������⣬��һ��ɾ��֮

	while(current != NULL)
	{
		cv::Mat pedROI = cv::Mat(sourceImage,cv::Rect(current->topLeftX,current->topLeftY,current->width,current->height));
		cv::resize(pedROI,tmpImage,Size(64,128),0,0,INTER_AREA);//��ͼ�����resize
		computeDescriptor(tmpImage,descriptor);
		if(!predict(descriptor,3780))
		{
			//ɾ���þ���
			LockedArea* tmp = current;
			post->next = current->next;
			current = current->next;
			delete tmp;
		}else{
			post = current;
			current = current->next;
		}
	}
	end = clock();
	std::cout<<"��ѡ������֤��ʱ��"<<end - start<<std::endl;
}
LockedArea* SVMDetector::getResultRect()
{
	return lockedPedArea->next;
}

//�ڽ�����Ƶ���֮ǰ�Ѿ���ɵ����������svmDetector�����룬�Գ��Լ��ĳ�ʼ�������������Ҫ����
int SVMDetector::detectOnVideo(const char* filename)
{
	initSymmetryParam(527,531,310,248,530,1.2);
	VideoCapture cap(filename);
	if(!cap.isOpened())
		return -1;
	namedWindow("frame",1);
	cv::Mat gray;
	while(cap.read(sourceImage))
	{
		cv::cvtColor(sourceImage,gray,CV_BGR2GRAY);
		cv::imshow("frame",sourceImage);
		detectBaseOnSymmetry(gray);

		LockedArea* current = lockedPedArea->next;
		while(current != NULL)
		{
			Rect rect = Rect(current->topLeftX,current->topLeftY,current->width,current->height);
			cv::rectangle(sourceImage,rect,Scalar(0,0,0),1);
			current= current->next;
		}
		imshow("sourceImage",sourceImage);
		if(cv::waitKey(1) == 27)
			break;
	}
	cap.release();
	return 0;
}
