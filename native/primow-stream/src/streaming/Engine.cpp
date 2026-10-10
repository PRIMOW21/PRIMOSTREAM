#include "Engine.hpp"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMetaObject>
#include <stdexcept>
#include <util/base.h>
#include <windows.h>
namespace primow {
static void require(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
static QString primaryMonitorId() {
    struct Result { QString id; };
    Result result;
    EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR monitor, HDC, LPRECT, LPARAM data) -> BOOL {
        MONITORINFOEXA info{};
        info.cbSize = sizeof(info);
        if (!GetMonitorInfoA(monitor, reinterpret_cast<LPMONITORINFO>(&info)) ||
            !(info.dwFlags & MONITORINFOF_PRIMARY)) return TRUE;
        DISPLAY_DEVICEA device{};
        device.cb = sizeof(device);
        auto &out = *reinterpret_cast<Result *>(data);
        if (EnumDisplayDevicesA(info.szDevice, 0, &device, EDD_GET_DEVICE_INTERFACE_NAME) &&
            device.DeviceID[0] != '\0') {
            out.id = QString::fromLocal8Bit(device.DeviceID);
        } else {
            out.id = QString::fromLocal8Bit(info.szDevice);
        }
        return FALSE;
    }, reinterpret_cast<LPARAM>(&result));
    return result.id;
}
// Discard engine logs: plugin messages can contain endpoint/credential material.
// A future diagnostic sink must use an explicit allowlist, not string redaction.
static void privateLog(int, const char *, va_list, void *) {}
Engine::Engine(QObject *p) : QObject(p) {}
void Engine::initialize() {
    base_set_log_handler(privateLog, nullptr);
    require(obs_startup("es-ES", nullptr, nullptr), "No se pudo iniciar libobs");
    initialized = true;
    QString root = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../..");
    obs_add_data_path((root + "/data/libobs").toUtf8().constData());
    obs_add_module_path((root + "/obs-plugins/64bit").toUtf8().constData(),
                        (root + "/data/obs-plugins/%module%").toUtf8().constData());
    obs_video_info vi{};
    vi.graphics_module = "libobs-d3d11.dll";
    vi.fps_num = 60; vi.fps_den = 1;
    vi.base_width = 1920; vi.base_height = 1080;
    vi.output_width = 1920; vi.output_height = 1080;
    vi.output_format = VIDEO_FORMAT_NV12; vi.gpu_conversion = true;
    vi.colorspace = VIDEO_CS_709; vi.range = VIDEO_RANGE_PARTIAL;
    vi.scale_type = OBS_SCALE_BILINEAR;
    require(obs_reset_video(&vi) == OBS_VIDEO_SUCCESS, "No se pudo inicializar Direct3D 11");
    obs_audio_info ai{}; ai.samples_per_sec = 48000; ai.speakers = SPEAKERS_STEREO;
    require(obs_reset_audio(&ai), "No se pudo inicializar audio");
    obs_load_all_modules(); obs_post_load_modules();
    scene = obs_scene_create("NERON STREAM PROJECT");
    require(scene, "No se pudo crear la escena");
    obs_set_output_source(0, obs_scene_get_source(scene));
    auto settings = obs_data_create();
    obs_data_set_string(settings, "device_id", "default");
    mic = obs_source_create("wasapi_input_capture", "Micrófono", settings, nullptr);
    obs_data_release(settings);
    require(mic, "Módulo WASAPI de micrófono no disponible");
    obs_set_output_source(1, mic);
    audio(false, {});
}
void Engine::select(Capture method, const QString &window, const QString &monitorId) {
    auto s = obs_data_create();
    obs_data_set_string(s, "window", window.toUtf8().constData());
    obs_data_set_int(s, "priority", 0);
    const bool display = method == Capture::Display;
    const QString selectedMonitor = display ? primaryMonitorId() : monitorId;
    require(!display || !selectedMonitor.isEmpty(), "No se pudo detectar la pantalla principal de Windows");
    obs_data_set_int(s, "method", display ? 1 : method == Capture::Window ? 2 : 0);
    obs_data_set_string(s, "capture_mode", "window");
    obs_data_set_bool(s, "capture_cursor", true);
    obs_data_set_bool(s, "allow_transparency", false);
    if (display) obs_data_set_string(s, "monitor_id", selectedMonitor.toUtf8().constData());
    const char *id = method == Capture::Game ? "game_capture" :
                     method == Capture::Display ? "monitor_capture" : "window_capture";
    auto next = obs_source_create(id, "Fuente", s, nullptr);
    obs_data_release(s);
    require(next, "Método no disponible. Selecciona ventana o pantalla.");
    // Output and encoders remain untouched during source changes.
    obs_enter_graphics();
    auto nextItem = obs_scene_add(scene, next);
    obs_sceneitem_set_bounds_type(nextItem, OBS_BOUNDS_SCALE_INNER);
    vec2 bounds{1920.0f, 1080.0f};
    obs_sceneitem_set_bounds(nextItem, &bounds);
    if (cameraItem) obs_sceneitem_set_order(cameraItem, OBS_ORDER_MOVE_TOP);
    if (item) obs_sceneitem_remove(item);
    item = nextItem;
    obs_leave_graphics();
    if (capture) obs_source_release(capture);
    capture = next;
}
void Engine::audio(bool application, const QString &window) {
    auto s = obs_data_create();
    obs_data_set_string(s, "device_id", "default");
    obs_data_set_string(s, "window", window.toUtf8().constData());
    obs_data_set_int(s, "priority", 0);
    auto next = obs_source_create(application ? "wasapi_process_output_capture" : "wasapi_output_capture",
                                  "Audio", s, nullptr);
    obs_data_release(s);
    require(next, "Captura de audio no disponible en este Windows");
    if (sound) obs_source_set_muted(next, obs_source_muted(sound));
    obs_set_output_source(2, next);
    if (sound) obs_source_release(sound);
    sound = next;
}
void Engine::mute(bool microphone, bool value) {
    if (auto source = microphone ? mic : sound) obs_source_set_muted(source, value);
}
void Engine::volume(float value) { if (mic) obs_source_set_volume(mic, value); }
bool Engine::captureReady() const {
    return capture && obs_source_get_width(capture) > 0 && obs_source_get_height(capture) > 0;
}
void Engine::start(const QString &server, const QString &key, const QString &serviceName,
                   int bitrate, int profile) {
    if (running) return;
    require(captureReady(), "Fuente sin imagen. Prueba ventana sin bordes u otro método.");
    releaseOutput();
    auto s = obs_data_create();
    obs_data_set_string(s, "rate_control", "cbr");
    obs_data_set_string(s, "preset", profile == 0 ? "p4" : profile == 1 ? "p5" : "p6");
    obs_data_set_string(s, "multipass", "disabled");
    obs_data_set_string(s, "profile", "main");
    obs_data_set_int(s, "bitrate", bitrate); obs_data_set_int(s, "keyint_sec", 2);
    obs_data_set_bool(s, "lookahead", false);
    obs_data_set_bool(s, "adaptive_quantization", false);
    video = obs_video_encoder_create("obs_nvenc_h264_tex", "NVENC H264", s, nullptr);
    obs_data_release(s);
    require(video, "NVENC H.264 no disponible; no se usará x264");
    s = obs_data_create(); obs_data_set_int(s, "bitrate", 160);
    aac = obs_audio_encoder_create("ffmpeg_aac", "AAC", s, 0, nullptr);
    obs_data_release(s);
    require(aac, "AAC no disponible");
    obs_encoder_set_video(video, obs_get_video());
    obs_encoder_set_audio(aac, obs_get_audio());
    s = obs_data_create();
    obs_data_set_string(s, "server", server.toUtf8().constData());
    obs_data_set_string(s, "key", key.toUtf8().constData());
    service = obs_service_create("rtmp_custom", serviceName.toUtf8().constData(), s, nullptr);
    obs_data_release(s);
    require(service, "Servicio RTMP no disponible");
    output = obs_output_create("rtmp_output", "Transmisión", nullptr, nullptr);
    require(output, "Salida RTMP no disponible");
    obs_output_set_service(output, service);
    obs_output_set_video_encoder(output, video);
    obs_output_set_audio_encoder(output, aac, 0);
    obs_output_set_reconnect_settings(output, 5, 3);
    auto outputSignalHandler = obs_output_get_signal_handler(output);
    signal_handler_connect(outputSignalHandler, "start", started, this);
    signal_handler_connect(outputSignalHandler, "stop", stopped, this);
    signal_handler_connect(outputSignalHandler, "reconnect", reconnect, this);
    signal_handler_connect(outputSignalHandler, "reconnect_success", reconnected, this);
    running = true; previousBytes = 0; previousTime = QDateTime::currentMSecsSinceEpoch();
    emit stateChanged("Conectando");
    if (!obs_output_start(output)) {
        running = false;
        emit stateChanged("Desconectado");
        throw std::runtime_error("No se pudo iniciar. Revisa NVENC, servidor y clave localmente.");
    }
}
void Engine::started(void *p, calldata_t *) {
    auto self = static_cast<Engine *>(p);
    QMetaObject::invokeMethod(self, [self] { emit self->stateChanged("En vivo"); }, Qt::QueuedConnection);
}
void Engine::stopped(void *p, calldata_t *data) {
    auto self = static_cast<Engine *>(p);
    auto code = calldata_int(data, "code");
    QMetaObject::invokeMethod(self, [self, code] {
        self->running = false;
        emit self->stateChanged(code == 0 ? "Finalizado" : QString("Finalizado con error (%1)").arg(code));
    }, Qt::QueuedConnection);
}
void Engine::reconnect(void *p, calldata_t *) {
    auto self = static_cast<Engine *>(p);
    QMetaObject::invokeMethod(self, [self] { emit self->stateChanged("Reconectando"); }, Qt::QueuedConnection);
}
void Engine::reconnected(void *p, calldata_t *d) { started(p, d); }
void Engine::stop() { if (output && running) { emit stateChanged("Deteniendo"); obs_output_stop(output); } }
QString Engine::metrics() {
    if (!output || !running) return "Sin transmisión";
    auto now = QDateTime::currentMSecsSinceEpoch();
    auto bytes = obs_output_get_total_bytes(output);
    double kbps = now > previousTime && bytes >= previousBytes ? double(bytes - previousBytes) * 8 / (now - previousTime) : 0;
    previousBytes = bytes; previousTime = now;
    return QString("%1 kbps · %2 FPS · %3 frames perdidos")
        .arg(kbps, 0, 'f', 0).arg(obs_get_active_fps(), 0, 'f', 1).arg(obs_output_get_frames_dropped(output));
}
void Engine::releaseOutput() {
    if (output) { obs_output_release(output); output = nullptr; }
    if (service) { obs_service_release(service); service = nullptr; }
    if (video) { obs_encoder_release(video); video = nullptr; }
    if (aac) { obs_encoder_release(aac); aac = nullptr; }
}
Engine::~Engine() {
    releaseOutput();
    if (initialized) {
        disableCamera();
        for (int i = 0; i < 3; ++i) obs_set_output_source(i, nullptr);
        if (scene) obs_scene_release(scene);
        if (capture) obs_source_release(capture);
        if (mic) obs_source_release(mic);
        if (sound) obs_source_release(sound);
        obs_shutdown();
    }
}
}
