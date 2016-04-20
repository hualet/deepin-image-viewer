#include "EditPanel.h"
#include <dimagebutton.h>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QDebug>
#include <QStackedWidget>
#include <dimagebutton.h>
#include <dtextbutton.h>
#include "controller/signalmanager.h"
#include "filters/FilterObj.h"
#include "FilterSetup.h"
#include "Cut.h"

using namespace Dtk::Widget;

EditPanel::EditPanel(QWidget *parent)
    : ModulePanel(parent)
{
    m_stack = new QStackedWidget(this); // why need a parent to avoid cutwidget crash?
    connect(SignalManager::instance(), &SignalManager::editImage, this, &EditPanel::openImage);
    m_view = new ImageWidget();
    m_cut = new CutWidget();
    m_stack->addWidget(m_view);
    m_stack->addWidget(m_cut);
    m_stack->setCurrentWidget(m_view);
    QHBoxLayout *hl = new QHBoxLayout();
    setLayout(hl);
    hl->addWidget(m_stack);
}

QWidget *EditPanel::toolbarBottomContent()
{
    if (m_stack->currentWidget() == m_view)
        return NULL;
    struct {
        qreal value;
        const char* text;
    } ratios[] = {
    {1.0, "1:1"},
    {4.0/3.0, "4:3"},
    {8.0/5.0, "8:5"},
    {16.0/9.0, "16:9"},
    {3.0/4.0, "3:4"},
    {5.0/8.0, "5:8"},
    {9.0/16.0, "9:16"},
    {0, "Custom"},
    {-1, NULL}
    };
    QWidget *w = new QWidget();    
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    w->setLayout(hb);
    QButtonGroup *bg = new QButtonGroup(w);
    connect(bg, (void (QButtonGroup::*)(int))&QButtonGroup::buttonClicked, [=](int id){
        m_cut->setAspectRatio(ratios[id].value);
    });
    for (int i = 0; ratios[i].text; ++i) {
        DTextButton *btn = new DTextButton(ratios[i].text);
        btn->setCheckable(true);
        btn->setChecked(ratios[i].value == 0);
        hb->addWidget(btn);
        bg->addButton(btn, i);
    }
    hb->addStretch();

    DTextButton *btn = new DTextButton(tr("Cancel"));
    hb->addWidget(btn);
    connect(btn, &DTextButton::clicked, [this](){
        m_stack->setCurrentWidget(m_view);
    });
    btn = new DTextButton(tr("Cut"));
    hb->addWidget(btn);
    connect(btn, &DTextButton::clicked, [&](){
        m_cut->setImage(m_image.copy(m_cut->cutRect()));
    });

    connect(m_stack, &QStackedWidget::currentChanged, [&](int){
        if (m_stack->currentWidget() == m_cut)
            SignalManager::instance()->showBottomToolbar();
        else
            SignalManager::instance()->hideBottomToolbar();
    });
    return w;
}

QWidget *EditPanel::toolbarTopLeftContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(0);
    w->setLayout(hb);
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/previous-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/previous-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/previous-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, SignalManager::instance(), &SignalManager::backToMainWindow);
    DTextButton *btn1 = new DTextButton(tr("Back"));
    hb->addWidget(btn1);
    connect(btn1, &DTextButton::clicked, SignalManager::instance(), &SignalManager::backToMainWindow);
    btn1 = new DTextButton(tr("Revert"));
    connect(btn1, &DTextButton::clicked, [this](){
        if (m_stack->currentWidget() == m_view)
            m_view->setImage(m_image);
        else
            m_cut->setImage(m_image);
    });
    hb->addWidget(btn1);
    hb->addStretch();
    return w;
}

QWidget *EditPanel::toolbarTopMiddleContent()
{
    QWidget *w = new QWidget();
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(0, 0, 0, 0);
    hb->setSpacing(10);
    w->setLayout(hb);
    hb->addStretch();
    DImageButton *btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/contrarotate-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/contrarotate-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/contrarotate-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateAntiClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/clockwise-rotation-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/clockwise-rotation-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/clockwise-rotation-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::rotateClockWise);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/filter-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/filter-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/filter-active.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        m_stack->setCurrentWidget(m_view);
        if (m_filterSetup) {
            if (m_filterSetup->imagePath() != m_path) {
                m_filterSetup->setImage(m_path);
            }
            m_filterSetup->resize(240, height() - 30);
        }

        Q_EMIT SignalManager::instance()->showExtensionPanel();
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/cutting-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/cutting-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/cutting-active.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, [this](){
        if (m_stack->currentWidget() == m_view) {
            m_cut->setImage(m_image);
            m_stack->setCurrentWidget(m_cut);
        } else {
            m_stack->setCurrentWidget(m_view);
        }
        Q_EMIT SignalManager::instance()->hideExtensionPanel();
        Q_EMIT SignalManager::instance()->updateBottomToolbarContent(toolbarBottomContent());
    });

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/flip-horizontal-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/flip-horizontal-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/flip-horizontal-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::flipX);

    btn = new DImageButton();
    btn->setNormalPic(":/images/icons/resources/images/icons/flip-vertical-normal.png");
    btn->setHoverPic(":/images/icons/resources/images/icons/flip-vertical-hover.png");
    btn->setPressPic(":/images/icons/resources/images/icons/flip-vertical-press.png");
    hb->addWidget(btn);
    connect(btn, &DImageButton::clicked, m_view, &ImageWidget::flipY);


    hb->addStretch();
    return w;
}

QWidget *EditPanel::extensionPanelContent()
{
    m_filterSetup = new FilterSetup();
    connect(m_filterSetup, &FilterSetup::filterIdChanged, this, &EditPanel::setFilterId);
    connect(m_filterSetup, &FilterSetup::filterIntensityChanged, this, &EditPanel::setFilterIntensity);
    if (m_filterSetup->imagePath() != m_path) {
        m_filterSetup->setImage(m_path);
    }
    m_filterSetup->resize(240, height() - 30);
    return m_filterSetup;
}

void EditPanel::setFilterId(int value)
{
    if (m_filterId == value)
        return;
    m_filterId = value;
    if (m_filter) {
        delete m_filter;
        m_filter = 0;
    }
    qDebug("filter id: %d", m_filterId);
    m_filter = filter2d::FilterObj::create(m_filterId);
    applyFilter();
}

void EditPanel::setFilterIntensity(qreal value)
{
    if (m_filterIntensity == value)
        return;
    m_filterIntensity = value;
    applyFilter();
}

void EditPanel::applyFilter()
{
    if (!m_filter)
        return;
    m_filter->setProperty("brightness", 0.6);
    m_filter->setProperty("hue", 0.6);
    m_filter->setProperty("contrast", 0.6);
    m_filter->setProperty("saturation", 0.6);
    qDebug("set intensity: %.3f", m_filterIntensity);
    m_filter->setIntensity(m_filterIntensity);
    QImage img(m_image);
    if (img.isNull())
        return;
    m_view->setImage(m_filter->apply(img));
}

void EditPanel::openImage(const QString &path)
{
    m_path = path;
    m_image = QImage(path);
    Q_EMIT SignalManager::instance()->gotoPanel(this);
    m_stack->setCurrentWidget(m_view);
    m_view->setImage(m_image);
}