/*
   Copyright (c) 2013, Sean Kasun
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLocale>

#include "minutor.h"

int main(int argc,char *argv[])
{
	QApplication app(argc,argv);

	QString locale = QLocale::system().name();

	QTranslator translator;
	translator.load(QString("minutor_")+locale);
	app.installTranslator(&translator);

	app.setApplicationName("Minutor");
    app.setApplicationVersion("2.0.1");
	app.setOrganizationName("seancode");

	Minutor minutor;

	// Process the cmdline arguments:
	QStringList args = app.arguments();
	size_t numArgs = args.size();
	for (size_t i = 0; i < numArgs; i++)
	{
		if (((args[i] == "-w") || (args[i] == "--world")) && (i + 1 < numArgs))
		{
			minutor.loadWorld(args[i + 1]);
			continue;
		}
		if (((args[i] == "-j") || (args[i] == "--jump")) && (i + 2 < numArgs))
		{
			minutor.jumpToXZ(args[i + 1].toInt(), args[i + 2].toInt());
		}
	}  // for itr - args[]

	minutor.show();
	return app.exec();
}
