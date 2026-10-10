#include "Engine.hpp"
#include <stdexcept>
#include <algorithm>

namespace primow {
QVector<QPair<QString, QString>> Engine::cameras() const {
    QVector<QPair<QString, QString>> result;
    auto props = obs_get_source_properties("dshow_input");
    if (!props) return result;
    auto devices = obs_properties_get(props, "video_device_id");
    if (devices) {
        for (size_t i = 0; i < obs_property_list_item_count(devices); ++i) {
            const auto id = obs_property_list_item_string(devices, i);
            if (id && *id && !obs_property_list_item_disabled(devices, i))
                result.push_back({QString::fromUtf8(obs_property_list_item_name(devices, i)),
                                  QString::fromUtf8(id)});
        }
    }
    obs_properties_destroy(props);
    return result;
}

void Engine::enableCamera(const QString &deviceId) {
    if (deviceId.isEmpty()) throw std::runtime_error("Selecciona una cámara conectada");
    // Native device defaults avoid requesting a resolution the camera cannot deliver.
    auto settings = obs_data_create();
    obs_data_set_string(settings, "video_device_id", deviceId.toUtf8().constData());
    obs_data_set_bool(settings, "deactivate_when_not_showing", true);
    auto next = obs_source_create("dshow_input", "Cámara web", settings, nullptr);
    obs_data_release(settings);
    if (!next) throw std::runtime_error("No se pudo abrir la cámara. Revisa permisos y otras aplicaciones.");
    // The selected microphone stays independent; never mix the webcam's audio.
    obs_source_set_audio_mixers(next, 0);
    auto nextItem = obs_scene_add(scene, next);
    if (!nextItem) {
        obs_source_release(next);
        throw std::runtime_error("No se pudo añadir la cámara a la transmisión");
    }
    disableCamera();
    camera = next;
    cameraItem = nextItem;
    positionCamera(3, 384);
}

void Engine::disableCamera() {
    if (cameraItem) obs_sceneitem_remove(cameraItem);
    cameraItem = nullptr;
    if (camera) obs_source_release(camera);
    camera = nullptr;
}

void Engine::positionCamera(int corner, int width) {
    if (!cameraItem) return;
    corner = std::clamp(corner, 0, 3);
    width = std::clamp(width, 192, 640);
    const float w = float(width), h = w * 9.0f / 16.0f;
    vec2 bounds{w, h};
    vec2 position{(corner % 2) ? 1920.0f - w - 24.0f : 24.0f,
                  (corner >= 2) ? 1080.0f - h - 24.0f : 24.0f};
    obs_sceneitem_defer_update_begin(cameraItem);
    obs_sceneitem_set_alignment(cameraItem, OBS_ALIGN_TOP | OBS_ALIGN_LEFT);
    obs_sceneitem_set_bounds_type(cameraItem, OBS_BOUNDS_SCALE_INNER);
    obs_sceneitem_set_bounds_alignment(cameraItem, OBS_ALIGN_CENTER);
    obs_sceneitem_set_bounds(cameraItem, &bounds);
    obs_sceneitem_set_pos(cameraItem, &position);
    obs_sceneitem_defer_update_end(cameraItem);
    obs_sceneitem_set_order(cameraItem, OBS_ORDER_MOVE_TOP);
}

bool Engine::cameraReady() const {
    return camera && obs_source_get_width(camera) > 0 && obs_source_get_height(camera) > 0;
}
}
