#include <QDebug>
#include <QStandardPaths>
#include <QScrollBar>

#include <anchors.h>

#include "dcrumbwidget.h"
#include "dcrumbbutton.h"
#include "windowmanager.h"
#include "dstatebutton.h"

#include "../app/fmevent.h"
#include "../app/global.h"

#include "../controllers/pathmanager.h"

#include "../deviceinfo/udiskdeviceinfo.h"
#include "../deviceinfo/udisklistener.h"

#include "widgets/singleton.h"

DCrumbWidget::DCrumbWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

void DCrumbWidget::initUI()
{
    Anchors<QWidget> background_widget(new QWidget(this));

    background_widget.setFill(this);
    background_widget->setObjectName("DCrumbBackgroundWidget");

    m_homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last();
    createArrows();
    m_listWidget = new ListWidgetPrivate(this);
    m_listWidget->setObjectName("DCrumbList");
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->addWidget(m_leftArrow);
    m_buttonLayout->addWidget(m_listWidget);
    m_buttonLayout->addWidget(m_rightArrow);
    m_buttonLayout->setContentsMargins(0,0,0,0);
    m_buttonLayout->setSpacing(0);
    setLayout(m_buttonLayout);
    setObjectName("DCrumbWidget");
    m_listWidget->setFlow(QListWidget::LeftToRight);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    setFixedHeight(24);
    setMinimumWidth(50);
}

void DCrumbWidget::addCrumb(const QStringList &list)
{
    qDebug() << list;
    for(int i = 0; i < list.size(); i++)
    {
        QString text = list.at(i);
        DCrumbButton * button;
        if(isHomeFolder(text)){
            button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/home_normal_16px.svg"),
                    QIcon(":/icons/images/icons/home_hover_16px.svg"),
                    QIcon(":/icons/images/icons/home_checked_16px.svg"),
                    text, this);
        }else if(isDeviceFolder(text)){
            UDiskDeviceInfo* info = deviceListener->getDeviceByPath(text);
            if (info->getMediaType() == UDiskDeviceInfo::camera && info->getName() == "iPhone"){
                button = createDeviceCrumbButtonByType(UDiskDeviceInfo::iphone, text);
            }else{
                button = createDeviceCrumbButtonByType(info->getMediaType(), text);
            }
        }
        else{
            button = new DCrumbButton(m_group.buttons().size(), text, this);
        }

        if (button){
            QString path = list.at(0);
            if(path == "/")
                path = "";
            for(int j = 1; j <= i; j++)
            {
                path += "/" + list.at(j);
            }

            if (!path.startsWith("/"))
                path.prepend('/');

            button->setPath(path);

            if (systemPathManager->systemPathsMap().values().contains(path)){
                foreach (QString key, systemPathManager->systemPathsMap().keys()) {
                    if (systemPathManager->systemPathsMap().value(key) == path){
                           button->setText(systemPathManager->getSystemPathDisplayName(key));
                    }
                }
            }

            button->setFocusPolicy(Qt::NoFocus);
            button->adjustSize();
            m_group.addButton(button, button->getIndex());
            connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
        }
        if (i == 0){
            if (button){
                button->setObjectName("DCrumbIconButton");
            }
        }
    }
    m_group.buttons().last()->setChecked(true);
}

void DCrumbWidget::setCrumb(const DUrl &path)
{
    qDebug() << path;
    if(path.isSearchFile())
        return;
    m_url = path;
    m_needArrows = false;
    clear();
    if(path.isRecentFile())
    {
        addRecentCrumb();
    }
    else if(path.isComputerFile())
    {
        addComputerCrumb();
    }
    else if(path.isTrashFile())
    {
        if (path.path().isEmpty()){
            addTrashCrumb();
        }else{
            addTrashCrumb();
            addLocalCrumbs(path);
        }
    }else if(path.isSMBFile()){
        addNetworkCrumb();
        addCrumb(QStringList() << path.toString());
    }else if(path.isNetWorkFile()){
        addNetworkCrumb();
    }
    else
    {
        addLocalCrumbs(path);
    }
    createCrumbs();
    repaint();
}

void DCrumbWidget::clear()
{
    m_listWidget->clear();
    m_prevCheckedId = m_group.checkedId();
    qDeleteAll(m_group.buttons());
}

QString DCrumbWidget::path()
{
    return m_url.toLocalFile();
}

DUrl DCrumbWidget::getUrl()
{
    return m_url;
}

DUrl DCrumbWidget::getCurrentUrl()
{
    DUrl result;

    const DCrumbButton *button = qobject_cast<DCrumbButton*>(m_group.checkedButton());
    const QString &path = button ? button->path() : QString();
    if (m_url.isLocalFile()){
        result = DUrl::fromLocalFile(path);
    }else if (m_url.isTrashFile()){
        result = DUrl::fromTrashFile(path);
    }else if (m_url.isComputerFile()){
        result = DUrl::fromComputerFile("/");
    }
    else
    {
        result = m_url;
    }

    return result;
}

void DCrumbWidget::addRecentCrumb()
{
    QString text = RECENT_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/recent_normal_16px.svg"),
                QIcon(":/icons/images/icons/recent_hover_16px.svg"),
                QIcon(":/icons/images/icons/recent_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setPath("/");
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addComputerCrumb()
{
    QString text = COMPUTER_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/disk_normal_16px.svg"),
                QIcon(":/icons/images/icons/disk_hover_16px.svg"),
                QIcon(":/icons/images/icons/disk_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setPath("/");
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addTrashCrumb()
{
    QString text = TRASH_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/trash_normal_16px.svg"),
                QIcon(":/icons/images/icons/trash_hover_16px.svg"),
                QIcon(":/icons/images/icons/trash_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setPath("/");
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addHomeCrumb()
{
    QString text = m_homePath;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/home_normal_16px.svg"),
                QIcon(":/icons/images/icons/home_hover_16px.svg"),
                QIcon(":/icons/images/icons/home_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    m_group.addButton(button, button->getIndex());
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

void DCrumbWidget::addNetworkCrumb()
{
    QString text = NETWORK_ROOT;
    DCrumbButton * button = new DCrumbIconButton(
                m_group.buttons().size(),
                QIcon(":/icons/images/icons/network_normal_16px.svg"),
                QIcon(":/icons/images/icons/network_hover_16px.svg"),
                QIcon(":/icons/images/icons/network_checked_16px.svg"),
                text, this);
    button->setFocusPolicy(Qt::NoFocus);
    button->adjustSize();
    button->setPath("/");
    m_group.addButton(button, button->getIndex());
    button->setChecked(true);
    connect(button, &DCrumbButton::clicked, this, &DCrumbWidget::buttonPressed);
}

DCrumbButton *DCrumbWidget::createDeviceCrumbButtonByType(UDiskDeviceInfo::MediaType type, const QString &mountPoint)
{
    DCrumbButton * button = NULL;
    switch (type) {
    case UDiskDeviceInfo::native:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/disk_normal_16px.svg"),
                    QIcon(":/icons/images/icons/disk_hover_16px.svg"),
                    QIcon(":/icons/images/icons/disk_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::phone:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/android_normal_16px.svg"),
                    QIcon(":/icons/images/icons/android_hover_16px.svg"),
                    QIcon(":/icons/images/icons/android_checked_16px.svg"),
                    mountPoint, this);
        break;
    }case UDiskDeviceInfo::camera:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/android_normal_16px.svg"),
                    QIcon(":/icons/images/icons/android_hover_16px.svg"),
                    QIcon(":/icons/images/icons/android_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::iphone:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/iphone_normal_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_hover_16px.svg"),
                    QIcon(":/icons/images/icons/iphone_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::removable:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/usb_normal_16px.svg"),
                    QIcon(":/icons/images/icons/usb_hover_16px.svg"),
                    QIcon(":/icons/images/icons/usb_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    case UDiskDeviceInfo::network:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/network_normal_16px.svg"),
                    QIcon(":/icons/images/icons/network_hover_16px.svg"),
                    QIcon(":/icons/images/icons/network_checked_16px.svg"),
                    mountPoint, this);
        break;
    }case UDiskDeviceInfo::dvd:{
        button = new DCrumbIconButton(
                    m_group.buttons().size(),
                    QIcon(":/icons/images/icons/dvd_normal_16px.svg"),
                    QIcon(":/icons/images/icons/dvd_hover_16px.svg"),
                    QIcon(":/icons/images/icons/dvd_checked_16px.svg"),
                    mountPoint, this);
        break;
    }
    default:
        qWarning() << "unknown type";
        break;
    }

    return button;
}

void DCrumbWidget::addLocalCrumbs(const DUrl & url)
{
    QStringList list;
    QString path = url.path();
    qDebug() << path << isInHome(path) << isInDevice(path);
    if(isInHome(path))
    {
        QString tmpPath = url.toLocalFile();
        tmpPath.replace(m_homePath, "");
        list.append(tmpPath.split("/"));
        list.insert(0, m_homePath);
        list.removeAll("");
    }else if (url == DUrl(FILE_ROOT)){
        list.insert(0, "/");
    }else if(isInDevice(path)){
        UDiskDeviceInfo* info;
        if (deviceListener->isDeviceFolder(path)){
            info = deviceListener->getDeviceByPath(path);
        }else{
            info = deviceListener->getDeviceByFilePath(path);
        }
        if (info){
            QString mountPoint = info->getMountPointUrl().toLocalFile();
            qDebug() << mountPoint << info << info->getDiskInfo();
            QString tmpPath = url.toLocalFile();
            tmpPath.replace(mountPoint, "");
            list.append(tmpPath.split("/"));
            list.insert(0, mountPoint);
            list.removeAll("");
        }
    }
    else
    {
        list.append(path.split("/"));
        if(url.isLocalFile())
            list.replace(0, "/");
        list.removeAll("");
    }
    if (!list.isEmpty())
        addCrumb(list);
}

bool DCrumbWidget::hasPath(const QString &path)
{
    return m_url.toLocalFile().contains(path);
}

bool DCrumbWidget::isInHome(const QString& path)
{
    return DUrl::childrenList(DUrl(path)).contains(DUrl(m_homePath));
}

bool DCrumbWidget::isHomeFolder(const QString& path)
{
    return path == m_homePath;
}

bool DCrumbWidget::isInDevice(const QString& path)
{
    return deviceListener->isInDeviceFolder(path);
}

bool DCrumbWidget::isDeviceFolder(const QString &path)
{
    return deviceListener->isDeviceFolder(path);
}

bool DCrumbWidget::isRootFolder(QString path)
{
    return path == "/";
}

void DCrumbWidget::createCrumbs()
{
    m_crumbTotalLen = 0;
    m_items.clear();
    foreach(QAbstractButton * button, m_group.buttons())
    {
        QListWidgetItem * item = new QListWidgetItem(m_listWidget);
        item->setSizeHint(QSize(button->size().width(), 18));
        m_listWidget->setItemWidget(item, button);
        DCrumbButton * localButton = (DCrumbButton *)button;
        localButton->setItem(item);
        m_items.append(item);
        m_crumbTotalLen += button->size().width();
    }

    if (!m_items.isEmpty()){
        m_listWidget->scrollToItem(m_items.last(), QListWidget::PositionAtBottom);
        m_listWidget->setHorizontalScrollMode(QListWidget::ScrollPerPixel);
        m_listWidget->horizontalScrollBar()->setPageStep(m_listWidget->width());
        checkArrows();
        m_listWidget->scrollToItem(m_items.last(), QListWidget::PositionAtBottom);
    }
}

void DCrumbWidget::createArrows()
{
    m_leftArrow = new QPushButton();
    m_leftArrow->setObjectName("backButton");
    m_leftArrow->setFixedWidth(26);
    m_leftArrow->setFixedHeight(24);
    m_leftArrow->setFocusPolicy(Qt::NoFocus);

    m_rightArrow = new QPushButton();
    m_rightArrow->setObjectName("forwardButton");
    m_rightArrow->setFixedWidth(26);
    m_rightArrow->setFixedHeight(24);
    m_rightArrow->setFocusPolicy(Qt::NoFocus);
    connect(m_leftArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToLeft);
    connect(m_rightArrow, &DStateButton::clicked, this, &DCrumbWidget::crumbMoveToRight);
}

void DCrumbWidget::checkArrows()
{
    if(m_crumbTotalLen < m_listWidget->width())
    {
        m_leftArrow->hide();
        m_rightArrow->hide();
    }
    else
    {
        QListWidgetItem *head = m_listWidget->itemAt(1,1);
        QListWidgetItem *end = m_listWidget->itemAt(m_listWidget->width() - 5,5);
        m_leftArrow->show();
        m_rightArrow->show();
        if(head == m_items.first())
        {
            m_leftArrow->setDisabled(true);
            m_rightArrow->setEnabled(true);
        }
        else if(end == m_items.last())
        {
            m_leftArrow->setEnabled(true);
            m_rightArrow->setDisabled(true);
        }
        else
        {
            m_leftArrow->setEnabled(true);
            m_rightArrow->setEnabled(true);
        }
    }
}

void DCrumbWidget::buttonPressed()
{
    DCrumbButton * button = static_cast<DCrumbButton*>(sender());

    FMEvent event;
    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
    QString text = button->path();
    DCrumbButton * localButton = qobject_cast<DCrumbButton*>(m_group.buttons().at(0));

    if(localButton->getName() == RECENT_ROOT)
    {
        event = DUrl::fromRecentFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == COMPUTER_ROOT)
    {
        event = DUrl::fromComputerFile(text.isEmpty() ? "/":text);
    }
    else if(localButton->getName() == TRASH_ROOT)
    {
        event = DUrl::fromTrashFile(text.isEmpty() ? "/":text);
    }else if(localButton->getName() == NETWORK_ROOT)
    {
        if (!text.isEmpty()){
            if (text.startsWith("/")){
                text.remove(0, 1);
            }

            event = DUrl(text);
        }else{
            event = DUrl(NETWORK_ROOT);
        }
    }
    else if(localButton->getName() == m_homePath)
    {
        event = DUrl::fromLocalFile(text);
    }
    else
    {
        event = DUrl::fromLocalFile(text.isEmpty() ? "/":text);
    }

    m_listWidget->scrollToItem(button->getItem());
    emit crumbSelected(event);

    m_listWidget->update();
}

void DCrumbWidget::crumbMoveToLeft()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    m_listWidget->scrollToItem(m_listWidget->itemAt(0,0), QAbstractItemView::PositionAtTop);
    checkArrows();
}

void DCrumbWidget::crumbMoveToRight()
{
    m_listWidget->horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    QListWidgetItem* item = m_listWidget->itemAt(m_listWidget->width() - 10,10);
    if (m_listWidget->itemWidget(item)->width() > m_listWidget->width()){
        int row = m_listWidget->row(item);
        if (row == (m_listWidget->count() - 1)){
            m_listWidget->scrollToBottom();
        }else{
            m_listWidget->scrollToItem(m_listWidget->item(row + 1), QAbstractItemView::PositionAtBottom);
        }
    }else{
        m_listWidget->scrollToItem(item, QAbstractItemView::PositionAtBottom);
    }
    checkArrows();
}

void DCrumbWidget::resizeEvent(QResizeEvent *e)
{
    checkArrows();
    QFrame::resizeEvent(e);
}

ListWidgetPrivate::ListWidgetPrivate(DCrumbWidget *crumbWidget)
    :QListWidget(crumbWidget)
{
    m_crumbWidget = crumbWidget;
}

void ListWidgetPrivate::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
    if(itemAt(event->pos()) == NULL)
    {
        emit m_crumbWidget->searchBarActivated();
    }
}
