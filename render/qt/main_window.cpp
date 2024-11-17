#include "main_window.h"

#include <ui_main_window.h>

#include <QTimer>

#include "../../audipi/util.h"

constexpr auto UPDATE_INTERVAL_MS = 20;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui_AudiPi), player(new audipi::Player),
                                          cd_rom(new audipi::CdRom("/dev/cdrom")) {
    ui->setupUi(this);
    main_timer = new QTimer(this);

    connect(ui->playPauseButton, &QPushButton::clicked, this, &MainWindow::play_pause); // NOLINT(*-unused-return-value)
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::next_track); // NOLINT(*-unused-return-value)
    connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::prev_track); // NOLINT(*-unused-return-value)
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stop); // NOLINT(*-unused-return-value)
    connect(ui->ejectButton, &QPushButton::clicked, this, &MainWindow::eject); // NOLINT(*-unused-return-value)

    connect(ui->playlistWidget, &QListWidget::itemClicked, this, &MainWindow::track_selected); // NOLINT(*-unused-return-value)

    connect(main_timer, &QTimer::timeout, this, &MainWindow::tick); // NOLINT(*-unused-return-value)

    QTimer::singleShot(UPDATE_INTERVAL_MS, this, &MainWindow::initialize);
}

MainWindow::~MainWindow() {
    delete ui;
    delete player;
    delete cd_rom;
}

void MainWindow::initialize() const {
    ui->statusLabel->setText("Initalizing...");
    ui->trackLabel->setText("");

    bool is_init = cd_rom->is_init();
    if (!is_init) {
        ui->statusLabel->setText("No CD drive present!");
        return;
    }

    auto disk_toc = cd_rom->read_toc();
    if (disk_toc) {
        player->enqueue_cd(*cd_rom, disk_toc.value());
    } else {
        ui->statusLabel->setText("Error while reading CD TOC"); // todo improve error reporting
        return;
    }

    for (auto [track_num, _1, _2] : disk_toc.value().entries) {
        auto label = std::string("Track ") + audipi::left_pad_string(std::to_string(+track_num), 2, '0');
        ui->playlistWidget->addItem(QString(label.c_str()));
    }

    main_timer->start(std::chrono::milliseconds(UPDATE_INTERVAL_MS)); // 50 UPS
}

void MainWindow::play_pause() const {
    if (player->get_state() == audipi::PlayerState::PLAYING) {
        player->pause();
        ui->playPauseButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
        ui->playPauseButton->setText("Play");
    } else {
        player->play();
        ui->playPauseButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
        ui->playPauseButton->setText("Pause");
    }
}

void MainWindow::next_track() const {
    player->next_track();
}

void MainWindow::prev_track() const {
    player->prev_track();
}

void MainWindow::track_selected(QListWidgetItem *item) const {
    auto trackIdx = item->text().split(" ")[1].toInt() - 1;

    player->jump_to_track(trackIdx);
}

void MainWindow::stop() const {
    player->stop();
}

void MainWindow::eject() const {
    player->clear_playlist();
    ui->playlistWidget->clear();
    if (!cd_rom->eject()) {
        ui->ejectButton->setEnabled(false);
        ui->ejectButton->setText("Eject not supported");
    }
}

void MainWindow::tick() const {
    player->tick();

    auto [state, current_track_index, current_track_name, current_location_in_track] = player->get_status();

    if (state == audipi::PlayerState::PLAYING || state == audipi::PlayerState::PAUSED) {
        ui->statusLabel->setText(current_track_name.c_str());
        ui->trackLabel->setText(msf_location_to_string(current_location_in_track).c_str());
    } else if (state == audipi::PlayerState::ERROR) {
        ui->statusLabel->setText("Error!");
        ui->trackLabel->setText("");
    } else {
        ui->statusLabel->setText("Ready");
        ui->trackLabel->setText("");
    }
}
