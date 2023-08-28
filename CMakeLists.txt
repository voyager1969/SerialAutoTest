cmake_minimum_required(VERSION 3.7)
project(ComAutoTest)
# 指定c++标准的版本
set(CMAKE_CXX_STANDARD 14)

# 设置Qt5的cmake模块所在目录，如果不设置将使用系统提供的版本C:\Qt\Qt5.7.1\5.7\mingw53_32\lib\cmake
# QT_DIR和QT_VERSION是指定了qt安装目录和版本的环境变量C:\msys64\mingw32\lib\cmake
set(CMAKE_PREFIX_PATH "D:\\msys64\\mingw64\\lib\\cmake")

#设置工程包含当前目录，非必须
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#打开全局moc,设置自动生成moc文件，一定要设置
set(CMAKE_AUTOMOC ON)
#打开全局uic，非必须
set(CMAKE_AUTOUIC ON)
#打开全局rcc，非必须，如需打开，注意修改qrc文件名
set(CMAKE_AUTORCC ON)

# Add compiler flags for building executables (-fPIE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

#设置运行时输出可执行文件目录(CMAKE源目录CMAKE_CURRENT_SOURCE_DIR，执行目录CMAKE_CURRENT_BINARY_DIR)set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)#设置运行时输出共享库文件目录SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)#查找需要的Qt库文件，最好每一个库都要写，Qt也会根据依赖关系自动添加
MESSAGE("======Searching for Qt5======")
set(Projects_QT5_COMPONENTS
        Core
        Gui
        Widgets
        NetWork
        SerialPort
        )
set(Projects_QT5_Includes
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        )
set(Projects_QT5_Libraries
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::SerialPort
        )
find_package(Qt5 COMPONENTS ${Projects_QT5_COMPONENTS} REQUIRED)
MESSAGE(${Qt5Widgets_INCLUDE_DIRS})
MESSAGE(${Projects_QT5_Libraries})


MESSAGE("======Set Include Dir.======")

#设置项目包含目录，包括将第三方库附加
INCLUDE_DIRECTORIES(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Projects_QT5_Includes}
        ${QWT_INCLUDE_DIR}
        ${OpenCV_INCLUDE_DIRS}
)



# 将第三方库附加到变量Projects_Extra_Libraries中，便于管理
list(APPEND Projects_Extra_Libraries
        ${QWT_LIBRARY}
        ${QNetWork_LIBRARY}
        )
# 等价于VS下 属性-<配置属性-<C/C++-<预处理器，预处理器定义：QWT_DLL LOG4QT_DLL
ADD_DEFINITIONS(-DQWT_DLL)

#查找当前文件夹中的所有源代码文件，也可以通过Set命令将所有文件设置为一个变量
FILE(GLOB SRC_FILES "./*.cpp")
#查找设置当前文件夹中所有的头文件
FILE(GLOB HEAD_FILES "./*.h")
#查找设置当前文件夹中所有的ui文件
FILE(GLOB UI_FILES "./*.ui")

#通过Ui文件生成对应的头文件，一定要添加
#qt5_wrap_ui(WRAP_FILES ${UI_FILES})

#添加资源文件，非必须，一旦采用，注意修改相应的qrc文件名
set(RCC_FILES ComAutoTest.qrc ComAutoTest.rc)

#将ui文件和生成文件整理在一个文件夹中，非必须
# source_group("Ui" FILES ${UI_FILES} ${WRAP_FILES} )

#创建工程文件

add_executable(${PROJECT_NAME} WIN32
        ${SRC_FILES}
        ${HEAD_FILES}
        ${RCC_FILES})

#添加Qt5依赖项,添加qwt等第三方依赖项
target_link_libraries(${PROJECT_NAME}
        ${Projects_QT5_Libraries}
        ${Projects_Extra_Libraries}
        )