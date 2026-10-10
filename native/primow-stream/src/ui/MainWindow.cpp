#include "MainWindow.hpp"
#include "../security/Credentials.hpp"
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QUrl>
#include <stdexcept>
namespace primow {
static QString serverFor(const QString &id) {
    if (id == "kick") return "rtmps://fa723fc1b171.global-contribute.live-video.net:443/app";
    if (id == "twitch") return "rtmps://ingest.global-contribute.live-video.net/app";
    return {};
}
static QString serviceFor(const QString &id) {
    if (id == "kick") return "Kick";
    if (id == "twitch") return "Twitch";
    if (id == "tiktok") return "TikTok";
    return "Personalizado";
}
MainWindow::MainWindow() {
    setWindowTitle("PRIMOW STREAM · Windows");
    resize(1040, 820);
    setMinimumSize(900, 620);
    auto scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto body = new QWidget;
    body->setObjectName("page");
    auto root = new QVBoxLayout(body);
    root->setContentsMargins(28, 24, 28, 24);
    root->setSpacing(18);

    auto header = new QHBoxLayout;
    auto heading = new QVBoxLayout;
    auto title = new QLabel("PRIMOW STREAM");
    title->setObjectName("title");
    auto subtitle = new QLabel("Transmisión para juegos y aplicaciones de Windows");
    subtitle->setObjectName("subtitle");
    heading->addWidget(title);
    heading->addWidget(subtitle);
    header->addLayout(heading);
    header->addStretch();
    status = new QLabel("Desconectado");
    status->setObjectName("status");
    header->addWidget(status, 0, Qt::AlignVCenter);
    root->addLayout(header);

    auto columns = new QGridLayout;
    columns->setHorizontalSpacing(18);
    columns->setVerticalSpacing(16);
    columns->setColumnStretch(0, 1);
    columns->setColumnStretch(1, 1);
    auto captureCard = new QGroupBox("Captura");
    auto captureForm = new QFormLayout(captureCard);
    captureForm->setSpacing(11);
    captureForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    sources = new QComboBox; methods = new QComboBox;
    methods->addItems({"Ventana", "Juego", "Pantalla completa · monitor principal"});
    captureForm->addRow("Ventana o juego", sources);
    captureForm->addRow("Método", methods);
    auto refreshButton = new QPushButton("Actualizar ventanas");
    auto apply = new QPushButton("Capturar fuente seleccionada");
    auto captureButtons = new QHBoxLayout;
    captureButtons->addWidget(refreshButton);
    captureButtons->addWidget(apply, 1);
    captureForm->addRow(captureButtons);
    follow = new QCheckBox("Seguir automáticamente la aplicación aprobada en primer plano");
    captureForm->addRow(follow);

    auto audioCard = new QGroupBox("Audio");
    auto audioForm = new QFormLayout(audioCard);
    audioForm->setSpacing(11);
    audioMode = new QComboBox; audioMode->addItems({"Audio del sistema", "Audio de la aplicación seleccionada"});
    audioForm->addRow("Fuente de audio", audioMode);
    auto mic = new QCheckBox("Silenciar micrófono");
    auto sound = new QCheckBox("Silenciar audio capturado");
    auto muteRow = new QHBoxLayout;
    muteRow->addWidget(mic);
    muteRow->addWidget(sound);
    audioForm->addRow(muteRow);
    auto volume = new QSlider(Qt::Horizontal); volume->setRange(0, 100); volume->setValue(100);
    audioForm->addRow("Volumen del micrófono", volume);

    auto destinationCard = new QGroupBox("Destino de transmisión");
    auto destinationForm = new QFormLayout(destinationCard);
    destinationForm->setSpacing(11);
    destinationForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    platform = new QComboBox;
    platform->addItem("Kick", "kick");
    platform->addItem("Twitch", "twitch");
    platform->addItem("TikTok LIVE", "tiktok");
    platform->addItem("Servidor RTMP personalizado", "custom");
    QSettings settings;
    const auto savedPlatform = settings.value("platform", "kick").toString();
    const int savedPlatformIndex = platform->findData(savedPlatform);
    if (savedPlatformIndex >= 0) platform->setCurrentIndex(savedPlatformIndex);
    destinationForm->addRow("Plataforma", platform);
    server = new QLineEdit;
    server->setPlaceholderText("rtmps://servidor:443/app");
    destinationForm->addRow("Servidor RTMP(S)", server);
    key = new QLineEdit;
    key->setEchoMode(QLineEdit::Password);
    key->setPlaceholderText("Pega aquí la clave, no el enlace del canal");
    showKey = new QCheckBox("Mostrar");
    auto keyRow = new QHBoxLayout;
    keyRow->addWidget(key, 1);
    keyRow->addWidget(showKey);
    destinationForm->addRow("Clave de transmisión", keyRow);
    platformHelp = new QLabel;
    platformHelp->setObjectName("hint");
    platformHelp->setWordWrap(true);
    destinationForm->addRow(platformHelp);
    auto save = new QPushButton("Guardar configuración local");
    destinationForm->addRow(save);
    profile = new QComboBox;
    profile->addItems({"MÁXIMO RENDIMIENTO · P4", "EQUILIBRADO · P5", "MÁXIMA CALIDAD · P6"});
    profile->setCurrentIndex(settings.value("profile", 0).toInt());
    bitrate = new QSpinBox; bitrate->setRange(1000, 8000); bitrate->setSuffix(" kbps");
    bitrate->setValue(settings.value("bitrate", 6000).toInt());
    destinationForm->addRow("Perfil general", profile);
    destinationForm->addRow("Bitrate", bitrate);

    columns->addWidget(captureCard, 0, 0);
    columns->addWidget(destinationCard, 0, 1, 2, 1);
    columns->addWidget(audioCard, 1, 0);
    root->addLayout(columns);

    auto start = new QPushButton("INICIAR STREAM");
    start->setObjectName("primaryButton");
    start->setMinimumHeight(50);
    auto footer = new QHBoxLayout;
    footer->addWidget(start);
    metrics = new QLabel("Sin transmisión");
    metrics->setObjectName("metrics");
    footer->addWidget(metrics, 1);
    root->addLayout(footer);
    auto note = new QLabel("H.264 NVENC · Salida 1080p60 · La captura de pantalla completa usa DXGI. "
                           "Windows puede mostrar sus propios indicadores de privacidad.");
    note->setObjectName("hint");
    note->setWordWrap(true);
    root->addWidget(note);
    scroll->setWidget(body);
    setCentralWidget(scroll);

    loadPlatformSettings();
    engine.initialize();
    refresh();
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refresh);
    connect(apply, &QPushButton::clicked, this, &MainWindow::select);
    connect(mic, &QCheckBox::toggled, this, [this](bool v) { engine.mute(true, v); });
    connect(sound, &QCheckBox::toggled, this, [this](bool v) { engine.mute(false, v); });
    connect(volume, &QSlider::valueChanged, this, [this](int v) { engine.volume(v / 100.0f); });
    connect(audioMode, &QComboBox::currentIndexChanged, this, [this](int v) {
        attempt([&] {
            if (v && current.isEmpty()) throw std::runtime_error("Selecciona primero una ventana");
            engine.audio(v == 1, current);
        });
    });
    connect(platform, &QComboBox::currentIndexChanged, this, [this] {
        loadPlatformSettings();
    });
    connect(methods, &QComboBox::currentIndexChanged, this, [=](int index) {
        const bool needsWindow = index != static_cast<int>(Capture::Display);
        sources->setEnabled(needsWindow);
        refreshButton->setEnabled(needsWindow);
        follow->setEnabled(needsWindow);
    });
    sources->setEnabled(true);
    refreshButton->setEnabled(true);
    follow->setEnabled(true);
    connect(showKey, &QCheckBox::toggled, this, [this](bool visible) {
        key->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });
    connect(save, &QPushButton::clicked, this, [=, this] {
        attempt([&] {
            QUrl url(server->text().trimmed());
            if ((url.scheme() != "rtmps" && url.scheme() != "rtmp") || url.host().isEmpty() ||
                !url.userInfo().isEmpty() || url.hasQuery() || url.hasFragment())
                throw std::runtime_error("Servidor inválido. Introduce URL RTMP(S) sin credenciales ni clave.");
            if (!saveKey(key->text().trimmed(), credentialService()))
                throw std::runtime_error("No se pudo guardar la clave de forma segura en Windows");
            QSettings s;
            const QString id = platform->currentData().toString();
            s.setValue("platform", id);
            s.setValue("servers/" + id, url.toString());
            if (id == "kick") s.setValue("server", url.toString());
            s.setValue("profile", profile->currentIndex());
            s.setValue("bitrate", bitrate->value());
            status->setText("Configuración guardada · no se inició transmisión");
        });
    });
    connect(start, &QPushButton::clicked, this, [=, this] {
        attempt([&] {
            if (engine.busy()) { engine.stop(); return; }
            QUrl url(server->text().trimmed());
            if ((url.scheme() != "rtmps" && url.scheme() != "rtmp") || url.host().isEmpty() ||
                !url.userInfo().isEmpty() || url.hasQuery() || url.hasFragment() || key->text().trimmed().isEmpty())
                throw std::runtime_error("Revisa servidor y Stream Key antes de iniciar");
            engine.start(url.toString(), key->text().trimmed(), platform->currentText(),
                         bitrate->value(), profile->currentIndex());
        });
    });
    connect(&engine, &Engine::stateChanged, this, [=, this](QString state) {
        status->setText(state);
        start->setText(engine.busy() ? "DETENER STREAM" : "INICIAR STREAM");
        start->setEnabled(state != "Deteniendo");
        profile->setEnabled(!engine.busy()); bitrate->setEnabled(!engine.busy());
        platform->setEnabled(!engine.busy()); server->setEnabled(!engine.busy());
        key->setEnabled(!engine.busy());
    });
    auto timer = new QTimer(this); timer->setInterval(2000);
    connect(timer, &QTimer::timeout, this, [=, this] {
        metrics->setText(engine.metrics());
        if (!follow->isChecked() || methods->currentIndex() == 2) return;
        const auto front = GetForegroundWindow();
        for (const auto &w : windows()) {
            if (w.handle == front && approved.contains(w.executable.toLower()) && w.obsId != current) {
                attempt([&] {
                    engine.select(static_cast<Capture>(methods->currentIndex()), w.obsId);
                    if (audioMode->currentIndex() == 1) engine.audio(true, w.obsId);
                    current = w.obsId;
                    status->setText("Fuente: " + w.title);
                });
                break;
            }
        }
    });
    timer->start();
}
void MainWindow::attempt(const std::function<void()> &fn) {
    try { fn(); }
    catch (const std::exception &e) { QMessageBox::warning(this, "PRIMOW STREAM", QString::fromUtf8(e.what())); }
}
void MainWindow::refresh() {
    const auto selected = sources->currentData().toString();
    available = windows(); sources->clear();
    for (const auto &w : available) sources->addItem(w.title + " — " + w.executable, w.obsId);
    auto index = sources->findData(selected); if (index >= 0) sources->setCurrentIndex(index);
}
QString MainWindow::credentialService() const {
    return serviceFor(platform->currentData().toString());
}
void MainWindow::loadPlatformSettings() {
    if (!server || !platform || !key || !platformHelp) return;
    const QString id = platform->currentData().toString();
    QSettings settings;
    QString savedServer = settings.value("servers/" + id).toString();
    if (savedServer.isEmpty() && id == "kick")
        savedServer = settings.value("server").toString();
    server->setText(savedServer.isEmpty() ? serverFor(id) : savedServer);
    key->setText(loadKey(credentialService()));
    if (id == "tiktok") {
        server->setPlaceholderText("Servidor RTMP que te entrega TikTok LIVE");
        platformHelp->setText("TikTok exige acceso a LIVE desde PC y una URL de servidor y clave "
                              "proporcionadas en TikTok LIVE Center. Si no aparecen en tu cuenta, "
                              "PRIMOW no puede generarlas ni habilitar ese acceso.");
    } else if (id == "custom") {
        server->setPlaceholderText("rtmps://servidor:443/app");
        platformHelp->setText("Introduce el servidor RTMP(S) y la clave entregados por tu plataforma.");
    } else {
        server->setPlaceholderText("rtmps://servidor:443/app");
        platformHelp->setText(id == "twitch"
            ? "Servidor seguro predeterminado de Twitch. Copia la clave desde el Panel del creador."
            : "Servidor seguro de Kick. Copia la clave desde el panel de creador de Kick.");
    }
}
void MainWindow::select() {
    attempt([&] {
        const bool display = methods->currentIndex() == 2;
        if (!display && sources->currentIndex() < 0) throw std::runtime_error("No hay ventana seleccionada");
        if (display && QMessageBox::question(this, "Privacidad",
            "Se capturará toda la pantalla principal, incluidas notificaciones. ¿Continuar?") != QMessageBox::Yes) return;
        if (methods->currentIndex() == 1 && QMessageBox::question(this, "Compatibilidad",
            "El plugin de OBS utiliza captura mediante hook. Úsalo solo donde esté permitido.\n"
            "No se modificará el juego ni su anticheat. ¿Continuar?") != QMessageBox::Yes) return;
        auto id = sources->currentData().toString();
        engine.select(static_cast<Capture>(methods->currentIndex()), id);
        if (audioMode->currentIndex() == 1) engine.audio(true, id);
        current = id;
        if (!display) approved.insert(available[sources->currentIndex()].executable.toLower());
        QTimer::singleShot(4000, this, [this] {
            if (!engine.captureReady()) QMessageBox::warning(this, "Captura sin imagen",
                "No se reciben dimensiones de imagen. Prueba captura de ventana, modo sin bordes o pantalla.\n"
                "No se intentará eludir protecciones. Esta comprobación no detecta todos los cuadros negros.");
        });
    });
}
void MainWindow::closeEvent(QCloseEvent *event) {
    if (engine.busy()) {
        QMessageBox::information(this, "Transmisión activa", "Detén la transmisión antes de cerrar.");
        event->ignore(); return;
    }
    event->accept();
}
}
