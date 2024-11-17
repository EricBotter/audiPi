#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QListWidget>
#include <QMainWindow>

#include "../../audipi/Player.h"

QT_BEGIN_NAMESPACE
class Ui_AudiPi;
QT_END_NAMESPACE

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void initialize() const;

    void play_pause() const;

    void next_track() const;

    void prev_track() const;

    void track_selected(QListWidgetItem *item) const;

    void stop() const;

    void eject() const;

    void tick() const;

private:
    Ui_AudiPi *ui;
    QTimer *main_timer;
    audipi::Player *player;
    audipi::CdRom *cd_rom;
};

#endif //MAIN_WINDOW_H
