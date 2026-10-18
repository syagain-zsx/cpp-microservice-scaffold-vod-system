cmake_minimum_required(VERSION 3.16)

project(BitPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets network)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
)

set(PROJECT_SOURCES
        main.cpp
        bitplayer.cpp
        bitplayer.h
        bitplayer.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(BitPlayer
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
        mpv/mpv/render_gl.h mpv/mpv/stream_cb.h mpv/client.h mpv/render.h mpv/render_gl.h mpv/stream_cb.h
        mpv/mpvplayer.h mpv/mpvplayer.cpp
        bulletscreenitem.h bulletscreenitem.cpp
        model/datacenter.h model/datacenter.cpp
        model/data.h
        model/data.cpp
        test/testJson.h
        netclient/netclient.h netclient/netclient.cpp
        test/TestHttp.h
        confirmdialog.h confirmdialog.cpp
        BitPlayer.rc










    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BitPlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BitPlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BitPlayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(BitPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set(MPV_DLL ${CMAKE_CURRENT_SOURCE_DIR}/mpv/dll/libmpv-2.dll) #设置mpv库目录
target_link_libraries(BitPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${MPV_DLL} Qt6::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BitPlayer)
endif()
set_target_properties(BitPlayer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BitPlayer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BitPlayer)
endif()
