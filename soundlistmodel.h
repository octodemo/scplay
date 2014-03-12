#ifndef SOUNDLISTMODEL_H
#define SOUNDLISTMODEL_H

# include <QAbstractListModel>
# include <QList>
# include <QTimer>

# include "soundlistitem.h"

class SoundListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SoundListModel(QObject *parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

protected:
    virtual void fillModel() = 0;

protected slots:
    virtual void updateModel() = 0;

    // --- attributes
protected:
    QTimer updateTimer;
    QList<SoundListItem> soundItems;
};

#endif // SOUNDLISTMODEL_H