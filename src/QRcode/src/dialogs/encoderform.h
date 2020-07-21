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


#ifndef __ENCODER_FORM_H__
#define __ENCODER_FORM_H__

#include "qrcodewidget.h"

#include <QtCore/QProcess>

/*
#1 Words
myqr https://github.com
Just input a URL or a sentence, then get your QR-Code named 'qrcode.png' in the current directory.

​

#2 -v, -l
myqr https://github.com -v 10 -l Q
The default size of QR-Code depends both on the numbers of words you input and the level, while the default level (Error Correction Level) is H (the highest).

Customize: If you want to control the size and the error-correction-level, use the -v and -l arguments.

-v representing the length is from a minimum of 1 to a maximum of 40.

-l representing the error correction level is one of L, M, Q and H, where L is the lowest level and H is the highest.

#3 -n, -d
myqr https://github.com   -n github_qr.jpg   -d .../paths/
The default output-filename is 'qrcode.png', while the default output-directory is current directory.

Customize: You can name the output-file and decide the output-directory. Notice that if the name is as same as a existing file, the old one will be deleted.

-n representing the output-filename could be in the format one of .jpg， .png ，.bmp ，.gif .

-d means directory.

Artistic QR-Code


#1 -p
myqr https://github.com -p github.jpg
The -p is to combine the QR-Code with the following picture which is in the same directory as the program. The resulting picture is black and white by default.
#2 -c
myqr https://github.com -p github.jpg -c
The -c is to make the resulting picture colorized.

​

#3 -con, -bri
myqr https://github.com -p github.jpg [-c] -con 1.5 -bri 1.6
The -con flag changes the contrast of the picture - a low number corresponds to low contrast and a high number to high contrast. Default: 1.0.

The -bri flag changes the brightness and the parameter values work the same as those for -con. Default: 1.0.
*/
struct QRCodePara {
	bool  m_c;		// -c is to make the resulting picture colorized.
	QChar m_l;		// -l representing the error correction level is one of L, M, Q and H, where L is the lowest level and H is the highest.
	int m_v;		// -v representing the length is from a minimum of 1 to a maximum of 40.
	qreal m_con;	// -con flag changes the contrast of the picture - a low number corresponds to low contrast and a high number to high contrast. Default: 1.0.
	qreal m_bri;	// -bri flag changes the brightness and the parameter values work the same as those for -con. Default: 1.0.

	void save();
	void load();
};

class EncoderFormUi;
class EncoderForm : public QRCodeWidget
{
    Q_OBJECT

public:
    explicit EncoderForm(QWidget *parent = 0);
    ~EncoderForm();

public:
  virtual void applyChanges();
  virtual void reload();


protected slots:
	void on_btnLoad_clicked();
	void on_btnGen_clicked();
	void slotGenFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void slotGenReadyReadStandardOutput();
	void on_pteContent_textChanged();
private:
	QString generatorArguments();
	

private:
    QScopedPointer<EncoderFormUi> m_ui;

	QProcess m_process;
	QString m_info_filepath;
	QString m_img_src;
};

#endif // __ENCODER_FORM_H__
