/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Use, modification and distribution is allowed without limitation,
** warranty, liability or support of any kind.
**
****************************************************************************/

#include "qtwin.h"
#include <QLibrary>
#include <QApplication>
#include <QWidget>
#include <QList>
#include <QPointer>
#include <QHash>
#include <QDebug>
#ifdef Q_OS_WIN

#include <qt_windows.h>

// Blur behind data structures
#define DWM_BB_ENABLE                 0x00000001  // fEnable has been specified
#define DWM_BB_BLURREGION             0x00000002  // hRgnBlur has been specified
#define DWM_BB_TRANSITIONONMAXIMIZED  0x00000004  // fTransitionOnMaximized has been specified
#define WM_DWMCOMPOSITIONCHANGED        0x031E    // Composition changed window message

typedef struct _DWM_BLURBEHIND {
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND, *PDWM_BLURBEHIND;

typedef struct _MARGINS {
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
} MARGINS, *PMARGINS;

typedef struct _WIDGETS_INFO {
    bool frameIntoClientAreaExtended;
    MARGINS Window_Margins;
    bool widgetBulerBehindEnabled;
} WIDGETS_INFO, *PWIDGETS_INFO;

static  QHash<QWidget*, WIDGETS_INFO> widgetsInfo = QHash<QWidget*, WIDGETS_INFO>();

typedef HRESULT(WINAPI* PtrDwmIsCompositionEnabled)(BOOL* pfEnabled);
typedef HRESULT(WINAPI* PtrDwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset);
typedef HRESULT(WINAPI* PtrDwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind);
typedef HRESULT(WINAPI* PtrDwmGetColorizationColor)(DWORD* pcrColorization, BOOL* pfOpaqueBlend);

static PtrDwmIsCompositionEnabled pDwmIsCompositionEnabled = 0;
static PtrDwmEnableBlurBehindWindow pDwmEnableBlurBehindWindow = 0;
static PtrDwmExtendFrameIntoClientArea pDwmExtendFrameIntoClientArea = 0;
static PtrDwmGetColorizationColor pDwmGetColorizationColor = 0;


/*
 * Internal helper class that notifies windows if the
 * DWM compositing state changes and updates the widget
 * flags correspondingly.
 */
class WindowNotifier : public QWidget
{
public:
    WindowNotifier() {
        winId();
    }
    void addWidget(QWidget* widget) {
        widgets.append(widget);
    }
    void removeWidget(QWidget* widget) {
        widgets.removeAll(widget);
    }
    bool winEvent(MSG* message, long* result);
    //void applyToAll(bool blur);

private:
    QWidgetList widgets;
};

static bool resolveLibs()
{
    if (!pDwmIsCompositionEnabled) {
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        pDwmIsCompositionEnabled = (PtrDwmIsCompositionEnabled)dwmLib.resolve("DwmIsCompositionEnabled");
        pDwmExtendFrameIntoClientArea = (PtrDwmExtendFrameIntoClientArea)dwmLib.resolve("DwmExtendFrameIntoClientArea");
        pDwmEnableBlurBehindWindow = (PtrDwmEnableBlurBehindWindow)dwmLib.resolve("DwmEnableBlurBehindWindow");
        pDwmGetColorizationColor = (PtrDwmGetColorizationColor)dwmLib.resolve("DwmGetColorizationColor");
    }
    return pDwmIsCompositionEnabled != 0;
}

#endif

/*!
  * Chekcs and returns true if Windows DWM composition
  * is currently enabled on the system.
  *
  * To get live notification on the availability of
  * this feature, you will currently have to
  * reimplement winEvent() on your widget and listen
  * for the WM_DWMCOMPOSITIONCHANGED event to occur.
  *
  */
bool QtWin::isCompositionEnabled()
{
#ifdef Q_OS_WIN
    if (resolveLibs()) {
        HRESULT hr = S_OK;
        BOOL isEnabled = false;
        hr = pDwmIsCompositionEnabled(&isEnabled);
        if (SUCCEEDED(hr)) {
            return isEnabled;
        }
    }
#endif
    return false;
}

/*!
  * Enables Blur behind on a Widget.
  *
  * \a enable tells if the blur should be enabled or not
  */
bool QtWin::enableBlurBehindWindow(QWidget* widget, bool enable)
{
    Q_ASSERT(widget);
    bool result = false;
#ifdef Q_OS_WIN
    if (resolveLibs()) {
        DWM_BLURBEHIND bb = {0};
        HRESULT hr = S_OK;
        bb.fEnable = enable;
        bb.dwFlags = DWM_BB_ENABLE;
        bb.hRgnBlur = NULL;
        widget->setAttribute(Qt::WA_TranslucentBackground, enable);
        widget->setAttribute(Qt::WA_NoSystemBackground, enable);
        hr = pDwmEnableBlurBehindWindow(widget->winId(), &bb);
        if (SUCCEEDED(hr)) {
            result = true;
            windowNotifier()->addWidget(widget);
            WIDGETS_INFO widgetInfo = widgetsInfo.value(widget);
            widgetInfo.widgetBulerBehindEnabled = enable;
            widgetsInfo.insert(widget, widgetInfo);
        }
    }
#endif
    return result;
}

/*!
  * ExtendFrameIntoClientArea.
  *
  * This controls the rendering of the frame inside the window.
  * Note that passing margins of -1 (the default value) will completely
  * remove the frame from the window.
  *
  * \note you should not call enableBlurBehindWindow before calling
  *       this functions
  *
  * \a enable tells if the blur should be enabled or not
  */
bool QtWin::extendFrameIntoClientArea(QWidget* widget, int left, int top, int right, int bottom)
{
    //return false;

    Q_ASSERT(widget);
    Q_UNUSED(left);
    Q_UNUSED(top);
    Q_UNUSED(right);
    Q_UNUSED(bottom);

    bool result = false;
#ifdef Q_OS_WIN
    if (resolveLibs()) {
        //QLibrary dwmLib(QString::fromAscii("dwmapi"));
        HRESULT hr = S_OK;
        MARGINS m = {left, top, right, bottom};
        hr = pDwmExtendFrameIntoClientArea(widget->winId(), &m);
        if (SUCCEEDED(hr)) {
            result = true;
            windowNotifier()->addWidget(widget);
            WIDGETS_INFO widgetInfo = widgetsInfo.value(widget);
            widgetInfo.Window_Margins = m;
            widgetInfo.widgetBulerBehindEnabled = true;
            widgetInfo.frameIntoClientAreaExtended = result;
            widgetsInfo.insert(widget, widgetInfo);
        }
        widget->setAttribute(Qt::WA_TranslucentBackground, result);
    }
#endif
    return result;
}

bool QtWin::easyBlurUnBlur(QWidget* widget, bool enable)
{
    Q_ASSERT(widget);
    bool result = false;
    if (enable) {
#ifndef Q_OS_WIN //X11
        widget->setAttribute(Qt::WA_TranslucentBackground);
        widget->setAttribute(Qt::WA_NoSystemBackground, false);
        QPalette pal = widget->palette();
        QColor bg = pal.window().color();
        bg.setAlpha(235);
        pal.setColor(QPalette::Window, bg);
        widget->setPalette(pal);
        widget->ensurePolished(); // workaround Oxygen filling the background
        widget->setAttribute(Qt::WA_StyledBackground, false);
        result = true;
#else
        if (QtWin::isCompositionEnabled()) {
            result = QtWin::extendFrameIntoClientArea(widget);
            widget->setContentsMargins(0, 0, 0, 0);
            if (!widget->inherits("QMainWindow")) {
                widget->setStyleSheet(widget->styleSheet() + " QGroupBox {border: 1px solid lightgray; border-radius: 5px; margin-top: 7px; margin-bottom: 7px; padding: 0px;}QGroupBox::title {top: -7 ex;left: 10px; subcontrol-origin: border;}");
            }
        }
#endif
    }
    else {
#ifndef Q_OS_WIN //X11
        widget->setAttribute(Qt::WA_TranslucentBackground, false);
        QPalette pal = widget->palette();
        QColor bg = pal.window().color();
        bg.setAlpha(255);
        pal.setColor(QPalette::Window, bg);
        widget->setPalette(pal);
        widget->ensurePolished(); // workaround Oxygen filling the background
        widget->setAttribute(Qt::WA_StyledBackground, true);
        result = true;
#else
        if (QtWin::isCompositionEnabled()) {
            widget->setAttribute(Qt::WA_NoSystemBackground, false);
            result = QtWin::enableBlurBehindWindow(widget, false);
        }
#endif
    }
    if (result) {
        widget->update();
    }
    return result;
}

/*!
  * Returns the current colorizationColor for the window.
  *
  * \a enable tells if the blur should be enabled or not
  */
QColor QtWin::colorizatinColor()
{
    QColor resultColor = QApplication::palette().window().color();

#ifdef Q_OS_WIN
    if (resolveLibs()) {
        DWORD color = 0;
        BOOL opaque = FALSE;
        QLibrary dwmLib(QString::fromAscii("dwmapi"));
        HRESULT hr = S_OK;
        hr = pDwmGetColorizationColor(&color, &opaque);
        if (SUCCEEDED(hr)) {
            resultColor = QColor(color);
        }
    }
#endif
    return resultColor;
}


#ifdef Q_OS_WIN
//void QtWin::blurAll()
//{
//  windowNotifier()->applyToAll(true);
//}

//void QtWin::unBlurAll()
//{
//  windowNotifier()->applyToAll(false);
//}

WindowNotifier* QtWin::windowNotifier()
{
    static WindowNotifier* windowNotifierInstance = 0;
    if (!windowNotifierInstance) {
        windowNotifierInstance = new WindowNotifier;
    }
    return windowNotifierInstance;
}


/* Notify all enabled windows that the DWM state changed */
bool WindowNotifier::winEvent(MSG* message, long* result)
{
    if (message && message->message == WM_DWMCOMPOSITIONCHANGED) {
        bool compositionEnabled = QtWin::isCompositionEnabled();
        qDebug() << Q_FUNC_INFO << "compositionEnabled" << compositionEnabled;
        foreach (QWidget* widget, widgets) {
            if (widget) {
                widget->setAttribute(Qt::WA_NoSystemBackground, compositionEnabled);
                WIDGETS_INFO widgetInfo = widgetsInfo.value(widget);
                //QtWin::enableBlurBehindWindow(widget, widgetInfo.widgetBulerBehindEnabled);
                if (compositionEnabled && widgetInfo.widgetBulerBehindEnabled) {
                    //tricky fix for a bug when enabling composition!
                    QtWin::enableBlurBehindWindow(widget, false);
                    QtWin::enableBlurBehindWindow(widget, true);
                    //QtWin::extendFrameIntoClientArea(widget);
                }
                widget->update();
//              WIDGETS_INFO widgetInfo = widgetsInfo.value(widget);
//              QtWin::enableBlurBehindWindow(widget, widgetInfo.widgetBulerBehindEnabled);
//              if (widgetInfo.frameIntoClientAreaExtended)
//              {
//                  MARGINS m = widgetInfo.Window_Margins;
//                  QtWin::extendFrameIntoClientArea(widget, m.cxLeftWidth, m.cyTopHeight, m.cxRightWidth, m.cyBottomHeight);
//              }
            }
        }
    }
    return QWidget::winEvent(message, result);
}

//void WindowNotifier::applyToAll(bool blur)
//{
//  foreach(QWidget * widget, widgets)
//  {
//      if (widget)
//      {
//          //qDebug() << "ENDDD";
//          //qDebug() << "blur="<<blur<<widget->objectName();
//          //qDebug() << "AFTER";
//          widget->setAttribute(Qt::WA_NoSystemBackground, blur);
//          QtWin::enableBlurBehindWindow(widget, blur);
//          widget->update();
//      }
//  }
//}
#endif
