cmake_minimum_required(VERSION 3.15)
project(MyQtApp VERSION 1.0 LANGUAGES CXX)

# 设置Qt路径 (根据实际路径修改)
set(Qt6_DIR "D:/lzq/QT/6.7.0/msvc2019_64/lib/cmake/Qt6")
set(OpenCV_DIR "D:/lzq/opencv/build/x64/vc16/lib") 

# 自动处理Qt元对象编译
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt模块
find_package(Qt6 COMPONENTS 
    Core 
    Widgets 
    Multimedia 
    MultimediaWidgets 
    REQUIRED
)
# 查opencv
find_package(OpenCV REQUIRED)

# 包含头文件
include_directories(${OpenCV_INCLUDE_DIRS})

# 添加可执行文件
add_executable(${PROJECT_NAME}
    main.cpp
    MainWindow/MainWindow.h
    MainWindow/MainWindow.cpp
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    PRIVATE 
        Qt6::Core 
        Qt6::Widgets
        Qt6::Multimedia         
        Qt6::MultimediaWidgets
        ${OpenCV_LIBS}
)


# 包含当前目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${Qt6MultimediaWidgets_INCLUDE_DIRS}
    ${Qt6Multimedia_INCLUDE_DIRS}
)