// Copyright 2019 Vilya Harvey
#ifndef VH_RENDERWIDGET_H
#define VH_RENDERWIDGET_H

#include "FileCache.h"
#include "FPSCounter.h"
#include "RenderData.h"
#include "ShaderToy.h"
#include "TextureVideoSurface.h"
#include "Timer.h"

#include <QFont>
#include <QHash>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMap>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QPen>
#include <QString>

#include <QCamera>
#include <QMediaPlayer>

#ifdef SHADERTOOL_USE_GL41
#include <QOpenGLFunctions_4_1_Core>
#else
#include <QOpenGLFunctions_4_5_Core>
#endif

namespace vh {

  //
  // Constants
  //

  enum class Action {
    eNone,
    eQuit,
    eToggleHUD,
    eToggleInputs,
    eToggleOutputs,
    eTogglePlayback,
    eRestartPlayback,
    eFastForward_Small,
    eFastForward_Medium,
    eFastForward_Large,
    eRewind_Small,
    eRewind_Medium,
    eRewind_Large,
    eToggleKeyboardInput,
    eCenterImage,
    eZoomImageIn_Coarse,
    eZoomImageIn_Fine,
    eZoomImageOut_Coarse,
    eZoomImageOut_Fine,
    eCaptureSingleFrame,
    eCaptureScreenshot,
  };


  enum class MouseAction {
    eNone,
    eSendToShader,
    ePanImage,
    eZoomImage,
  };


  enum class Capture {
    eNothing,
    eSingleFrame,
    eScreenshot,
  };


  static constexpr uint kHUD_FrameNum       = 1u << 0;
  static constexpr uint kHUD_Time           = 1u << 1;
  static constexpr uint kHUD_MillisPerFrame = 1u << 2;
  static constexpr uint kHUD_FramesPerSec   = 1u << 3;
  static constexpr uint kHUD_MousePos       = 1u << 4;
  static constexpr uint kHUD_MouseDownPos   = 1u << 5;

  static constexpr uint kHUD_All = kHUD_FrameNum | kHUD_Time | kHUD_MillisPerFrame |
                                   kHUD_FramesPerSec | kHUD_MousePos | kHUD_MouseDownPos;


  //
  // Structs
  //

  struct KeyBinding {
    int key;
    Qt::KeyboardModifiers modifiers;

    inline bool operator == (const KeyBinding& other) const { return key == other.key && modifiers == other.modifiers; }
    inline bool operator != (const KeyBinding& other) const { return key != other.key || modifiers != other.modifiers; }

    static KeyBinding fromEvent(const QKeyEvent* event) {
      return KeyBinding{ event->key(), event->modifiers() };
    }
  };


  inline uint qHash(KeyBinding kb)
  {
    return ::qHash(kb.key) * 17 + ::qHash(kb.modifiers);
  }


  struct MouseBinding {
    MouseAction activeAction; // The action that this binding applies for.
    Qt::MouseButton button;               // The button that triggers the actionn, i.e. Qt::LeftButton.
    Qt::KeyboardModifiers modifiers;  // The modifier keys that must be active when the button is pressed for the action to trigger.

    inline bool operator == (const MouseBinding& other) const { return activeAction == other.activeAction && button == other.button && modifiers == other.modifiers; }
    inline bool operator != (const MouseBinding& other) const { return activeAction != other.activeAction || button != other.button || modifiers != other.modifiers; }

    static MouseBinding fromEvent(MouseAction activeAction, const QMouseEvent* event) {
      return MouseBinding{ activeAction, event->button(), event->modifiers() };
    }
  };


  inline uint qHash(MouseBinding mb)
  {
    return (::qHash(int(mb.activeAction)) * 17 + ::qHash(mb.button)) * 17 + ::qHash(mb.modifiers);
  }


  enum class WheelDirection {
    eUp,
    eDown,
  };


  struct WheelBinding {
    WheelDirection direction;
    Qt::KeyboardModifiers modifiers;

    inline bool operator == (const WheelBinding& other) const { return direction == other.direction && modifiers == other.modifiers; }
    inline bool operator != (const WheelBinding& other) const { return direction != other.direction || modifiers != other.modifiers; }

    static WheelBinding fromEvent(const QWheelEvent* event) {
      WheelDirection direction = (event->angleDelta().y() >= 0) ? WheelDirection::eUp : WheelDirection::eDown;
      return WheelBinding{ direction, event->modifiers() };
    }
  };


  inline uint qHash(WheelBinding wb)
  {
    return ::qHash(int(wb.direction)) * 17 + ::qHash(wb.modifiers);
  }


  //
  // RenderWidget class
  //

  class RenderWidget :
      public QOpenGLWidget,
    #ifdef SHADERTOOL_USE_GL41
      protected QOpenGLFunctions_4_1_Core
    #else
      protected QOpenGLFunctions_4_5_Core
    #endif // SHADERTOOL_USE_GL41
  {
    Q_OBJECT

  public:
    explicit RenderWidget(QWidget* parent = nullptr);
    virtual ~RenderWidget();

    void setFileCache(FileCache* cache);

    ShaderToyDocument* currentShaderToyDocument() const;
    void setShaderToyDocument(ShaderToyDocument* newDoc);

    bool keyboardShaderInput() const;

    uint hudFlags() const;

    int renderWidth() const;
    int renderHeight() const;
    int displayWidth() const;
    int displayHeight() const;
    void displayRect(int& dstX, int& dstY, int& dstW, int& dstH) const;

  signals:
    void closeRequested();

    void currentShaderToyDocumentChanged();

    void keyboardShaderInputChanged(bool newValue);
    void mouseShaderInputChanged(bool newValue);

    void frameCaptured(const QImage& frame);

  public slots:
    void startPlayback();
    void stopPlayback();
    void resumePlayback();
    void adjustPlaybackTime(double amountMS);
    void togglePlayback();

    void setFixedRenderResolution(int w, int h);
    void setRelativeRenderResolution(float windowScale);
    void setDisplayOptions(bool fitWidth, bool fitHeight, float scale);
    void setDisplayPassByOutputID(int outputID);
    void toggleHUDFlag(uint flag);
    void toggleHUD();
    void toggleInputs();
    void toggleOutputs();
    void recenterImage();

    void zoom(QPoint center, float newScale);

    void setKeyboardShaderInput(bool enabled);

    void reloadCurrentShaderToyDocument();

    void doAction(Action action);

  protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void wheelEvent(QWheelEvent* event);

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

  private:
    void makePendingDocCurrent();

    void setupRenderData();
    void teardownRenderData();
    void updateRenderData();
    void renderMain();
    void renderIntermediates();
    void renderEmpty();
    void renderHUD(QPainter& painter);

    bool inputIsRenderPass(const ShaderToyInput& input) const;

    void createRenderPassTexture(Texture& tex, PassType passType);
    void resizeRenderPassTexture(Texture& tex);
    bool loadImageTexture(const QString& filename, bool flip, bool srgb, Texture& tex);
    bool loadCubemapTexture(const QString& filename, bool flip, bool srgb, Texture& tex);

    bool loadVideo(const QString& filename, bool flip, int vidIndex);
    bool loadAudio(const QString& filename, int audIndex);

    int allocVideoTexture(); // Texture has no storage yet, because we don't know the width & height until after this is called.
    int allocAudioTexture();
    void resizeTextureForVideo(TextureVideoSurface* surface, QOpenGLTexture* texObj);
    void flipTexture(QOpenGLTexture* texObj, QOpenGLTexture* flippedTexObj);

    void updateShaderMousePos(QPoint mousePosWithFlippedY, bool setDownPos);

    float framebufferWidth() const;
    float framebufferHeight() const;
    QPoint framebufferPos(const QPoint& widgetPos) const;

    void screenshot();    //!< Captures at display resolution, includes all visible decorations (HUD, inputs/outputs, etc).
    void captureFrame();  //!< Captures at render resolution, no decorations visible.

  private slots:
    void fileChanged(const QString& path);
    void videoError(QMediaPlayer::Error err, int vidIndex);
    void audioError(QMediaPlayer::Error err, int audIndex);

  private:
    Timer _runtimeTimer;
    Timer _playbackTimer;
    float _prevTime = 0.0f;
    FPSCounter _fpsCounter;

    bool _showHUD = true;
    QFont _hudFont;
    QPen _hudPen;
    int _lineHeight;

    bool _showInputs = false;
    bool _showOutputs = false;

    ShaderToyDocument* _currentDoc = nullptr;
    ShaderToyDocument* _pendingDoc = nullptr;
    bool _forceReload = false;
    bool _resized = false;

    bool _clearTextures = true;

    RenderData _renderData;

    int _displayPass = -1; // Which render pass to display output from.

    int _renderWidth            = 800;
    int _renderHeight           = 450;
    float _renderScale          = 0.5f;
    float _renderHeightScale    = 0.5f;
    bool _useRelativeRenderSize = false;

    bool _displayFitWidth = true;
    bool _displayFitHeight = false;
    float _displayScale = 1.0f;
    float _displayPanX = 0.0f;
    float _displayPanY = 0.0f;

    bool _keyboardShaderInput = false;

    QHash<KeyBinding, Action> _keyPressBindings;
    QHash<KeyBinding, Action> _keyReleaseBindings;
    QHash<MouseBinding, MouseAction> _mousePressBindings;
    QHash<MouseBinding, MouseAction> _mouseReleaseBindings;
    QHash<WheelBinding, Action> _wheelBindings;

    FileCache* _cache = nullptr;

    MouseAction _mouseAction = MouseAction::eNone;
    QPoint _mouseDown = QPoint();
    QPoint _mousePos = QPoint();
    float _initialDisplayScale = 1.0f;
    float _initialPanX = 0.0f;
    float _initialPanY = 0.0f;

    uint _hudFlags = kHUD_All; // A bit field. See the kHUD_<foo> constants above for what each bit means.

    Capture _capture = Capture::eNothing;
  };

} // namespace vh

Q_DECLARE_METATYPE(vh::Action)

Q_DECLARE_METATYPE(vh::MouseAction)

#endif // VH_RENDERWIDGET_H
