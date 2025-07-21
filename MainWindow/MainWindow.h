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
//ʹ��opencv��ȡ����ͷ��
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void updateCameraFrame();  // ��������ͷ֡ �������϶��ǲ����е�̫����
    void switchPage(int index);

private:
    int lowThreshold = 10;  // ����ֵĬ��ֵ
    int highThreshold = 50; // ����ֵĬ��ֵ

    //������
    QWidget* navPanel;
    QVBoxLayout* navLayout;
    QButtonGroup* navButtonGroup;

    // ����ҳ������
    QStackedWidget* pageContainer;
    QWidget* cameraPage;    // ����ͷҳ��
    QWidget* settingsPage;  // ����ҳ��

    // ������
    QWidget* centralWidget;
    QHBoxLayout* mainLayout;

    // �������
    QWidget* leftPanel;
    QVBoxLayout* leftPanelLayout;
    //���ϵ�׷�ٻ���
    QLabel* mainDisplay;

    //������������
    QWidget* downContainer;
    QHBoxLayout* downLayout;
    QLabel* edgeDisplay;
    QLabel* cameraDisplay;



    cv::VideoCapture videoCapture;
    
    QTimer* frameTimer;

    void setupUi();
    void setupCamera();

    void createNavigation(); // ����������������
    void createPages();      // ����������ҳ��
    //void postureDemo();

    //��opencv��mat��תΪQimage��ʽ
    QImage cvMatToQImage(const cv::Mat& inMat);
    cv::Mat detectEdges(const cv::Mat& input);



};

#endif // MAINWINDOW_H