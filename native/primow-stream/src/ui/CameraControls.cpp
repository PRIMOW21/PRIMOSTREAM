#include "CameraControls.hpp"
#include "../streaming/Engine.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <exception>

namespace primow {
CameraControls::CameraControls(Engine &engine, QWidget *parent)
    : QGroupBox("Cámara web · opcional", parent) {
    auto form = new QFormLayout(this);
    auto devices = new QComboBox;
    auto refresh = new QPushButton("Buscar cámaras");
    auto toggle = new QPushButton("Activar cámara");
    auto corner = new QComboBox;
    corner->addItems({"Superior izquierda", "Superior derecha", "Inferior izquierda", "Inferior derecha"});
    corner->setCurrentIndex(3);
    auto size = new QComboBox;
    size->addItem("Pequeña", 288);
    size->addItem("Mediana", 384);
    size->addItem("Grande", 480);
    size->setCurrentIndex(1);
    auto diagram = new QLabel;
    auto state = new QLabel("Cámara apagada");
    state->setWordWrap(true);
    auto note = new QLabel("Esquema de posición, no vista previa de vídeo. "
        "La cámara solo se abre al activarla. Si ya estás en vivo, se añadirá inmediatamente.");
    note->setObjectName("hint");
    note->setWordWrap(true);
    form->addRow("Dispositivo", devices);
    form->addRow(refresh);
    form->addRow("Esquina", corner);
    form->addRow("Tamaño", size);
    form->addRow(diagram);
    form->addRow(note);
    form->addRow(toggle);
    form->addRow(state);
    auto draw = [=, &engine] {
        const int c = corner->currentIndex(), w = size->currentData().toInt();
        QPixmap image(320, 180);
        image.fill(QColor("#0e141d"));
        QPainter painter(&image);
        painter.setPen(QColor("#596579"));
        painter.drawRect(0, 0, 319, 179);
        painter.drawText(image.rect(), Qt::AlignCenter, "Juego / pantalla");
        const int width = w / 6, height = w * 9 / 16 / 6;
        const QRect box(c % 2 ? 320 - width - 4 : 4, c >= 2 ? 180 - height - 4 : 4,
                        width, height);
        painter.fillRect(box, QColor("#a889ff"));
        painter.setPen(QColor("#10141c"));
        painter.drawText(box, Qt::AlignCenter, "CAM");
        painter.end();
        diagram->setPixmap(image);
        engine.positionCamera(c, w);
    };
    auto scan = [=, &engine] {
        const QString selected = devices->currentData().toString();
        devices->clear();
        for (const auto &device : engine.cameras()) devices->addItem(device.first, device.second);
        const int index = devices->findData(selected);
        if (index >= 0) devices->setCurrentIndex(index);
        toggle->setEnabled(devices->count() > 0 || engine.cameraEnabled());
        if (!devices->count()) state->setText("No hay cámaras disponibles. Conecta una y pulsa Buscar cámaras.");
    };
    connect(refresh, &QPushButton::clicked, this, scan);
    connect(corner, &QComboBox::currentIndexChanged, this, draw);
    connect(size, &QComboBox::currentIndexChanged, this, draw);
    connect(toggle, &QPushButton::clicked, this, [=, &engine] {
        try {
            if (engine.cameraEnabled()) engine.disableCamera();
            else {
                if (QMessageBox::question(this, "Activar cámara",
                    "La cámara será visible en la transmisión. ¿Activarla?",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
                engine.enableCamera(devices->currentData().toString());
                draw();
            }
            devices->setEnabled(!engine.cameraEnabled());
            refresh->setEnabled(!engine.cameraEnabled());
            toggle->setText(engine.cameraEnabled() ? "Apagar cámara" : "Activar cámara");
            state->setText(engine.cameraEnabled() ? "Abriendo cámara…" : "Cámara apagada");
        } catch (const std::exception &error) {
            QMessageBox::warning(this, "Cámara", QString::fromUtf8(error.what()));
        }
    });
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=, &engine] {
        if (engine.cameraEnabled())
            state->setText(engine.cameraReady()
                ? "Cámara activa · incorporada a la composición"
                : "Sin imagen de cámara. Revisa permisos de Windows, conexión y otras aplicaciones.");
    });
    timer->start(2000);
    // Engine initialization happens before this widget is created.
    scan();
    draw();
}
}
