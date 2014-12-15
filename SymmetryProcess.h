#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <time.h>

struct LockedArea
{
	int topLeftX;
	int topLeftY;
	int width;
	int height;
	LockedArea* next;
};
class SymmetryProcess
{
public:
	SymmetryProcess(void);
	SymmetryProcess(float ax,float ay,float u0,float v0,float f);//�ڲΣ�����Ҳ���Կ������ڲε�һ���֣��Ͼ��ǲ��ᷢ���仯��
	SymmetryProcess(float ax,float ay,float u0,float v0,float f,float theta,float high);
	SymmetryProcess(float ax,float ay,float u0,float v0,float f,float theta,float high,
				int Rx,int Ry,float aspectRatio,float minHigh, float maxHigh);

	~SymmetryProcess(void);

	void setResolutionRatio(int Rx,int Ry);//�趨�ֱ���
	void setExternalParam(float theta,float high);//�趨���
	void setAspectRatio(float aspectRatio);//�趨���˸߿��
	void setRealObjectHigh(float minHigh, float maxHigh);//�趨������С���߶�

	void initParam();//��������������Գ���������������м��㣬

	void loadImage(const char* filename);//���ش�����ͼ��
	void loadImage(cv::Mat& image);//����ͼ��

	void cannyProc();//��ͼ��õ���Ӧ�ı�Եͼ��
	void AddScanLines();//�Ա�Եͼ�����ɨ���ߣ�

	float computeWeight(int x,int center,int width);
	void showScanningWindows();//�����趨ɨ�贰�ڴ�С
	void computeSymmetryCurve();//��ɨ���߼���Գ�ֵ����
	void plotCurve();//���ݾ�����ƶԳ�ֵ����

	void eliminate();//����������Լ�������ĶԳ�ֵ
	void statisticNew();//�µ�ͳ�ƹ��̣�ͳ�Ƶ׶�λ�ã��������ۼ�ֵ
	void extractPeaks();//��ԶԳ�ֵ������ȡ��ֵ
	void lockPedestrianArea();//���ݶԳ�ֵ��ֵ��Ϣ�����������������������֮��symmetryProcess��������Ѿ�����ˣ�֮���
						//�Ĺ�����Ӧ������������������ɣ���֤��Ĺ��ܵ�רһ��
	LockedArea* getAreaInfo();

	void getArea();//�õ���ͬģ����Ե�ɨ�����򣬣��ɵ׶˱�Ե��ͳ����Ϣ��ã�����ı�Ե�����Ǿ����򵥹��˵õ��ģ�
	void plotArea();//��ԭʼͼ���н�ɨ������������������ʵ����������չʾ����
	void plotBox(cv::Mat &targetImage,int topLeftX,int topLeftY,int width,int height);//�������ұ߽缰�׶ˣ���ԭʼͼ���н��л���
	
	//void clusterPeaks();//�Է�ֵ���о���������õ���������
	//void estimateBoundBox();//���ݾ������Ĺ��Ʊ߽��
	//��Ϊ�����ɨ��������д���������Ҫɨ��������Ϣ���б�����
	void scanningAndVerify();//���ɨ���������ɨ�裬ȷ���Ƿ��ڲ������˴���
	void getTemplateMinMax(int linesNum,int &minTemplate,int &maxTemplate);//����linesNum��1~12��ȷ����ǰɨ������ѡ��ģ��ߴ緶Χ���߶ȷ�Χ
	int GetMin(int a, int b, int c, int d, int e);
	void productModel(const char* filename);//������Ե��Ӧֵƥ��ģ��


	/*Ŀǰ������ɵĹ����������Ե�ǰ�����������ͳһ����
	�Գ�����иĽ�
	1����ɨ���ߵ����ұ߽������չ����img�߽�λ�ã�
	2�����ƶԳ�ֵ����С��ֵ�����������������������
	3����������ĺϲ�����չ������κ�������
	4�����ݴ�����ɨ��������ȷ����ɨ����������½緶Χ��Ҳ����ȷ�����յĴ�ɨ������
	5����ȷ���յõ��Ĵ�ɨ��������Ϊ�������ļ��ϣ����Բ�ͬλ�õ������ȡ��Ӧ����չ/�����������ٽ��ɷ��������з���
	�����������Ƶõ�ԭʼͼ���е���������
	�����кܶ๤��Ҫ��ɵģ�����ʹ�ò�����֪�����ȡͼ�񣬲�����������
	�����������ʱ��������������������Լ����Լ�ץ���ˣ���
	˼·�ǽ�����ժȡ����Ȼ�����resize����⣬�ڻ�ԭ��ԭʼͼ���жԵ�λ�ã��������У��᲻�����Ӽ����ʱ�临�Ӷ�
	*/
private:
	//�������������,�ڶ���������б궨֮�󣬿���ֱ������
	float ax;
	float ay;
	float u0;
	float v0;
	float f;//����

	float theta;//�����������
	float high;//������������߶�
	int Rx,Ry;//��ǰ������ֱ���

	float aspectRatio;//ͳ�Ƶõ������˿�߱�
	float maxRealHobj,minRealHobj;//ʵ�����˸߶�ȡֵ��Χ[1.5,1.9]

	//�ɸ�����������õ��ı�������,���Ǵ���˼·�������ĵط�
	//ɨ�蹤�����ڳߴ�仯��ģ�240*320��ͼ����չ����
	float Wcoff,Hcoff;//�ߴ�����仯ϵ��
	float aspectRatioNew;//��320*240ͼ�������˸߿�ȣ�
	int groundLine;//��ƽ����ʧλ�ã�ԭʼͼ����
	int startPos;//ɨ������㣬ɨ��ͼ����
	int endPos;//ɨ�����յ�,ɨ��ͼ����

	int highLayer[5][2];//�ֲ��¼startPos��endPos����Ӧ�������С�߶ȣ�ÿ��Ļ���������ʵ�����[1.5,1.9]���
						//ÿ������������ݣ���ǰ�����С�߶ȣ����߶ȣ�������ֵ
	//��Щ���ݿ�����΢����һЩ�������ܲ�û����ȫ���ã�ֻ��˵������˱�Ե�Գ��Լ�ⲿ�֣�
	//����ķ�������Ŀ�����ȷ�ϵĹ�����û��չ����
	int interval;//��ʾɨ���ߵļ��
	

	cv::Mat sourceImage;//����ԭʼͼ�� 
	cv::Mat destImage;//��ԭʼͼ��ı�Ϊ320*240��Сɨ��ͼ��
	cv::Mat edgeImage;//����ɨ��ͼ��õ���Ե��Ϣͼ��
	cv::Mat responseModel;//ɨ�贰�ڱ�Ե��Ӧֵƥ��ģ��
	
	int modelWidth;
	int modelHeight;

	static const int linesNum = 12;//ɨ���߸���,ȷ��Ϊ12
	static const int scanningWidth = 320;//ɨ���߿��ȷ��Ϊ320
	int bottomInfo[scanningWidth][2];//����320�жԳ�ֵ��Ϣ����һ�����ݱ�ʾ��ǰ�е��±�Եλ��[1,12]���ڶ������ݱ�ʾ��ǰ�еĶԳ�ֵ�ۼӺ�
	int peakInfo[linesNum * 2];//��¼�Գ�ֵ�ۼ�ֵ ��ֵ����λ�ã�
	int symmetryCurve[linesNum][scanningWidth + 1];//���һ��Ԫ�أ���ŵ�ǰ�������ֵ


	LockedArea* lockedPedArea;//���ڱ�����ȡ������ԭʼͼ�������˾��ο���Ϣ,������ʽ����Ϊ��֪��������ȡ������Ŀ

	struct AreaInfo
	{
		int linesNum;
		int startPos;
		int endPos;
		AreaInfo* next;
	};
	AreaInfo scanningArea[linesNum];//�洢ÿ��ɨ���������п���ɨ���������Ϣ

};


