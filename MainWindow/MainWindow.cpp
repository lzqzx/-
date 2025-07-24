#include "MainWindow.h"
#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>
#include <opencv2/imgproc.hpp>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPixmap>


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	setupUi();
	setupCamera();
	updatePointCloudView();
}

MainWindow::~MainWindow() {
	if (videoCapture.isOpened()) {
		videoCapture.release();
	}
}

QPointF MainWindow::projectPoint(float x, float y, float z) {
	// 简单透视投影 (可调整投影参数)
	float perspective = 0.001f;
	float scale = 1000.0f; // 缩放系数根据数据范围调整

	return QPointF(
		(x - z * 0.5) * perspective * scale,  // X投影
		(y - z * 0.3) * perspective * scale   // Y投影
	);
}

void MainWindow::updatePointCloudView() {
	// 原始数据
	const float points[16][3] = {
	{-82291.98352011, -78089.04482531, -67948.55108767},
	{-30815.31443639, -60096.74555043, 4828.00131839},
	{-42591.48582729, -73165.76994067, -131567.28872775},
	{-35521.97165458, -47295.23665129, -639.37112722},
	{-36341.06987475, -76012.24126566, -105631.99077126},
	{-55991.15293342, -39345.40013184, -78757.32300593},
	{-28657.82003955, -15228.00725115, -86627.35135135},
	{-54953.5939354, -35733.5464733, -72557.13579433},
	{-27067.2504944, 5389.41924852, -129737.66249176},
	{-51339.23665129, -52875.82201714, -141974.24192485},
	{-113816.88793672, -65350.87145682, -125281.9683586},
	{-82866.70138431, -84950.47923533, -66318.22808174},
	{-16058.0, -45217.0, -78592.0},
	{-21166.0, 5402.0, -126100.0},
	{69375.99802241, -55858.39815425, -194621.89452868},
	{-7655.83915623, -16324.0316414, -148623.16611734}
	};
	// 获取绘制区域尺寸
	QSize size = cameraDisplay->size() * 0.9; // 保留10%边距
	if (size.isEmpty()) return;

	// 创建画布
	QPixmap pixmap(cameraDisplay->size());
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	// 设置半透明白色背景（与样式表匹配）
	painter.setBrush(QColor(255, 255, 255, 140));
	painter.setPen(Qt::NoPen);
	painter.drawRoundedRect(cameraDisplay->rect(), 18, 18);

	// 计算投影点并确定边界
	QVector<QPointF> projected;
	projected.reserve(16);

	float minX = std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float maxY = std::numeric_limits<float>::lowest();

	for (const auto& pt : points) {
		QPointF p = projectPoint(pt[0], pt[1], pt[2]);
		projected.push_back(p);

		if (p.x() < minX) minX = p.x();
		if (p.y() < minY) minY = p.y();
		if (p.x() > maxX) maxX = p.x();
		if (p.y() > maxY) maxY = p.y();
	}

	// 计算缩放比例（保持纵横比）
	float rangeX = maxX - minX;
	float rangeY = maxY - minY;
	float scaleFactor = qMin(size.width() / rangeX, size.height() / rangeY);
	if (scaleFactor == 0 || std::isinf(scaleFactor)) scaleFactor = 1.0f;

	QPointF centerOffset(size.width() / 2, size.height() / 2);

	// 绘制网格
	painter.setPen(QPen(QColor(65, 105, 225), 1.5)); // 网格线颜色
	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			int idx = row * 4 + col;
			QPointF point = (projected[idx] - QPointF(minX, minY)) * scaleFactor;
			point = QPointF(point.x(), size.height() - point.y()); // 翻转Y轴
			point += centerOffset - QPointF((rangeX * scaleFactor) / 2, (rangeY * scaleFactor) / 2);

			// 绘制点
			painter.setBrush(QColor(220, 20, 60, 200)); // 点颜色
			painter.drawEllipse(point, 4, 4);

			// 连接行
			if (col < 3) {
				int nextIdx = row * 4 + col + 1;
				QPointF nextPoint = (projected[nextIdx] - QPointF(minX, minY)) * scaleFactor;
				nextPoint = QPointF(nextPoint.x(), size.height() - nextPoint.y());
				nextPoint += centerOffset - QPointF((rangeX * scaleFactor) / 2, (rangeY * scaleFactor) / 2);

				// 根据深度设置线宽（模拟3D效果）
				float depthFactor = 1.0 - qMin(points[idx][2] / 500000.0f, 1.0f);
				painter.setPen(QPen(QColor(65, 105, 225), 1.0 + depthFactor * 2.0));

				painter.drawLine(point, nextPoint);
			}

			// 连接列
			if (row < 3) {
				int nextIdx = (row + 1) * 4 + col;
				QPointF nextPoint = (projected[nextIdx] - QPointF(minX, minY)) * scaleFactor;
				nextPoint = QPointF(nextPoint.x(), size.height() - nextPoint.y());
				nextPoint += centerOffset - QPointF((rangeX * scaleFactor) / 2, (rangeY * scaleFactor) / 2);

				// 根据深度设置线宽
				float depthFactor = 1.0 - qMin(points[idx][2] / 500000.0f, 1.0f);
				painter.setPen(QPen(QColor(65, 105, 225), 1.0 + depthFactor * 1.5));

				painter.drawLine(point, nextPoint);
			}
		}
	}

	painter.end();
	cameraDisplay->setPixmap(pixmap);
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
	edgeDisplay->setAlignment(Qt::AlignCenter);
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
	cameraDisplay->setAlignment(Qt::AlignCenter);
	cameraDisplay->setStyleSheet(
		"background-color:rgba(255, 255, 255, 0.55);"
		"border: 1px solid rgba(255, 255, 255, 0.18);"
		"border-radius: 18px;"
		"backdrop-filter: blur(20px); "
		"box-shadow: 0 8px 32px rgba(0, 0, 0, 0.08);"
		"padding: 0px;"
	);

	cameraDisplay->setMaximumSize(1920, 1080);
	edgeDisplay->setMaximumSize(1920, 1080);


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

	// 设置滑块范围和初始值
	thresholdSlider->setRange(10, 200);  // 调整适合的值范围
	thresholdSlider->setValue(lowThreshold);

	// 连接滑块值变化信号
	connect(thresholdSlider, &QSlider::valueChanged, this, [=](int value) {
		//解释： [=](int value)把新的value值（由qt实时传入）加上thresholdSlider, &QSlider::valueChanged, this,这些静态的东西，扔给下面的逻辑计算
		
		// 设置新的阈值关系（高阈值为低阈值的2-3倍）
		lowThreshold = value;
		highThreshold = lowThreshold * 2 + 20;  // 可调整比例关系

		// 立即更新边缘检测显示
		if (cameraPage == pageContainer->currentWidget()) {
			updateCameraFrame();
		}
		});


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
	QLabel* settingsTitle = new QLabel("System Settings");
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
			//cameraDisplay->setText("Camera not available");

			return;
		}

		//定时器设置
		frameTimer = new QTimer(this);
		connect(frameTimer, &QTimer::timeout, this, &MainWindow::updateCameraFrame);
		frameTimer->start(33); //30帧/秒
	}
	catch (const std::exception& e) {
		qDebug() << "Camera initialization error：" << e.what();
		//cameraDisplay->setText("Camera error");
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
		//cameraDisplay->setPixmap(
		//	QPixmap::fromImage(origImg)
		//);

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

		/*==============追踪=========================*/
		static std::vector<cv::Point2f> trackedPoints;
		static cv::Mat prevGray;

		// 参数配置
		const int MIN_TRACK_POINTS = 15;
		const int MAX_TRACK_POINTS = 80;
		const float GRADIENT_THRESH_RATIO = 0.15f;
		const int LINE_SAMPLING_INTERVAL = 8;
		const int MOTION_HISTORY = 3;

		// 存储点运动历史
		static std::deque<std::vector<cv::Point2f>> motionHistory;
		// 更新跟踪点处理
		if (trackedPoints.size() < MIN_TRACK_POINTS) {
			trackedPoints.clear();
			motionHistory.clear();

			// 多方向梯度计算
			cv::Mat gradX, gradY, gradMag, gradDir;
			cv::Sobel(gray, gradX, CV_32F, 1, 0, 5);
			cv::Sobel(gray, gradY, CV_32F, 0, 1, 5);
			cv::cartToPolar(gradX, gradY, gradMag, gradDir);

			// 梯度幅值归一化
			double minVal, maxVal;
			cv::minMaxLoc(gradMag, &minVal, &maxVal);
			gradMag.convertTo(gradMag, CV_32F, 1.0 / (maxVal - minVal), -minVal / (maxVal - minVal));

			// 多方向非极大值抑制
			cv::Mat nmsEdges = cv::Mat::zeros(gray.size(), CV_8U);
			float neighbor1, neighbor2;
			for (int y = 2; y < gray.rows - 2; y++) {
				for (int x = 2; x < gray.cols - 2; x++) {
					if (gradMag.at<float>(y, x) > GRADIENT_THRESH_RATIO) {
						float angle = gradDir.at<float>(y, x) * 180 / CV_PI;
						if (angle < 0) angle += 180;

						// 0度方向 (水平)
						if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180)) {
							neighbor1 = gradMag.at<float>(y, x - 1);
							neighbor2 = gradMag.at<float>(y, x + 1);
						}
						// 45度方向
						else if (angle >= 22.5 && angle < 67.5) {
							neighbor1 = gradMag.at<float>(y - 1, x + 1);
							neighbor2 = gradMag.at<float>(y + 1, x - 1);
						}
						// 90度方向 (垂直)
						else if (angle >= 67.5 && angle < 112.5) {
							neighbor1 = gradMag.at<float>(y - 1, x);
							neighbor2 = gradMag.at<float>(y + 1, x);
						}
						// 135度方向
						else {
							neighbor1 = gradMag.at<float>(y - 1, x - 1);
							neighbor2 = gradMag.at<float>(y + 1, x + 1);
						}

						if (gradMag.at<float>(y, x) >= neighbor1 && gradMag.at<float>(y, x) >= neighbor2) {
							nmsEdges.at<uchar>(y, x) = 255;
						}
					}
				}
			}

			// 线段检测与采样
			std::vector<std::vector<cv::Point>> contours;
			cv::findContours(nmsEdges.clone(), contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

			for (const auto& contour : contours) {
				// 跳过短线段
				if (contour.size() < 20) continue;

				// 曲线简化
				std::vector<cv::Point> approxCurve;
				cv::approxPolyDP(contour, approxCurve, 2.0, false);

				// 沿曲线均匀采样
				for (int i = 0; i < approxCurve.size(); i += LINE_SAMPLING_INTERVAL) {
					if (trackedPoints.size() >= MAX_TRACK_POINTS) break;
					trackedPoints.push_back(cv::Point2f(approxCurve[i].x, approxCurve[i].y));
				}
			}
		}
		// 进行光流跟踪
		else {
			std::vector<cv::Point2f> newPoints;
			std::vector<uchar> status;
			std::vector<float> err;
			cv::TermCriteria criteria = cv::TermCriteria(
				cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
				20, 0.03
			);

			// 执行金字塔LK光流法
			cv::calcOpticalFlowPyrLK(
				prevGray, gray,
				trackedPoints, newPoints,
				status, err,
				cv::Size(25, 25),  // 增大搜索窗口
				3,                 // 金字塔层数
				criteria
			);

			// 运动一致性检查
			std::vector<cv::Point2f> goodPoints;
			std::vector<float> motions;

			// 计算平均运动向量
			cv::Point2f avgMotion(0, 0);
			int validCount = 0;
			for (size_t i = 0; i < trackedPoints.size(); i++) {
				if (status[i] && err[i] < 20.0) {
					cv::Point2f motion = newPoints[i] - trackedPoints[i];
					avgMotion += motion;
					validCount++;
				}
			}
			if (validCount > 0) avgMotion /= validCount;

			// 过滤异常点
			for (size_t i = 0; i < trackedPoints.size(); i++) {
				if (status[i] && err[i] < 20.0) {
					cv::Point2f motion = newPoints[i] - trackedPoints[i];
					float motionDist = cv::norm(motion - avgMotion);

					// 接收运动方向一致的点和静止点
					if (motionDist < 5.0 || cv::norm(motion) < 0.5) {
						goodPoints.push_back(newPoints[i]);
					}
				}
			}

			// 运动平滑处理 (使用历史数据)
			motionHistory.push_back(goodPoints);
			if (motionHistory.size() > MOTION_HISTORY) {
				motionHistory.pop_front();
			}

			// 应用加权平均
			trackedPoints.clear();
			if (!motionHistory.empty()) {
				size_t historySize = motionHistory.size();
				for (size_t i = 0; i < goodPoints.size(); i++) {
					cv::Point2f smoothedPoint(0, 0);
					float weightSum = 0;
					for (int h = 0; h < historySize; h++) {
						float weight = (h + 1) / static_cast<float>(historySize);
						if (i < motionHistory[h].size()) {
							smoothedPoint += motionHistory[h][i] * weight;
							weightSum += weight;
						}
					}
					if (weightSum > 0) {
						smoothedPoint /= weightSum;
						trackedPoints.push_back(smoothedPoint);
					}
				}
			}
			else {
				trackedPoints = goodPoints;
			}

			// 可视化处理
			cv::Scalar lineColor(0, 200, 0);   // 亮绿色
			cv::Scalar pointColor(0, 0, 255);  // 红色

			// 绘制点之间的连线
			for (size_t i = 1; i < trackedPoints.size(); i++) {
				cv::line(result, trackedPoints[i - 1], trackedPoints[i], lineColor, 2, cv::LINE_AA);
			}

			// 绘制跟踪点
			for (const auto& pt : trackedPoints) {
				cv::circle(result, pt, 4, pointColor, -1);
			}
		}

		// 更新前一帧图像
		prevGray = gray.clone();



		QImage resultImg = cvMatToQImage(result);
		mainDisplay->setPixmap(
			QPixmap::fromImage(resultImg));

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

	cv::Canny(gray, edge, lowThreshold, highThreshold);

	return edge;
}

