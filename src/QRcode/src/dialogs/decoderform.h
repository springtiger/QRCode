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


#ifndef __DECODER_FORM_H__
#define __DECODER_FORM_H__

#include "qrcodewidget.h"


class DecoderFormUi;
class QZXing;
class DecoderForm : public QRCodeWidget
{
    Q_OBJECT

public:
    explicit DecoderForm(QWidget *parent = 0);
    ~DecoderForm();

public:
  virtual void applyChanges();
  virtual void reload();


protected slots:
	void on_btnLoad_clicked();
	void on_btnCopy_clicked();

private:
    QScopedPointer<DecoderFormUi> m_ui;

	QZXing * m_zxing;
};

#endif // __ENCODER_FORM_H__
