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


void MainWindow::createPages() {
	pageContainer = new QStackedWidget();

	// ============ҳ��1
	cameraPage = new QWidget();
	cameraPage->setStyleSheet("background-color: #F5F5F7;");

	QHBoxLayout* cameraLayout = new QHBoxLayout(cameraPage);
	cameraLayout->setContentsMargins(20, 20, 20, 20);
	cameraLayout->setSpacing(20);

	// �����ʾ����
	leftPanel = new QWidget();
	leftPanelLayout = new QVBoxLayout(leftPanel);
	leftPanelLayout->setContentsMargins(0, 0, 0, 0);
	leftPanelLayout->setSpacing(20);

	// ��ӳߴ�Լ������
	auto setLabelPolicy = [](QLabel* label) {
		label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		label->setScaledContents(true);
		label->setMinimumSize(1, 1); // ��ֹ���ֱ���
		};

	//����ʾ��׷�٣�
	mainDisplay = new QLabel();
	setLabelPolicy(mainDisplay); //Ӧ��Լ��
	mainDisplay->setAlignment(Qt::AlignCenter);
	mainDisplay->setStyleSheet(
		"background-color:rgba(255, 255, 255, 0.55);"
		"border: 1px solid rgba(255, 255, 255, 0.18);"
		"border-radius: 18px;"
		"backdrop-filter: blur(20px); "
		"box-shadow: 0 8px 32px rgba(0, 0, 0, 0.08);"
		"padding: 0px;"
	);

	downContainer = new QWidget();
	downLayout = new QHBoxLayout(downContainer);
	downLayout->setContentsMargins(0, 0, 0, 0);
	downLayout->setSpacing(20); // ���������֮��ļ��



	edgeDisplay = new QLabel();
	setLabelPolicy(edgeDisplay); // Ӧ��Լ��
	edgeDisplay->setStyleSheet(
		"background-color:rgba(255, 255, 255, 0.55);"
		"border: 1px solid rgba(255, 255, 255, 0.18);"
		"border-radius: 18px;"
		"backdrop-filter: blur(20px); "
		"box-shadow: 0 8px 32px rgba(0, 0, 0, 0.08);"
		"padding: 0px;"
	);

	cameraDisplay = new QLabel();
	setLabelPolicy(cameraDisplay); // Ӧ��Լ��
	cameraDisplay->setStyleSheet(
		"background-color:rgba(255, 255, 255, 0.55);"
		"border: 1px solid rgba(255, 255, 255, 0.18);"
		"border-radius: 18px;"
		"backdrop-filter: blur(20px); "
		"box-shadow: 0 8px 32px rgba(0, 0, 0, 0.08);"
		"padding: 0px;"
	);

	leftPanelLayout->setSpacing(0);
	leftPanelLayout->addWidget(mainDisplay, 7);

	downLayout->addWidget(edgeDisplay);
	downLayout->addWidget(cameraDisplay);

	leftPanelLayout->addSpacing(20);
	leftPanelLayout->addWidget(downContainer, 3);




	// �Ҳ���� - iOS����̨��ʽ
	QWidget* rightPanel = new QWidget();
	rightPanel->setStyleSheet(
		"background: #FFFFFF;"
		"border-radius: 12px;"
		"box-shadow: 0 4px 15px rgba(0,0,0,0.08);"
	);
	QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
	rightLayout->setContentsMargins(20, 20, 20, 20);
	rightLayout->setSpacing(20);


	// ���ʾ������Ԫ��
	QLabel* controlsHeader = new QLabel("Detection settings");
	controlsHeader->setStyleSheet(
		"font-size: 22px;"
		"font-weight: 600;"
		"color: #000000;"
		"padding-bottom: 6px;"
		"border-bottom: 1px solid #F0F0F0;"
	);
	controlsHeader->setAlignment(Qt::AlignLeft);

	// ����ؼ�
	QWidget* sliderGroup = new QWidget();
	QVBoxLayout* sliderLayout = new QVBoxLayout(sliderGroup);
	sliderLayout->setContentsMargins(0, 10, 0, 10);
	sliderLayout->setSpacing(15);
	QLabel* thresholdLabel = new QLabel("Edge detection threshold");
	thresholdLabel->setStyleSheet(
		"font-size: 17px;"
		"font-weight: 500;"
		"color: #333333;"
	);
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

	// ��ӿؼ���������
	sliderLayout->addWidget(thresholdLabel);
	sliderLayout->addWidget(thresholdSlider);

	// �������ÿؼ� (ռλ)
	QPushButton* startButton = new QPushButton("Start detection");
	startButton->setStyleSheet(
		"QPushButton {"
		"   background-color: #007AFF;"
		"   color: white;"
		"   border-radius: 10px;"
		"   padding: 12px;"
		"   font-size: 17px;"
		"   font-weight: 600;"
		"}"
		"QPushButton:hover {"
		"   background-color: #0062CC;"
		"}"
		"QPushButton:pressed {"
		"   background-color: #004D99;"
		"}"
	);

	// ��ӿؼ����Ҳ����
	rightLayout->addWidget(controlsHeader);
	rightLayout->addWidget(sliderGroup);
	rightLayout->addStretch(); // ����ռλ
	rightLayout->addWidget(startButton);


	cameraLayout->addWidget(leftPanel, 7);
	cameraLayout->addWidget(rightPanel, 3);


	pageContainer->addWidget(cameraPage);  // ����0


	// =============ҳ��2
	settingsPage = new QWidget();
	QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
	settingsLayout->setContentsMargins(40, 40, 40, 40);

	// �������ҳ������ʾ��
	QLabel* settingsTitle = new QLabel("ϵͳ����");
	settingsTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333;");

	QWidget* settingItem = new QWidget();
	QHBoxLayout* itemLayout = new QHBoxLayout(settingItem);
	itemLayout->setContentsMargins(0, 0, 0, 0);

	QLabel* itemLabel = new QLabel("��Ե�����ֵ");
	QSlider* itemSlider = new QSlider(Qt::Horizontal);

	itemLayout->addWidget(itemLabel, 1);
	itemLayout->addWidget(itemSlider, 2);

	settingsLayout->addWidget(settingsTitle);
	settingsLayout->addSpacing(30);
	settingsLayout->addWidget(settingItem);
	settingsLayout->addStretch();  // ����

	pageContainer->addWidget(settingsPage);  // ����1
}

void MainWindow::switchPage(int index) {
	pageContainer->setCurrentIndex(index);
}

void MainWindow::createNavigation() {
	//�������
	navPanel = new QWidget();
	navPanel->setStyleSheet(
		"background: #FFFFFF;"
		"border-right: 1px solid #E0E0E0;");

	navLayout = new QVBoxLayout(navPanel);
	navLayout->setContentsMargins(5, 20, 5, 20);
	navLayout->setSpacing(10);

	//������ť��
	navButtonGroup = new QButtonGroup(this);
	navButtonGroup->setExclusive(true);

	//����ͷҳ�水ť  ps��Ϊʲô�����ﶨ�壿����ͷ�ļ��ж��壿
	QPushButton* cameraBtn = new QPushButton("����ͷ");
	cameraBtn->setCheckable(true);
	cameraBtn->setChecked(true);
	cameraBtn->setStyleSheet(
		"QPushButton {"
		"   text-align: left;"
		"   padding: 12px 16px;"
		"   border-radius: 8px;"
		"   font-size: 14px;"
		"}"
		"QPushButton:checked {"
		"   background: #E3F2FD;"
		"   color: #1976D2;"
		"   font-weight: bold;"
		"}"
		"QPushButton:hover {"
		"   background: #F5F5F5;"
		"}"
	);
	navLayout->addWidget(cameraBtn);

	//ҳ�水ť
	QPushButton* settingsBtn = new QPushButton("����");
	settingsBtn->setCheckable(true);
	settingsBtn->setStyleSheet(cameraBtn->styleSheet());
	navLayout->addWidget(settingsBtn);

	// ��Ӱ�ť���鲢�����ź�
	navButtonGroup->addButton(cameraBtn, 0);
	navButtonGroup->addButton(settingsBtn, 1);
	//�����
	connect(navButtonGroup, &QButtonGroup::idClicked,
		this, &MainWindow::switchPage);
	// ��ӵ���ʹ��ť�ö�
	navLayout->addStretch();
}

void MainWindow::setupUi() {

	// ���������ڱ���ΪiOS���ǳɫ��
	centralWidget = new QWidget(this);
	centralWidget->setStyleSheet("background-color: #F5F5F7;");  // iOSϵͳ��ɫ����
	setCentralWidget(centralWidget);

	//�޸�������
	mainLayout = new QHBoxLayout(centralWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);  // ������Χ�߾�
	mainLayout->setSpacing(0);  // ����������

	// ��������
	createNavigation();

	//������ȹ̶�
	navPanel->setFixedWidth(150);
	mainLayout->addWidget(navPanel); //���������ռ��1/10

	//ҳ������
	createPages();
	mainLayout->addWidget(pageContainer, 9);

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
		//��ʾ���ؼ�ʱʹ��ʵ�ʴ�С
		cameraDisplay->setPixmap(
			QPixmap::fromImage(origImg)
		);

		// ��Ե��� 
		cv::Mat edges = detectEdges(frame);

		cv::Mat colorEdges;
		cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);
		cv::bitwise_and(rgbFrame, colorEdges, colorEdges);

		// ��ʾ��Եͼ��
		QImage edgeImg = cvMatToQImage(colorEdges);
		edgeDisplay->setPixmap(
			QPixmap::fromImage(edgeImg));

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
		mainDisplay->setPixmap(
			QPixmap::fromImage(resultImg));
		//// ��ȡ��ʾ�ؼ���ʵ�ʳߴ�
		//const QSize availableSize = cameraDisplay->size();

		//// ����Ŀ��ߴ磨�������ֱ��ʣ�
		//QSize targetSize;
		//// ���������
		//targetSize = QSize(qMin(availableSize.width(), 1280), 0);
		//targetSize.setHeight(targetSize.width() * 9 / 16); // 16:9����

		//mainDisplay->setPixmap(QPixmap::fromImage(resultImg)
		//	.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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

