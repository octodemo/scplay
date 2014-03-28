# include "soundlistdelegate.h"
# include "likelistmodel.h"
# include "listitem.h"

# include <QPainter>

SoundListDelegate::SoundListDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize SoundListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    return QSize(option.rect.width(), 40);
}

void SoundListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    ListItem sound = ((LikeListModel*)index.model())->getItem(index);

    QPoint titleBaseline = option.rect.topLeft() + QPoint(5, 33);
    QPoint userBaseline = option.rect.topLeft() + QPoint(5, 17);

    QColor titleColor(51,51,51);
    QColor userColor(153,153,153);

    QFont font = painter->font();

    font.setPointSize(11);
    painter->setPen(titleColor);
    painter->setFont(font);
    painter->drawText(titleBaseline, sound.getTitle());

    font.setPointSize(10);
    painter->setPen(userColor);
    painter->setFont(font);
    painter->drawText(userBaseline, sound.getUser());
}
