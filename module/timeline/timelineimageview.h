#ifndef TIMELINEIMAGEVIEW_H
#define TIMELINEIMAGEVIEW_H

#include "sliderframe.h"
#include "toptimelinetips.h"
#include <QMap>
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class TimelineViewFrame;

class TimelineImageView : public QScrollArea
{
    Q_OBJECT
public:
    explicit TimelineImageView(QWidget *parent = 0);
    void setIconSize(const QSize &iconSize);
    QStringList selectedImages();
    QString currentMonth();

protected:
    void resizeEvent(QResizeEvent *e);

private:
    void initSliderFrame();
    void initTopTips();
    void initContents();

    void inserFrame(const QString &timeline);
    void removeFrame(const QString &timeline);
    void updateSliderFrmaeRect();
    void updateContentRect();
    void updateTopTipsRect();
    int getMinContentsWidth();
    QString currentTimeline();
    QString getMonthByTimeline(const QString &timeline);
    double scrollingPercent();

private:
    SliderFrame *m_sliderFrame;
    TopTimelineTips *m_topTips;
    QVBoxLayout *m_contentLayout;
    QFrame *m_contentFrame;
    QMap<QString, TimelineViewFrame *> m_frames;
    bool m_ascending;
    QSize m_iconSize;
};

#endif // TIMELINEIMAGEVIEW_H