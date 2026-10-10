#include "MainWindow.hpp"
#include "../security/Credentials.hpp"
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QUrl>
#include <stdexcept>
namespace primow {
MainWindow::MainWindow() {
    setWindowTitle("PRIMOW STREAM · Windows");
    resize(760, 680);
    auto body = new QWidget(this); setCentralWidget(body);
    auto form = new QFormLayout(body);
    form->setContentsMargins(32, 28, 32, 28); form->setSpacing(16);
    auto title = new QLabel("PRIMOW STREAM");
    title->setStyleSheet("font-size:28px;font-weight:700;color:#fa424f");
    form->addRow(title);
    status = new QLabel("Desconectado"); form->addRow(status);
    auto info = new QLabel("Kick · H.264 NVENC · 1080p60\nSin cambios en la resolución o frecuencia del monitor.");
    form->addRow(info);
    sources = new QComboBox; methods = new QComboBox;
    methods->addItems({"Ventana (Windows Graphics Capture)", "Juego (plugin OBS)", "Pantalla principal"});
    form->addRow("Fuente", sources); form->addRow("Método", methods);
    auto refreshButton = new QPushButton("Actualizar ventanas");
    auto apply = new QPushButton("Capturar fuente seleccionada");
    form->addRow(refreshButton, apply);
    follow = new QCheckBox("Seguir primer plano: solo aplicaciones autorizadas al capturarlas");
    form->addRow(follow);
    audioMode = new QComboBox; audioMode->addItems({"Audio del sistema", "Audio de la aplicación seleccionada"});
    form->addRow("Audio", audioMode);
    auto mic = new QCheckBox("Silenciar micrófono");
    auto sound = new QCheckBox("Silenciar audio capturado");
    form->addRow(mic, sound);
    auto volume = new QSlider(Qt::Horizontal); volume->setRange(0, 100); volume->setValue(100);
    form->addRow("Volumen del micrófono", volume);
    auto server = new QLineEdit;
    server->setPlaceholderText("rtmps://servidor-del-panel-de-Kick/app");
    QSettings settings; server->setText(settings.value("server").toString());
    auto key = new QLineEdit; key->setEchoMode(QLineEdit::Password); key->setText(loadKey());
    auto save = new QPushButton("Guardar configuración local");
    form->addRow("Servidor Kick", server); form->addRow("Stream Key", key); form->addRow(save);
    auto profile = new QComboBox;
    profile->addItems({"MÁXIMO RENDIMIENTO · P4", "EQUILIBRADO · P5", "MÁXIMA CALIDAD · P6"});
    profile->setCurrentIndex(settings.value("profile", 0).toInt());
    auto bitrate = new QSpinBox; bitrate->setRange(1000, 8000); bitrate->setSuffix(" kbps");
    bitrate->setValue(settings.value("bitrate", 6000).toInt());
    form->addRow("Perfil general", profile); form->addRow("Bitrate", bitrate);
    auto start = new QPushButton("INICIAR STREAM");
    start->setMinimumHeight(56); start->setStyleSheet("background:#ca2839;font-weight:bold");
    form->addRow(start);
    auto metrics = new QLabel("Sin transmisión"); form->addRow(metrics);
    auto note = new QLabel("Versión de desarrollo · Sin vista previa todavía.\n"
        "Captura de juego sujeta a compatibilidad y autorización del juego.\n"
        "Si no hay imagen, selecciona Ventana. Pantalla requiere confirmación.");
    note->setWordWrap(true); form->addRow(note);
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
    connect(save, &QPushButton::clicked, this, [=, this] {
        attempt([&] {
            QUrl url(server->text().trimmed());
            if ((url.scheme() != "rtmps" && url.scheme() != "rtmp") || url.host().isEmpty() ||
                !url.userInfo().isEmpty() || url.hasQuery() || url.hasFragment())
                throw std::runtime_error("Servidor inválido. Introduce URL RTMP(S) sin credenciales ni clave.");
            if (!saveKey(key->text().trimmed())) throw std::runtime_error("No se pudo guardar la clave en Windows");
            QSettings s; s.setValue("server", url.toString()); s.setValue("profile", profile->currentIndex());
            s.setValue("bitrate", bitrate->value());
            status->setText("Configuración guardada en este PC; no se inició transmisión.");
        });
    });
    connect(start, &QPushButton::clicked, this, [=, this] {
        attempt([&] {
            if (engine.busy()) { engine.stop(); return; }
            QUrl url(server->text().trimmed());
            if ((url.scheme() != "rtmps" && url.scheme() != "rtmp") || url.host().isEmpty() ||
                !url.userInfo().isEmpty() || url.hasQuery() || url.hasFragment() || key->text().trimmed().isEmpty())
                throw std::runtime_error("Revisa servidor y Stream Key antes de iniciar");
            engine.start(url.toString(), key->text().trimmed(), bitrate->value(), profile->currentIndex());
        });
    });
    connect(&engine, &Engine::stateChanged, this, [=, this](QString state) {
        status->setText(state);
        start->setText(engine.busy() ? "DETENER STREAM" : "INICIAR STREAM");
        start->setEnabled(state != "Deteniendo");
        profile->setEnabled(!engine.busy()); bitrate->setEnabled(!engine.busy());
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
