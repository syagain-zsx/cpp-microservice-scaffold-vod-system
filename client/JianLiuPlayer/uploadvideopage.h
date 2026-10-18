#ifndef UPLOADVIDEOPAGE_H
#define UPLOADVIDEOPAGE_H

#include <QWidget>
#include "./mpv/mpvplayer.h"

namespace Ui {
class UploadVideoPage;
}

class UploadVideoPage : public QWidget
{
    Q_OBJECT

public:
    explicit UploadVideoPage(QWidget *parent = nullptr);
    ~UploadVideoPage();
    // 设置上传文件的名称
    void setVideoTittle(const QString& videoPath);
    //  重置界面数据
    void resetPage();

private slots:
    void onCommitBtnClicked();
    void onLineEditTextChanged(const QString& text);
    void onPlainEditTextChaged();
    void onChangeButtonClicked();
    // QComboBox中选择项发生变化时对应的槽函数
    void onUpdateTags(const QString& kind);
    // 上传视频完成
    void onUploadVideoDone(const QString& videoId);
    // 获取视频总时长完成
    void getDurationDone(int64_t duration);

private:
    void addTagsByKind(const QString& kind);
    // 上传视频封面图
    void uploadPhoto(const QString& photoPath);


signals:
    void switchMyselfPage(int pageId);
private:
    Ui::UploadVideoPage *ui;
    QString videoPath;
    QString videoFileId;
    bool isUploadVideoOK = false;
    QString coverImageId;
    bool isUploadPhotoOK = false;
    MpvPlayer* mpvPlayer = nullptr;
    int64_t duration;
    bool isDurationOK = false;
};

#endif // UPLOADVIDEOPAGE_H
