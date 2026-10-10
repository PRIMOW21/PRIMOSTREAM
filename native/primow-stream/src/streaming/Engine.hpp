#pragma once
#include <QObject>
#include <QStringList>
#include <obs.h>
namespace primow {
enum class Capture { Window, Game, Display };
class Engine : public QObject {
    Q_OBJECT
public:
    explicit Engine(QObject *parent = nullptr);
    ~Engine();
    void initialize();
    void select(Capture method, const QString &window, const QString &monitorId = {});
    void audio(bool application, const QString &window);
    void mute(bool microphone, bool value);
    void volume(float value);
    void start(const QString &server, const QString &key, const QString &serviceName,
               int bitrate, int profile);
    void stop();
    bool busy() const { return running; }
    QString metrics();
    bool captureReady() const;
signals:
    void stateChanged(QString state);
private:
    bool initialized = false, running = false;
    obs_source_t *capture = nullptr, *mic = nullptr, *sound = nullptr;
    obs_scene_t *scene = nullptr;
    obs_sceneitem_t *item = nullptr;
    obs_output_t *output = nullptr;
    obs_service_t *service = nullptr;
    obs_encoder_t *video = nullptr, *aac = nullptr;
    uint64_t previousBytes = 0;
    qint64 previousTime = 0;
    static void started(void *, calldata_t *);
    static void stopped(void *, calldata_t *);
    static void reconnect(void *, calldata_t *);
    static void reconnected(void *, calldata_t *);
    void releaseOutput();
};
}
