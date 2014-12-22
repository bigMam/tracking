#include "SymmetryProcess.h"
#include <iostream>
#include <math.h>
using namespace cv;
using namespace std;
SymmetryProcess::SymmetryProcess(void)
{
	//linesNum = 12;//[120~230]
}

SymmetryProcess::SymmetryProcess(float ax,float ay,float u0,float v0,float f)//�ڲΣ�����Ҳ���Կ������ڲε�һ���֣��Ͼ��ǲ��ᷢ���仯��
{
	this->ax = ax;
	this->ay = ay;
	this->u0 = u0;
	this->v0 = v0;
	this->f = f;
}
SymmetryProcess::SymmetryProcess(float ax,float ay,float u0,float v0,float f,float theta,float high)
{
	new (this)SymmetryProcess(ax,ay,u0,v0,f);
	setExternalParam(theta,high);
}
SymmetryProcess::SymmetryProcess(float ax,float ay,float u0,float v0,float f,float theta,float high,
	int Rx,int Ry,float aspectRatio,float minHigh,float maxHigh)
{
	new (this)SymmetryProcess(ax,ay,u0,v0,f,theta,high);
	setResolutionRatio(Rx,Ry);
	setAspectRatio(aspectRatio);
	setRealObjectHigh(minHigh,maxHigh);
}
void SymmetryProcess::setResolutionRatio(int Rx,int Ry)//�趨�ֱ���
{
	this->Rx = Rx;
	this->Ry = Ry;
}
void SymmetryProcess::setExternalParam(float theta,float high)//�趨���
{
	this->theta = theta;
	this->high = high;
}
void SymmetryProcess::setAspectRatio(float aspectRatio)//�趨���˸߿��
{
	this->aspectRatio = aspectRatio;
}

void SymmetryProcess::setRealObjectHigh(float minHigh,float maxHigh)
{
	this->minRealHobj = minHigh;
	this->maxRealHobj = maxHigh;
}

SymmetryProcess::~SymmetryProcess(void)
{
}




void SymmetryProcess::initParam()
{

	//////////��ʱ����������Ĵ���������֪���ܹ���������Ϳ����ˣ���Щ��������⻹�д���һ����ʵ�飬��ʱ����///////////
	Wcoff = 320.0 / Rx;
	Hcoff = 240.0 / Ry;
	aspectRatioNew = aspectRatio * (Wcoff / Hcoff);

	groundLine = v0 - ay * tanf(theta);
	int groundLineN = groundLine * Hcoff;
	startPos = groundLineN + 5;
	endPos = 235;
	interval = (endPos - startPos)/11;

	int startPosS = startPos / Hcoff;
	int endPosS = endPos / Hcoff;//��ɨ����λ�÷�����ԭʼimgͼ���е�λ��
	
	float startHigh,endHigh;
	float startZw = (ay * high) / (startPosS - groundLine);//�ɷ��Ƶõ���ɨ����λ��ȷ��ʵ�ʾ��룬���ɾ����Ƶ��߶�
	float endZw = (ay * high) / (endPosS - groundLine);
	float realHigh = 1.5;

	for(int i = 0; i < 5; i++)
	{
		startHigh = (f / startZw) * realHigh;
		endHigh = (f / endZw) * realHigh;
		highLayer[i][0] = startHigh * Hcoff;//��ɨ��ͼ����ɨ�贰�ڵ���С�߶ȣ�startPos
		highLayer[i][1] = endHigh * Hcoff;//ɨ��ͼ���е�ɨ�贰�ڵ����߶ȣ�endPos
		//highLayer[i][2] = (highLayer[i][1] - highLayer[i][0]) / (linesNum - 1);
		realHigh = realHigh + 0.1;
	}
	//��Щ�����ڳߴ�仯��ͼ���еĲ�����ԭ����Щ�����Ǿ���ԭʼimgͼ��������仯ϵ����ͬ������

	//aspectRatioNew = 0.3f;//�趨��߱�
	//maxHigh = 180;//��λΪ����,
	//minHigh = 60;
	//startPos = 120;
	//endPos = 230;
	//interval = (endPos - startPos)/11;//10

	lockedPedArea = new LockedArea(); 
	lockedPedArea->topLeftX = 0;
	lockedPedArea->topLeftY = 0;
	lockedPedArea->width = 0;
	lockedPedArea->height = 0;
	lockedPedArea->next = NULL;



}
void SymmetryProcess::loadImage(const char* filename)
{
	sourceImage = cv::imread(filename,0);
	int rows = sourceImage.rows;
	int cols = sourceImage.cols;
	cout<<"rows:"<<rows;
	cout<<"cols:"<<cols;
	cout<<"channels:"<<sourceImage.channels()<<endl;
	Wcoff = 320.0/cols;
	Hcoff = 240.0/rows;
	groundLine = rows / 2;//�����ȴ����д�ϸ������ʱ����
}

void SymmetryProcess::loadImage(cv::Mat& image)
{
	sourceImage = image;//ʹ��ͼ��ͷָ����ͬ�ڴ�ռ�
}

void SymmetryProcess::cannyProc()
{
	clock_t start,end;
	resize(sourceImage,destImage,Size(320,240));//>>>O_O<<<<����������⣬ֱ�ӽ�ԭʼͼ��ת��Ϊ320*240����ͼ����һ���̶�������α䣬
	
	//��Ҫ���ǽ��������߻�һ��˼·����ֱ��ǿ�иı䣬���ǰ��ձ�����������
	start = clock();
	GaussianBlur(destImage, destImage, Size(7,7), 1.5, 1.5);
	int low = 40;//֮ǰ�������Ž��Ϊ��40
	double high = low / 0.4;
	
	Canny(destImage, edgeImage, low, high, 3);//>>>O_O<<<<������һ�����¿���������ѡ�����ŵ���ֵ��ʹ��ǰ���뱳�������ܶ�ķ��뿪��
	
	end = clock();
	std::cout<<"��Ե��ȡ��ʱ��"<< end - start <<std::endl;
	//��֤���������ܵ�͹�Գ����������¿���
	//imshow("edge",edgeImage);
}
void SymmetryProcess::AddScanLines()
{
	//����Ĵ�������edgeImage�Ͻ��л��ߣ����ո���������ɨ������edgeImage��������
	//����������ʹ�ã��������ȥ����ֱ�ۣ�������˵����Ĳ���
	int count = 0;
	for(int i = startPos; count < linesNum;)
	{
		uchar *edgePtr = edgeImage.ptr<uchar>(i);
		for(int j = 0; j < edgeImage.cols; j++)//�ӻ��߿�ʼ��ɨ�������ҽ�����չ
		{
			if (edgePtr[j] != 255)
				edgePtr[j] = 180;
		}
		i = i + interval;
		count++;
	}
	imshow("ScanLine",edgeImage);
}
// ��ʾ��ͬɨ�������趨��ɨ�贰�ڷ�Χ
void SymmetryProcess::showScanningWindows()
{
	float increase = (highLayer[4][1] - highLayer[4][0]) / (linesNum - 1);//�õ����ڸ߶ȵ���ֵ
	cv::Mat showWindows = cv::Mat::zeros(240,320,CV_8UC1);
	//���м�λ�ý��л��ƣ�����Ч�����
	for(int i = 0; i <linesNum; i++)
	{
		int winHigh = highLayer[4][0] + i * increase;
		float winWidth = winHigh * aspectRatioNew;//����õ���ǰɨ�贰�ڴ�С
		int rowPos = startPos + i * interval;//�õ���ǰɨ����λ��
		plotBox(showWindows,110,rowPos-winHigh,winWidth,winHigh);
	}
	imshow("scanning Window",showWindows);
}

//��������x �������ĵľ��룬����Ȩ�أ�����ʹ�ø�˹�����ļ�ģʽ
//���Ը�Ϊƽ���Ĺ��ɡ���ǰ��û��ʹ�ø�˹���������Ƿֶκ��������ʹ���ģ�
//�û��Ƶ�Ч����ֻ��������һ���ˣ������и�Ϊ����ķ����������Ǳ����ϵķ�첹���������кܴ�����
//�������������ţ���ʼ������������н������ɨ�������������
float SymmetryProcess::computeWeight(int x,int center,int width)
{
	int distance = std::abs(x - center);
	float weight;

	//float delta = width / 6.0;
	//weight = 1 / delta * sqrt(2 * 3.14) * exp(- distance * distance / (2 * delta * delta));
	if(distance < width / 6.0)
	{
		weight = 1 - 0.5 * distance / (width / 2.0) * 3;
	}
	else if(distance < width / 3.0)
	{
		weight = 0.75 - 0.25 * distance / (width / 2.0) * 3;
	}
	else
	{
		weight = 0.5 - 0.125 * distance / ( width / 2.0) * 3;
	}
	return weight;
}

//>>>O_O<<<<�����ɨ����̣��϶�����Ҫ��һ���޸ģ�������������������ĺܶ�ط�����ǰ�������ǣ����������̹�һ��
//���ｫ����ɨ���߷ֱ����Գ�ֵ�����Ǳ�������ص㲿��
//���ڵ���Ҫ�����ǣ����ȷ��ÿ��ɨ�����ϵ�ɨ�贰�ڴ�С��
//����ɨ����ΪĿ�������λ�ã��ڸ�ǰ�������£�����ɨ�贰�ڵĴ�С���趨���˿�߱�Ϊ0.4�����˸߶�Ϊ1.5~1.8�������������ô��
//�����ȷ�����������������йأ���Ҫ�������
//��û������������£��ȼ򵥼��裬��ʵ�������֤
//��ν��м��裿�趨��߱ȣ��߶������С���м����εݼ���
void SymmetryProcess::computeSymmetryCurve()
{
	//�ԶԳ�ֵ������г�ʼ������
	for(int i = 0 ; i < linesNum; i++)
	{
		for(int j = 0; j < scanningWidth + 1; j++)
		{
			symmetryCurve[i][j] = 0;
		}
	}
	float increase = (highLayer[4][1] - highLayer[4][0]) / (linesNum - 1);//�õ��߶ȵ���
	uchar* scanLinePtr;
	uchar* scanWinTP;
	//uchar *modelPtr = edgeImage.ptr<uchar>(0);//ָ��ƥ��ģ�����ʼλ��
	//��ÿ��ɨ���߽��д����������£����ν��� 
	for(int i = 0; i < linesNum; i++)
	{
		int winHigh = highLayer[4][0] + i * increase;
		int winWidth = winHigh * aspectRatioNew;//����õ���ǰɨ�贰�ڴ�С

		//��֪ɨ�贰�ڳߴ�������£�Ҫȷ��ɨ�贰����Ԫ�أ�����Ӧģ���һһ��Ӧ��ϵ,��Ҫע����ǲ��ܷ���Խ��
		//float widthTR = modelWidth / winWidth;
		//float heightTR = (float)modelHeight / winHigh;

		int rowPos = startPos + i * interval;//�õ���ǰɨ����λ��
		if(rowPos - winHigh > 0)//��ǰ��ɨ�贰�ڶ���δԽ��
		{
			scanLinePtr = edgeImage.ptr<uchar>(rowPos);//���ָ��ɨ���������е�ָ��
			int maxVal = 0;//��¼ÿ���е����ֵ
			//����ÿ���ɼ����ĶԳ��ԣ�����Ŀɼ����ָ���ǣ��õ��Ӧ��ɨ�贰����ͼ��Χ֮�ڣ���û�з���Խ�磬
			for(int j = 0; j < scanningWidth ; j = j + 1)//�ܼ�ɨ����ΪscanningWidth��Ϊ�Ѿ�֮ǰ�趨�õ�
			{
				if(j - winWidth / 2.0 > 0 && j + winWidth / 2.0 < 320)//��ǰλ�ã���Ӧɨ�贰�����ұ߽�δԽ��
				{
					int maxDistance = (int)sqrt((double)(winHigh * winHigh + (winWidth/2.0) * (winWidth/2.0)));
					int symVal = 0;//��ʼ��ÿ�����ĵ�Գ�ֵΪ��
					int oldVal = symVal;//oldVal�����ж϶Գ�ֵ���ֲ������������
					int continued = 0;//��¼SymValֵ����δ�����仯�Ĵ������������δ�仯�Ƿ�����أ�
					//��δ����Խ�������£���ɨ�贰���ڹ������ĵĶԳ�ֵ���м��㣬���������и����Ĺ�ʽ
					//�������30��Symvalֵδ�����仯������жϵ�ǰδ�������ˣ����ڵ����ɶԳ�ֵ��Ϊ������
					for(int y = winHigh ;y > 0; y--)//���¶��Ͻ��м���
					{
						scanWinTP = scanLinePtr - (winHigh - y)*scanningWidth + j - winWidth/2;//�ٴη����ϴεĴ��󣬲�Ӧ��
						for(int x = 1;x < winWidth / 2.0; x++)
						{
							//����Ҳ�ǳ������Ĳ�forѭ����û�п��ܽ���һ�������أ������Ż�
							//�����ԶԳ�ֵ���������������������屾��Ĳ������ԣ�ѡ��ǰ�㣬��Ӧ���������λ����
							//����Գ��ԣ�������ʱ��ʹ��x�᷽�򣬱��ٽ�

							if(scanWinTP[x] == 255 && (scanWinTP[x] == scanWinTP[winWidth - x - 1]|| 
								scanWinTP[x] == scanWinTP[winWidth - x]||scanWinTP[ x] == scanWinTP[winWidth - x + 1]))
							{
								//symVal = symVal + 1;
								//����ԶԳ�ֵ�������иĽ��������Ǽ򵥼���1��
								//______����һ���Ծ������߾���x����ǰ�������Area��Ϊ������ֵ
								//symVal = symVal + 1;
								//symVal = symVal + 100.0 * (winWidth / 2.0 - x) / (winWidth/2.0) * 100.0 *((float)(y)/winHigh);
								//______���Ǿ���׶��е���룬����ֵ����볤����
								//symVal = symVal + maxDistance / sqrt((double)((winHigh - y)*(winHigh - y) + x * x));

								//�����Գ�ֵ�������̣����Է������������Ȩֵ�����ƵĽ������仮�֣�֮����зֱ�������
								//����yֵ���л��֣������Ȼ��֣���ͬ��λ���趨��ͬ�ı�ԵȨ���趨

								if( y < winHigh /7)//ͷ��
								{
									int center = winWidth * 2 / 3.0;
									int distance = std::abs(x - center);//x ����Ȩ�����ֵ����룬Ȩ�������ɷ���
									float weight = computeWeight(x,center,winWidth);
									symVal = symVal + 1000.0 * weight * 100.0 *(((float)y) / winHigh);
								}
								else if(y < (winHigh * 4.0) / 7)//�ϰ���
								{
									int center = 0;
									int distance = std::abs(x - center);//x ����Ȩ�����ֵ����룬Ȩ�������ɷ���
									float weight = computeWeight(x,center,winWidth);
									symVal = symVal + 1000.0 * weight * 100.0 *(((float)y) / winHigh);
								}
								else //�°���
								{
									int center = winWidth * 2 / 3.0;
									int distance = std::abs(x - center);//x ����Ȩ�����ֵ����룬Ȩ�������ɷ���
									float weight = computeWeight(x,center,winWidth);
									symVal = symVal + 1000.0 * weight * 100.0 *(((float)y) / winHigh);
								}
								//Ŀǰ������������Ч�����ţ����������õ���ô��Ϊʲô��˹�������ܹ�ʹ��

								//���ｫ����*100 / winHigh�����ۼӽ���֮�󣬼��ټ�����
							}
							else if(scanWinTP[j - x] != scanWinTP[j + x])
							{
								//symVal = symVal -  1;//00.0 * x / (winWidth/2.0);//* 10000.0 / (winHigh * winHigh);
							}
						}
						int k = interval*0.8;
						if((winHigh - y) == k && symVal == 0)//���¶������������ж�û�жԳ�ֵ���֣�˵�����������ˣ��е�çײ
							break;

						//������жϻ��д���һ��ȷ������ǰ������������ȷ��û�дﵽԤ�ڵ�Ч��
						if(symVal == oldVal)
						{
							continued++;
							if(continued == 40)//�ݶ�Ϊ30�У��������壬û��ʲô�ر������
							{
								symVal = 0;
								break;
							}
						}
						else
						{
							continued = 0;
						}
						oldVal = symVal;
					}
					symVal = symVal * 100.0 / (winHigh);
					if(symVal > 0)//�����ڲ����ǷǶԳƲ�����Ӱ�������£���Ϊ��ֵ
						symmetryCurve[i][j] = symVal;//j ��symmetryCurve�е��±��������Ӧ��
					else
						symmetryCurve[i][j] = 0;
					if (symVal > maxVal) 
						maxVal = symVal;
				}
			}
			symmetryCurve[i][320] = maxVal;//���������ֵ���б��棬������ʷ�������⣬�����Ѿ�û���ô��ˣ�������ֵ���趨����
		}
	}
}
//���ݾ���SymmetryCurve�������ߣ����ߵĻ���Ҳ���Ǳ���ģ�������ʹ�ù��ɸ���Ȼ�����������
void SymmetryProcess::plotCurve()
{
	cv::Mat curve = cv::Mat::zeros(240,320,CV_8UC1);//�õ�һ��ȫ�����
	//��ÿ�����߽����ֶ����ƣ���Ϊ�㣬��ֱ�ӻ��ƣ���߲�����4�����
	int maxHigh = 4 * interval;
	float maxVal = 0;
	for(int i = 0; i < linesNum;i++)
	{
		if(maxVal < symmetryCurve[i][320])
			maxVal = symmetryCurve[i][320];
	}
	//��ȡ���жԳ�ֵ�����ֵ
	float ratio = maxHigh / maxVal;//���ÿ��valֵ���ڸ߶���ʾ�еı���ֵ

	for(int i = 0; i < linesNum; i++)
	{
		int rowPos = startPos + i * interval;//�õ���ǰɨ����λ��
		uchar *edgePtr = curve.ptr<uchar>(rowPos);//���ָ��ǰ������ָ��
		for(int j = 0; j < scanningWidth; j++)//�����л���
		{
			uchar *edgeP = edgePtr + j;
			int currVal = symmetryCurve[i][j];
			int high = currVal * ratio;//��õ�ǰֵ�����ص���Ը߶�
			edgeP = edgeP - 320*high;
			edgeP[0] = 255;//��ָ����Ԫ���и�ֵ
		}
	}
	imshow("curve",curve);
}

//����������Լ���������У���ǰ������������Ҫ������
//�����У�ָ���ǵ�ǰɨ�����ϵ������Գ�ֵ����С��5������������10��������û�жԳ�ֵ�������趨��ֵ������ʱ�ģ���Ҫ���в���
//����Գ�ֵ�ۼ�ֵС�ڶ�ֵ������Ķ�ֵ��ÿ��ɨ���߶�Ӧһ��ֵ��С�ڸö�ֵ��˵����ǰ�ĶԳ�ֵ�����һ�����������˱�Ե������
//Ŀǰ�뵽�����ݾ�ֻ����ô�࣬�Ƚ���ʵ�ְ�
void SymmetryProcess::eliminate()
{
	int minWidth = 5;//�����Գ�ֵ��С���
	int maxBlank = 10;//�������հ׿��
	//�����ܷ�ͬʱ����أ��Լ���ѭ��������˼·������������д��������
	for(int i = 0; i < linesNum; i++)//����ÿһ�У����ν�����������
	{
		int maxVal = symmetryCurve[i][320];//�õ�ÿ�е����ֵ

		int leftBorder = 0;
		int rightBorder = 0;//�����Գ�ֵ�����ұ߽�
		int leftBlankLength = 0;
		int rightBlankLength = 0;
		int continuesLength = 0;//�����Գ�ֵ���ҿհ׳��ȣ������ж��Ƿ񽫸����������

		bool isFromBlank = true;//��ʾ��ǰԪ�ص�ǰһ��Ԫ������,ǰһ��Ԫ�ز���blank����symmVal����ʼ�趨Ϊtrue������һ��blank
		bool isLeft = true;//��ʶ��ǰ�հ���������ԳƲ���λ��

		//��������һ�α���
		for(int j = 0; j < scanningWidth; j++)
		{
			int symmetryVal = symmetryCurve[i][j];
			if(symmetryVal < maxVal * 0.2)//������趨��ʵ�ǲ�����ģ���Ϊ��������ɨ�����϶������ں���ĶԳ�ֵ
				//����˲�����ɨ�����Ͼ�һ���ᱣ�����֣���������һ����������жԳ�ֵ��������������ɾ������
				//�����Ƿ�Ϊ�㣬ֻҪС�ڶ�ֵ����һ�ɰ����հ״���
			{
				symmetryCurve[i][j] = 0;//����
				if(isFromBlank)
				{
					if(isLeft)//���Ϊ��հף�Ĭ��Ϊ��հ�
						leftBlankLength++;
					else
						rightBlankLength++;
				}
				else//����ǰһ����ԪΪsymmVal
				{
					rightBorder = j - 1;
					isLeft = false;//һ���жϹ����� һ��һ�ң������ڼ�¼������ֻ��һ����Ϊfalse
					rightBlankLength++;
				}
			}
			else
			{
				if(isFromBlank)
				{
					if(isLeft)
					{
						leftBorder = j;
						continuesLength++;
						isFromBlank = false;//��ǰ�Ѿ�����symm��Χ���´��ٽ����ж��ǣ���ֵ��ȻҪ�趨Ϊfalse��
					}
					else//���Ҳ�հ������µ�symmVal
					{
						isFromBlank = false;//isFromBlank == false��isLeft == False�Ҳ�հ�����symmVal�������³���
									//����һ�ֿ��ܻ���֣���symmVal�ոս����ҿհ�ʱ����ʱ��isLeft == false��isFromBlank == false
					}
				}
				else
				{
					continuesLength++;
				}
			}

			//����ܷ�������м�¼����
			//�Լ�¼��Ϣ�����жϣ�ʲôʱ���ܽ����жϣ����������ֵĺ�һ���հ������ڽ����ж�
			if(!isLeft)//����Ϊ�ҿհ�
			{
				if(continuesLength > minWidth)//��������Ҫ���������������ٴν��г�ʼ������
				{
					leftBorder = 0;
					rightBorder = 0;
					leftBlankLength = rightBlankLength;
					rightBlankLength = 0;
					continuesLength = 0;
					isFromBlank = true;
					isLeft = true;
				}
				else//��Ҫ��һ��ͨ�����ҿհ׽����ж�
				{
					if (leftBlankLength < maxBlank)
					{
						leftBorder = 0;
						rightBorder = 0;
						leftBlankLength = rightBlankLength;
						rightBlankLength = 0;
						continuesLength = 0;
						isFromBlank = true;
						isLeft = true;
					}
					else//��ǰ��Ҫ���Ҳ�հ׳��Ƚ����жϣ�һ�����������ɾ������
					{
						if(rightBlankLength > maxBlank)//��Ҫ����ɾ��
						{
							for(int k = leftBorder; k < rightBorder + 1; k++)
							{
								symmetryCurve[i][k] = 0;//����
							}
							leftBorder = 0;
							rightBorder = 0;
							leftBlankLength = leftBlankLength + continuesLength + rightBlankLength;
							rightBlankLength = 0;
							continuesLength = 0;
							isFromBlank = true;
							isLeft = true;
						}
						else if(isFromBlank)
						{
							//��������ѭ������
						}
						else
						{
							if(!(j - rightBorder == 1))//�������Ǹոս����Ҳ�հ�
							{
								//�ҿհ�����symmVal�������ҿհ׳��Ȳ��㣬ֱ���ٴν��г�ʼ��
								leftBorder = j;
								rightBorder = 0;
								leftBlankLength = rightBlankLength;
								rightBlankLength = 0;
								continuesLength = 1;
								isFromBlank = false;
								isLeft = true;
							}
						}
					}
				}
			}
		}
	}

	//���ƴ�����ͼ��
	float maxVal = 0;
	for(int i = 0; i < linesNum;i++)
	{
		if(maxVal < symmetryCurve[i][320])
			maxVal = symmetryCurve[i][320];
	}
	float ratio = 200 / maxVal;

	//�ԶԳ�ֵ������м�ģ������
	int tmp[12][scanningWidth];
	for(int i = 0; i < 12;i++)
	{
		tmp[i][0] = symmetryCurve[i][0];
		for(int j = 1; j < scanningWidth - 1; j++)
		{
			tmp[i][j] = (symmetryCurve[i][j -1] + symmetryCurve[i][j]*2 + symmetryCurve[i][j+1])/4;
		}
		tmp[i][scanningWidth - 1] = symmetryCurve[i][scanningWidth - 1];
	}
	cv::Mat target = Mat(Size(320,120),CV_8U);
	//���½�������и�ֵ����
	for(int i = 0;i < linesNum; i++)
	{
		int rowPos = i * 10;
		uchar *edgePtr = target.ptr<uchar>(rowPos);
		//�����н��и�ֵ
		for(int j = 0;j < scanningWidth;j++)
		{
			if(bottomInfo[j][0] != 0)
				edgePtr[j] = tmp[i][j] * ratio;//ʹ��ģ���������
			else
				edgePtr[j] = 0;
		}
		//��ʣ����н���ֱ�ӿ���
		for(int k = 1;k < 10; k++)
		{
			target.row(rowPos).copyTo(target.row(rowPos + k));
		}
	}
	imshow("after statistics target",target);
}

//�µ�ͳ�ƹ��̣�ͳ�Ƶ׶�λ�ã��������ۼ�ֵ
void SymmetryProcess::statisticNew()
{
	//�ֱ��¼ÿ��ɨ������Ϊ�׶˵��е��ۼ����ֵ����ʱ������������������
	int thresholdMax[linesNum];
	//��������forѭ�����Խ��кϲ�
	for(int j = 0; j < scanningWidth; j++)//��symmetryCurve�е�ÿһ�����¶��Ͻ���ͳ�ƣ���¼���һ����Ϊ�������λ��
	{
		bottomInfo[j][0] = 0;
		for(int i = linesNum - 1; i > 1;i--)
		{
			if(symmetryCurve[i][j] != 0)
			{
				bottomInfo[j][0] = (i + 1);
				break;
			}
		}
	}
	//���ÿ���ۼӶԳ�ֵ֮��
	for(int j = 0; j < scanningWidth; j++)
	{
		bottomInfo[j][1] = 0;
		for(int i = 0; i < bottomInfo[j][0]; i++)
		{
			bottomInfo[j][1] = bottomInfo[j][1] + symmetryCurve[i][j];
		}
		//��¼ÿ���׶˶�Ӧ�ĶԳ�ֵ���ȡֵ�����Ŀ���Ǻ������������
		//������ʱ�ȷ��������������
		if(bottomInfo[j][0] != 0)//���׶�bottomInfo[j][0]��Ϊ��ʱ���Ƚ��ۼ�ֵ�뵱ǰ��¼ֵ�Ĵ�С��ϵ
		{
			int bottom = bottomInfo[j][0] - 1;//bottomInfo[j][0]��ȡֵ��Χ��[1,linesNum]�����������Ҫ���м�һ����
			if(thresholdMax[bottom] < bottomInfo[j][1])
			{
				thresholdMax[bottom] = bottomInfo[j][1];
			}
		}
		if(j > 1)
		{
			bottomInfo[j-1][1] = (bottomInfo[j -2][1] + bottomInfo[j-1][1]*2 + bottomInfo[j][1])/4;
		}
	}

	//for(int j = 0; j < scanningWidth; j++)//��symmetryCurve�е�ÿһ�����¶��Ͻ���ͳ�ƣ���¼���һ����Ϊ�������λ��
	//{
	//	bottomInfo[j][0] = 0;
	// 	bool isFirst = true;
	//	for(int i = linesNum - 1; i > 2;i--)
	//	{
	//		if(symmetryCurve[i][j] != 0 && isFirst)
	//		{
	//			bottomInfo[j][0] = (i + 1);
	//			isFirst == false;
	//		}
	//		bottomInfo[j][1] = bottomInfo[j][1] + symmetryCurve[i][j];
	//	}
	//	if(bottomInfo[j][0] != 0)//���׶�bottomInfo[j][0]��Ϊ��ʱ���Ƚ��ۼ�ֵ�뵱ǰ��¼ֵ�Ĵ�С��ϵ
	//	{
	//		int bottom = bottomInfo[j][0] - 1;//bottomInfo[j][0]��ȡֵ��Χ��[1,linesNum]�����������Ҫ���м�һ����
	//		if(thresholdMax[bottom] < bottomInfo[j][1])
	//		{
	//			thresholdMax[bottom] = bottomInfo[j][1];
	//		}
	//	}
	//}

	//��ǰ������Ϣ��ÿ�еĵ׶�λ�ã�ÿ�еĵ׶��ۼ�ֵ��
	//���ÿ��Ǹ����׶��ڲ�ͬ��ɨ�����ϣ����յ��ۼ�Ч����һ���ģ����Ե�ǰֻ��Ҫ��bottomInfo[i][1]��Ѱ�ҷ�ֵ�Ϳ�����


	////�Է�ֵ���л���
	//int maxPlotLength = 50;
	//int maxSymVal = thresholdMax[10];
	//float ra = (float)maxPlotLength / maxSymVal;
	//for(int i = 0; i < scanningWidth; i++)
	//{
	//	int symmVal = bottomInfo[i][1];
	//	int currentHigh = symmVal * ra;
	//	for(int k = 0; k < currentHigh; k++)
	//	{
	//		std::cout<<".";
	//	}
	//	std::cout<<endl;
	//}
}

//��ȡ��ֵ��Ϣ,
//������ȡ��ֵ�ͺϲ��ٽ���ֵ�Ĺ��������ǷǼ���ֵ����
void SymmetryProcess::extractPeaks()
{
	for(int j = 0; j < linesNum * 2; j++)
	{
		peakInfo[j] = 0;//�Է�ֵ��Ϣ���г�ʼ��
	}
	
	int recordSub[scanningWidth / 2];
	int k = 0;//����kֵ��reocrdSub���м�¼����
	int p,q;//������ηǼ�����������ֵ��recordSub�еı߽�
	int i = 1;
	while( i + 1 < scanningWidth)
	{
		if(bottomInfo[i][1] > bottomInfo[i + 1][1])
		{
			if(bottomInfo[i][1] > bottomInfo[i - 1][1] || bottomInfo[i][1] == bottomInfo[i - 1][1])
			{
				//cout<<"peak1 at "<<i<<endl;
				recordSub[k++] = i;
			}
		}
		else
		{
			i = i + 1;
			while(i + 1 < scanningWidth && (bottomInfo[i][1] < bottomInfo[i + 1][1] || bottomInfo[i][1] == bottomInfo[i + 1][1]) )
			{
				i = i + 1;
			}
			if(i + 1 < scanningWidth)
			{
				//peak at i;
				//cout<<"peak1 at "<<i<<endl;
				recordSub[k++] = i;
			}
		}
		i = i + 2;
	}
	recordSub[k++] = -1;
	p = k;
	//�ٴζ�����÷�ֵ���÷Ǽ������ƣ�Ѱ��������ֵ
	i = 1;
	while(i + 1 < p - 1)
	{
		if(bottomInfo[recordSub[i]][1] > bottomInfo[recordSub[i + 1]][1])
		{
			if(bottomInfo[recordSub[i]][1] > bottomInfo[recordSub[i -1]][1] || 
				bottomInfo[recordSub[i]][1] == bottomInfo[recordSub[i -1]][1])
			{
				//cout<<"peak2 at "<<recordSub[i]<<endl;
				recordSub[k++] = recordSub[i];
			}
		}
		else
		{
			i = i + 1;
			while(recordSub[i + 1] != -1 &&
				(bottomInfo[recordSub[i]][1] < bottomInfo[recordSub[i + 1]][1] ||
				bottomInfo[recordSub[i]][1] == bottomInfo[recordSub[i + 1]][1]))
			{
				i = i + 1;
			}
			if(i + 1 < p - 1)
			{
				//peak at i;
				//cout<<"peak2 at "<<recordSub[i]<<endl;
				recordSub[k++] = recordSub[i];
			}
		}
		i = i + 2;
	}
	q = k;
	//�ڶ��ηǼ���ֵ�������õ���Ϊ������ֵ����recordSub�еķ�Χ��[p,q],�������÷Ǽ������ƣ��ɵ�Ư��
	k = p;
	for(int j = 0; k < q;j++,k++)
	{
		peakInfo[j] = recordSub[k];//����ֵ��Ϣ���б���
	}
}
//��һ����������ԭʼͼ���п��
//�ڶ��������ڵ�ͼ����ȡ��������ģ��ƥ�䣬���������Ϊ�漰�� 
//ͼ���resize�����ܻ�ȽϺ�ʱ�����Ϲ����������������Ч�� ���֮���ٽ��н�һ���Ĵ���
void SymmetryProcess::lockPedestrianArea()

{
	//������ֱ�ӽ��г�ʼ���������������������Ļ�������Ҫ��lockedPedArea������Ϣ�����ͷţ��м��м�
	LockedArea* current = lockedPedArea->next;
	LockedArea* tmp;
	while(current)
	{
		tmp = current->next;
		delete current;
		current = tmp;
	}
	lockedPedArea->next = NULL;
	int location,scanLineNum;
	float increase;
	int bottomPos,bottomPosS;
	int winHigh,winWidth,winHighS,winWidthS,topLeftX,topLeftY;

	for(int i = 0; peakInfo[i]!= 0;i++)
	{
		location = peakInfo[i];
		scanLineNum = bottomInfo[location][0];//�õ���ֵ���ڵ׶�λ��[1,12]
		//ÿ��λ�÷ֱ�������߿�
		//for(int j = 0; j < 5;j++)
		//{
		//	increase = (float)(highLayer[j][1] - highLayer[j][0]) / (linesNum - 1);
		//	winHigh = highLayer[j][0] + (scanLineNum - 1) * increase;//����ĸ߶�Ϊ���߶�1.9����Ӧ�ĸ߶ȣ����ȷ����ͬ�߶ȶ�Ӧֵ��
		//							//�����趨��Χ��[1.5,1.9]��5����Χ��ֻҪ�Ǹ߶�ȷ���ˣ���Ӧ�Ŀ��Ҳ��ȷ����
		//	winWidth = winHigh * aspectRatioNew;
		//	winHighS = winHigh / Hcoff;
		//	winWidthS = winWidth / Wcoff;

		//	bottomPos = startPos + (scanLineNum - 1) * interval;
		//	bottomPosS = bottomPos / Hcoff;

		//	topLeftX = location / Wcoff - winWidthS / 2;
		//	topLeftY = bottomPosS - winHighS;

		//	LockedArea* tmpArea = new LockedArea();
		//	tmpArea->topLeftX = topLeftX;
		//	tmpArea->topLeftY = topLeftY;
		//	tmpArea->width = winWidthS;
		//	tmpArea->height = winHighS;
		//	tmpArea->next = lockedPedArea->next;
		//	lockedPedArea->next = tmpArea;
		//	plotBox(sourceImage,topLeftX,topLeftY + 3*i ,winWidthS,winHighS);
		//}

		increase = (float)(highLayer[3][1] - highLayer[3][0]) / (linesNum - 1);
		winHigh = highLayer[2][0] + (scanLineNum - 1) * increase;//����ĸ߶�Ϊ���߶�1.9����Ӧ�ĸ߶ȣ����ȷ����ͬ�߶ȶ�Ӧֵ��
		//�����趨��Χ��[1.5,1.9]��5����Χ��ֻҪ�Ǹ߶�ȷ���ˣ���Ӧ�Ŀ��Ҳ��ȷ����
		winWidth = winHigh * aspectRatioNew;
		winHighS = winHigh / Hcoff;
		winWidthS = winWidth / Wcoff;

		bottomPos = startPos + (scanLineNum - 1) * interval;
		bottomPosS = bottomPos / Hcoff;

		topLeftX = location / Wcoff - winWidthS / 2;
		topLeftY = bottomPosS - winHighS;

		LockedArea* tmpArea = new LockedArea();
		tmpArea->topLeftX = topLeftX;
		tmpArea->topLeftY = topLeftY;
		tmpArea->width = winWidthS;
		tmpArea->height = winHighS;
		tmpArea->next = lockedPedArea->next;
		lockedPedArea->next = tmpArea;
		plotBox(sourceImage,topLeftX,topLeftY ,winWidthS,winHighS);

	}
	//��ɶ���ȡ��������ı��棬��һ�����ڶ������resize��������õ������������ӽ��з���ƥ��
	//LockedArea* p = lockedPedArea->next;
	//while(p!= NULL)
	//{
	//	cout<<p->height<<" ";
	//	p = p->next;
	//}
	//std::cout<<std::endl;
	imshow("sourceImage0",sourceImage);//��ʾЧ��������һ����������������ȡ��������������ֵ���㼰���ڵ�ƥ��
	
}


LockedArea* SymmetryProcess::getAreaInfo()
{
	return lockedPedArea;
}
//�����getArea�����ɾ�������м��ϲ������еĲ���
void SymmetryProcess::getArea()
{
	//�õ�����ģ���ɨ�����򣬼��������ǣ����ɵ׶˱�Ե��Ϣ��bottomInfo�������ͳ�Ƶõ�
	//������ȷ��һ���ǣ�����õ������ս����ɨ�����򣬶������ض���ɨ�贰�ڵĶ�λ���п��ܻ����һ���̶ȵĺϲ���
	//���ڵ�����,1���������������ݽṹ���д洢
	//2����ȡ�����Ĳ��Խ���������⡣��򵥵�˼·����ֱ�ӽ���ͳ��
	
	//��������Ϣ���г�ʼ������
	for(int i = 0; i < linesNum; i++)
	{
		scanningArea[i].linesNum = 0;
		scanningArea[i].startPos = 0;
		scanningArea[i].endPos = 0;
		scanningArea[i].next = NULL;
	}

	for(int i = scanningWidth - 1; i > 0; )
	{
		while(bottomInfo[i][0] == 0 && i > 0)
		{
			i--;
		}//�Թ��׶�Ϊ0�������У�����ʱ����ǰ�е׶˲�Ϊ��
		int firstVal = bottomInfo[i][0];//��¼��ǰ�׶�ֵ[1,12]
		AreaInfo* area = new AreaInfo();
		area->linesNum = bottomInfo[i][0];
		area->endPos = i;
		i--;
		while(bottomInfo[i][0] == firstVal && i > 0)
		{
			i--;
		}//����ʱ��ʾ��ǰ�еĵ׶�ֵ����ΪfirstVal������Ϊ�㣬Ҳ����Ϊ����һ��ֵ

		area->startPos = i + 1;
		area->next = scanningArea[firstVal - 1].next;
		scanningArea[firstVal - 1].next = area;
	}
	//��ʾ
	for(int i = 0; i < linesNum; i++)
	{
		AreaInfo* p = scanningArea[i].next;
		if (p != NULL)
		{
			cout<<"linesNum "<<p->linesNum<<":";
		}
		while(p != NULL)
		{
			cout<<p->startPos<<","<<p->endPos<<" ";
			p = p ->next;
		}
		cout<<endl;
	}
	//ͳ�ƽ���֮�󣬶��ڽ����ֽ��кϲ���ͬʱ���˵�����һ�е�Ԫ��,�޳�����Ԫ�ؽ�Ϊ�򵥣��鷳������ν��кϲ���

	//��Ҫ�趨�������ڶ��ٽ���ɾ��������С�ڶ��ٽ��кϲ�
	//�������ж��ܷ���кϲ����ܺϲ�����ϲ�֮�������û�н��кϲ�������£����ж��Ƿ���Ҫ�������
	
	int minCols = 3;//�趨��������
	int maxDistance = 8;//�趨�����������е����ɺϲ����룬������趨��ʱû��ʲô���ݣ�ֱ�۸��ܶ���
	AreaInfo* preNode;//ָ��ǰ����������ڵ�֮ǰ�Ľڵ�
	AreaInfo* first;//ָ��ǰ����ĵ�һ���ڵ�
	AreaInfo* second;//ָ��ǰ����ĵڶ����ڵ�
	for(int i = 0; i < linesNum; i++)
	{
		preNode = &scanningArea[i];
		AreaInfo* p = scanningArea[i].next;
		//��Ҫ����ͳһ����
		while( p != NULL)
		{
			first = p;
			p = p->next;
			if (p != NULL)//��������second��������жϣ��ϲ���������
			{
				//���ȶ��Ƿ���Խ��кϲ������ж�
				second = p;
				if((second->startPos - first->endPos) + 1 < maxDistance)//���Խ��кϲ�����
				{
					first->endPos = second->endPos;
					first->next = second->next;
					delete second;
					p = first;
				}
				//���Ƿ����ɾ�������жϣ��������׶˷ֱ�����ж�
				else if((first->endPos - first->startPos + 1) < minCols && (second->endPos - second->startPos + 1) < minCols)
				{
					//ͬʱɾ�������ڵ�
					preNode->next = second->next;
					delete first;
					delete second;
					p = preNode->next;

				}
				else if((first->endPos - first->startPos + 1) < minCols)
				{
					//ɾ����һ���ڵ�
					preNode->next = second;
					delete first;
					p = second;
				}
				else if((second->endPos - second->startPos + 1) < minCols)
				{
					//ɾ���ڶ����ڵ�
					first->next = second->next;
					delete second;
					p = first;
				}
				else
				{
					//�����ڵ����������
					p = second;
				}

			}
			else//������һ��first����ֱ�ӽ���minCols�ж�
			{
				if((first->endPos - first->startPos + 1) < minCols)
				{
					//ɾ��first�ڵ�
					preNode->next = first->next;
					delete first;
					p = preNode->next;
				}
			}
		}
	}
	
	//��ʾ
	for(int i = 0; i < linesNum; i++)
	{
		AreaInfo* p = scanningArea[i].next;
		if (p != NULL)
		{
			cout<<"linesNum "<<p->linesNum<<":";
		}
		while(p != NULL)
		{
			cout<<p->startPos<<","<<p->endPos<<" ";
			p = p ->next;
		}
		cout<<endl;
	}
}
//��ԭʼͼ���л���ָ��λ�ñ߿�
void SymmetryProcess::plotBox(Mat &targetImage,int topLeftX,int topLeftY,int width,int height)
{
	//��ԭʼͼ��sourceImage�л���ɨ������߿�
	//uchar* topPtr = targetImage.ptr<uchar>(topLeftY);
	//uchar* bottomPtr = targetImage.ptr<uchar>(topLeftY + height);
	//for(int i = 0; i < width;i++)
	//{
	//	topPtr[topLeftX + i] = 255;
	//	bottomPtr[topLeftX + i] = 255;
	//}

	//for(int j = 0; j < height;j++)
	//{
	//	topPtr[topLeftX + j * targetImage.cols] = 255;
	//	topPtr[topLeftX + width + j * targetImage.cols] = 255;
	//}
	Rect rect = Rect(topLeftX,topLeftY,width,height);
	cv::rectangle(sourceImage,rect,Scalar(255,255,255),1);
	
}

void SymmetryProcess::plotArea()
{
	float increase = (highLayer[4][1] - highLayer[4][0]) / (linesNum - 1);
	for(int i = 0; i < linesNum; i++)
	{
		AreaInfo* p = scanningArea[i].next;
		int winHigh = highLayer[4][0] + i * increase;
		int winWidth = winHigh * aspectRatioNew;
		int winHighS = winHigh / Hcoff;
		int winWidthS = winWidth / Wcoff;
		if (p != NULL)
		{
			cout<<"linesNum"<<p->linesNum<<":";
		}
		while(p != NULL)
		{
			
			int left = p->startPos;
			int leftS = left / Wcoff;
			
			int bottom = startPos + (p->linesNum - 1) * interval;
			int bottomS = bottom / Hcoff;
			//��leftS��rightS��bottomS��ͬȷ��ɨ�������С

			plotBox(sourceImage,leftS,bottomS - winHighS,winWidthS,winHighS);
			p = p ->next;
		}
		cout<<endl;
	}

	imshow("plotAre",sourceImage);
}
//���������������ʲô�أ�
void SymmetryProcess::getTemplateMinMax(int num,int &minTemplate,int &maxTemplate)
{
	int bottom = startPos + (num - 1) * interval;
	int bottomS = bottom / Hcoff;
	float Zw = (ay * high) /(bottomS - groundLine);
	minTemplate = (f / Zw) * minRealHobj;
	maxTemplate = (f / Zw) * maxRealHobj;
}

void scanning(){}

//��ȷ����ɨ���������ȷ�ϲ�����
void SymmetryProcess::scanningAndVerify()
{
	int num = 0;
	int minTemplate;
	int maxTemplate;
	for(int i = 0; i < linesNum; i++)
	{
		AreaInfo* p = scanningArea[i].next;
		if (p != NULL)
		{
			num = p->linesNum;
			getTemplateMinMax(num,minTemplate,maxTemplate);
		}
		while(p != NULL)
		{
			int left = p->startPos;
			int right = p->endPos;
			int leftS = left / Wcoff;
			int rightS = right / Wcoff;
			int bottom = startPos + (num - 1) * interval;
			int bottomS = bottom / Hcoff;
			//��leftS��rightS��bottomS��ͬȷ��ɨ�������С
			scanning();
			p = p ->next;
		}
		cout<<endl;
	}
}
int SymmetryProcess::GetMin(int a, int b, int c, int d, int e)
{
	int t = (a < b ? a : b) < c ? (a < b ? a : b) : c;
	return ((t < d ? t : d) < e ? (t < d ? t : d) : e);
}
//����ͼ���趨��Ե���Ǹ��壬���л��ƣ��ľ��룬���øþ�����Ϣ����Ϊ��Ե��Ӧֵ����Ȩֵ
void SymmetryProcess::productModel(const char* filename)
{
	responseModel = cv::imread(filename,0);
	int cols = responseModel.cols;//54
	int rows = responseModel.rows;//128
	modelWidth = cols;
	modelHeight = rows;

	int p0,p1,p2,p3,p4;
	int step = responseModel.step;
	int max = 0;
	cv::Mat destImage = cv::Mat(360,150,CV_8UC1);
	for(int i = 0; i < rows;i++)
	{
		uchar* ptr = responseModel.ptr<uchar>(i);
		//uchar* destPtr = destImage.ptr<uchar>(i);
		for(int j = 0; j < cols;j++)
		{
			uchar* pTemp = ptr;
			int min;
			if((i > 0 && i < rows - 1) && (j > 0 && j < cols - 1))
			{
				
					p0 = pTemp[j];
					p4 = pTemp[j - 1] + 3;
					pTemp = ptr - step;
					p1 = pTemp[j - 1] + 4;
					p2 = pTemp[j] + 3;
					p3 = pTemp[j + 1] + 4;
					min = GetMin(p0,p1,p2,p3,p4);
					//destPtr[j] = min;
					ptr[j] = std::min(255,min);
				
			}
			else
			{
				//destPtr[j] = 0;
				ptr[j] = 255;
			}
		}
	}
	for(int i = rows - 1; i > -1; i--)
	{
		uchar* ptr = responseModel.ptr<uchar>(i);
		//uchar* destPtr = destImage.ptr<uchar>(i);
		for(int j = cols - 1;j > -1; j--)
		{
			uchar* pTemp = ptr;
			int min;
			if((i > 0 && i < rows - 1) && (j > 0 && j < cols - 1))
			{
			
					p0 = pTemp[j];
					p1 = pTemp[j + 1] + 3;
					pTemp = ptr + step;
					p2 = pTemp[j - 1] + 4;
					p3 = pTemp[j] + 3;
					p4 = pTemp[j + 1] + 4;
					min = GetMin(p0,p1,p2,p3,p4);
					//destPtr[j] = min;
					ptr[j] = min;
					max = max < min ? min : max; 
				
			}
			else
			{
				//destPtr[j] = 0;
				ptr[j] = 255;
			}
		}
	}
	for(int i = 0; i < rows; i++)
	{
		uchar* ptr = responseModel.ptr<uchar>(i);
		for(int j = 0; j < cols; j++)
		{
			//float temp = destPtr[j];
			//destPtr[j] = 255 * temp / max;
			if(ptr[j] != 255)
			{
				float temp = ptr[j];
				ptr[j] = 150 * ((max - temp) / max);
			}
			else
			{
				ptr[j] = 0;
			}
		}
	}
	cv::imshow("responsemodel",responseModel);
}


