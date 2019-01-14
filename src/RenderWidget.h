// Copyright 2019 Vilya Harvey
#ifndef VH_RENDERWIDGET_H
#define VH_RENDERWIDGET_H

#include "FPSCounter.h"
#include "RenderData.h"
#include "ShaderToy.h"
#include "Timer.h"

#include <QFont>
#include <QHash>
#include <QKeyEvent>
#include <QKeySequence>
#include <QMap>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QPen>
#include <QString>

namespace vh {

  //
  // Constants
  //

  enum class Action {
    eNoAction,
    eQuit,
    eToggleOverlay,
    eTogglePlayback,
    eRestartPlayback,
    eFastForward_Small,
    eFastForward_Medium,
    eFastForward_Large,
    eRewind_Small,
    eRewind_Medium,
    eRewind_Large,
  };


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


  //
  // RenderWidget class
  //

  class RenderWidget :
      public QOpenGLWidget,
      protected QOpenGLFunctions_4_5_Core
  {
    Q_OBJECT

  public:
    explicit RenderWidget(QWidget* parent = nullptr);
    virtual ~RenderWidget();

    void setShaderToyDocument(ShaderToyDocument* newDoc);

  signals:
    void closeRequested();

  public slots:
    void startPlayback();
    void stopPlayback();
    void resumePlayback();
    void adjustPlaybackTime(double amountMS);

    void setFixedRenderResolution(int w, int h);
    void setRelativeRenderResolution(float wScale, float hScale);
    void setDisplayOptions(bool fitWidth, bool fitHeight, float scale);

    void doAction(Action action);

  protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);

  private:
    void makePendingDocCurrent();

    void setupRenderData();
    void teardownRenderData();
    void updateRenderData();
    void render();

    void createRenderPassTexture(Texture& tex);
    void resizeRenderPassTexture(Texture& tex);
    void loadImageTexture(const QString& filename, bool flip, Texture& tex);
    void loadCubemapTexture(const QString& filename, bool flip, Texture& tex);

    int renderWidth() const;
    int renderHeight() const;
    void displayRect(int& dstX, int& dstY, int& dstW, int& dstH) const;

    void widgetToRenderCoords(int widgetX, int widgetY, float& renderX, float& renderY);

  private:
    Timer _runtimeTimer;
    Timer _playbackTimer;
    float _prevTime = 0.0f;
    FPSCounter _fpsCounter;

    bool _showOverlay = true;
    QFont _overlayFont;
    QPen _overlayPen;
    int _lineHeight;

    ShaderToyDocument* _currentDoc = nullptr;
    ShaderToyDocument* _pendingDoc = nullptr;
    bool _resized = false;

    bool _clearTextures = true;

    RenderData _renderData;

    int _renderWidth            = 640;
    int _renderHeight           = 360;
    float _renderWidthScale     = 0.5f;
    float _renderHeightScale    = 0.5f;
    bool _useRelativeRenderSize = false;

    bool _displayFitWidth = true;
    bool _displayFitHeight = false;
    float _displayScale = 1.0f;

    QHash<KeyBinding, Action> _keyPressBindings;
    QHash<KeyBinding, Action> _keyReleaseBindings;
  };

} // namespace vh

Q_DECLARE_METATYPE(vh::Action)

#endif // VH_RENDERWIDGET_H
