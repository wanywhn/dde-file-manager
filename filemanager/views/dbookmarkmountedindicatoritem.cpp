#include "dbookmarkmountedindicatoritem.h"
#include "../app/global.h"
#include <QDebug>

DBookmarkMountedIndicatorItem::DBookmarkMountedIndicatorItem(DBookmarkItem *parentItem):
    m_parentItem(parentItem)
{
    init();
}

DBookmarkMountedIndicatorItem::~DBookmarkMountedIndicatorItem()
{

}

void DBookmarkMountedIndicatorItem::init()
{
    setParentItem(m_parentItem);
    setDefaultItem(true);
    boundImageToHover(":/icons/images/icons/unmount_hover.png");
    boundImageToPress(":/icons/images/icons/unmount_press.png");
    boundImageToRelease(":/icons/images/icons/unmount_normal.png");
    boundImageToChecked(":/icons/images/icons/unmount_active.png");
    setReleaseBackgroundColor(QColor(Qt::transparent));
    setPressBackgroundColor(QColor(Qt::transparent));
    setHoverBackgroundColor(QColor(Qt::transparent));
    setCheckable(true);
    setHoverBackgroundEnable(true);
    setPressBackgroundEnable(true);
    setDraggable(false);
    setBounds(0, 0, 30, 20);
    setPos(QPoint(160, 5));
    hide();
}

void DBookmarkMountedIndicatorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    DBookmarkItem::paint(painter, option, widget);
}

void DBookmarkMountedIndicatorItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    setHovered(true);
    update();
}

void DBookmarkMountedIndicatorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setPress(false);
    setHovered(false);
    qDebug() << event << "=========" << m_parentItem->getSysPath();
    deviceListener->unmount(m_parentItem->getSysPath());
    update();
    qDebug() << event << "/////////";
}

void DBookmarkMountedIndicatorItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

}
