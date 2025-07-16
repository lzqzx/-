#include "MainWindow.h"
#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>
#include <QGraphicsDropShadowEffect>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {

	setupUi();
	setupCamera();
}

MainWindow::~MainWindow() {
	if (videoCapture.isOpened()) {
		videoCapture.release();
	}
}

void MainWindow::setupUi() {
	// ���������ڱ���ΪiOS���ǳɫ��
	centralWidget = new QWidget(this);
	centralWidget->setStyleSheet("background-color: #F5F5F7;");  // iOSϵͳ��ɫ����
	setCentralWidget(centralWidget);

	mainLayout = new QHBoxLayout(centralWidget);
	mainLayout->setContentsMargins(20, 20, 20, 20);  // ������Χ�߾�
	mainLayout->setSpacing(20);  // ����������

	leftPanel = new QWidget();
	leftPanelLayout = new QVBoxLayout(leftPanel);
	leftPanelLayout->setContentsMargins(0, 0, 0, 0);
	leftPanelLayout->setSpacing(15);  // ���������

	//�ָ�
	downContainer = new QWidget();
	downContainer->setStyleSheet("background: transparent; border: none;"); // ͸���������ޱ߿�
	downLayout = new QHBoxLayout(downContainer);
	downLayout->setContentsMargins(0, 0, 0, 0);
	downLayout->setSpacing(15); // ���������֮��ļ��

	edgeDisplay = new QLabel();
	edgeDisplay->setStyleSheet(
		"background: white;"
		"border-radius: 12px;"
		"padding: 12px;"
		"font-size: 16px;"
		"color: #8E8E93;"
		"border: none;"
		"box-shadow: 0 2px 10px rgba(0,0,0,0.05);"
	);
	edgeDisplay->setMinimumHeight(100);

	cameraDisplay = new QLabel();
	cameraDisplay->setStyleSheet(
		"background: white;"
		"border-radius: 12px;"
		"padding: 12px;"
		"font-size: 16px;"
		"color: #8E8E93;"
		"border: none;"
		"box-shadow: 0 2px 10px rgba(0,0,0,0.05);");
	cameraDisplay->setMinimumHeight(100);

	downLayout->addWidget(edgeDisplay);
	downLayout->addWidget(cameraDisplay);


	//���ϵ�׷�ٻ���
	mainDisplay = new QLabel();
	mainDisplay->setMinimumSize(640, 480);
	mainDisplay->setAlignment(Qt::AlignCenter);
	mainDisplay->setStyleSheet(
		"background: white;"
		"border-radius: 12px;"       // Բ��Ч��
		"padding: 8px;"
		"font-size: 18px;"
		"font-weight: 500;"
		"color: #8E8E93;"            // iOS�����ı�ɫ
		"border: none;"
		"box-shadow: 0 2px 10px rgba(0,0,0,0.05);"  // �����Ӱ
	);

	// ��ӵ���಼��
	leftPanelLayout->addWidget(mainDisplay);
	leftPanelLayout->addWidget(downContainer);


	 // �Ҳ���� - iOS����̨��ʽ
	 QWidget* rightPanel = new QWidget();
	 rightPanel->setStyleSheet(
	     "background: #FFFFFF;"
	     "border-radius: 12px;"
	     "box-shadow: 0 4px 15px rgba(0,0,0,0.08);"
	     );
	 QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
	 rightLayout->setContentsMargins(20, 20, 20, 20);

	 // ���ʾ������Ԫ��
	 QLabel* controlsTitle = new QLabel("EDGE SETTINGS");
	 controlsTitle->setStyleSheet("font-size: 20px; font-weight: 600; color: #000000;");
	 controlsTitle->setAlignment(Qt::AlignCenter);

	 // ʾ������ؼ�
	 QSlider* thresholdSlider = new QSlider(Qt::Horizontal);
	 thresholdSlider->setStyleSheet(
	     "QSlider::groove:horizontal {"
	     "   height: 6px;"
	     "   background: #E0E0E0;"
	     "   border-radius: 3px;"
	     "}"
	     "QSlider::handle:horizontal {"
	     "   background: #007AFF;"   // iOSϵͳ��ɫ
	     "   width: 22px;"
	     "   height: 22px;"
	     "   margin: -8px 0;"
	     "   border-radius: 11px;"
	     "}"
	     );

	 rightLayout->addWidget(controlsTitle);
	 rightLayout->addSpacing(15);
	 rightLayout->addWidget(new QLabel("Edge Threshold:"));
	 rightLayout->addWidget(thresholdSlider);
	 rightLayout->addStretch();  // ���Կռ�

	// // ��ӵ�������
	mainLayout->addWidget(leftPanel, 7);
	 mainLayout->addWidget(rightPanel, 3);
}


void MainWindow::setupCamera() {

	try {
		//����ͷindex
		videoCapture.open(0);

		if (!videoCapture.isOpened()) {
			qDebug() << "Camera not turned on";
			cameraDisplay->setText("Camera not available");

			return;
		}

		//��ʱ������
		frameTimer = new QTimer(this);
		connect(frameTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);
		frameTimer->start(33); //30֡/��
	}
	catch (const std::exception& e) {
		qDebug() << "Camera initialization error��" << e.what();
		cameraDisplay->setText("Camera error");
	}
}


void MainWindow::updateCameraFrame() {
	cv::Mat frame;
	if (videoCapture.read(frame)) {
		//qDebug() << "!!!!!!!!!!";
		cv::Mat rgbFrame;
		cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
		QImage origImg = cvMatToQImage(rgbFrame);

		// ��ʾԭʼͼ��
		cameraDisplay->setPixmap(QPixmap::fromImage(origImg)
			.scaled(cameraDisplay->width(),
				cameraDisplay->height(),
				Qt::KeepAspectRatio));

		// ��Ե��� 
		cv::Mat edges = detectEdges(frame);

		cv::Mat colorEdges;
		cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);
		cv::bitwise_and(rgbFrame, colorEdges, colorEdges);

		// ��ʾ��Եͼ��
		QImage edgeImg = cvMatToQImage(colorEdges);
		edgeDisplay->setPixmap(QPixmap::fromImage(edgeImg)
			.scaled(edgeDisplay->width(),
				edgeDisplay->height(),
				Qt::KeepAspectRatio));		//����Ӧ����

		cv::Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// �������ͼ����ԭʼ֡�ϻ��ƣ�
		cv::Mat result = frame.clone();

		// ��̬�������ָ���״̬
		static std::vector<cv::Point2f> trackedPoints;
		static cv::Mat prevGray;
		

		 //��ʼ���򵱵����ʱ���¼��
		if (trackedPoints.empty() || trackedPoints.size() < 5)	//5�������е�̫����
		{

			trackedPoints.clear();
			
			//�滻�ǵ���
			//����ָ��������ݶ� X�����ʾ��ֱ��Ե
			cv::Mat gradX;
			cv::Sobel(gray, gradX, CV_32F, 1, 0);	//dx=1,dy=0 ���ⴹֱ��Ե

			//�ݶ�ͼ��ֵ��
			cv::Mat absGrad, gradThresh;	//�洢�ݶȾ���ֵͼ��Ͷ�ֵ������ݶ�ͼ��
			absGrad = cv::abs(gradX);
			double minVal, maxVal;
			cv::minMaxLoc(absGrad, &minVal, &maxVal);	//��absGrad�е�����ֵ��Χ[a,b]ͨ��ָ�봫�ݸ�min&maxVal
			cv::threshold(absGrad, gradThresh, maxVal * 0.1, 255, cv::THRESH_BINARY);	// 10%����ݶ���ֵ
			gradThresh.convertTo(gradThresh, CV_8U);

			//��ֱ�����ϵķǼ���ֵ����
			cv::Mat nmsEdges = cv::Mat::zeros(gray.size(), CV_8U);
			for (int y = 1; y < gradThresh.rows - 1; y++) {
				for (int x = 1; x < gradThresh.cols - 1; x++) {
					if (gradThresh.at<uchar>(y, x) > 0) {
						// ��ֱ��Ե���Ƚ��������أ��ݶȷ���
						if (gradThresh.at<uchar>(y, x) > 0) {
							if (gradX.at<float>(y, x) >= gradX.at<float>(y - 1, x) &&
								gradX.at<float>(y, x) >= gradX.at<float>(y + 1, x)
								) {
								nmsEdges.at<uchar>(y, x) = 255;
							}
						}
					}
				}
			}
			//��������ȡ
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(nmsEdges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			//�ر�Ե������ ÿn������ȡһ����
			const int samplingInterval = 10;  // �������
			for (const auto& contour : contours) {
				for (int i = 0; i < contour.size(); i += samplingInterval) {
					if (trackedPoints.size() >= 100) break;  // ���Ƶ���
					trackedPoints.emplace_back(contour[i].x, contour[i].y);
				}
			}


			//ʹ�ýǵ��⣨���������ʺϸ��٣�
			//cv::goodFeaturesToTrack(
			//	gray,               // ����Ҷ�ͼ
			//	trackedPoints,      // ����ǵ�
			//	100,                // ���ǵ���
			//	0.01,               // �����ȼ�
			//	10,                 // ��С���
			//	cv::Mat(),          // ��ģ
			//	3,                  // ����ߴ�
			//	false,              // ��ʹ��Harris
			//	0.04                // Harris����
			//);
		}
		// ����֡���й�������
		else if (!prevGray.empty()) {
			std::vector<cv::Point2f> newPoints;
			std::vector<uchar> status;
			std::vector<float> err;

			// LK����������
			cv::calcOpticalFlowPyrLK(
				prevGray, gray,
				trackedPoints, newPoints,
				status, err,
				cv::Size(21, 21),   // ��������
				3                   // ����������
			);

			// ������Ч���ٵ�
			std::vector<cv::Point2f> goodPoints;
			for (size_t i = 0; i < trackedPoints.size(); i++) {
				if (status[i]) {
					goodPoints.push_back(newPoints[i]);
					// ���ٵ�
					cv::circle(result, newPoints[i], 5, cv::Scalar(0, 255, 0), -1);
				}
			}

			//std::cout << "Tracked Points:" << std::endl;
			//for (const auto& point : trackedPoints) {
			//	std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
			//}
			// vector����һ�Ѷ�ά�������꣬����Ǩ�Ƶ�ģ����0704-25
			// ����ʹ�ô���̬ͼ��ķ���������һ������������ȥ��ͼ���Ե�������ӵ�
			// ������Ѫ��ģ������ô���ٻ���Ҫ���飬�뷨�Ǳ�����̬�仯�ĵ㣬
			// ����ʵ��Ҫ��Ѫ��ģ������֮�����
			//Tracked Points :
			//(719, 531)
			//	(750, 515)
			//	(751, 546)
			//	(713, 521)
			//	(780, 500)

		}
		prevGray = gray.clone();


		QImage resultImg = cvMatToQImage(result);
		mainDisplay->setPixmap(QPixmap::fromImage(resultImg)
			.scaled(mainDisplay->width(),
				mainDisplay->height(),
				Qt::KeepAspectRatio));

	}
	else {
		qDebug() << "Unable to read camera frames";
		frameTimer->stop();
		cameraDisplay->setText("Camera disconnected");
		edgeDisplay->setText("No input");
	}
}


QImage MainWindow::cvMatToQImage(const cv::Mat& inMat) {
	switch (inMat.type()) {
	case CV_8UC3: {
		QImage image(inMat.data, inMat.cols, inMat.rows, static_cast<int>(inMat.step), QImage::Format_RGB888);
		return image.rgbSwapped(); //BGR->RGB
	}
	case CV_8UC1: {
		QImage image(inMat.data,
			inMat.cols, inMat.rows,
			static_cast<int>(inMat.step),
			QImage::Format_Grayscale8);
		return image;
	}
	default:
		qWarning() << "Unsupported image format:" << inMat.type();
		// ת��ΪĬ�ϸ�ʽ
		cv::Mat dst;
		cv::cvtColor(inMat, dst, cv::COLOR_BGR2RGB);
		return QImage(dst.data, dst.cols, dst.rows,
			static_cast<int>(dst.step),
			QImage::Format_RGB888);
	}

}


cv::Mat MainWindow::detectEdges(const cv::Mat& input) {
	cv::Mat gray, edge;

	cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);

	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);

	cv::Canny(gray, edge, 10, 50);

	return edge;
}

