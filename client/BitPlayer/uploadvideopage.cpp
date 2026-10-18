#include "uploadvideopage.h"
#include "ui_uploadvideopage.h"
#include "bitplayer.h"
#include "util.h"
#include <QFileDialog>
#include "./model/datacenter.h"
#include "util.h"
#include "./mpv/mpvplayer.h"
#include "toast.h"

UploadVideoPage::UploadVideoPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UploadVideoPage)
{
    ui->setupUi(this);

    // 获取所有分类数据
    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    ui->kinds->addItems(kindAndTagPtr->getAllKinds());
    ui->kinds->setCurrentIndex(-1);

    // 默认情况下，将上传视频的图标隐藏，待视频上传成功之后再显示出来
    ui->downIcon->hide();

    connect(ui->commitBtn, &QPushButton::clicked, this, &UploadVideoPage::onCommitBtnClicked);
    connect(ui->videoTittle, &QLineEdit::textChanged, this, &UploadVideoPage::onLineEditTextChanged);
    connect(ui->plainTextEdit, &QPlainTextEdit::textChanged, this, &UploadVideoPage::onPlainEditTextChaged);
    connect(ui->changeButton, &QPushButton::clicked, this, &UploadVideoPage::onChangeButtonClicked);
    connect(ui->kinds, &QComboBox::currentTextChanged, this, &UploadVideoPage::onUpdateTags);

    // 上传视频完成
    connect(dataCenter, &model::DataCenter::uploadVideoDone, this, &UploadVideoPage::onUploadVideoDone);
    // 上传图片完成
    connect(dataCenter, &model::DataCenter::uploadPhotoDone, this, [=](const QString& photoId, QWidget* whichPage){
        if(whichPage != ui->imageLabel){
            return;
        }
        coverImageId = photoId;
        isUploadPhotoOK = true;
    });

    // 上传视频信息成功
    connect(dataCenter, &model::DataCenter::uploadVideoDescDone, this, [=](){
        // 清空本次上传视频时在界面上的数据
        resetPage();
        // 切换回我的页面
        emit switchMyselfPage(MyselfPage);
    });
}

UploadVideoPage::~UploadVideoPage()
{
    delete ui;
}

void UploadVideoPage::setVideoTittle(const QString &videoPath)
{
    this->videoPath = videoPath;
    // 从文件路径中截取出文件的名称
    // D:/work/abc/111.mp4
    int start = videoPath.lastIndexOf('/')+1;
    QString videoTitle = videoPath.mid(start);
    ui->videoTittle->setText(videoTitle);
    ui->fileName->setText(videoTitle);
}

void UploadVideoPage::onCommitBtnClicked()
{
    if(!isUploadVideoOK){
        Toast::showMessage("等待视频上传完成");
        return;
    }

    if(!isUploadPhotoOK){
        Toast::showMessage("等待视频封面图上传完成");
        return;
    }

    if(!isDurationOK){
        Toast::showMessage("等待视频总时长获取完成");
        return;
    }

    // 将页面中的视频信息上传到服务器
    if(isUploadVideoOK && isUploadPhotoOK && isDurationOK){
        // 构建视频上传信息结构体
        model::VideoDesc videoDesc;
        videoDesc.videoFileId = videoFileId;
        videoDesc.photoFileId = coverImageId;
        videoDesc.videoTitle = ui->videoTittle->text();
        videoDesc.videoDesc = ui->plainTextEdit->toPlainText();
        videoDesc.kind = ui->kinds->currentText();
        videoDesc.duration = duration;

        QList<QPushButton*> tagBtns = ui->tagContent->findChildren<QPushButton*>();
        for(auto& tagBtn : tagBtns){
            if(tagBtn->isChecked()){
                QString tagText = tagBtn->text();
                videoDesc.tags.append(tagText);
            }
        }

        if(videoDesc.tags.size() > 5){
            Toast::showMessage("最多只能选择5个标签");
            return;
        }

        // 给服务器发送上传视频信息请求
        auto dataCenter = model::DataCenter::getInstance();
        dataCenter->uploadVideoDescAsync(videoDesc);
    }
}

void UploadVideoPage::onLineEditTextChanged(const QString &text)
{
    // 先要从wordLeft中获取最大的输入字数
    QString wordLleftText = ui->wordLeft->text();
    // 6/80
    int linePos = wordLleftText.indexOf('/');
    int totalCount = wordLleftText.mid(linePos+1).toInt();

    // 检测text中的字数是否超过wordLeft，如果没有超过，将text的长度和最大字数拼接起来
    int wordCount = text.length();
    if(wordCount <= totalCount){
        ui->wordLeft->setText(QString::number(wordCount) + "/" + QString::number(totalCount));
    }else{
        LOG()<<"视频标题超过80个字";
        ui->videoTittle->setText(text.mid(0, 80));
    }
}

void UploadVideoPage::onPlainEditTextChaged()
{
    // 获取QPlainTextEdit中的文本
    QString text = ui->plainTextEdit->toPlainText();
    int wordCount = text.length();

    // 获取最大字数
    // 10/1000
    QString wordLeft = ui->briefLeftWord->text();
    int linePos = wordLeft.indexOf('/');
    QString totalCount = wordLeft.mid(linePos+1);

    // 检测实际输入文本长度是否超过最大长度
    if(wordCount <= totalCount.toInt()){
        ui->briefLeftWord->setText(QString::number(wordCount) + "/" + totalCount);
    }else{
        ui->plainTextEdit->setPlainText(text.mid(0, totalCount.toInt()));

        // 设置滚动条一直在底部
        // textCursor: 获取当前光标的位置
        // movePosition: 移动光标的位置
        // QTextCursor::End 将光标移动到文本的末尾
        // QTextCursor::MoveAnchor 光标在移动时，光标的位置是固定的，不会创建一个选取
        QTextCursor textCursor =  ui->plainTextEdit->textCursor();
        textCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->plainTextEdit->setTextCursor(textCursor);
    }
}

void UploadVideoPage::onChangeButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, "选择视频封面图", "", "Images (*.png *jpg)");
    if(fileName.isEmpty()){
        LOG()<<"用户取消封面图的选择";
        return;
    }

    QPixmap pixmap(fileName);
    pixmap = pixmap.scaled(ui->imageLabel->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui->imageLabel->setPixmap(pixmap);

    // 将用户选择的图片作为视频的封面图，同时将其上传到服务器
    uploadPhoto(fileName);
}

void UploadVideoPage::onUpdateTags(const QString &kind)
{
    addTagsByKind(kind);
}

void UploadVideoPage::onUploadVideoDone(const QString &videoId)
{
    ui->uploadProgress->setText("上传完成");
    ui->downIcon->show();
    this->videoFileId = videoId;
    isUploadVideoOK = true;

    // 设置视频首帧
    QString firstFrame = MpvPlayer::getVideoFirstFrame(videoPath);
    if(firstFrame.isEmpty()){
        LOG()<<"获取视频首帧失败";
        return;
    }

    // 将视频首帧作为视频封面图
    QPixmap pixmap(firstFrame);
    pixmap = pixmap.scaled(ui->imageLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    ui->imageLabel->setPixmap(pixmap);

    // 将封面图上传服务器
    uploadPhoto(firstFrame);
    QFile::remove(firstFrame);

    // 获取视频总时长
    // 不需要让视频渲染出来，即不需要传视频渲染窗口
    LOG()<<"获取视频总时长打印"<<videoPath;
    mpvPlayer = new MpvPlayer();
    connect(mpvPlayer, &MpvPlayer::durationChanged, this, &UploadVideoPage::getDurationDone, Qt::UniqueConnection);
    mpvPlayer->startPlay(videoPath);
    mpvPlayer->pause();
}

void UploadVideoPage::getDurationDone(int64_t duration)
{
    LOG()<<duration;
    this->duration = duration;
    isDurationOK = true;
    disconnect(mpvPlayer, &MpvPlayer::durationChanged, nullptr, nullptr);
    delete mpvPlayer;
    mpvPlayer = nullptr;
}

void UploadVideoPage::uploadPhoto(const QString &photoPath)
{
    // 读取图片数据，并将数据发送给服务器
    QByteArray imageData = loadFileToByteArray(photoPath);
    if(imageData.isEmpty()){
        LOG()<<"读取封面图失败!!!";
        return;
    }

    auto dataCenter = model::DataCenter::getInstance();
    dataCenter->uploadPhotoAsync(imageData, ui->imageLabel);
}

void UploadVideoPage::resetPage()
{
    ui->fileName->setText("【这里是文件名】直接取源文件名称即可");
    ui->uploadProgress->setText("上传中...");
    ui->downIcon->hide();

    ui->videoTittle->setText("");
    ui->plainTextEdit->setPlainText("");
    ui->imageLabel->setStyleSheet("#imageLabel{border-image : url(:/images/uploadVideoPage/fenmian.png);}");

    // 分类和标签恢复到默认值
    ui->kinds->setCurrentIndex(-1);
    addTagsByKind("");

    videoPath = "";
    isUploadVideoOK = false;
    isUploadPhotoOK = false;
    isDurationOK = false;
    videoFileId = "";
    coverImageId = "";
    duration = 0;
}

void UploadVideoPage::addTagsByKind(const QString &kind)
{
    // 1. 清除掉之前的数据
    // 获取layout中的元素，将其删除掉
    QList<QPushButton*> tagBtnList = ui->tagWidget->findChildren<QPushButton*>();
    for(auto& tagBtn : tagBtnList){
        // removeWidget：只是将元素从布局器中移除了，并不会真正的删除按钮
        ui->tagLayout->removeWidget(tagBtn);
        delete tagBtn;
    }

    // 获取到空白间距并将其删除掉
    QLayoutItem* spaceItem = ui->tagLayout->itemAt(ui->tagLayout->count() - 1);
    ui->tagLayout->removeItem(spaceItem);

    // 2. 获取当前分类对应的标签
    if(kind.isEmpty()){
        return;
    }

    auto dataCenter = model::DataCenter::getInstance();
    auto kindAndTagPtr = dataCenter->getKindAndTagClassPtr();
    auto tags = kindAndTagPtr->getTagsByKind(kind).keys();

    // 3. 添加标签按钮到layout中
    for(auto& tag : tags){
        QPushButton* tagBtn = new QPushButton(ui->tagWidget);
        tagBtn->setFixedSize(98, 49);
        tagBtn->setText(tag);
        tagBtn->setCheckable(true);   // 设置按钮的状态有两种：选中和未选中状态
        // QPushButton::checked  当前按钮被选中
        // QPushButton::unchecked 当前按钮未选中
        tagBtn->setStyleSheet("QPushButton{"
                              "border : 1px solid #3ECEFE;"
                              "border-radius : 4px;"
                              "color : #3ECEFE;"
                              "font-family : 微软雅黑;"
                              "font-size : 16px;"
                              "background-color : #FFFFFF;}"
                              "QPushButton:checked{"
                              "background-color : #3ECEFE;"
                              "color : #FFFFFF;}"
                              "QPushButton:unchecked{"
                              "background-color : #FFFFFF;"
                              "color : #3ECEFE;}");
        ui->tagLayout->addWidget(tagBtn);
    }

    // 在tagLayout的最后，添加一个空白间距，将按钮挤到左侧去
    ui->tagLayout->insertSpacing(tags.size(), ui->tagWidget->width() - (98+20)*tags.size());
    ui->tagLayout->setSpacing(20);
}



