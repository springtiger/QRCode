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


#include "ImageWidget.h"
#include "moc/moc_ImageWidget.cpp"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QSettings>
#include "nativeapp.h"
#include <math.h>
#include <string>

// static const var
const char* ImageMarquees::ERR_MSG_NULL_IMAGE = "No Image Selected!";
const char* ImageMarquees::ERR_MSG_INVALID_FILE_PATH = "Invalid File Path!";

const QImage ImageWidget::NULL_QIMAGE = QImage();
const QPoint ImageWidget::NULL_POINT = QPoint(0, 0);
const QSize ImageWidget::NULL_SIZE = QSize(0, 0);
const QRect ImageWidget::NULL_RECT = QRect(0, 0, 0, 0);

QString getSaveImageFile(const QString &defaultName, QWidget * parent) {
	QSettings settings;
	QString defaultDir = QDir(settings.value("WorkingDir").toString()).absoluteFilePath(defaultName);
	QStringList ableExtent = { "png", "jpg", "bmp","tiff" };
	QStringList filters = { ImageWidget::tr("Images(*.png *.bmp *.jpg *.tiff )")
		,ImageWidget::tr("png files (*.png)")
		,ImageWidget::tr("jpg files (*.jpg)")
		,ImageWidget::tr("bmp files (*.bmp)")
		,ImageWidget::tr("tiff files (*.tiff)") };
	QString filename = app().getSaveFileName(parent, ImageWidget::tr("Save File"), defaultDir, filters.join(";;"), ableExtent);

	return filename;
}

ImageMarquees::ImageMarquees(QWidget* parent, int marqueesEdgeWidtht)
    : QWidget(parent)
{
    mouseStatus = Qt::NoButton;
    this->marqueesEdgeWidth = marqueesEdgeWidtht;

    // Initialize context menu
    mMenu = new QMenu(this);
    mActionReset = mMenu->addAction(tr("Reset")); // Reset
    mActionSavePaintImage = mMenu->addAction(tr("Save as (zoom image)")); // Save as (From the zoomed image)
    mActionSaveOriginalImage = mMenu->addAction(tr("Save as (actual image)")); // Save as (From the original image)
    mActionExit = mMenu->addAction(tr("Exit")); // Exit

    connect(mActionExit, SIGNAL(triggered()), this, SLOT(exit()));
    connect(mActionSavePaintImage, SIGNAL(triggered()), this, SLOT(cropPaintImage()));
    connect(mActionSaveOriginalImage, SIGNAL(triggered()), this, SLOT(cropOriginalImage()));
    connect(mActionReset, SIGNAL(triggered()), this, SLOT(reset()));


    // Release resources after closing
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setFocusPolicy(Qt::StrongFocus);
}

ImageMarquees::~ImageMarquees()
{
    inputImg = nullptr;
    paintImg = nullptr;
    paintImageRect = nullptr;
    mMenu = nullptr;
}

void ImageMarquees::setImage(QImage* inputImg, QImage* paintImg, QRect* paintImageRect,const QString &imgPath)
{
    this->inputImg = inputImg;
    this->paintImg = paintImg;
    this->paintImageRect = paintImageRect;
	this->m_imgPath = imgPath;
    isLoadImage = true;
}

void ImageMarquees::setMarqueesEdgeWidth(int width) { this->marqueesEdgeWidth = width; }

void ImageMarquees::recvParentWidgetSizeChangeSignal()
{
    ImageWidget* parentWidget = static_cast<ImageWidget*>(this->parent());
    this->setGeometry(0, 0, parentWidget->width(), parentWidget->height());
    update();
}

void ImageMarquees::paintEvent(QPaintEvent* /*event*/)
{
    QPainterPath transparentArea, cropArea;
    transparentArea.addRect(this->geometry());
    cropArea.addRect(cropRect[CR_CENTER]);
    transparentArea = transparentArea.subtracted(cropArea);
    QPainter painter(this);
    painter.fillPath(transparentArea, QBrush(QColor(0, 0, 0, 160)));
    if (isCropRectStable) {
        painter.setPen(QPen(QColor(0, 140, 255, 255), 1));
        painter.drawRect(cropRect[CR_CENTER]);
		cropArea = QPainterPath();
        for (int i = 1; i < 5; i++)
            cropArea.addRect(cropRect[i]);
        painter.fillPath(cropArea, QBrush(QColor(0, 140, 255, 255)));
    } else {
        painter.setPen(QPen(QColor(255, 0, 0, 255), 1));
        painter.drawRect(cropRect[CR_CENTER]);
    }
}

void ImageMarquees::mousePressEvent(QMouseEvent* event)
{
    switch (event->button()) {
    case Qt::LeftButton:
        mouseStatus = Qt::LeftButton;
        mouseLeftClickedPos = event->pos();
        // Turn off mouse tracking to save resources
        this->setMouseTracking(false);
        break;
    case Qt::RightButton:
        mouseStatus = Qt::RightButton;
        break;
    case Qt::MiddleButton:
        mouseStatus = Qt::MiddleButton;
        break;
    default:
        mouseStatus = Qt::NoButton;
    }
}

void ImageMarquees::mouseMoveEvent(QMouseEvent* event)
{
    if (mouseStatus == Qt::LeftButton) {
        isCropRectStable = false;
        isCropRectExisted = true;
        cropRectChangeEvent(cursorPosInCropRect, event->pos());
        update();
    }
    // Determine whether the mouse is in the rectangle
    if (mouseStatus == Qt::NoButton && isCropRectStable) {
        if (cropRect[CR_ENTIRETY].contains(event->pos())) {
            cursorPosInCropRect = getSubRectInCropRect(event->pos());
        } else {
            cursorPosInCropRect = CR_NULL;
            this->setCursor(Qt::ArrowCursor);
        }
    }
}

void ImageMarquees::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    if (mouseStatus == Qt::LeftButton) {
        // Correct width and height to be positive
        cropRect[CR_CENTER] = cropRect[CR_CENTER].normalized();

        // Limit the selection box of the first drawing in the widget
        if (cursorPosInCropRect == CR_NULL) {
            cropRect[CR_CENTER] = this->rect().intersected(cropRect[CR_CENTER]);
        }
        // Backup
        prevCropRect = cropRect[CR_CENTER];
        mouseStatus = Qt::NoButton;
        calcMarqueesEdgeRect();
        isCropRectStable = true;
        isCropRectExisted = true;
        update();

        // Turn on mouse tracking
        this->setMouseTracking(true);
        mouseStatus = Qt::NoButton;
    }
}

void ImageMarquees::contextMenuEvent(QContextMenuEvent* /*event*/)
{
    mMenu->exec(QCursor::pos());
    mouseStatus = Qt::NoButton;
}

void ImageMarquees::wheelEvent(QWheelEvent* event) { eventFilter(this, event); }

bool ImageMarquees::eventFilter(QObject* /*watched*/, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
            // Support ESC key exit
        case Qt::Key_Escape:
            return keyEscapePressEvent();
        default:
            break;
        }
    }
    // Truncate wheelevent
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        wheelEvent->accept();
        return true;
    }
    return false;
}

void ImageMarquees::keyPressEvent(QKeyEvent* event) { eventFilter(this, event); }

QRect ImageMarquees::getCropRectInImage(const QRect& paintImageRect, const QRect& cropRect)
{
    QRect result = paintImageRect.intersected(cropRect);
    result.translate(-paintImageRect.topLeft());
    return result;
}

void ImageMarquees::calcMarqueesEdgeRect()
{
    // TODO: Reduce the amount of computation or qrect
    cropRect[CR_TOPLEFT].setTopLeft(cropRect[CR_CENTER].topLeft() + QPoint(-marqueesEdgeWidth, -marqueesEdgeWidth));
    cropRect[CR_TOPLEFT].setBottomRight(cropRect[CR_CENTER].topLeft() + QPoint(-1, -1));

    cropRect[CR_TOPRIGHT].setTopRight(cropRect[CR_CENTER].topRight() + QPoint(marqueesEdgeWidth, -marqueesEdgeWidth));
    cropRect[CR_TOPRIGHT].setBottomLeft(cropRect[CR_CENTER].topRight() + QPoint(1, -1));

    cropRect[CR_BOTTOMRIGHT].setTopLeft(cropRect[CR_CENTER].bottomRight() + QPoint(1, 1));
    cropRect[CR_BOTTOMRIGHT].setBottomRight(cropRect[CR_CENTER].bottomRight() + QPoint(marqueesEdgeWidth, marqueesEdgeWidth));

    cropRect[CR_BOTTOMLEFT].setTopRight(cropRect[CR_CENTER].bottomLeft() + QPoint(-1, 1));
    cropRect[CR_BOTTOMLEFT].setBottomLeft(cropRect[CR_CENTER].bottomLeft() + QPoint(-marqueesEdgeWidth, marqueesEdgeWidth));

    cropRect[CR_TOP].setTopLeft(cropRect[CR_TOPLEFT].topRight() + QPoint(1, 0));
    cropRect[CR_TOP].setBottomRight(cropRect[CR_TOPRIGHT].bottomLeft() + QPoint(-1, 0));

    cropRect[CR_RIGHT].setTopLeft(cropRect[CR_TOPRIGHT].bottomLeft() + QPoint(0, 1));
    cropRect[CR_RIGHT].setBottomRight(cropRect[CR_BOTTOMRIGHT].topRight() + QPoint(0, -1));

    cropRect[CR_BOTTOM].setTopLeft(cropRect[CR_BOTTOMLEFT].topRight() + QPoint(1, 0));
    cropRect[CR_BOTTOM].setBottomRight(cropRect[CR_BOTTOMRIGHT].bottomLeft() + QPoint(-1, 0));

    cropRect[CR_LEFT].setTopLeft(cropRect[CR_TOPLEFT].bottomLeft() + QPoint(0, 1));
    cropRect[CR_LEFT].setBottomRight(cropRect[CR_BOTTOMLEFT].topRight() + QPoint(0, -1));

    cropRect[CR_ENTIRETY].setTopLeft(cropRect[CR_TOPLEFT].topLeft());
    cropRect[CR_ENTIRETY].setBottomRight(cropRect[CR_BOTTOMRIGHT].bottomRight());
}

int ImageMarquees::getSubRectInCropRect(QPoint cursorPos)
{
    // TODO: Reduce contains calls
    if (cropRect[CR_CENTER].contains(cursorPos)) {
        this->setCursor(Qt::SizeAllCursor);
        return CR_CENTER;
    }
    if (cropRect[CR_TOPLEFT].contains(cursorPos)) {
        this->setCursor(Qt::SizeFDiagCursor);
        return CR_TOPLEFT;
    }
    if (cropRect[CR_TOP].contains(cursorPos)) {
        this->setCursor(Qt::SizeVerCursor);
        return CR_TOP;
    }
    if (cropRect[CR_TOPRIGHT].contains(cursorPos)) {
        this->setCursor(Qt::SizeBDiagCursor);
        return CR_TOPRIGHT;
    }
    if (cropRect[CR_RIGHT].contains(cursorPos)) {
        this->setCursor(Qt::SizeHorCursor);
        return CR_RIGHT;
    }
    if (cropRect[CR_BOTTOMRIGHT].contains(cursorPos)) {
        this->setCursor(Qt::SizeFDiagCursor);
        return CR_BOTTOMRIGHT;
    }
    if (cropRect[CR_BOTTOM].contains(cursorPos)) {
        this->setCursor(Qt::SizeVerCursor);
        return CR_BOTTOM;
    }
    if (cropRect[CR_BOTTOMLEFT].contains(cursorPos)) {
        this->setCursor(Qt::SizeBDiagCursor);
        return CR_BOTTOMLEFT;
    }
    if (cropRect[CR_LEFT].contains(cursorPos)) {
        this->setCursor(Qt::SizeHorCursor);
        return CR_LEFT;
    }
    return CR_NULL;
}

void ImageMarquees::cropRectChangeEvent(int SR_LOCATION, const QPoint& cursorPos)
{
    switch (SR_LOCATION) {
    case CR_NULL:
        // Limited to mask
        cropRect[CR_CENTER].setTopLeft(mouseLeftClickedPos);
        cropRect[CR_CENTER].setWidth(cursorPos.x() - mouseLeftClickedPos.x());
        cropRect[CR_CENTER].setHeight(cursorPos.y() - mouseLeftClickedPos.y());
        break;
    case CR_CENTER:
        cropRect[CR_CENTER].moveTo(prevCropRect.topLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_TOPLEFT:
        cropRect[CR_CENTER].setTopLeft(prevCropRect.topLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_TOP:
        cropRect[CR_CENTER].setTop(prevCropRect.top() + (cursorPos.y() - mouseLeftClickedPos.y()));
        break;
    case CR_TOPRIGHT:
        cropRect[CR_CENTER].setTopRight(prevCropRect.topRight() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_RIGHT:
        cropRect[CR_CENTER].setRight(prevCropRect.right() + (cursorPos.x() - mouseLeftClickedPos.x()));
        break;
    case CR_BOTTOMRIGHT:
        cropRect[CR_CENTER].setBottomRight(prevCropRect.bottomRight() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_BOTTOM:
        cropRect[CR_CENTER].setBottom(prevCropRect.bottom() + (cursorPos.y() - mouseLeftClickedPos.y()));
        break;
    case CR_BOTTOMLEFT:
        cropRect[CR_CENTER].setBottomLeft(prevCropRect.bottomLeft() + (cursorPos - mouseLeftClickedPos));
        break;
    case CR_LEFT:
        cropRect[CR_CENTER].setLeft(prevCropRect.left() + (cursorPos.x() - mouseLeftClickedPos.x()));
        break;
    default:
        break;
    }
}

bool ImageMarquees::keyEscapePressEvent()
{
    if (isCropRectExisted) {
        reset();
    } else {
        this->exit();
    }
    return true;
}

void ImageMarquees::showErrorMsgBox(const char* errMsg)
{
    QMessageBox msgBox(QMessageBox::Critical, tr("ERROR"), tr(errMsg));
    msgBox.exec();
}

void ImageMarquees::exit()
{
    emit sendExitSignal();
    this->close();
}

void ImageMarquees::reset()
{
    cropRect[CR_CENTER] = QRect(0, 0, 0, 0);
    prevCropRect = QRect(0, 0, 0, 0);

    isCropRectStable = false;
    isCropRectExisted = false;
    // Turn off mouse tracking to save resource
    this->setMouseTracking(false);
    this->setCursor(Qt::ArrowCursor);
    cursorPosInCropRect = CR_NULL;
    mouseStatus = Qt::NoButton;
    update();
}

void ImageMarquees::cropPaintImage()
{
    cropRectInImage = getCropRectInImage(*paintImageRect, cropRect[CR_CENTER]);
    saveImage(paintImg, cropRectInImage);
}

void ImageMarquees::cropOriginalImage()
{
    cropRectInImage = getCropRectInImage(*paintImageRect, cropRect[CR_CENTER]);
    QPoint topLeft = getCursorPosInImage(*inputImg, *paintImg, ImageWidget::NULL_POINT, cropRectInImage.topLeft());
    QPoint bottomRight = getCursorPosInImage(*inputImg, *paintImg, ImageWidget::NULL_POINT, cropRectInImage.bottomRight());
    cropRectInImage.setTopLeft(topLeft);
    cropRectInImage.setBottomRight(bottomRight);
    saveImage(inputImg, cropRectInImage);
}

void ImageMarquees::saveImage(const QImage* img, const QRect& rect)
{
    // Receive intercept box information
    if (!isLoadImage || !img || img->isNull() || rect.width() <= 0 || rect.height() <= 0) {
        showErrorMsgBox(ERR_MSG_NULL_IMAGE);
        return;
    }
    // TODO: It should be possible to select ROI like OpenCV's mat to avoid coping
	QString fileName;
	if (!m_imgPath.isEmpty()) {
		fileName = QFileInfo(m_imgPath).completeBaseName();
	}
	QString filename = getSaveImageFile(fileName,this);

    if (filename.isEmpty() || filename.isNull()) {
        showErrorMsgBox(ERR_MSG_INVALID_FILE_PATH);
        return;
    }
    QImage saveImageTemp = img->copy(rect);
    saveImageTemp.save(filename);
}

ImageWidget::ImageWidget(QWidget* parent)
    : QWidget(parent)
{
    isCropImageMode = false;
    imageWidgetPaintRect = QRect(-PAINT_AREA_OFFEST, -PAINT_AREA_OFFEST, this->width() + 2 * PAINT_AREA_OFFEST, this->height() + 2 * PAINT_AREA_OFFEST);
    initializeContextmenu();
}

ImageWidget::~ImageWidget() {}

bool ImageWidget::setImage(const QImage& img, bool isDeepCopy)
{
    if (img.isNull()) {
        return false;
    }
    // By default, the shallow copy of QImage is used, and the data reference in QImage is 
	// automatically managed to avoid using pointers and passing in local variables to cause crash
    if (isDeepCopy) {
        inputImg = img.copy();
    } else {
        inputImg = img;
    }
    initShowImage();
    return true;
}

bool ImageWidget::setImage(const QString& filePath) { return loadImageFromPath(filePath); }

bool ImageWidget::setImage(const std::string& filePath) { return loadImageFromPath(QString::fromStdString(filePath)); }

bool ImageWidget::loadImageFromPath(const QString& filePath)
{
    if (filePath.isEmpty() || filePath.isNull()) {
        return false;
    }
    inputImg.load(filePath);
    if (inputImg.isNull()) {
        return false;
    }
	m_imgPath = filePath;
    initShowImage();
    return true;
}

void ImageWidget::setImageAttributeWithAutoFitFlag(bool enableAutoFit)
{
    if (enableAutoFit) {
        // Scale image based on widget size
        paintImg = inputImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio);
    } else {
        zoomScale = 1.0;
        paintImg = inputImg;
    }
    // Calculate the upper left coordinate of the image displayed in the widget
    paintImageRect.setTopLeft(getImageTopLeftPosWhenShowInCenter(paintImg, this));
    paintImageRect.setSize(paintImg.size());
    paintImageLastTopLeft = paintImageRect.topLeft();
}

void ImageWidget::fixPaintImageTopLeft()
{
    if (restrictMode == RM_INNER) {
        fixPaintImageTopLeftInOutterMode(imageWidgetPaintRect, paintImageRect);
    } else {
        fixPaintImageTopLefInInnerMode(imageWidgetPaintRect, paintImageRect);
    }
}

void ImageWidget::initShowImage()
{
    paintImg = inputImg.copy();
    setImageAttributeWithAutoFitFlag(enableAutoFitWidget);
    updateImageWidget();
}

void ImageWidget::clear()
{
    if (!inputImg.isNull()) {
        inputImg = NULL_QIMAGE;
        paintImg = NULL_QIMAGE;
        lastPaintImgSize = NULL_SIZE;
        zoomScale = 1.0;
        mouseLeftKeyPressDownPos = NULL_POINT;
        paintImageLastTopLeft = NULL_POINT;
        paintImageRect = NULL_RECT;
        update();
    }
}

void ImageWidget::setEnableOnlyShowImage(bool flag) { enableOnlyShowImage = flag; }

ImageWidget* ImageWidget::setEnableDrag(bool flag)
{
    enableDragImage = flag;
    mActionEnableDrag->setChecked(enableDragImage);
    return this;
}

ImageWidget* ImageWidget::setEnableZoom(bool flag)
{
    enableZoomImage = flag;
    mActionEnableZoom->setChecked(enableZoomImage);
    return this;
}

ImageWidget* ImageWidget::setEnableAutoFit(bool flag)
{
    enableAutoFitWidget = flag;
    mActionImageAutoFitWidget->setChecked(enableAutoFitWidget);
    resetImageWidget();
    return this;
}

ImageWidget* ImageWidget::setMaxZoomScale(double scale)
{
    MAX_ZOOM_SCALE = scale;
    return this;
}

ImageWidget* ImageWidget::setMinZoomScale(double scale)
{
    MIN_ZOOM_SCALE = scale;
    return this;
}

ImageWidget* ImageWidget::setMaxZoomedImageSize(int width, int height)
{
    MAX_ZOOMED_IMG_SIZE.setWidth(width);
    MAX_ZOOMED_IMG_SIZE.setHeight(height);
    return this;
}

ImageWidget* ImageWidget::setMinZoomedImageSize(int width, int height)
{
    MIN_ZOOMED_IMG_SIZE.setWidth(width);
    MIN_ZOOMED_IMG_SIZE.setHeight(height);
    return this;
}

ImageWidget* ImageWidget::setPaintAreaOffset(int offset)
{
    this->PAINT_AREA_OFFEST = offset;
    return this;
}

ImageWidget* ImageWidget::setPaintImageRestrictMode(ImageWidget::RestrictMode rm)
{
    restrictMode = rm;
    return this;
}

ImageWidget* ImageWidget::setEnableSendLeftClickedPosInWidget(bool flag)
{
    enableSendLeftClickedPosInWidget = flag;
    return this;
}

ImageWidget* ImageWidget::setEnableSendLeftClickedPosInImage(bool flag)
{
    enableSendLeftClickedPosInImage = flag;
    return this;
}

QPoint ImageWidget::getDrawImageTopLeftPos() const { return paintImageRect.topLeft(); }

void ImageWidget::wheelEvent(QWheelEvent* e)
{
    if (!inputImg.isNull() && !enableOnlyShowImage && enableZoomImage) {
        int numDegrees = e->delta();
        if (numDegrees > 0) {
            imageZoomOut();
        } else {
            imageZoomIn();
        }
        updateZoomedImage();
        updateImageWidget();
    }
}

void ImageWidget::mousePressEvent(QMouseEvent* e)
{
    if (!inputImg.isNull() && !enableOnlyShowImage) {
        switch (e->button()) {
        case Qt::LeftButton:
            mouseStatus = Qt::LeftButton;
            mouseLeftKeyPressDownPos = e->pos();
            break;
        case Qt::RightButton:
            mouseStatus = Qt::RightButton;
            break;
        case Qt::MiddleButton:
            mouseStatus = Qt::MiddleButton;
            break;
        default:
            mouseStatus = Qt::NoButton;
        }
    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* e)
{
    // Click events
    if (mouseStatus == Qt::LeftButton && !isImageDragging) {
        sendLeftClickedSignals(e);
        mouseStatus = Qt::NoButton;
    }
    if (!inputImg.isNull() && !enableOnlyShowImage && enableDragImage) {
        if (mouseStatus == Qt::LeftButton) {
            // Record last image vertex
            paintImageLastTopLeft = paintImageRect.topLeft();
            // After release, the mouse status is set to No
            mouseStatus = Qt::NoButton;
            isImagePosChanged = true;
            isImageDragging = false;
            updateImageWidget();
        }
    }
}

void ImageWidget::fixPaintImageTopLeftInOutterMode(const QRect& imageWidgetRect, QRect& qImgZoomedRect)
{
    // Calculate and correct the coordinate deviation
    if (qImgZoomedRect.right() < imageWidgetRect.left()) {
        qImgZoomedRect.moveRight(imageWidgetRect.left());
    }
    if (qImgZoomedRect.left() > imageWidgetRect.right()) {
        qImgZoomedRect.moveLeft(imageWidgetRect.right());
    }
    if (qImgZoomedRect.bottom() < imageWidgetRect.top()) {
        qImgZoomedRect.moveBottom(imageWidgetRect.top());
    }
    if (qImgZoomedRect.top() > imageWidgetRect.bottom()) {
        qImgZoomedRect.moveTop(imageWidgetRect.bottom());
    }
}

void ImageWidget::fixPaintImageTopLefInInnerMode(const QRect& imageWidgetRect, QRect& paintImageRect)
{
    // Calculate and correct the coordinate deviation
    if (paintImageRect.width() <= imageWidgetRect.width()) {
        if (paintImageRect.left() < imageWidgetRect.left()) {
            paintImageRect.moveLeft(imageWidgetRect.left());
        }
        if (paintImageRect.right() > imageWidgetRect.right()) {
            paintImageRect.moveRight(imageWidgetRect.right());
        }
    }
    if (paintImageRect.height() <= imageWidgetRect.height()) {
        if (paintImageRect.top() < imageWidgetRect.top()) {
            paintImageRect.moveTop(imageWidgetRect.top());
        }
        if (paintImageRect.bottom() > imageWidgetRect.bottom()) {
            paintImageRect.moveBottom(imageWidgetRect.bottom());
        }
    }
    if (paintImageRect.width() > imageWidgetRect.width()) {
        if (paintImageRect.left() > imageWidgetRect.left()) {
            paintImageRect.moveLeft(imageWidgetRect.left());
        }
        if (paintImageRect.right() < imageWidgetRect.right()) {
            paintImageRect.moveRight(imageWidgetRect.right());
        }
    }
    if (paintImageRect.height() > imageWidgetRect.height()) {
        if (paintImageRect.top() > imageWidgetRect.top()) {
            paintImageRect.moveTop(imageWidgetRect.top());
        }
        if (paintImageRect.bottom() < imageWidgetRect.bottom()) {
            paintImageRect.moveBottom(imageWidgetRect.bottom());
        }
    }
}

void ImageWidget::mouseMoveEvent(QMouseEvent* e)
{
    // TODO: After the image moves out of the widget, define drawimagetopleftpos to prevent the image from flying too far out of the widget
    if (!inputImg.isNull() && !enableOnlyShowImage && enableDragImage) {
        if (mouseStatus == Qt::LeftButton) {
            // e->pos()Converts the current mouse coordinates to relative movement distance
            paintImageRect.moveTopLeft(paintImageLastTopLeft + (e->pos() - mouseLeftKeyPressDownPos));
            isImageDragging = true;
            updateImageWidget();
        }
    }
}

void ImageWidget::paintEvent(QPaintEvent* /*e*/)
{
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(200, 200, 200)));
    painter.drawRect(0, 0, this->width(), this->height());
    if (!inputImg.isNull()) {
        painter.drawImage(paintImageRect.topLeft(), paintImg);
    }
}

void ImageWidget::contextMenuEvent(QContextMenuEvent* /*e*/)
{
    if (!enableOnlyShowImage) {
        mMenu->exec(QCursor::pos());
        // After the right-click menu pops up, the mouse status is set to No
        mouseStatus = Qt::NoButton;
    }
}

void ImageWidget::resizeEvent(QResizeEvent* /*e*/)
{
    imageWidgetPaintRect = QRect(-PAINT_AREA_OFFEST, -PAINT_AREA_OFFEST, this->width() + 2 * PAINT_AREA_OFFEST, this->height() + 2 * PAINT_AREA_OFFEST);
    if (!inputImg.isNull()) {
        // If the image is not dragged or zoomed, zoom in and out
        if (!isImagePosChanged && enableAutoFitWidget) {
            paintImg = inputImg.scaled(this->width(), this->height(), Qt::KeepAspectRatio);
            paintImageRect.moveTopLeft(getImageTopLeftPosWhenShowInCenter(paintImg, this));
            paintImageRect.setSize(paintImg.size());
            paintImageLastTopLeft = paintImageRect.topLeft();
        } else {
            // TODO: How to change the image size after dragging the widget
        }
        if (isCropImageMode)
            emit sendParentWidgetSizeChangedSignal();
        fixPaintImageTopLeft();
        paintImageLastTopLeft = paintImageRect.topLeft();
    }
}

void ImageWidget::resetImageWidget()
{
    setImageAttributeWithAutoFitFlag(enableAutoFitWidget);
    isImagePosChanged = false;
    updateImageWidget();
}

void ImageWidget::imageZoomOut()
{
    // TODO: Scale and size are used to limit the size
    if (zoomScale < MAX_ZOOM_SCALE) {
        zoomScale *= 1.1;
        isZoomedParametersChanged = true;
    }
}

void ImageWidget::imageZoomIn()
{
    // TODO: Scale and size are used to limit the size
    if (zoomScale > MIN_ZOOM_SCALE) {
        zoomScale *= 1.0 / 1.1;
        isZoomedParametersChanged = true;
    }
}

void ImageWidget::enterCropImageMode()
{
    if (!inputImg.isNull()) {
        isCropImageMode = true;
        ImageMarquees* m = new ImageMarquees(this);
        m->setGeometry(0, 0, this->geometry().width(), this->geometry().height());
        connect(m, SIGNAL(sendExitSignal()), this, SLOT(exitCropImageMode()));
        connect(this, SIGNAL(sendParentWidgetSizeChangedSignal()), m, SLOT(recvParentWidgetSizeChangeSignal()));
        m->setImage(&inputImg, &paintImg, &paintImageRect, m_imgPath);
        m->show();
    }
}

void ImageWidget::initializeContextmenu()
{
    mMenu = new QMenu(this);
    mMenuAdditionalFunction = new QMenu(mMenu);

    mActionResetParameters = mMenu->addAction(tr("Reset")); // Reset
    mActionSave = mMenu->addAction(tr("Save As")); // Save As
    mActionCrop = mMenu->addAction(tr("Crop")); // Crop
    mMenuAdditionalFunction = mMenu->addMenu(tr("More Function")); // More Function
    mActionEnableDrag = mMenuAdditionalFunction->addAction(tr("Enable Drag")); // Enable Drag
    mActionEnableZoom = mMenuAdditionalFunction->addAction(tr("Enable Zoom")); // Enable Zoom
    mActionImageAutoFitWidget = mMenuAdditionalFunction->addAction(tr("Enable Image Fit Widget")); // Enable Image Fit Widget

    mActionEnableDrag->setCheckable(true);
    mActionEnableZoom->setCheckable(true);
    mActionImageAutoFitWidget->setCheckable(true);

    mActionEnableDrag->setChecked(enableDragImage);
    mActionEnableZoom->setChecked(enableZoomImage);
    mActionImageAutoFitWidget->setChecked(enableAutoFitWidget);

    connect(mActionResetParameters, SIGNAL(triggered()), this, SLOT(resetImageWidget()));
    connect(mActionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(mActionCrop, SIGNAL(triggered()), this, SLOT(enterCropImageMode()));
    connect(mActionEnableDrag, SIGNAL(toggled(bool)), this, SLOT(setEnableDrag(bool)));
    connect(mActionEnableZoom, SIGNAL(toggled(bool)), this, SLOT(setEnableZoom(bool)));
    connect(mActionImageAutoFitWidget, SIGNAL(toggled(bool)), this, SLOT(setEnableAutoFit(bool)));
}

void ImageWidget::sendLeftClickedSignals(QMouseEvent* e)
{
    if (enableSendLeftClickedPosInWidget)
        emit sendLeftClickedPosInWidgetSignal(e->x(), e->y());

    if (enableSendLeftClickedPosInImage) {
        if (inputImg.isNull()) {
            emit sendLeftClickedPosInImageSignal(-1, -1);
        } else {
            QPoint cursorPosInImage = getCursorPosInImage(inputImg, paintImg, paintImageRect.topLeft(), e->pos());
            // Returns if the cursor is not on the image-1
            if (cursorPosInImage.x() < 0 || cursorPosInImage.y() < 0 || cursorPosInImage.x() > inputImg.width() - 1
                || cursorPosInImage.y() > inputImg.height() - 1) {
                cursorPosInImage.setX(-1);
                cursorPosInImage.setY(-1);
            }
            emit sendLeftClickedPosInImageSignal(cursorPosInImage.x(), cursorPosInImage.y());
        }
    }
}

QPoint getCursorPosInImage(const QImage& originalImage, const QImage& zoomedImage, const QPoint& imageLeftTopPos, const QPoint& cursorPos)
{
    // Calculate the position of the current cursor in the original image coordinate system relative to the image origin
    QPoint resPoint;
    int distanceX = cursorPos.x() - imageLeftTopPos.x();
    int distanceY = cursorPos.y() - imageLeftTopPos.y();
    double xDivZoomedImageW = double(distanceX) / double(zoomedImage.width());
    double yDivZoomedImageH = double(distanceY) / double(zoomedImage.height());
    resPoint.setX(int(std::floor(originalImage.width() * xDivZoomedImageW)));
    resPoint.setY(int(std::floor(originalImage.height() * yDivZoomedImageH)));
    return resPoint;
}

void ImageWidget::save()
{
    if (!inputImg.isNull()) {
        QImage temp = inputImg.copy();

		QString fileName;
		if (!m_imgPath.isEmpty()) {
			fileName = QFileInfo(m_imgPath).completeBaseName();
		}
		QString filename = getSaveImageFile(fileName,this);
		if (!filename.isEmpty() || !filename.isNull()) {
			temp.save(filename);
		}            
    }
}

void ImageWidget::exitCropImageMode() { isCropImageMode = false; }

void ImageWidget::updateImageWidget()
{
    fixPaintImageTopLeft();
    update();
}

void ImageWidget::updateZoomedImage()
{
    // If the image is empty, return it directly
    if (inputImg.isNull())
        return;
    if (isZoomedParametersChanged) {
        lastPaintImgSize = paintImg.size();
    }
    // Reduce QImage:: scaled caused by dragging
    if (enableAutoFitWidget) {
        paintImg = inputImg.scaled(this->width() * zoomScale, this->height() * zoomScale, Qt::KeepAspectRatio);
    } else {
        paintImg = inputImg.scaled(inputImg.width() * zoomScale, inputImg.height() * zoomScale, Qt::KeepAspectRatio);
    }
    // TODO: The position of the image should not change during scaling
    if (isZoomedParametersChanged) {
        QSize zoomedImageChanged = lastPaintImgSize - paintImg.size();
        // Get the current cursor and calculate the position of the cursor in the image
        QPoint cursorPosInWidget = this->mapFromGlobal(QCursor::pos());
        QPoint cursorPosInImage = getCursorPosInImage(inputImg, paintImg, paintImageRect.topLeft(), cursorPosInWidget);
        if (cursorPosInImage.x() < 0) {
            cursorPosInImage.setX(0);
        }
        if (cursorPosInImage.y() < 0) {
            cursorPosInImage.setY(0);
        }
        if (cursorPosInImage.x() > inputImg.width()) {
            cursorPosInImage.setX(inputImg.width() - 1);
        }
        if (cursorPosInImage.y() > inputImg.height()) {
            cursorPosInImage.setY(inputImg.height() - 1);
        }
        // Adjust the position of the cursor in the image, and keep the mouse hover point as the zoom center
        paintImageRect.moveTopLeft(paintImageLastTopLeft
            + QPoint(int(double(zoomedImageChanged.width()) * double(cursorPosInImage.x()) / double(inputImg.width() - 1)),
                int(double(zoomedImageChanged.height()) * double(cursorPosInImage.y()) / double(inputImg.height() - 1))));
        paintImageRect.setSize(paintImg.size());

        if (paintImageRect.topLeft() != paintImageLastTopLeft) {
            isImagePosChanged = true;
        }
        paintImageLastTopLeft = paintImageRect.topLeft();
    }
    isZoomedParametersChanged = false;
}

QPoint ImageWidget::getImageTopLeftPosWhenShowInCenter(const QImage& img, const QWidget* iw)
{
    return QPoint((iw->width() - img.width()) / 2, (iw->height() - img.height()) / 2);
}
