# include "mainwindow.h"
# include "ui_mainwindow.h"

# include <QMenu>
# include <QPropertyAnimation>
# include <QParallelAnimationGroup>
# include <QSettings>
# include <QStandardPaths>

# include "soundcloudapi.h"
# include "enterusernamewidget.h"
# include "playerwidget.h"
# include "introwidget.h"
# include "soundlistdelegate.h"

# define VERSION "0.1.6"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent, Qt::FramelessWindowHint), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->hide();

    // --- other setups
    setupTrayIcon();
    setupModels();
    setupSoundListView();
    setupReloadButton();

#ifdef QT_DEBUG
    SoundCloudApi::getInstance().setUserId(62853215);
#else
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/scplay/config.ini", QSettings::IniFormat);

    int uid = settings.value("userId", -1).toInt();

    if (uid < 0) {
        setupWelcomeScreen();
    } else {
        SoundCloudApi::getInstance().setUserId(uid);
    }
#endif
}

MainWindow::~MainWindow() {
#ifndef QT_DEBUG
    int uid = SoundCloudApi::getInstance().getUserId();

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/scplay/config.ini", QSettings::IniFormat);
    settings.setValue("userId", uid);
#endif

    delete ui;
}

// --- public functions
void MainWindow::focusOutEvent(QFocusEvent *e) {
#ifndef QT_DEBUG
    if (e->lostFocus()) {
        this->hide();
    }
#endif
}

// --- public slots
void MainWindow::handleTrayIconActivation(QSystemTrayIcon::ActivationReason activationReason) {
    switch (activationReason) {
        case QSystemTrayIcon::Trigger:
            handleTrayIconSingleClick();
            break;

        case QSystemTrayIcon::Context:
            break;

        default:
            qDebug("Unhandled tray icon activation: %i", (int)activationReason);
            break;
    }
}

void MainWindow::handleAnimationEnd() {
    ui->footerLabel->setPixmap(QPixmap(":/icons/grey_130x20.png"));
}

void MainWindow::displayNewSongNotification(QString title, QString user) {
    if (!this->isVisible()) {
        trayIcon->showMessage(QString("Now playing %1").arg(user), title);
    }
}

// --- private functions
void MainWindow::setupTrayIcon() {
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/white.png"), this);

    QMenu* trayMenu = new QMenu();
    QAction* versionBar = new QAction("scplay " VERSION, this);
    versionBar->setDisabled(true);

    trayMenu->addAction("\u25B6 / \u2016",    ui->playerWidget, SLOT(togglePlayPause()));
    trayMenu->addAction("\u25B6\u25B6\u007C", ui->playerWidget, SLOT(playNextSong()));
    trayMenu->addAction("\u007C\u25C0\u25C0", ui->playerWidget, SLOT(playPreviousSong()));
    trayMenu->addAction("Exit", this, SLOT(close()));
    trayMenu->addAction(versionBar);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(ui->playerWidget, SIGNAL(songChanged(QString,QString)), this, SLOT(displayNewSongNotification(QString,QString)));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(handleTrayIconActivation(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::setupModels() {
    soundStorage  = new SoundStorage(this);
    likeModel     = new LikeModel(soundStorage, this);
    playlistModel = new PlaylistModel(soundStorage, this);

    connect(&SoundCloudApi::getInstance(), SIGNAL(isReady()), soundStorage, SLOT(fill()));
}

void MainWindow::setupSoundListView() {
    ui->songView->setModel(likeModel);
    ui->songView->setItemDelegate(new SoundListDelegate(this));

    QParallelAnimationGroup *animationGroup = new QParallelAnimationGroup(this);
    QPropertyAnimation *slideInAnimation = new QPropertyAnimation(ui->playerWidget, "geometry", animationGroup);
    slideInAnimation->setDuration(500);
    slideInAnimation->setStartValue(QRect(QPoint(0, -140), ui->playerWidget->size()));
    slideInAnimation->setEndValue(QRect(QPoint(0,0), ui->playerWidget->size()));

    QPropertyAnimation *shrinkAnimation = new QPropertyAnimation(ui->soundViewAndControls, "geometry", animationGroup);
    shrinkAnimation->setDuration(500);
    shrinkAnimation->setStartValue(ui->soundViewAndControls->geometry());
    shrinkAnimation->setEndValue(QRect(QPoint(10,140), ui->soundViewAndControls->size() - QSize(0,130)));

    animationGroup->addAnimation(slideInAnimation);
    animationGroup->addAnimation(shrinkAnimation);

    connect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), ui->playerWidget, SLOT(handlePlayRequest(QModelIndex)));
    connect(ui->playerWidget, SIGNAL(playbackStarted()), animationGroup, SLOT(start()));

    connect(animationGroup, SIGNAL(finished()), animationGroup, SLOT(deleteLater()));
    connect(animationGroup, SIGNAL(finished()), this, SLOT(handleAnimationEnd()));

    connect(ui->likeButton, SIGNAL(clicked()), this, SLOT(switchToLikeDisplay()));
    connect(ui->playlistButton, SIGNAL(clicked()), this, SLOT(switchToPlaylistListingDisplay()));
}

void MainWindow::setupWelcomeScreen() {
    QFrame*  helloUserFrame  = new QFrame(this);
    QWidget* helloUserScreen = new EnterUserNameWidget(helloUserFrame);
    IntroWidget* introScreen = new IntroWidget(helloUserFrame);

    introScreen->setGeometry(300,0,300,400);

    QPropertyAnimation *slideOutAnimation1 = new QPropertyAnimation(helloUserFrame, "geometry");
    QPropertyAnimation *slideOutAnimation2 = new QPropertyAnimation(helloUserFrame, "geometry");

    slideOutAnimation1->setDuration(500);
    slideOutAnimation1->setStartValue(QRect(0,0, 600, 400));
    slideOutAnimation1->setEndValue(QRect(-300, 0, 600, 400));

    slideOutAnimation2->setDuration(500);
    slideOutAnimation2->setStartValue(QRect(-300,0, 600, 400));
    slideOutAnimation2->setEndValue(QRect(-600, 0, 600, 400));

    helloUserFrame->setMinimumSize(QSize(600,400));
    helloUserFrame->setStyleSheet("background-color: #333;");

    connect(&SoundCloudApi::getInstance(), SIGNAL(isReady()), slideOutAnimation1, SLOT(start()));
    connect(slideOutAnimation1, SIGNAL(finished()), slideOutAnimation1, SLOT(deleteLater()));
    connect(slideOutAnimation1, SIGNAL(finished()), introScreen, SLOT(setFocus()));
    connect(introScreen, SIGNAL(introDone()), slideOutAnimation2, SLOT(start()));
    connect(slideOutAnimation2, SIGNAL(finished()), helloUserFrame, SLOT(deleteLater()));
}

void MainWindow::setupReloadButton() {
    connect(ui->reloadButton, SIGNAL(clicked()), soundStorage, SLOT(fill()));
}

void MainWindow::handleTrayIconSingleClick() {
    static QPoint windowPosition;

    // initialize the static var windowPosition first
    if (windowPosition.isNull()) {
        QPoint iconPosition = trayIcon->geometry().center();
        QSize mySize = this->size();

        if (iconPosition.x() - mySize.width() < 0) {
            // icon is located in the left corner
            if (iconPosition.y() - mySize.height() < 0) {
                // icon is located top-left
                windowPosition = QPoint(iconPosition.x(), iconPosition.y() + 20);
            } else {
                // icon is located somwhere lower-left
                windowPosition = QPoint(iconPosition.x(), iconPosition.y() - 20 - mySize.height());
            }
        } else {
            // icon is located somewhere right of the left corner
            if (iconPosition.y() - mySize.height() < 0) {
                // icon is located top-right
                windowPosition = QPoint(iconPosition.x() - mySize.width(), iconPosition.y() + 20);
            } else {
                // icon is located somwhere lower-right
                windowPosition = QPoint(iconPosition.x() - mySize.width(), iconPosition.y() - 20 - mySize.height());
            }
        }
    }

    this->move(windowPosition);
    this->show();
}

void MainWindow::switchToPlaylistListingDisplay() {
    disconnect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), ui->playerWidget, SLOT(handlePlayRequest(QModelIndex)));

    ui->songView->setModel(playlistModel);
    ui->playlistButton->setStyleSheet("");
    ui->playlistButton->setText("Playlists");

    connect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectPlaylist(QModelIndex)));
}

void MainWindow::switchToLikeDisplay() {
    disconnect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectPlaylist(QModelIndex)));
    ui->songView->setModel(likeModel);
    connect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), ui->playerWidget, SLOT(handlePlayRequest(QModelIndex)));
}

void MainWindow::selectPlaylist(QModelIndex index) {
    // introducing a possible memory leak here....
    PlaylistSoundListModel *playlistSoundModel = new PlaylistSoundListModel(soundStorage);
    int selectedPlaylistId = ((ListModelBase*)index.model())->getItem(index).getId();
    Playlist selectedPlaylist = soundStorage->getPlaylistById(selectedPlaylistId);

    playlistSoundModel->updateSoundIds(selectedPlaylist.getSounds());

    ui->playlistButton->setStyleSheet("font-style: italic");
    ui->playlistButton->setText(selectedPlaylist.getTitle());

    // put the model and connections into place here
    disconnect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(selectPlaylist(QModelIndex)));
    ui->songView->setModel(playlistSoundModel);
    connect(ui->songView, SIGNAL(doubleClicked(QModelIndex)), ui->playerWidget, SLOT(handlePlayRequest(QModelIndex)));
}
