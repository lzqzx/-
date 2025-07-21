#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
//使用opencv获取摄像头流
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateCameraFrame();  // 更新摄像头帧 这里的耦合度是不是有点太高了
    void switchPage(int index);

private:
    int lowThreshold = 10;  // 低阈值默认值
    int highThreshold = 50; // 高阈值默认值

    //导航栏
    QWidget* navPanel;
    QVBoxLayout* navLayout;
    QButtonGroup* navButtonGroup;

    // 新增页面容器
    QStackedWidget* pageContainer;
    QWidget* cameraPage;    // 摄像头页面
    QWidget* settingsPage;  // 设置页面

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

    void createNavigation(); // 新增：创建导航栏
    void createPages();      // 新增：创建页面
    //void postureDemo();

    //将opencv的mat类转为Qimage格式
    QImage cvMatToQImage(const cv::Mat& inMat);
    cv::Mat detectEdges(const cv::Mat& input);



};

#endif // MAINWINDOW_H