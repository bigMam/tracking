//完成功能，由底层提供行人矩形框架，产生可靠tracklet，并提交给上层manager
#pragma once

#include "SymmetryProcess.h"//lockedArea
#include "featureExtractor.h"//blockFeature
#include "discriminator.h"
#include "opencv2/video/tracking.hpp"

typedef struct _trackerlet
{
	int trackerletID;//这里设定编号，方便在多个存储单元内对同一目标进行查找，倒不是为了标识之类的
	int topLeftX;
	int topLeftY;
	int width;
	int height;
	int occupied;//表示当前trackerlet被几个使用者所占用，最后一个使用者，可以将其删除，否则仅仅能够对其移除，
	blockFeature featureSet;//每个trackerlet都有对应的特征提取，用于之后进行前后差异性对比
	_trackerlet* next;
	_trackerlet()
	{
		trackerletID = 0;
		topLeftX = 0;
		topLeftY = 0;
		width = 0;
		height = 0;
		occupied = 0;//这里的占有者，主要有三个，distrator数组，targetTrackerletList列表,还有就是manager中的目标池
		next = NULL;
	}
	void setBlockFeature(blockFeature& blockfeatures)
	{
		featureSet = blockfeatures;
	}
}Trackerlet;

class Tracker
{
	int stateNum;//状态矩阵[x,y,dx,dy,width,height,dw,dh]
	int measureNum;//测量矩阵，[x,y,width,height]
	cv::Mat state; // (x,y,dX,dY)
	cv::Mat processNoise;
	cv::Mat measurement;
	FeatureExtractor extractor;
	Discriminator discriminator;//分辨器
	cv::KalmanFilter KF;//先设定一个kalman滤波器，看一下，如何进行操作

	LockedArea* lockedPedArea;//检测得到行人存在区域含头结点
	Trackerlet* targetTrackerlet;
	//也是链表的形式,是链表的形式，需要对所有检测得到tracklet进行操作，会不会耗时呢？
	//先仅仅跟踪一个行人，之后再进行调整，不可能一次将所有内容都考虑进来
	//这里targetTrackerlet为列表形式，第一个元素为跟踪目标，其后为当前检测得到其他目标

	double weights[8];//更新权重，分别对当前discriminator变量，及manager中discriminator变量进行权重更新

	Trackerlet* distratorList[6];//这里是将抛弃tracklet内容保存下来用于更新特征值权重，这里需要另外一个空间来辅助判断队空or队满
	static const int capacity = 6;//distrator列表容量上限，超过则将oldest one删除
	int front;//队头下标
	int rear;//队尾下标

	int letNumber;//trackerlet编号

public:
	Tracker();
	~Tracker();
	void setLoackedPedArea(LockedArea *result);
	//对之前tracklet进行更新，及产生新的tracklet，用于管理,如果更新失败，则设定request
	//haveRectBoxing表示当前是根据矩形框内容进行更新，但是这时又存在一个新的问题
	//对当前图像中行人只完成部分检测，这样这里的haveBoxing参数的含义就不明确的
	//从这里可以稍微延伸一下，对于你想到的所有问题，一时不能够全部解决，你需要明确首要目标，
	//分清主次关系，才可以保证自己不会走偏，有些内容是可以进行延后的，
	bool update(cv::Mat &souceImage,bool haveRectBoxing);
	bool update(cv::Mat &souceImage);//新的更新过程，包含对distrator的管理？功能太多了，可能需要进一步细分
	void extractTracklet(cv::Mat &sourceImage,LockedArea* lockedPedArea,Trackerlet* tracklet);//根据rect提取tracklet
	double distinguish(blockFeature& target, blockFeature& current);//计算两特征向量区分度
	void featureWeighting(blockFeature& current);//在线根据当前得到内容对各个特征向量权重进行调整
	void insertDistrator(Trackerlet* tracklet);//将丢弃tracklet加入distrator，同时保证distrator容量上限
	bool isTargetTrackerlet(Trackerlet* current);//判断当前trackerlet是否为目标targetTrackerlet


	void correctTarget(Trackerlet* correctTrackerlet);//对跟踪目标进行修正过程
	Trackerlet* getTrackerlist();
	void clearList();
	void insertList(Trackerlet* trackerlet);
};