#pragma once
#ifndef _AGRITHNMCONTROL_H_
#define _AGRITHNMCONTROL_H_
#include "MyCamera.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "cpptoml.h"
#include <thread>
#include<iostream>
#include <cstdlib>
#include <iostream>
#include <list>
#include <mutex>
#include <map>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>

#include "zmq/zmq_msg_handle.h"

using namespace std;
using namespace cv;
#define MaxRingBufferLen 100

class CircularQueue {
public:
    CircularQueue(int size) {
        capacity = size;
        front = 0;
        rear = 0;
        data.resize(capacity);
    }
    CircularQueue() {};
    ~CircularQueue() {};
    bool isEmpty() const {
        return front == rear;
    }

    bool isFull() const {
        return (rear + 1) % capacity == front;
    }

    bool enqueue(cv::Mat item) {
        if (isFull()) {
            front = (front + 1) % capacity;
            data[rear] = item.clone();
            rear = (rear + 1) % capacity;
            std::cerr << "The queue is full, start discarding an image" << std::endl;
            return true;
        }

        data[rear] = item.clone();
        rear = (rear + 1) % capacity;
        return true;
    }

    bool dequeue(cv::Mat &item) {
        if (isEmpty()) {
            std::cerr << "Queue is empty. Cannot dequeue." << std::endl;
            return true;
        }
        item = data[front].clone();
        front = (front + 1) % capacity;
        return true;
    }
    int curSize() {
        return abs(rear - front);
    }

private:
    std::vector<cv::Mat> data;
    int capacity;
    int front;
    int rear;
};

class AgriConCore
{
public:
	AgriConCore(std::string cfgPath);
	~AgriConCore();
    void ControlCore();
private:
	MV_CC_DEVICE_INFO_LIST pstDevList = {0};
	CMyCamera* CMCam;
    static ZmqMsgHandle* m_zmq;
	static std::vector<void*> m_CamHandleList;   
    static std::vector<CircularQueue> m_FrameRingBufferVecs;
    std::vector<std::thread> m_rev_threads;
    std::vector<std::thread> m_pro_threads;

    void ZmqInit(std::string netConfigPath, int ServerOrClient);
    void CamInitial();
    static void CamDestroy();
    static void AgriRunTime(int CamHandleOder);
    void AgriRunTimeByMutilThread();
	static void CamFrameSaveToBuffer(int CamHandleOder);
	void CamFrameSaveToBuffersByMutilThread();
};




#endif // !_AGRITHNMCONTROL_H_
