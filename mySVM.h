#pragma once
//�����ô��룬���ֶ���ʵ�֣�������ԣ�֪��������ķѵ�ʱ��
#include "SymmetryProcess.h"
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>

#include <time.h>

class MySVM : public CvSVM
{
public:
	//���SVM�ľ��ߺ����е�alpha����
	double * get_alpha_vector()
	{
		return this->decision_func->alpha;
	}

	//���SVM�ľ��ߺ����е�rho����,��ƫ����
	float get_rho()
	{
		return this->decision_func->rho;
	}
};

class SVMDetector
{
public:
	SVMDetector();
	~SVMDetector(){};//��ɱ�Ҫ���������
	void loadImage(const char* filename);//���ش�����ͼ��
	void loadImage(cv::Mat& image);//��������һ�ּ���ͼ��ʽ
	void saveImage(const char* filename);//����Ŀ��ͼ��

	int computeDetectorVector();
	void saveDetectorVector(const char* filename);
	void loadDetectorVector(const char* filename);

	void computeDescriptor(cv::Mat& tmpImage,std::vector<float>& descriptor);
	bool predict(std::vector<float>& samples,int var_count);//���ø����ķ�������ָ��samples����Ԥ�⣬

	void initSymmetryParam();//�Ի��ڶԳ������˼��Ĳ������г�ʼ����
	void initSymmetryParam(float ax,float ay,float u0,float v0,float f,float h);
	void detectBaseOnSymmetry(cv::Mat& sourceImage);//��������Ϊ��ͨ���Ҷ�ͼ��

	int detectOnVideo(const char* filename);//����Ƶ�����м�⣬

	LockedArea* getResultRect();

private:
	
	SymmetryProcess sp;
	cv::Mat sourceImage;//������ͼ����Ϣ
	LockedArea* lockedPedArea;
	MySVM svm_classifier;//����svm���������ڻ������ڼ���ѡ������֤�����о�ʹ�ø÷�����
	cv::HOGDescriptor hog;

	std::vector<float> myDetector;//��������Ԥ���
	int detectorDim;//��¼detector��ά��

	cv::vector<cv::Rect> resultRect;
};
//����������ҷ�һ�ţ��ص���������ģ�