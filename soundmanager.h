#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

# include <QObject>

# include <QMediaPlayer>
# include <QMediaPlaylist>

class SoundManager : public QObject
{
    Q_OBJECT
public:
    explicit SoundManager(QObject *parent = 0);

    bool isPlaying();

    void playUrl(const QUrl& url);
    bool isUrlStillValid(const QUrl &url);

public slots:
    void play();
    void pause();
    void next();
    void previous();

    void playSound(int id);
    void enqueueSound(int id);

    void handlePlayerStateChange(QMediaPlayer::State state);
    void receiveStreamUrl(int id, QUrl url);

signals:
    void finished(); // emitted when the last song has finished playing
    void started();
    void nextSongStarted(); // emitted when the next song is begun


    // --- attributes ---
public:
private:
    QMediaPlayer*   player;
    QMediaPlaylist* playlist;

    QHash<int, QUrl> idsForUrls;

    QString streamLocation;

    int lastRequestedSong;
};

#endif // SOUNDMANAGER_H
