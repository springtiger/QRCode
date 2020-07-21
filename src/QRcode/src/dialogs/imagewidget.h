/***************************************************************************
**
**  Copyright (C) 2020 KuCAD <www.kucad.ltd>
**  Contact: springtiger@aliyun.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see http://www.gnu.org/licenses/.
**
****************************************************************************/

#include <QMenu>
#include <QPainter>
#include <QWidget>

#ifndef __IMAGEWIDGET_H__
#define __IMAGEWIDGET_H__

// Common function
QPoint getCursorPosInImage(const QImage& originalImage, const QImage& zoomedImage, const QPoint& imageLeftTopPos, const QPoint& cursorPos);

class ImageMarquees : public QWidget {
    Q_OBJECT
public:
    ImageMarquees(QWidget* parent = nullptr, int marqueesEdgeWidth = 5);
    ~ImageMarquees();
    void setImage(QImage* inputImg, QImage* paintImg, QRect* paintImageRect, const QString &imgPath);
    void setMarqueesEdgeWidth(int width);

protected:
signals:
    void sendExitSignal();

public slots:

private slots:
    void recvParentWidgetSizeChangeSignal();
    void exit();
    void reset();
    void cropPaintImage();
    void cropOriginalImage();

private:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void wheelEvent(QWheelEvent* event);
    bool eventFilter(QObject* watched, QEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void saveImage(const QImage* img, const QRect& rect);
    QRect getCropRectInImage(const QRect& paintImageRect, const QRect& rect);
    void calcMarqueesEdgeRect();
    int getSubRectInCropRect(QPoint cursorPos);
    void cropRectChangeEvent(int SR_LOCATION, const QPoint& cursorPos);
    bool keyEscapePressEvent();
    void showErrorMsgBox(const char *errMsg);

    QMenu* mMenu = nullptr;
    QAction* mActionReset = nullptr;
    QAction* mActionSavePaintImage = nullptr;
    QAction* mActionSaveOriginalImage = nullptr;
    QAction* mActionExit = nullptr;
    // CropRect
    enum { CR_NULL = -1, CR_CENTER, CR_TOPLEFT, CR_TOPRIGHT, CR_BOTTOMRIGHT, CR_BOTTOMLEFT, CR_TOP, CR_RIGHT, CR_BOTTOM, CR_LEFT, CR_ENTIRETY };
    // Selected range in widget
    QRect cropRect[10];
    QRect prevCropRect;
    // Selected range in image
    QRect cropRectInImage;
    QRect* paintImageRect = nullptr;
    QPoint mouseLeftClickedPos = QPoint(0, 0);
    int mouseStatus;
    QImage* inputImg = nullptr;
    QImage* paintImg = nullptr;
    bool isLoadImage = false;
    bool isCropRectStable = false;
    bool isCropRectExisted = false;
    int cursorPosInCropRect = CR_NULL;
    int marqueesEdgeWidth = 5;
	QString m_imgPath;

    static const char* ERR_MSG_NULL_IMAGE;
    static const char* ERR_MSG_INVALID_FILE_PATH;
};


class ImageWidget : public QWidget {
    Q_OBJECT
public:
    // Static empty images are used to free memory
    static const QImage NULL_QIMAGE;
    // Static variable image size correlation
    static const QPoint NULL_POINT;
    static const QSize NULL_SIZE;
    static const QRect NULL_RECT;
    // Image limiting mode
    enum RestrictMode { RM_INNER, RM_OUTTER };

    explicit ImageWidget(QWidget* parent = nullptr);
    ~ImageWidget();
    // External unified rendering setImage() interface
    bool setImage(const QImage& img, bool isDeepCopy = false);
    bool setImage(const QString& filePath);
    bool setImage(const std::string& filePath);

    void setEnableOnlyShowImage(bool flag = false);

    // Sending the click position coordinate signal is closed by default and needs to be turned on before use
    ImageWidget* setEnableSendLeftClickedPosInWidget(bool flag = false);
    ImageWidget* setEnableSendLeftClickedPosInImage(bool flag = false);
    QPoint getDrawImageTopLeftPos() const;

signals:
    void sendParentWidgetSizeChangedSignal();
    void sendLeftClickedPosInWidgetSignal(int x, int y);
    void sendLeftClickedPosInImageSignal(int x, int y);

public slots:
    void clear();
    ImageWidget* setEnableDrag(bool flag = true);
    ImageWidget* setEnableZoom(bool flag = true);
    ImageWidget* setEnableAutoFit(bool flag = true);
    ImageWidget* setMaxZoomScale(double scale);
    ImageWidget* setMinZoomScale(double scale);
    ImageWidget* setMaxZoomedImageSize(int width, int height);
    ImageWidget* setMinZoomedImageSize(int width, int height);
    ImageWidget* setPaintAreaOffset(int offset);
    ImageWidget* setPaintImageRestrictMode(RestrictMode rm);

private slots:
    void resetImageWidget();
    void save();
    void enterCropImageMode();
    void exitCropImageMode();
    // R1
    void updateImageWidget();

protected:
private:
    // Magnification
    double MAX_ZOOM_SCALE = 20.0;
    double MIN_ZOOM_SCALE = 0.04;
    // Maximum image size
    QSize MAX_ZOOMED_IMG_SIZE = QSize(100000, 10000);
    // Minimum image size
    QSize MIN_ZOOMED_IMG_SIZE = QSize(10, 10);
    // Paint area offset
    int PAINT_AREA_OFFEST = 0;
    // original image
    QImage inputImg;
	QString m_imgPath;

    // Paint image
    QImage paintImg;
    QSize lastPaintImgSize = NULL_SIZE;

    // ImageWidge Paint Area
    QRect imageWidgetPaintRect;
    // Paint area of zoomed image
    QRect paintImageRect;

    double zoomScale = 1.0;

    QPoint mouseLeftKeyPressDownPos = NULL_POINT;
    QPoint paintImageLastTopLeft = NULL_POINT;

    // status flags
    bool isCropImageMode = false;
    bool isImagePosChanged = false;
    bool isImageDragging = false;
    bool isZoomedParametersChanged = false;

    bool enableOnlyShowImage = false;
    bool enableDragImage = true;
    bool enableZoomImage = true;
    bool enableAutoFitWidget = true;
    bool enableLoadImageWithDefaultConfig = false;
    bool enableSendLeftClickedPosInWidget = false;
    bool enableSendLeftClickedPosInImage = false;
    RestrictMode restrictMode = RM_INNER;

    int mouseStatus = Qt::NoButton;
    // 1
    QMenu* mMenu = nullptr;
    QAction* mActionResetParameters = nullptr;
    QAction* mActionSave = nullptr;
    QAction* mActionCrop = nullptr;
    // 2
    QMenu* mMenuAdditionalFunction = nullptr;
    QAction* mActionEnableDrag = nullptr;
    QAction* mActionEnableZoom = nullptr;
    QAction* mActionImageAutoFitWidget = nullptr;

    void updateZoomedImage();
    void imageZoomOut();
    void imageZoomIn();
    void initializeContextmenu();
    void sendLeftClickedSignals(QMouseEvent* e);
    void setDefaultParameters();
    QPoint getImageTopLeftPosWhenShowInCenter(const QImage& img, const QWidget* iw);
    // R1
    void initShowImage();
    bool loadImageFromPath(const QString& filePath);
    void setImageAttributeWithAutoFitFlag(bool enableAutoFit);
    void fixPaintImageTopLeft();
    void fixPaintImageTopLeftInOutterMode(const QRect& imageWidgetPaintRect, QRect& paintImageRect);
    void fixPaintImageTopLefInInnerMode(const QRect& imageWidgetPaintRect, QRect& paintImageRect);

    void wheelEvent(QWheelEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);
    void contextMenuEvent(QContextMenuEvent* e);
    void resizeEvent(QResizeEvent* e);
};

#endif // __IMAGEWIDGET_H__
