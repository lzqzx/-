#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
//使用opencv获取摄像头流
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateCameraFrame();  // 更新摄像头帧

private:
    // 主布局
    QWidget* centralWidget;
    QHBoxLayout* mainLayout;

    // 左侧区域
    QWidget* leftPanel;
    QVBoxLayout* leftPanelLayout;
    //右上的追踪画面
    QLabel* mainDisplay;

    //右下两个格子
    QWidget* downContainer;
    QHBoxLayout* downLayout;
    QLabel* edgeDisplay;
    QLabel* cameraDisplay;


    cv::VideoCapture videoCapture;
    
    QTimer* frameTimer;

    void setupUi();
    void setupCamera();
    //void postureDemo();

    //将opencv的mat类转为Qimage格式
    QImage cvMatToQImage(const cv::Mat& inMat);
    cv::Mat detectEdges(const cv::Mat& input);



};

#endif // MAINWINDOW_H