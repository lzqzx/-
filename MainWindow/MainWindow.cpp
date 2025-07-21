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

	// ============页面1
	cameraPage = new QWidget();
	cameraPage->setStyleSheet("background-color: #F5F5F7;");

	QHBoxLayout* cameraLayout = new QHBoxLayout(cameraPage);
	cameraLayout->setContentsMargins(20, 20, 20, 20);
	cameraLayout->setSpacing(20);

	// 左侧显示区域
	leftPanel = new QWidget();
	leftPanelLayout = new QVBoxLayout(leftPanel);
	leftPanelLayout->setContentsMargins(0, 0, 0, 0);
	leftPanelLayout->setSpacing(20);

	// 添加尺寸约束策略
	auto setLabelPolicy = [](QLabel* label) {
		label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		label->setScaledContents(true);
		label->setMinimumSize(1, 1); // 防止布局崩溃
		};

	//主显示（追踪）
	mainDisplay = new QLabel();
	setLabelPolicy(mainDisplay); //应用约束
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
	downLayout->setSpacing(20); // 两个子组件之间的间隔



	edgeDisplay = new QLabel();
	setLabelPolicy(edgeDisplay); // 应用约束
	edgeDisplay->setStyleSheet(
		"background-color:rgba(255, 255, 255, 0.55);"
		"border: 1px solid rgba(255, 255, 255, 0.18);"
		"border-radius: 18px;"
		"backdrop-filter: blur(20px); "
		"box-shadow: 0 8px 32px rgba(0, 0, 0, 0.08);"
		"padding: 0px;"
	);

	cameraDisplay = new QLabel();
	setLabelPolicy(cameraDisplay); // 应用约束
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




	// 右侧面板 - iOS控制台样式
	QWidget* rightPanel = new QWidget();
	rightPanel->setStyleSheet(
		"background: #FFFFFF;"
		"border-radius: 12px;"
		"box-shadow: 0 4px 15px rgba(0,0,0,0.08);"
	);
	QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
	rightLayout->setContentsMargins(20, 20, 20, 20);
	rightLayout->setSpacing(20);


	// 添加示例控制元素
	QLabel* controlsHeader = new QLabel("Detection settings");
	controlsHeader->setStyleSheet(
		"font-size: 22px;"
		"font-weight: 600;"
		"color: #000000;"
		"padding-bottom: 6px;"
		"border-bottom: 1px solid #F0F0F0;"
	);
	controlsHeader->setAlignment(Qt::AlignLeft);

	// 滑块控件
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
		"   background: #007AFF;"   // iOS系统蓝色
		"   width: 22px;"
		"   height: 22px;"
		"   margin: -8px 0;"
		"   border-radius: 11px;"
		"}"
	);

	// 添加控件到滑块组
	sliderLayout->addWidget(thresholdLabel);
	sliderLayout->addWidget(thresholdSlider);

	// 其他设置控件 (占位)
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

	// 添加控件到右侧面板
	rightLayout->addWidget(controlsHeader);
	rightLayout->addWidget(sliderGroup);
	rightLayout->addStretch(); // 弹簧占位
	rightLayout->addWidget(startButton);


	cameraLayout->addWidget(leftPanel, 7);
	cameraLayout->addWidget(rightPanel, 3);


	pageContainer->addWidget(cameraPage);  // 索引0


	// =============页面2
	settingsPage = new QWidget();
	QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
	settingsLayout->setContentsMargins(40, 40, 40, 40);

	// 添加设置页面内容示例
	QLabel* settingsTitle = new QLabel("系统设置");
	settingsTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #333333;");

	QWidget* settingItem = new QWidget();
	QHBoxLayout* itemLayout = new QHBoxLayout(settingItem);
	itemLayout->setContentsMargins(0, 0, 0, 0);

	QLabel* itemLabel = new QLabel("边缘检测阈值");
	QSlider* itemSlider = new QSlider(Qt::Horizontal);

	itemLayout->addWidget(itemLabel, 1);
	itemLayout->addWidget(itemSlider, 2);

	settingsLayout->addWidget(settingsTitle);
	settingsLayout->addSpacing(30);
	settingsLayout->addWidget(settingItem);
	settingsLayout->addStretch();  // 弹簧

	pageContainer->addWidget(settingsPage);  // 索引1
}

void MainWindow::switchPage(int index) {
	pageContainer->setCurrentIndex(index);
}

void MainWindow::createNavigation() {
	//导航面板
	navPanel = new QWidget();
	navPanel->setStyleSheet(
		"background: #FFFFFF;"
		"border-right: 1px solid #E0E0E0;");

	navLayout = new QVBoxLayout(navPanel);
	navLayout->setContentsMargins(5, 20, 5, 20);
	navLayout->setSpacing(10);

	//导航按钮组
	navButtonGroup = new QButtonGroup(this);
	navButtonGroup->setExclusive(true);

	//摄像头页面按钮  ps：为什么在这里定义？不在头文件中定义？
	QPushButton* cameraBtn = new QPushButton("摄像头");
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

	//页面按钮
	QPushButton* settingsBtn = new QPushButton("设置");
	settingsBtn->setCheckable(true);
	settingsBtn->setStyleSheet(cameraBtn->styleSheet());
	navLayout->addWidget(settingsBtn);

	// 添加按钮到组并连接信号
	navButtonGroup->addButton(cameraBtn, 0);
	navButtonGroup->addButton(settingsBtn, 1);
	//待理解
	connect(navButtonGroup, &QButtonGroup::idClicked,
		this, &MainWindow::switchPage);
	// 添加弹簧使按钮置顶
	navLayout->addStretch();
}

void MainWindow::setupUi() {

	// 设置主窗口背景为iOS风格浅色调
	centralWidget = new QWidget(this);
	centralWidget->setStyleSheet("background-color: #F5F5F7;");  // iOS系统灰色背景
	setCentralWidget(centralWidget);

	//修改主界面
	mainLayout = new QHBoxLayout(centralWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);  // 增加外围边距
	mainLayout->setSpacing(0);  // 增加组件间距

	// 创建导航
	createNavigation();

	//导航宽度固定
	navPanel->setFixedWidth(150);
	mainLayout->addWidget(navPanel); //导航栏宽度占比1/10

	//页面容器
	createPages();
	mainLayout->addWidget(pageContainer, 9);

}


void MainWindow::setupCamera() {

	try {
		//摄像头index
		videoCapture.open(0);

		if (!videoCapture.isOpened()) {
			qDebug() << "Camera not turned on";
			cameraDisplay->setText("Camera not available");

			return;
		}

		//定时器设置
		frameTimer = new QTimer(this);
		connect(frameTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);
		frameTimer->start(33); //30帧/秒
	}
	catch (const std::exception& e) {
		qDebug() << "Camera initialization error：" << e.what();
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
	
		// 显示原始图像
		//显示到控件时使用实际大小
		cameraDisplay->setPixmap(
			QPixmap::fromImage(origImg)
		);

		// 边缘检测 
		cv::Mat edges = detectEdges(frame);

		cv::Mat colorEdges;
		cv::cvtColor(edges, colorEdges, cv::COLOR_GRAY2BGR);
		cv::bitwise_and(rgbFrame, colorEdges, colorEdges);

		// 显示边缘图像
		QImage edgeImg = cvMatToQImage(colorEdges);
		edgeDisplay->setPixmap(
			QPixmap::fromImage(edgeImg));

		cv::Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		// 创建结果图像（在原始帧上绘制）
		cv::Mat result = frame.clone();

		// 静态变量保持跟踪状态
		static std::vector<cv::Point2f> trackedPoints;
		static cv::Mat prevGray;
		

		 //初始检测或当点过少时重新检测
		if (trackedPoints.empty() || trackedPoints.size() < 5)	//5个好像有点太少了
		{

			trackedPoints.clear();
			
			//替换角点检测
			//计算指定方向的梯度 X方向表示垂直边缘
			cv::Mat gradX;
			cv::Sobel(gray, gradX, CV_32F, 1, 0);	//dx=1,dy=0 表检测垂直边缘

			//梯度图二值化
			cv::Mat absGrad, gradThresh;	//存储梯度绝对值图像和二值化后的梯度图像
			absGrad = cv::abs(gradX);
			double minVal, maxVal;
			cv::minMaxLoc(absGrad, &minVal, &maxVal);	//将absGrad中的像素值范围[a,b]通过指针传递给min&maxVal
			cv::threshold(absGrad, gradThresh, maxVal * 0.1, 255, cv::THRESH_BINARY);	// 10%最大梯度阈值
			gradThresh.convertTo(gradThresh, CV_8U);

			//垂直方向上的非极大值抑制
			cv::Mat nmsEdges = cv::Mat::zeros(gray.size(), CV_8U);
			for (int y = 1; y < gradThresh.rows - 1; y++) {
				for (int x = 1; x < gradThresh.cols - 1; x++) {
					if (gradThresh.at<uchar>(y, x) > 0) {
						// 垂直边缘：比较上下像素（梯度方向）
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
			//特征点提取
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(nmsEdges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			//沿边缘采样点 每n个像素取一个点
			const int samplingInterval = 10;  // 采样间隔
			for (const auto& contour : contours) {
				for (int i = 0; i < contour.size(); i += samplingInterval) {
					if (trackedPoints.size() >= 100) break;  // 限制点数
					trackedPoints.emplace_back(contour[i].x, contour[i].y);
				}
			}


			//使用角点检测（比轮廓更适合跟踪）
			//cv::goodFeaturesToTrack(
			//	gray,               // 输入灰度图
			//	trackedPoints,      // 输出角点
			//	100,                // 最大角点数
			//	0.01,               // 质量等级
			//	10,                 // 最小间距
			//	cv::Mat(),          // 掩模
			//	3,                  // 邻域尺寸
			//	false,              // 不使用Harris
			//	0.04                // Harris参数
			//);
		}
		// 后续帧进行光流跟踪
		else if (!prevGray.empty()) {
			std::vector<cv::Point2f> newPoints;
			std::vector<uchar> status;
			std::vector<float> err;

			// LK光流法跟踪
			cv::calcOpticalFlowPyrLK(
				prevGray, gray,
				trackedPoints, newPoints,
				status, err,
				cv::Size(21, 21),   // 搜索窗口
				3                   // 金字塔层数
			);

			// 更新有效跟踪点
			std::vector<cv::Point2f> goodPoints;
			for (size_t i = 0; i < trackedPoints.size(); i++) {
				if (status[i]) {
					goodPoints.push_back(newPoints[i]);
					// 跟踪点
					cv::circle(result, newPoints[i], 5, cv::Scalar(0, 255, 0), -1);
				}
			}

			//std::cout << "Tracked Points:" << std::endl;
			//for (const auto& point : trackedPoints) {
			//	std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
			//}
			// vector中是一堆二维像素坐标，可以迁移到模型中0704-25
			// 可以使用处理静态图像的方法，划定一个像素区域来去除图像边缘地区的杂点
			// 但是在血管模型中怎么跟踪还需要检验，想法是保留动态变化的点，
			// 具体实现要等血管模型来了之后检验
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
		//// 获取显示控件的实际尺寸
		//const QSize availableSize = cameraDisplay->size();

		//// 创建目标尺寸（限制最大分辨率）
		//QSize targetSize;
		//// 限制最大宽度
		//targetSize = QSize(qMin(availableSize.width(), 1280), 0);
		//targetSize.setHeight(targetSize.width() * 9 / 16); // 16:9比例

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
		// 转换为默认格式
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

