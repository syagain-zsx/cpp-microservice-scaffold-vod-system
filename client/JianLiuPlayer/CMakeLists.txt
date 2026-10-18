cmake_minimum_required(VERSION 3.16)

project(JianLiuPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
)

set(PROJECT_SOURCES
        main.cpp
        jianliuplayer.cpp
        jianliuplayer.h
        jianliuplayer.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(JianLiuPlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        iamgeRes.qrc
        startuppage.h startuppage.cpp
        pageswitchbutton.h pageswitchbutton.cpp
        util.h
        homepagewidget.h homepagewidget.cpp homepagewidget.ui
        searchlineedit.h searchlineedit.cpp
        videobox.h videobox.cpp videobox.ui
        playerpage.h playerpage.cpp playerpage.ui
        playslider.h playslider.cpp playslider.ui
        volume.h volume.cpp volume.ui
        playspeed.h playspeed.cpp playspeed.ui
        barrageedit.h barrageedit.cpp
        myselfwidget.h myselfwidget.cpp myselfwidget.ui
        avatarbutton.h avatarbutton.cpp
        modifymyselfdialog.h modifymyselfdialog.cpp modifymyselfdialog.ui
        newpassworddialog.h newpassworddialog.cpp newpassworddialog.ui
        uploadvideopage.h uploadvideopage.cpp uploadvideopage.ui
        adminwidget.h adminwidget.cpp adminwidget.ui
        checktable.h checktable.cpp checktable.ui
        roletable.h roletable.cpp roletable.ui
        checktableitem.h checktableitem.cpp checktableitem.ui
        roletableitem.h roletableitem.cpp roletableitem.ui
        editadmindialog.h editadmindialog.cpp editadmindialog.ui
        pagebutton.h pagebutton.cpp
        paginator.h paginator.cpp
        login.h login.cpp login.ui
        toast.h toast.cpp
        mpv/client.h mpv/render.h mpv/render_gl.h mpv/stream_cb.h
        mpv/mpvplayer.h mpv/mpvplayer.cpp
        bulletscreenitem.h bulletscreenitem.cpp
        model/datacenter.h model/datacenter.cpp
        model/data.h
        model/data.cpp
        netclient/netclient.h netclient/netclient.cpp
        confirmdialog.h confirmdialog.cpp
        JianLiuPlayer.rc










    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET JianLiuPlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(JianLiuPlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(JianLiuPlayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(JianLiuPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(WIN32)
    # Windows：直接链接 mpv/dll 下的导入库（libmpv-2.dll）
    set(MPV_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/mpv/dll/libmpv-2.dll)
    if(NOT EXISTS ${MPV_LIBRARY})
        message(FATAL_ERROR
            "缺少 libmpv：${MPV_LIBRARY}\n"
            "请从 https://sourceforge.net/projects/mpv-player-windows/files/libmpv/ 下载 libmpv-2.dll，"
            "放到 client/JianLiuPlayer/mpv/dll/ 后重新配置。")
    endif()
else()
    # Linux/macOS：使用系统或私有前缀的 libmpv（便于在 Linux 上做编译检查）
    find_library(MPV_LIBRARY NAMES mpv)
    if(NOT MPV_LIBRARY)
        message(FATAL_ERROR "未找到 libmpv：Linux 请安装 libmpv-dev，Windows 请把 libmpv-2.dll 放到 mpv/dll/")
    endif()
endif()
target_link_libraries(JianLiuPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${MPV_LIBRARY} Qt6::Network)

# Windows：构建后把 libmpv-2.dll 复制到可执行文件目录，避免运行时报「找不到 libmpv-2.dll」
if(WIN32)
    add_custom_command(TARGET JianLiuPlayer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MPV_LIBRARY}" "$<TARGET_FILE_DIR:JianLiuPlayer>"
        COMMENT "复制 libmpv-2.dll 到输出目录")
endif()

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.JianLiuPlayer)
endif()
set_target_properties(JianLiuPlayer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS JianLiuPlayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(JianLiuPlayer)
endif()
