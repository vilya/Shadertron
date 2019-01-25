// Copyright 2019 Vilya Harvey
#include "RenderWidget.h"

#include <QMessageLogger>
#include <QOpenGLPixelTransferOptions>
#include <QPainter>
#include <QRegularExpression>
#include <QTimer>

#include <QMediaPlaylist>

namespace vh  {

  //
  // Constants
  //

  static constexpr double kSmallStepMS = 100.0;
  static constexpr double kMediumStepMS = 1000.0;
  static constexpr double kLargeStepMS = 10000.0;

  static const QString kMainFunc_Image =
      "void main() {\n"
      "  vec2 fragCoord = gl_FragCoord.xy;\n"
      "  vec4 fragColor;\n"
      "  mainImage(fragColor, fragCoord);\n"
      "  ShaderToolQt_oColor = fragColor;\n"
      "}\n";


  //
  // Private helper functions
  //

  static void handleGLError(GLenum source,
                            GLenum type,
                            GLuint /*id*/,
                            GLenum severity,
                            GLsizei /*length*/,
                            const GLchar* message,
                            const void* /*userParam*/)
  {
    const char* sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API:             sourceStr = "api"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "window system"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "shader compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "thirdparty"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "other"; break;
    default:                              sourceStr = "<unknown>"; break;
    }

    const char* typeStr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "deprecated behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "undefined behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "performance"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "other"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "push group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "pop group"; break;
    default:                                typeStr = "<unknown>"; break;
    };

    const char* severityStr;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "notification"; break;
    default:                             severityStr = "<unknown>"; break;
    };

    if (type == GL_DEBUG_TYPE_ERROR) {
      qCritical("OpenGL message [source=%s, type=%s, severity=%s]: %s", sourceStr, typeStr, severityStr, message);
    }
    else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
      qInfo("OpenGL message [source=%s, type=%s, severity=%s]: %s", sourceStr, typeStr, severityStr, message);
    }
    else {
      qWarning("OpenGL message [source=%s, type=%s, severity=%s]: %s", sourceStr, typeStr, severityStr, message);
    }
  }


  static QString preprocessShaderSource(const QString& filename, const QMap<QString, QString>& macros)
  {
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists()) {
      return "";
    }

    // Replace all #macro lines with their replacement text.
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      return "";
    }

    QString code = QString::fromLocal8Bit(file.readAll());
    file.close();

    QMapIterator<QString, QString> it(macros);
    while (it.hasNext()) {
      it.next();
      QString before = QString("#macro %1").arg(it.key());
      QString after = it.value();
      code.replace(before, after);
    }

    // Remove any remaining unmatched #macro lines.
    QRegularExpression macroRE("#macro.*$", QRegularExpression::MultilineOption);
    code.replace(macroRE, "");

    return code;
  }


  //
  // RenderWidget public methods
  //

  RenderWidget::RenderWidget(QWidget* parent) :
    QOpenGLWidget(parent),
    _hudFont("Consolas", 16, QFont::Bold),
    _hudPen()
  {
    setFocusPolicy(Qt::ClickFocus);

    _hudPen.setStyle(Qt::DashLine);
    _hudPen.setColor(Qt::red);

    _lineHeight = QFontMetrics(_hudFont).height();

    _keyPressBindings[KeyBinding{ Qt::Key_Escape, 0 }] = Action::eQuit;
    _keyPressBindings[KeyBinding{ Qt::Key_Tab,    0 }] = Action::eToggleHUD;
    _keyPressBindings[KeyBinding{ Qt::Key_Space,  0 }] = Action::eTogglePlayback;
    _keyPressBindings[KeyBinding{ Qt::Key_Enter,  0 }] = Action::eRestartPlayback;
    _keyPressBindings[KeyBinding{ Qt::Key_Return, 0 }] = Action::eRestartPlayback;

    _keyPressBindings[KeyBinding{ Qt::Key_Right,  Qt::ShiftModifier   }] = Action::eFastForward_Small;
    _keyPressBindings[KeyBinding{ Qt::Key_Right,  0                   }] = Action::eFastForward_Medium;
    _keyPressBindings[KeyBinding{ Qt::Key_Right,  Qt::ControlModifier }] = Action::eFastForward_Large;
    _keyPressBindings[KeyBinding{ Qt::Key_Left,   Qt::ShiftModifier   }] = Action::eRewind_Small;
    _keyPressBindings[KeyBinding{ Qt::Key_Left,   0                   }] = Action::eRewind_Medium;
    _keyPressBindings[KeyBinding{ Qt::Key_Left,   Qt::ControlModifier }] = Action::eRewind_Large;

    _keyReleaseBindings[KeyBinding{ Qt::Key_F2, 0 }] = Action::eToggleKeyboardInput;

    _mousePressBindings[MouseBinding{ MouseAction::eNone, Qt::LeftButton, Qt::NoModifier      }] = MouseAction::eSendToShader;
    _mousePressBindings[MouseBinding{ MouseAction::eNone, Qt::LeftButton, Qt::AltModifier     }] = MouseAction::ePanImage;
    _mousePressBindings[MouseBinding{ MouseAction::eNone, Qt::LeftButton, Qt::ControlModifier }] = MouseAction::eZoomImage;

//    _mouseReleaseBindings[MouseBinding{ MouseAction::ePanImage,  Qt::LeftButton, Qt::AltModifier     }] = MouseAction::eNone;
//    _mouseReleaseBindings[MouseBinding{ MouseAction::eZoomImage, Qt::LeftButton, Qt::ControlModifier }]  = MouseAction::eNone;

    _wheelBindings[WheelBinding{ WheelDirection::eUp,   Qt::NoModifier    }] = Action::eZoomImageIn_Coarse;
    _wheelBindings[WheelBinding{ WheelDirection::eUp,   Qt::ShiftModifier }] = Action::eZoomImageIn_Fine;
    _wheelBindings[WheelBinding{ WheelDirection::eDown, Qt::NoModifier    }] = Action::eZoomImageOut_Coarse;
    _wheelBindings[WheelBinding{ WheelDirection::eDown, Qt::ShiftModifier }] = Action::eZoomImageOut_Fine;

    _runtimeTimer.start();
  }


  RenderWidget::~RenderWidget()
  {
    if (_pendingDoc != _currentDoc) {
      delete _pendingDoc;
    }
  }


  void RenderWidget::setFileCache(FileCache* cache)
  {
    _cache = cache;
  }


  ShaderToyDocument* RenderWidget::currentShaderToyDocument() const
  {
    return _currentDoc;
  }


  void RenderWidget::setShaderToyDocument(ShaderToyDocument* newDoc)
  {
    _pendingDoc = newDoc;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  bool RenderWidget::keyboardShaderInput() const
  {
    return _keyboardShaderInput;
  }


  uint RenderWidget::hudFlags() const
  {
    return _hudFlags;
  }


  int RenderWidget::renderWidth() const
  {
    return _useRelativeRenderSize ? int(width() * _renderScale) : _renderWidth;
  }


  int RenderWidget::renderHeight() const
  {
    return _useRelativeRenderSize ? int(height() * _renderScale) : _renderHeight;
  }


  int RenderWidget::displayWidth() const
  {
    return int(float(renderWidth()) * _displayScale);
  }


  int RenderWidget::displayHeight() const
  {
    return int(float(renderHeight()) * _displayScale);
  }


  void RenderWidget::displayRect(int& dstX, int& dstY, int& dstW, int& dstH) const
  {
    dstX = _displayPanX;
    dstY = _displayPanY;
    dstW = displayWidth();
    dstH = displayHeight();
  }


  //
  // RenderWidget public slots
  //

  void RenderWidget::startPlayback()
  {
    bool wasPlayingBack = _playbackTimer.running();

    for (int i = 0; i < _renderData.numVideos; i++) {
      Video& vid = _renderData.videos[i];
      vid.surface->unpause();
      vid.player->stop();
      vid.player->play();
    }

    if (_renderData.hasCamera) {
      _renderData.camera.obj->start();
    }

    _renderData.iTime = 0.0f;
    _renderData.iFrame = 0;

    _playbackTimer.start();
    _prevTime = 0.0f;

    if (!wasPlayingBack) {
      update();
    }
  }


  void RenderWidget::stopPlayback()
  {
    for (int i = 0; i < _renderData.numVideos; i++) {
      Video& vid = _renderData.videos[i];
      vid.surface->pause();
      vid.player->pause();
    }

    if (_renderData.hasCamera) {
      _renderData.camera.obj->stop();
    }

    _playbackTimer.stop();

    update();
  }


  void RenderWidget::resumePlayback()
  {
    bool wasPlayingBack = _playbackTimer.running();

    for (int i = 0; i < _renderData.numVideos; i++) {
      Video& vid = _renderData.videos[i];
      vid.surface->unpause();
      vid.player->play();
    }

    if (_renderData.hasCamera) {
      _renderData.camera.obj->start();
    }

    float prevTimeDelta = _renderData.iTime - _prevTime;
    _playbackTimer.resume();
    _prevTime = _playbackTimer.elapsedSecs() - prevTimeDelta;

    if (!wasPlayingBack) {
      update();
    }
  }


  void RenderWidget::adjustPlaybackTime(double amountMS)
  {
    bool wasPlayingBack = _playbackTimer.running();

    for (int i = 0; i < _renderData.numVideos; i++) {
      Video& vid = _renderData.videos[i];
      if (vid.player->isSeekable()) {
        vid.player->setPosition(vid.player->position() + qint64(amountMS));
      }
    }

    _playbackTimer.adjustTimeMS(amountMS);

    if (!wasPlayingBack) {
      update();
    }
  }


  void RenderWidget::togglePlayback()
  {
    if (_playbackTimer.running()) {
      stopPlayback();
    }
    else {
      resumePlayback();
    }
  }


  void RenderWidget::setFixedRenderResolution(int w, int h)
  {
    int oldDisplayW = displayWidth();
    int oldDisplayH = displayHeight();

    _useRelativeRenderSize = false;
    _renderWidth = w;
    _renderHeight = h;

    _resized = true;

    _displayPanX -= float(displayWidth()  - oldDisplayW) * 0.5f;
    _displayPanY -= float(displayHeight() - oldDisplayH) * 0.5f;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::setRelativeRenderResolution(float windowScale)
  {
    int oldDisplayW = displayWidth();
    int oldDisplayH = displayHeight();

    _useRelativeRenderSize = true;
    _renderScale = windowScale;

    _resized = true;

    _displayPanX -= float(displayWidth()  - oldDisplayW) * 0.5f;
    _displayPanY -= float(displayHeight() - oldDisplayH) * 0.5f;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::setDisplayOptions(bool fitWidth, bool fitHeight, float scale)
  {
    int oldDisplayW = displayWidth();
    int oldDisplayH = displayHeight();

    _displayFitWidth = fitWidth;
    _displayFitHeight = fitHeight;

    if (fitWidth) {
      _displayScale = float(width()) / float(renderWidth());
    }
    else if (fitHeight) {
      _displayScale = float(height()) / float(renderHeight());
    }
    else {
      _displayScale = scale;
    }

    if (fitWidth || fitHeight) {
      _displayPanX = (width()  - displayWidth()) * 0.5f;
      _displayPanY = (height() - displayHeight()) * 0.5f;
    }
    else {
      _displayPanX -= (displayWidth()  - oldDisplayW) * 0.5f;
      _displayPanY -= (displayHeight() - oldDisplayH) * 0.5f;
    }

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::setDisplayPassByType(PassType passType, int index)
  {
    if (passType == PassType::eSound || passType == PassType::eCubemap) {
      return;
    }

    int newPassIndex = _displayPass;
    for (int i = 0; i < _renderData.numRenderpasses; i++) {
      if (_renderData.renderpasses[i].type == passType) {
        if (index == 0) {
          newPassIndex = i;
          break;
        }
        else {
          --index;
        }
      }
    }

    if (newPassIndex == _displayPass) {
      return;
    }

    _displayPass = newPassIndex;
    qDebug("Display pass set to %s (idx = %d)", qPrintable(_renderData.renderpasses[_displayPass].name), _displayPass);

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::toggleHUDFlag(uint flag)
  {
    flag &= kHUD_All;
    if (flag == 0) {
      return;
    }

    _hudFlags ^= flag;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::toggleHUD()
  {
    _showHUD = !_showHUD;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::toggleInputs()
  {
    _showInputs = !_showInputs;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::toggleOutputs()
  {
    _showOutputs = !_showOutputs;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::recenterImage()
  {
    _displayPanX = (width() -  displayWidth())  * 0.5f;
    _displayPanY = (height() - displayHeight()) * 0.5f;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::zoom(QPoint center, float newScale)
  {
    float cx = float(center.x());
    float cy = float(center.y());
    float relScale = newScale / _displayScale;
    _displayPanX = (_displayPanX - cx) * relScale + cx;
    _displayPanY = (_displayPanY - cy) * relScale + cy;
    _displayScale = newScale;

    if (!_playbackTimer.running()) {
      update();
    }
  }


  void RenderWidget::setKeyboardShaderInput(bool enabled)
  {
    _keyboardShaderInput = enabled;
  }


  void RenderWidget::reloadCurrentShaderToyDocument()
  {
    _forceReload = true;
  }


  void RenderWidget::doAction(Action action)
  {
    switch (action) {
    case Action::eQuit:
      emit closeRequested();
      break;
    case Action::eToggleHUD:
      toggleHUD();
      break;
    case Action::eToggleInputs:
      toggleInputs();
      break;
    case Action::eToggleOutputs:
      toggleOutputs();
      break;
    case Action::eTogglePlayback:
      togglePlayback();
      break;
    case Action::eRestartPlayback:
      startPlayback();
      break;
    case Action::eFastForward_Small:
      adjustPlaybackTime(kSmallStepMS);
      break;
    case Action::eFastForward_Medium:
      adjustPlaybackTime(kMediumStepMS);
      break;
    case Action::eFastForward_Large:
      adjustPlaybackTime(kLargeStepMS);
      break;
    case Action::eRewind_Small:
      adjustPlaybackTime(-kSmallStepMS);
      break;
    case Action::eRewind_Medium:
      adjustPlaybackTime(-kMediumStepMS);
      break;
    case Action::eRewind_Large:
      adjustPlaybackTime(-kLargeStepMS);
      break;
    case Action::eToggleKeyboardInput:
      // Do nothing. The event should bubble up to be handled by the main window.
//      setKeyboardShaderInput(!_keyboardShaderInput);
//      emit keyboardShaderInputChanged(_keyboardShaderInput);
      break;
    case Action::eCenterImage:
      // Do nothing. The event should bubble up to be handled by the main window.
      break;
    case Action::eZoomImageIn_Coarse:
      zoom(_mousePos, _displayScale * 1.5f);
      break;
    case Action::eZoomImageIn_Fine:
      zoom(_mousePos, _displayScale * 1.05f);
      break;
    case Action::eZoomImageOut_Coarse:
      zoom(_mousePos, _displayScale * 0.5f);
      break;
    case Action::eZoomImageOut_Fine:
      zoom(_mousePos, _displayScale * 0.95f);
      break;
    default:
      break;
    }
  }


  //
  // RenderWidget protected methods
  //

  void RenderWidget::initializeGL()
  {
    initializeOpenGLFunctions();

    // Set up OpenGL debugging
    glDebugMessageCallback(handleGLError, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);                      // Enable all messages
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);   // Disable messages with severity='notification' messages
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);     // Disable messages with type='other' and severity='low'
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);     // Disable messages with type='performance' and severity='low'
    glDebugMessageControl(GL_DEBUG_SOURCE_APPLICATION, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);       // Enable all messages with source='application'
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  }


  void RenderWidget::paintGL()
  {
    _fpsCounter.newFrame(_runtimeTimer.elapsedMS());

    QPainter painter(this);
    painter.beginNativePainting();

    updateRenderData();
    if (_currentDoc != nullptr) {
      renderMain();
      renderIntermediates(); // returns early if nothing to be drawn.
    }
    else {
      renderEmpty();
    }

    painter.endNativePainting();

    if (_showHUD) {
      renderHUD(painter);
    }

    if (_currentDoc != nullptr) {
      ++_renderData.iFrame;
      _prevTime = _renderData.iTime;
    }

    // Clear the "key pressed" flag for all keys. The flag only stays set for
    // the duration of one frame.
    for (int i = 0; i < 256; i++) {
      _renderData.keyboardTexData[1][i] = 0;
    }

    if (_playbackTimer.running()) {
      QTimer::singleShot(1, this, SLOT(update()));
    }
  }


  void RenderWidget::resizeGL(int /*w*/, int /*h*/)
  {
    if (_useRelativeRenderSize) {
      // Resizing of resources will be done during the next `paintGL` call, so
      // that we can ensure it doesn't happen while we're trying to render.
      _resized = true;
    }

    _initialDisplayScale = _displayScale;
    if (_displayFitWidth) {
      _displayScale = float(width()) / float(renderWidth());
    }
    else if (_displayFitHeight) {
      _displayScale = float(height()) / float(renderHeight());
    }

    recenterImage();
  }


  void RenderWidget::mousePressEvent(QMouseEvent* event)
  {
    _mouseDown = event->pos();
    _mouseDown.setY(height() - _mouseDown.y());

    // Figure out what action we're performing.
    _mouseAction = _mousePressBindings.value(MouseBinding::fromEvent(_mouseAction, event), MouseAction::eNone);

    // Begin the action.
    switch (_mouseAction) {
    case MouseAction::eSendToShader:
      updateShaderMousePos(_mouseDown, true);
      break;
    case MouseAction::ePanImage:
      _initialPanX = _displayPanX;
      _initialPanY = _displayPanY;
      break;
    case MouseAction::eZoomImage:
      _initialDisplayScale = _displayScale;
      _initialPanX = _displayPanX;
      _initialPanY = _displayPanY;
      break;
    default:
      break;
    }
  }


  void RenderWidget::mouseMoveEvent(QMouseEvent* event)
  {
    _mousePos = event->pos();
    _mousePos.setY(height() - _mousePos.y());

    switch (_mouseAction) {
    case MouseAction::eSendToShader:
      updateShaderMousePos(_mousePos, false);
      break;
    case MouseAction::ePanImage:
      _displayPanX = _initialPanX + _mousePos.x() - _mouseDown.x();
      _displayPanY = _initialPanY + _mousePos.y() - _mouseDown.y();
      break;
    case MouseAction::eZoomImage:
      {
        float minScale = 4.0f / qMin(renderWidth(), renderHeight()); // Don't go below 4 pixels wide or tall.
        _displayScale = qMax(_initialDisplayScale + float(_mousePos.x() - _mouseDown.x()) / 200.0f, minScale);
        float relativeScale = _displayScale /  _initialDisplayScale;
        _displayPanX = (_initialPanX - _mouseDown.x()) * relativeScale + _mouseDown.x();
        _displayPanY = (_initialPanY - _mouseDown.y()) * relativeScale + _mouseDown.y();
      }
      break;
    default:
      break;
    }
  }


  void RenderWidget::mouseReleaseEvent(QMouseEvent* event)
  {
    // Figure out what action we're performing.
    MouseAction newMouseAction = _mouseReleaseBindings.value(MouseBinding::fromEvent(_mouseAction, event), MouseAction::eNone);

    switch (_mouseAction) {
    case MouseAction::eSendToShader:
      _renderData.iMouse[2] = -_renderData.iMouse[2];
      _renderData.iMouse[3] = -_renderData.iMouse[3];
      break;
    case MouseAction::ePanImage:
      break;
    case MouseAction::eZoomImage:
      break;
    default:
      break;
    }

    _mouseAction = newMouseAction;
  }


  void RenderWidget::wheelEvent(QWheelEvent* event)
  {
    // For now, ignore wheel events while there's a mouse action in progress.
    if (_mouseAction != MouseAction::eNone) {
      event->ignore();
      return;
    }

    // `angleDelta()` is in 8ths of a degree, so we must divide the value by
    // 8 to get the number of degrees. Most mice increment by 15 degrees per
    // mouse wheel click.
    QPoint numDegrees = event->angleDelta();
    if (numDegrees.isNull() || numDegrees.y() == 0) {
      return;
    }

    _mousePos = event->pos();
    _mousePos.setY(height() - _mousePos.y());

    Action action = _wheelBindings.value(WheelBinding::fromEvent(event), Action::eNone);
    if (action != Action::eNone) {
      doAction(action);
    }
    else {
      event->ignore();
    }
  }


  void RenderWidget::keyPressEvent(QKeyEvent* event)
  {
    // Note: QKeyEvent is implicitly accepted. You have to call event->ignore()
    // if you don't want to accept it.

    Action action = _keyPressBindings.value(KeyBinding::fromEvent(event), Action::eNone);
    if (_keyboardShaderInput) {
      // Even if we're sending keyboard input to the shader, there are still
      // certain actions we should be able to trigger via the keyboard.
      if (action == Action::eToggleKeyboardInput ||
          action == Action::eQuit) {
        doAction(action);
        return;
      }

      // Update the keyboard texture
      int key = (event->text().size() == 1) ? event->text().toUpper().at(0).toLatin1() : event->nativeVirtualKey();
      _renderData.keyboardTexData[0][key] = 255;  // The key down flag
      _renderData.keyboardTexData[1][key] = 255;  // The key pressed flag, non-zero only on the frame where the key is first pressed.
      _renderData.keyboardTexData[2][key] ^= 255; // The key toggle. Flips each time the key is pressed.
    }
    else if (action != Action::eNone) {
      doAction(action);
    }
    else {
      event->ignore();
    }
  }


  void RenderWidget::keyReleaseEvent(QKeyEvent* event)
  {
    // Note: QKeyEvent is implicitly accepted. You have to call event->ignore()
    // if you don't want to accept it.

    Action action = _keyReleaseBindings.value(KeyBinding::fromEvent(event), Action::eNone);
    if (_keyboardShaderInput) {
      // Even if we're sending keyboard input to the shader, there are still
      // certain actions we should be able to trigger via the keyboard.
      if (action == Action::eToggleKeyboardInput) {
        event->ignore();
        return;
      }
      else if (action == Action::eQuit) {
        doAction(action);
        return;
      }

      // Update the keyboard texture
      int key = (event->text().size() == 1) ? event->text().toUpper().at(0).toLatin1() : event->nativeVirtualKey();
      _renderData.keyboardTexData[0][key] = 0;  // The key down flag
    }
    else if (action != Action::eNone) {
      doAction(action);
    }
    else {
      event->ignore();
    }
  }


  //
  // RenderWidget private methods
  //

  void RenderWidget::makePendingDocCurrent()
  {
    assert(_pendingDoc != _currentDoc);
    if (_currentDoc != nullptr) {
      teardownRenderData();
    }
    _currentDoc = _pendingDoc;
    if (_currentDoc != nullptr) {
      setupRenderData();
    }
    recenterImage();
    emit currentShaderToyDocumentChanged();
  }


  void RenderWidget::setupRenderData()
  {
    assert(_currentDoc != nullptr);

    // Assume that any old render data has already been cleared.

    glGenVertexArrays(1, &_renderData.defaultVAO);
    glGenFramebuffers(1, &_renderData.defaultFBO);
    glGenFramebuffers(1, &_renderData.flipFBO);

    _renderData.backBuffer = 0;
    _renderData.frontBuffer = 1;

    int commonIdx = _currentDoc->findRenderPassByType(kRenderPassType_Common);
    if (commonIdx != -1) {
      _renderData.commonSourceCode = _currentDoc->renderpasses[commonIdx].code;
    }

    _renderData.numTextures = kNumSpecialTextures;

    // Allocate the "no texture" texture.
    {
      Texture& tex = _renderData.textures[kTexture_PlaceholderImage];
      uchar blackPixel[3] = { 0, 0, 0 };
      QImage blackImage = QImage(blackPixel, 1, 1, QImage::Format_RGB888);
      tex.obj = new QOpenGLTexture(blackImage);
      tex.isBuffer = false;
      tex.playbackTime = 0.0f;
    }

    // Allocate the "no cubemap" cubemap.
    {
      Texture& tex = _renderData.textures[kTexture_PlaceholderCubemap];
      uchar whitePixel[3] = { 255, 255, 255 };
      tex.obj = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
      tex.obj->setSize(1, 1);
      tex.obj->setFormat(QOpenGLTexture::RGB8_UNorm);
      tex.obj->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
      tex.obj->setWrapMode(QOpenGLTexture::ClampToEdge);
      tex.obj->allocateStorage();
      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(1);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapNegativeX, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapPositiveX, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapNegativeY, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapPositiveY, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapNegativeZ, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->setData(0, 0, 1, QOpenGLTexture::CubeMapPositiveZ, QOpenGLTexture::RGB, QOpenGLTexture::UInt8, whitePixel, &transferOptions);
      tex.obj->generateMipMaps();
      tex.isBuffer = false;
      tex.playbackTime = 0.0f;
    }

    // Allocate the keyboard texture.
    {
      Texture& tex = _renderData.textures[kTexture_Keyboard];
      tex.obj = new QOpenGLTexture(QOpenGLTexture::Target2D);
      tex.obj->setSize(256, 3);
      tex.obj->setFormat(QOpenGLTexture::R8_UNorm);
      tex.obj->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
      tex.obj->setWrapMode(QOpenGLTexture::ClampToEdge);
      tex.obj->setAutoMipMapGenerationEnabled(false);
      tex.obj->allocateStorage();

      for (int row = 0; row < 3; row++) {
        for (int key = 0; key < 256; key++) {
          _renderData.keyboardTexData[row][key] = 0;
        }
      }

      tex.obj->setData(QOpenGLTexture::Red, QOpenGLTexture::UInt8, _renderData.keyboardTexData);
      tex.isBuffer = false;
      tex.playbackTime = 0.0f;
    }

    // This maps from a texture ID in the ShaderToyDocument structures to the
    // corresponding index in the RenderData::textures array. We only use this
    // for static assets & not render pass outputs, since they will actually
    // have TWO textures associated with them.
    QHash<int, int> assetIDtoVideoIndex;
    QHash<TextureReference, int> assetIDtoTextureIndex;
    QHash<int, int> assetIDtoRenderpassIndex;

    // Calculate the order to process render passes in. Ignoring any passes
    // which aren't present, this should be: Buf A -> Buf B -> Buf C -> Buf D -> Cube A -> Image
    int renderPassOrder[kMaxRenderpasses];
    int numRenderPasses = 0;
    for (int i = 0; i < _currentDoc->renderpasses.size(); i++) {
      if (_currentDoc->renderpasses[i].type == kRenderPassType_Buffer) {
        renderPassOrder[numRenderPasses++] = i;
        if (numRenderPasses == 4) {
          break;
        }
      }
    }
    renderPassOrder[numRenderPasses] = _currentDoc->findRenderPassByType(kRenderPassType_CubeMap);
    if (renderPassOrder[numRenderPasses] != -1) {
      ++numRenderPasses;
    }
    renderPassOrder[numRenderPasses] = _currentDoc->findRenderPassByType(kRenderPassType_Image);
    if (renderPassOrder[numRenderPasses] != -1) {
      ++numRenderPasses;
    }

    // Set up the render passes, allocating output textures and samplers for them as needed.
    for (int passOrderIdx = 0; passOrderIdx < numRenderPasses; passOrderIdx++) {
      int passIdx = renderPassOrder[passOrderIdx];
      ShaderToyRenderPass& passIn = _currentDoc->renderpasses[passIdx];

      RenderPass& passOut = _renderData.renderpasses[_renderData.numRenderpasses];
      if (passIn.outputs.length() > 0) {
        assetIDtoRenderpassIndex[passIn.outputs[0].id] = _renderData.numRenderpasses;
      }
      _renderData.numRenderpasses++;

      passOut.name = passIn.name;
      if (passIn.type == kRenderPassType_Buffer) {
        passOut.type = PassType::eBuffer;
      }
      else if (passIn.type == kRenderPassType_CubeMap) {
        passOut.type = PassType::eCubemap;
      }
      else if (passIn.type == kRenderPassType_Sound) {
        passOut.type = PassType::eSound;
      }
      else {
        passOut.type = PassType::eImage;
      }

      for (int i = 0; i < 2; i++) {
        Texture& tex = _renderData.textures[_renderData.numTextures];
        createRenderPassTexture(tex);

        passOut.outputs[i] = _renderData.numTextures;

        _renderData.numTextures++;
      }

      glGenSamplers(kMaxInputs, passOut.samplers);
      for (int inputIdx = 0; inputIdx < passIn.inputs.size(); inputIdx++) {
        ShaderToyInput& input = passIn.inputs[inputIdx];

        GLenum minFilter = GL_NEAREST;
        GLenum magFilter = GL_NEAREST;
        if (input.sampler.filter == kSamplerFilterType_Mipmap) {
          minFilter = GL_LINEAR_MIPMAP_LINEAR;
          magFilter = GL_LINEAR;
        }
        else if (input.sampler.filter == kSamplerFilterType_Linear) {
          minFilter = GL_LINEAR;
          magFilter = GL_LINEAR;
        }
        glSamplerParameteri(passOut.samplers[input.channel], GL_TEXTURE_MIN_FILTER, minFilter);
        glSamplerParameteri(passOut.samplers[input.channel], GL_TEXTURE_MAG_FILTER, magFilter);

        GLenum wrap = GL_REPEAT;
        if (input.sampler.wrap == kSamplerWrapType_Clamp) {
          wrap = GL_CLAMP_TO_EDGE;
        }
        glSamplerParameteri(passOut.samplers[input.channel], GL_TEXTURE_WRAP_S, wrap);
        glSamplerParameteri(passOut.samplers[input.channel], GL_TEXTURE_WRAP_T, wrap);
      }

      passOut.sourceCode = passIn.code;
      passOut.sourceFile = passIn.filename;
    }

    // Set up all render pass inputs, loading assets as we encounter them.
    for (int dstPassIndex = 0; dstPassIndex < numRenderPasses; dstPassIndex++) {
      int passIdx = renderPassOrder[dstPassIndex];
      ShaderToyRenderPass& passIn = _currentDoc->renderpasses[passIdx];
      if (passIn.type == kRenderPassType_Common) {
        continue;
      }

      RenderPass& passOut = _renderData.renderpasses[dstPassIndex];

      for (int inputIdx = 0; inputIdx < passIn.inputs.size(); inputIdx++) {
        ShaderToyInput& input = _currentDoc->renderpasses[passIdx].inputs[inputIdx];

        // If this input refers to a renderpass.
        if (input.ctype == kInputType_Buffer) {
          int srcPassIndex = assetIDtoRenderpassIndex[input.id];
          const RenderPass& srcPass = _renderData.renderpasses[srcPassIndex];
          // We want to read from the output which has been rendered to most
          // recently, to ensure we have to most up-to-date input values. If
          // the src pass has already been run in this frame (i.e.
          // `srcPassIndex < dstPassIndex`) then that will be the back buffer.
          // Otherwise it will be the front buffer: in this case, the front
          // buffer will have values calculated at frame N-1 and the back
          // buffer will have values from frame N-2.
          //
          // The minor index we use when looking up an input is the current
          // front buffer index.
          int readBackbuffer = (srcPassIndex < dstPassIndex) ? 1 : 0;
          passOut.inputs[input.channel][0] = srcPass.outputs[readBackbuffer];
          passOut.inputs[input.channel][1] = srcPass.outputs[readBackbuffer ^ 1];
          continue;
        }

        // If this input refers to the keyboard texture...
        if (input.ctype == kInputType_Keyboard) {
          passOut.inputs[input.channel][0] = kTexture_Keyboard;
          passOut.inputs[input.channel][1] = kTexture_Keyboard;
          continue;
        }

        TextureReference tr;
        tr.id = input.id;
        tr.flip = (input.sampler.vflip == "true");
        tr.srgb = (input.sampler.srgb == "true");
        if (input.ctype == kInputType_Video) {
          tr.srgb = false; // We ignore this setting for videos.
        }

        // If we've already loaded the asset...
        if (assetIDtoTextureIndex.contains(tr)) {
          int texIndex = assetIDtoTextureIndex[tr];
          passOut.inputs[input.channel][0] = texIndex;
          passOut.inputs[input.channel][1] = texIndex;
          continue;
        }

        // If we're asking for the flipped version of a video asset which
        // we've already loaded...
        if (input.ctype == kInputType_Video && assetIDtoVideoIndex.contains(tr.id) && tr.flip) {
          int vidIndex = assetIDtoVideoIndex[tr.id];
          Video& vid = _renderData.videos[vidIndex];
          if (vid.flippedTexOutput < 0) {
            vid.flippedTexOutput = allocVideoTexture();
          }
          assetIDtoTextureIndex[tr] = vid.flippedTexOutput;
          passOut.inputs[input.channel][0] = vid.flippedTexOutput;
          passOut.inputs[input.channel][1] = vid.flippedTexOutput;
          continue;
        }

        // If this is a texture we haven't loaded yet...
        if (input.ctype == kInputType_Texture) {
          int texIndex = _renderData.numTextures;
          if (loadImageTexture(input.src, tr.flip, tr.srgb, _renderData.textures[texIndex])) {
            ++_renderData.numTextures;
          }
          else {
            texIndex = kTexture_PlaceholderImage;
          }
          assetIDtoTextureIndex[tr] = texIndex;
          passOut.inputs[input.channel][0] = texIndex;
          passOut.inputs[input.channel][1] = texIndex;
          continue;
        }

        // If this is a cubemap we haven't loaded yet...
        if (input.ctype == kInputType_CubeMap) {
          int texIndex = _renderData.numTextures;
          if (loadCubemapTexture(input.src, tr.flip, tr.srgb, _renderData.textures[texIndex])) {
            ++_renderData.numTextures;
          }
          else {
            texIndex = kTexture_PlaceholderCubemap;
          }
          assetIDtoTextureIndex[tr] = texIndex;
          passOut.inputs[input.channel][0] = texIndex;
          passOut.inputs[input.channel][1] = texIndex;
          continue;
        }

        // If this is a video we haven't loaded yet...
        if (input.ctype == kInputType_Video) {
          int vidIndex = _renderData.numVideos;
          int texIndex = kTexture_PlaceholderImage;
          if (loadVideo(input.src, tr.flip, vidIndex)) {
            ++_renderData.numVideos;
            Video& vid = _renderData.videos[vidIndex];
            assetIDtoVideoIndex[tr.id] = vidIndex;

            if (tr.flip) {
              texIndex = vid.flippedTexOutput;
              assetIDtoTextureIndex[tr] = vid.flippedTexOutput;

              // Add reference for the unflipped texture too, since we have to load that as well.
              tr.flip = false;
              assetIDtoTextureIndex[tr] = vid.texOutput;
            }
            else {
              texIndex = vid.texOutput;
              assetIDtoTextureIndex[tr] = vid.texOutput;
            }
          }
          passOut.inputs[input.channel][0] = texIndex;
          passOut.inputs[input.channel][1] = texIndex;
          continue;
        }

        // If this is a webcam input...
        if (input.ctype == kInputType_Webcam) {
          // ShaderToy seemingly provides unflipped video from the webcam,
          // whereas Qt presents us with flipped video, so for us it's only
          // when flip is *false* that we should flip the video.
          tr.flip = !tr.flip;

          Camera& cam = _renderData.camera;
          int texIndex = kTexture_PlaceholderImage;
          if (!_renderData.hasCamera) {
            cam.obj = new QCamera(this);
            cam.surface = new TextureVideoSurface(cam.obj);
            cam.obj->setViewfinder(cam.surface);
            cam.texOutput = allocVideoTexture();
            _renderData.hasCamera = true;
          }
          if (tr.flip && _renderData.camera.flippedTexOutput < kNumSpecialTextures) {
            cam.flippedTexOutput = allocVideoTexture();
          }

          if (tr.flip) {
            texIndex = cam.flippedTexOutput;
            assetIDtoTextureIndex[tr] = cam.flippedTexOutput;

            // Add reference for the unflipped texture too, since we have to load that as well.
            tr.flip = false;
            assetIDtoTextureIndex[tr] = cam.texOutput;
          }
          else {
            texIndex = cam.texOutput;
            assetIDtoTextureIndex[tr] = cam.texOutput;
          }
          passOut.inputs[input.channel][0] = texIndex;
          passOut.inputs[input.channel][1] = texIndex;
          continue;
        }

        // TODO: other input types.
      }
    }

    // Compile all the shaders & look up the uniform locations.
    for (int i = 0; i < _renderData.numRenderpasses; i++) {
      RenderPass& passOut = _renderData.renderpasses[i];

      QMap<QString, QString> macros;
      macros["SAMPLER_0_TYPE"] =  _renderData.textures[passOut.inputs[0][0]].samplerType(0);
      macros["SAMPLER_1_TYPE"] =  _renderData.textures[passOut.inputs[1][0]].samplerType(1);
      macros["SAMPLER_2_TYPE"] =  _renderData.textures[passOut.inputs[2][0]].samplerType(2);
      macros["SAMPLER_3_TYPE"] =  _renderData.textures[passOut.inputs[3][0]].samplerType(3);
      macros["COMMON_CODE"] = _renderData.commonSourceCode;
      macros["USER_CODE"] = passOut.sourceCode;
      macros["MAIN"] = kMainFunc_Image;

      QString fullSource = preprocessShaderSource(":/glsl/template.frag", macros);

      passOut.program = new QOpenGLShaderProgram(this);
      passOut.program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/fullscreen.vert");
      passOut.program->addShaderFromSourceCode(QOpenGLShader::Fragment, fullSource);
      passOut.program->link();

      passOut.iResolutionLoc        = passOut.program->uniformLocation("iResolution");
      passOut.iTimeLoc              = passOut.program->uniformLocation("iTime");
      passOut.iTimeDeltaLoc         = passOut.program->uniformLocation("iTimeDelta");
      passOut.iFrameLoc             = passOut.program->uniformLocation("iFrame");
      passOut.iMouseLoc             = passOut.program->uniformLocation("iMouse");
      passOut.iChannelTimeLoc       = passOut.program->uniformLocation("iChannelTime");
      passOut.iChannelResolutionLoc = passOut.program->uniformLocation("iChannelResolution");
      passOut.iChannel0Loc          = passOut.program->uniformLocation("iChannel0");
      passOut.iChannel1Loc          = passOut.program->uniformLocation("iChannel1");
      passOut.iChannel2Loc          = passOut.program->uniformLocation("iChannel2");
      passOut.iChannel3Loc          = passOut.program->uniformLocation("iChannel3");
      passOut.iDateLoc              = passOut.program->uniformLocation("iDate");
      passOut.iSampleRateLoc        = passOut.program->uniformLocation("iSampleRate");

      passOut.program->bind();
      passOut.program->setUniformValue(passOut.iChannel0Loc, 0);
      passOut.program->setUniformValue(passOut.iChannel1Loc, 1);
      passOut.program->setUniformValue(passOut.iChannel2Loc, 2);
      passOut.program->setUniformValue(passOut.iChannel3Loc, 3);
      passOut.program->release();
    }

    // Compile the shader for drawing textured quads in the viewport.
    {
      TexturedQuadShader& quadShader = _renderData.texturedQuadShader;
      quadShader.program = new QOpenGLShaderProgram(this);
      quadShader.program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/textured-quad.vert");
      quadShader.program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/textured-quad.frag");
      quadShader.program->link();

      quadShader.iResolutionLoc = quadShader.program->uniformLocation("iResolution");
      quadShader.iShapeLoc      = quadShader.program->uniformLocation("iSize");

      quadShader.program->bind();
      quadShader.program->setUniformValue("iChannel0", 0);
      quadShader.program->release();
    }

    // Display the "image" pass
    _displayPass = _renderData.numRenderpasses - 1;
  }


  void RenderWidget::teardownRenderData()
  {
    // Delete the default vertex array.
    glDeleteVertexArrays(1, &_renderData.defaultVAO);
    _renderData.defaultVAO = 0;

    glDeleteFramebuffers(1, &_renderData.defaultFBO);
    _renderData.defaultFBO = 0;

    glDeleteFramebuffers(1, &_renderData.flipFBO);
    _renderData.flipFBO = 0;

    // Clear out the common source code.
    _renderData.commonSourceCode = QString();
    _renderData.commonSourceFile = QString();

    // Delete all render passes and any associated framebuffers.
    for (int passIdx = 0; passIdx < _renderData.numRenderpasses; passIdx++) {
      RenderPass& pass = _renderData.renderpasses[passIdx];

      delete pass.program;
      pass.program = nullptr;

      glDeleteSamplers(kMaxInputs, pass.samplers);

      for (int i = 0; i < kMaxInputs; i++) {
        pass.inputs[i][0] = 0;
        pass.inputs[i][1] = 0;

        pass.samplers[i] = 0;
      }

      pass.outputs[0] = 0;
      pass.outputs[1] = 0;

      pass.sourceCode = QString();
      pass.sourceFile = QString();

      pass.iResolutionLoc        = -1;
      pass.iTimeLoc              = -1;
      pass.iTimeDeltaLoc         = -1;
      pass.iFrameLoc             = -1;
      pass.iMouseLoc             = -1;
      pass.iChannelTimeLoc       = -1;
      pass.iChannelResolutionLoc = -1;
      pass.iChannel0Loc          = -1;
      pass.iChannel1Loc          = -1;
      pass.iChannel2Loc          = -1;
      pass.iChannel3Loc          = -1;
      pass.iDateLoc              = -1;
      pass.iSampleRateLoc        = -1;
    }
    _renderData.numRenderpasses = 0;

    // Delete the camera
    if (_renderData.camera.obj != nullptr) {
      _renderData.camera.obj->stop();
    }
    delete _renderData.camera.obj;
    // camera.surface is parented to camera.obj, so gets deleted automatically.
    _renderData.camera.obj = nullptr;
    _renderData.camera.surface = nullptr;
    _renderData.camera.texOutput = -1;
    _renderData.camera.flippedTexOutput = -1;
    _renderData.hasCamera = false;

    // Delete all videos.
    for (int vidIdx = 0; vidIdx < _renderData.numVideos; vidIdx++) {
      Video& vid = _renderData.videos[vidIdx];
      if (vid.player) {
        vid.player->stop();
      }
      delete vid.player;
      // vid.surface is parented to vid.player, so gets deleted automatically.
      vid.player = nullptr;
      vid.surface = nullptr;
      vid.texOutput = -1;
      vid.flippedTexOutput = -1;
    }
    _renderData.numVideos = 0;

    // Delete all textures.
    for (int texIdx = 0; texIdx < _renderData.numTextures; texIdx++) {
      Texture& tex = _renderData.textures[texIdx];
      delete tex.obj;
      tex.obj = nullptr;

      tex.isBuffer = false;
      tex.playbackTime = 0.0;
    }
    _renderData.numTextures = 0;

    // Delete utility shaders.
    delete _renderData.texturedQuadShader.program;
    _renderData.texturedQuadShader.program        = nullptr;
    _renderData.texturedQuadShader.iResolutionLoc = -1;
    _renderData.texturedQuadShader.iShapeLoc      = -1;
  }


  void RenderWidget::updateRenderData()
  {
    if (_pendingDoc != _currentDoc) {
      stopPlayback();
      makePendingDocCurrent();
      startPlayback();
    }
    else if (_forceReload) {
      if (_currentDoc != nullptr) {
        stopPlayback();
        teardownRenderData();
        setupRenderData();
        _forceReload = false;
        startPlayback();
      }
    }
    else {
      // Resize all the output textures if the window was resized.
      if (_resized) {
        for (int i = 0; i < _renderData.numTextures; i++) {
          if (_renderData.textures[i].isBuffer) {
            resizeRenderPassTexture(_renderData.textures[i]);
          }
        }
        _resized = false;
        _clearTextures = true;
      }
    }

    if (_currentDoc != nullptr) {
      _renderData.iResolution[0] = float(renderWidth());
      _renderData.iResolution[1] = float(renderHeight());
      _renderData.iResolution[2] = 0.0f;

      _renderData.iTime = static_cast<float>(_playbackTimer.elapsedSecs());
      _renderData.iTimeDelta = _renderData.iTime - _prevTime;

      _renderData.textures[kTexture_Keyboard].obj->setData(QOpenGLTexture::Red,  QOpenGLTexture::UInt8, _renderData.keyboardTexData);

      if (_renderData.iFrame == 0) {
        _clearTextures = true;
      }

      // Upload the current frame for each active video to its corresponding texture.
      bool videoWasFlipped = false;
      for (int i = 0; i < _renderData.numVideos; i++) {
        Video& vid = _renderData.videos[i];
        if (vid.surface == nullptr || !vid.surface->hasCurrentFrame() || vid.texOutput < kNumSpecialTextures) {
          continue;
        }

        float playbackTime = float(vid.player->position()) / 1000.0f;

        QOpenGLTexture* texObj = _renderData.textures[vid.texOutput].obj;
        resizeTextureForVideo(vid.surface, texObj);
        vid.surface->copyToTexture(texObj);
        _renderData.textures[vid.texOutput].playbackTime = playbackTime;

        if (vid.flippedTexOutput >= kNumSpecialTextures) {
          QOpenGLTexture* flippedTexObj = _renderData.textures[vid.flippedTexOutput].obj;
          resizeTextureForVideo(vid.surface, flippedTexObj);
          flipTexture(texObj, flippedTexObj);
          _renderData.textures[vid.flippedTexOutput].playbackTime = playbackTime;
          videoWasFlipped = true;
        }
      }

      // Upload the current camera frame to its corresponding texture.
      if (_renderData.hasCamera) {
        Camera& cam = _renderData.camera;
        if (cam.surface != nullptr && cam.surface->hasCurrentFrame() && cam.texOutput >= kNumSpecialTextures) {
          QOpenGLTexture* texObj = _renderData.textures[cam.texOutput].obj;
          resizeTextureForVideo(cam.surface, texObj);
          cam.surface->copyToTexture(texObj);
          _renderData.textures[cam.texOutput].playbackTime = _renderData.iTime;

          if (cam.flippedTexOutput >= kNumSpecialTextures) {
            QOpenGLTexture* flippedTexObj = _renderData.textures[cam.flippedTexOutput].obj;
            resizeTextureForVideo(cam.surface, flippedTexObj);
            flipTexture(texObj, flippedTexObj);
            _renderData.textures[cam.flippedTexOutput].playbackTime = _renderData.iTime;
            videoWasFlipped = true;
          }
        }
      }

      if (videoWasFlipped) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      }
    }
  }


  void RenderWidget::renderMain()
  {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0);

    glBindVertexArray(_renderData.defaultVAO);

    glBindFramebuffer(GL_FRAMEBUFFER, _renderData.defaultFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glViewport(0, 0, renderWidth(), renderHeight());
    glClearColor(0.0, 1.0, 0.0, 1.0);

    // If we're on frame 0, clear all the output textures to make sure any
    // render pass which reads from them doesn't get garbage values.
    if (_clearTextures) {
      for (int i = 0; i < _renderData.numRenderpasses; i++) {
        for (int j = 0; j < 2; j++) {
          int texIndex = _renderData.renderpasses[i].outputs[j];
          QOpenGLTexture* texObj = _renderData.textures[texIndex].obj;
          glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj->textureId(), 0);
          glClear(GL_COLOR_BUFFER_BIT);
        }
      }
      _clearTextures = false;
    }

    for (int i = 0; i < _renderData.numRenderpasses; i++) {
      const RenderPass& pass = _renderData.renderpasses[i];

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _renderData.textures[pass.outputs[_renderData.backBuffer]].obj->textureId(), 0);
      glClear(GL_COLOR_BUFFER_BIT);

      pass.program->bind();

      pass.program->setUniformValueArray(pass.iResolutionLoc, _renderData.iResolution, 1, 3);
      pass.program->setUniformValue(pass.iTimeLoc, _renderData.iTime);
      pass.program->setUniformValue(pass.iTimeDeltaLoc, _renderData.iTimeDelta);
      pass.program->setUniformValue(pass.iFrameLoc, _renderData.iFrame);
      pass.program->setUniformValueArray(pass.iMouseLoc, _renderData.iMouse, 1, 4);

      glBindSamplers(0, GLuint(kMaxInputs), pass.samplers);

      float iChannelResolution[4][3];
      float iChannelTime[4];
      for (int inputIdx = 0; inputIdx < kMaxInputs; inputIdx++) {
        int texIdx = pass.inputs[inputIdx][_renderData.frontBuffer];
        QOpenGLTexture* tex = _renderData.textures[texIdx].obj;
        tex->bind(inputIdx);

        iChannelResolution[inputIdx][0] = static_cast<float>(tex->width());
        iChannelResolution[inputIdx][1] = static_cast<float>(tex->height());
        iChannelResolution[inputIdx][2] = static_cast<float>(tex->depth());

        iChannelTime[inputIdx] = _renderData.textures[texIdx].playbackTime;
      }
      pass.program->setUniformValueArray(pass.iChannelResolutionLoc, reinterpret_cast<float*>(iChannelResolution), 4, 3);
      pass.program->setUniformValueArray(pass.iChannelTimeLoc, iChannelTime, 4, 1);

      glDrawArrays(GL_TRIANGLES, 0, 3);

      for (int inputIdx = 0; inputIdx < kMaxInputs; inputIdx++) {
        int texIdx = pass.inputs[inputIdx][_renderData.frontBuffer];
        _renderData.textures[texIdx].obj->release();
      }

      pass.program->release();

      // TODO: only generate mipmaps if a downstream pass requires them.
      _renderData.textures[pass.outputs[_renderData.backBuffer]].obj->generateMipMaps();
    }

    glBindSamplers(0, GLuint(kMaxInputs), nullptr);

    int displayTexIdx = _renderData.renderpasses[_displayPass].outputs[_renderData.backBuffer];
    QOpenGLTexture* texObj = _renderData.textures[displayTexIdx].obj;

    int srcW = texObj->width();
    int srcH = texObj->height();
    int dstX, dstY, dstW, dstH;
    displayRect(dstX, dstY, dstW, dstH);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _renderData.defaultFBO);
    if (_displayPass != _renderData.numRenderpasses - 1) {
      glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj->textureId(), 0);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, srcW, srcH, dstX, dstY, dstX + dstW, dstY + dstH, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindVertexArray(0);

    _renderData.frontBuffer ^= 1;
    _renderData.backBuffer ^= 1;
  }


  void RenderWidget::renderIntermediates()
  {
    if (!_showOutputs && !_showInputs) {
      return;
    }

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindVertexArray(_renderData.defaultVAO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());

    glViewport(0, 0, width(), height());

    // Calculate the dimensions for each intermediate, preserving their aspect ratio.
    int rw = renderWidth();
    int rh = renderHeight();
    int w = width() / 4;
    int h = rh * w / rw;
    int level = 0;

    if (_showOutputs) {
      int x = width() - w;
      int y = height() - h;
      for (int i = 0; i < kMaxRenderpasses; i++) {
        const RenderPass& pass = _renderData.renderpasses[i];
        if (pass.type == PassType::eBuffer || pass.type == PassType::eImage) {
          QOpenGLTexture* texObj = _renderData.textures[pass.outputs[_renderData.frontBuffer]].obj;
          glBindFramebuffer(GL_READ_FRAMEBUFFER, _renderData.defaultFBO);
          glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj->textureId(), level);
          glBlitFramebuffer(0, 0, rw, rh, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
          y -= h;
        }
      }
    }
    if (_showInputs) {
      int x = 0;
      int y = height() - h;
      const RenderPass& pass = _renderData.renderpasses[_displayPass];
      for (int i = 0; i < kMaxInputs; i++) {
        int texIndex = pass.inputs[i][_renderData.frontBuffer];
        QOpenGLTexture* texObj = _renderData.textures[texIndex].obj;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _renderData.defaultFBO);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj->textureId(), level);
        glBlitFramebuffer(0, 0, rw, rh, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        y -= h;
      }
    }

    glBindVertexArray(0);
  }


  void RenderWidget::renderEmpty()
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }


  void RenderWidget::renderHUD(QPainter& painter)
  {
    if (!_showHUD) {
      return;
    }

    painter.setFont(_hudFont);
    painter.setPen(_hudPen);

    int x = 10;
    int y = _lineHeight + 8;
    if (_hudFlags & kHUD_FrameNum) {
      painter.drawText(x, y, QString("Frame #%1").arg(_renderData.iFrame));
      y += _lineHeight;
    }
    if (_hudFlags & kHUD_Time) {
      painter.drawText(x, y, QString("Time %1").arg(_renderData.iTime, 0, 'f', 2));
      y += _lineHeight;
    }
    if (_hudFlags & kHUD_MillisPerFrame) {
      painter.drawText(x, y, QString("%1 ms/frame").arg(_fpsCounter.msPerFrame(), 0, 'f', 2));
      y += _lineHeight;
    }
    if (_hudFlags & kHUD_FramesPerSec) {
      painter.drawText(x, y, QString("%1 FPS").arg(_fpsCounter.framesPerSec(), 0, 'f', 2));
      y += _lineHeight;
    }
    if (_hudFlags & kHUD_MousePos) {
      painter.drawText(x, y, QString("Mouse Pos %1,%2").arg(_renderData.iMouse[0], 0, 'f', 2).arg(_renderData.iMouse[1], 0, 'f', 2));
      y += _lineHeight;
    }
    if (_hudFlags & kHUD_MouseDownPos) {
      painter.drawText(x, y, QString("Mouse Down %1,%2").arg(_renderData.iMouse[2], 0, 'f', 2).arg(_renderData.iMouse[3], 0, 'f', 2));
      y += _lineHeight;
    }
  }


  void RenderWidget::createRenderPassTexture(Texture& tex)
  {
    qDebug("Creating render pass texture with resolution %dx%d", renderWidth(), renderHeight());

    tex.obj = new QOpenGLTexture(QOpenGLTexture::Target2D);
    tex.obj->setSize(renderWidth(), renderHeight());
    tex.obj->setFormat(QOpenGLTexture::RGBA32F);
    tex.obj->setAutoMipMapGenerationEnabled(false);
    tex.obj->setMagnificationFilter(QOpenGLTexture::Nearest);
    tex.obj->setMinificationFilter(QOpenGLTexture::Linear);
    tex.obj->setWrapMode(QOpenGLTexture::ClampToEdge);
    tex.obj->setMaximumLevelOfDetail(0);
    tex.obj->allocateStorage();

    tex.isBuffer = true;
    tex.playbackTime = 0.0;
  }


  void RenderWidget::resizeRenderPassTexture(Texture& tex)
  {
    int newW = renderWidth();
    int newH = renderHeight();
    if (tex.obj->width() == newW && tex.obj->height() == newH) {
      // Texture is already the correct size.
      return;
    }

    qDebug("Resizing texture from %dx%d to %dx%d", tex.obj->width(), tex.obj->height(), newW, newH);

    delete tex.obj;
    createRenderPassTexture(tex);
  }


  bool RenderWidget::loadImageTexture(const QString& filename, bool flip, bool srgb, Texture& tex)
  {
    QString adjustedFilename = filename;
    if (_cache != nullptr && _cache->isCached(filename)) {
      adjustedFilename = _cache->pathForCachedFile(filename);
    }

    QImage img(adjustedFilename);
    if (img.isNull()) {
      qDebug("failed to load texture %s", qPrintable(filename));
      return false;
    }
    img = img.convertToFormat(QImage::Format_RGBA8888);
    if (flip) {
      img = img.mirrored();
    }

    QOpenGLTexture::TextureFormat targetFormat = srgb ? QOpenGLTexture::SRGB8_Alpha8 : QOpenGLTexture::RGBA8_UNorm;
    QOpenGLTexture::PixelFormat sourceFormat = QOpenGLTexture::RGBA;
    QOpenGLTexture::PixelType sourceType = QOpenGLTexture::UInt8;

    tex.obj = new QOpenGLTexture(QOpenGLTexture::Target2D);
    tex.obj->setSize(img.width(), img.height());
    tex.obj->setFormat(targetFormat);
    tex.obj->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    tex.obj->setWrapMode(QOpenGLTexture::ClampToEdge);
    tex.obj->allocateStorage();
    for (int i = 0; i < 6; i++) {
      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(4);

      tex.obj->setData(sourceFormat, sourceType, img.constBits(), &transferOptions);
    }
    tex.obj->generateMipMaps();

    tex.isBuffer = false;
    tex.playbackTime = 0.0f;

    return true;
  }


  bool RenderWidget::loadCubemapTexture(const QString& filename, bool flip, bool srgb, Texture& tex)
  {
    QFileInfo fileInfo(filename);
    QString path = fileInfo.path();
    QString basename = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();

    QString facePaths[6];
    facePaths[0] = QString("%1/%2.%3"  ).arg(path).arg(basename).arg(suffix);
    facePaths[1] = QString("%1/%2_1.%3").arg(path).arg(basename).arg(suffix);
    facePaths[2] = QString("%1/%2_2.%3").arg(path).arg(basename).arg(suffix);
    facePaths[3] = QString("%1/%2_3.%3").arg(path).arg(basename).arg(suffix);
    facePaths[4] = QString("%1/%2_4.%3").arg(path).arg(basename).arg(suffix);
    facePaths[5] = QString("%1/%2_5.%3").arg(path).arg(basename).arg(suffix);

    for (int i = 0; i < 6; i++) {
      if (_cache != nullptr && _cache->isCached(filename)) {
        facePaths[i] = _cache->pathForCachedFile(facePaths[i]);
      }
    }

    QImage faces[6];
    bool allFacesLoaded = true;
    for (int i = 0; i < 6; i++) {
      faces[i] = QImage(facePaths[i]);
      if (faces[i].isNull()) {
        qDebug("cubemap %s is missing face %d", qPrintable(facePaths[i]), i);
        allFacesLoaded = false;
        break;
      }
    }
    if (!allFacesLoaded) {
      qDebug("failed to load cubemap %s", qPrintable(filename));
      return false;
    }

    QOpenGLTexture::CubeMapFace cubeMapFaces[6];
    cubeMapFaces[0] = QOpenGLTexture::CubeMapPositiveX;
    cubeMapFaces[1] = QOpenGLTexture::CubeMapNegativeX;
    cubeMapFaces[2] = QOpenGLTexture::CubeMapPositiveY;
    cubeMapFaces[3] = QOpenGLTexture::CubeMapNegativeY;
    cubeMapFaces[4] = QOpenGLTexture::CubeMapPositiveZ;
    cubeMapFaces[5] = QOpenGLTexture::CubeMapNegativeZ;

    for (int i = 0; i < 6; i++) {
      faces[i] = faces[i].convertToFormat(QImage::Format_RGBA8888);
      if (flip) {
        faces[i] = faces[i].mirrored();
      }
    }

    QOpenGLTexture::TextureFormat targetFormat = srgb ? QOpenGLTexture::SRGB8_Alpha8 : QOpenGLTexture::RGBA8_UNorm;
    QOpenGLTexture::PixelFormat sourceFormat = QOpenGLTexture::RGBA;
    QOpenGLTexture::PixelType sourceType = QOpenGLTexture::UInt8;

    tex.obj = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
    tex.obj->setSize(faces[0].width(), faces[0].height());
    tex.obj->setFormat(targetFormat);
    tex.obj->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
    tex.obj->setWrapMode(QOpenGLTexture::ClampToEdge);
    tex.obj->allocateStorage();
    for (int i = 0; i < 6; i++) {
      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(4);

      tex.obj->setData(0, 0, 1, cubeMapFaces[i], sourceFormat, sourceType, faces[i].constBits(), &transferOptions);
    }
    tex.obj->generateMipMaps();

    tex.isBuffer = false;
    tex.playbackTime = 0.0f;

    return true;
  }


  bool RenderWidget::loadVideo(const QString& filename, bool flip, int vidIndex)
  {
    Video& vid = _renderData.videos[vidIndex];

    QString adjustedFilename = filename;
    if (_cache != nullptr && _cache->isCached(filename)) {
      adjustedFilename = _cache->pathForCachedFile(adjustedFilename);
    }

    vid.player = new QMediaPlayer(this);
    vid.surface = new TextureVideoSurface(vid.player);

    vid.player->setVideoOutput(vid.surface);
    vid.player->setMuted(true);

    QMediaPlaylist* playlist = new QMediaPlaylist(vid.player);
    playlist->addMedia(QUrl(adjustedFilename));
    playlist->setPlaybackMode(QMediaPlaylist::Loop);
    vid.player->setPlaylist(playlist);

    connect(vid.player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [this, vidIndex](QMediaPlayer::Error err){ this->videoError(err, vidIndex); });

    // Neither of these textures have any storage allocated yet because we
    // don't know what size the frames will be until we start playing the
    // video.
    vid.texOutput = allocVideoTexture();
    vid.flippedTexOutput = flip ? allocVideoTexture() : -1;
    return true;
  }


  int RenderWidget::allocVideoTexture()
  {
    int texIndex = _renderData.numTextures++;

    Texture& tex = _renderData.textures[texIndex];

    tex.obj = new QOpenGLTexture(QOpenGLTexture::Target2D);
    tex.obj->setFormat(QOpenGLTexture::RGBA8_UNorm);

    tex.isBuffer = false;
    tex.playbackTime = 0.0f;

    return texIndex;
  }


  void RenderWidget::resizeTextureForVideo(TextureVideoSurface* surface, QOpenGLTexture* texObj)
  {
    if (surface->frameWidth() != texObj->width() || surface->frameHeight() != texObj->height()) {
      texObj->destroy();
      texObj->setSize(surface->frameWidth(), surface->frameHeight());
      texObj->setFormat(QOpenGLTexture::RGBA8_UNorm);
      texObj->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Linear);
      texObj->setWrapMode(QOpenGLTexture::ClampToEdge);
      texObj->setAutoMipMapGenerationEnabled(true);
      texObj->setSwizzleMask(QOpenGLTexture::BlueValue,
                             QOpenGLTexture::GreenValue,
                             QOpenGLTexture::RedValue,
                             QOpenGLTexture::AlphaValue);
      texObj->allocateStorage();
    }
  }


  void RenderWidget::flipTexture(QOpenGLTexture* texObj, QOpenGLTexture* flippedTexObj)
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _renderData.flipFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj->textureId(), 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _renderData.defaultFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, flippedTexObj->textureId(), 0);

    int srcW = texObj->width();
    int srcH = texObj->height();
    int dstW = texObj->width();
    int dstH = texObj->height();
    glBlitFramebuffer(0, 0,    srcW, srcH,
                      0, dstH, dstW, 0,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }


  void RenderWidget::updateShaderMousePos(QPoint mousePosWithFlippedY, bool setDownPos)
  {
    int dstX, dstY, dstW, dstH;
    displayRect(dstX, dstY, dstW, dstH);

    int srcW = renderWidth();
    int srcH = renderHeight();

    _renderData.iMouse[0] = float(mousePosWithFlippedY.x() - dstX) / float(dstW) * float(srcW);
    _renderData.iMouse[1] = float(mousePosWithFlippedY.y() - dstY) / float(dstH) * float(srcH);

    if (setDownPos) {
      _renderData.iMouse[2] = _renderData.iMouse[0];
      _renderData.iMouse[3] = _renderData.iMouse[1];
    }
  }


  //
  // RenderWidget private slots
  //

  void RenderWidget::fileChanged(const QString& path)
  {
    qDebug("%s changed, reloading shader", qPrintable(path));
    reloadCurrentShaderToyDocument();
  }


  void RenderWidget::videoError(QMediaPlayer::Error err, int vidIndex)
  {
    Video& vid = _renderData.videos[vidIndex];
    qDebug("video %d error %d: %s", vidIndex, int(err), qPrintable(vid.player->errorString()));
  }

} // namespace vh
