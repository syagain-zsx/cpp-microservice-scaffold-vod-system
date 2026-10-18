#ifndef CHECKTABLEITEM_H
#define CHECKTABLEITEM_H

#include <QWidget>
#include "./model/data.h"
#include "playerpage.h"

namespace Ui {
class CheckTableItem;
}

class CheckTableItem : public QWidget
{
    Q_OBJECT

public:
    explicit CheckTableItem(QWidget *parent, const model::VideoInfo& videoInfo);
    ~CheckTableItem();
private:
    void initStyleSheet();

private slots:
    // 视频封面按钮点击
    void onVideoBtnClicked();
    // 操作按钮点击
    void onOperatorBtnClicked();
    // 操作按钮2点击
    void onOperator2BtnClicked();

    // 下载用户头像成功
    void downloadUserAvatarSuccess(const QString& userId, QByteArray userAvatar);

private:
    Ui::CheckTableItem *ui;
    model::VideoInfo videoInfo;
    QHash<QString, QString> styleSheet;
    PlayerPage* playerPage;
    QPixmap userAvatar;
};

#endif // CHECKTABLEITEM_H
