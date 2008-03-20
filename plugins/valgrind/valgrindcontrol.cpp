/* This file is part of KDevelop
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
   Copyright 2002 Harald Fernengel <harry@kdevelop.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "valgrindcontrol.h"

#include <QXmlInputSource>
#include <QXmlSimpleReader>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QBuffer>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KProcess>

#include <processlinemaker.h>

#include "valgrindmodel.h"
#include "valgrindplugin.h"

ValgrindControl::ValgrindControl(ValgrindPlugin* parent)
    : QObject(parent)
    , m_process(new KProcess(this))
    , m_server(0)
    , m_connection(0)
    , m_model(new ValgrindModel(this))
    , m_applicationOutput(new KDevelop::ProcessLineMaker(this))
{
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_model->setDevice(m_process);

    connect(m_applicationOutput, SIGNAL(receivedStdoutLines(QStringList)), SLOT(applicationOutput(QStringList)));

    connect(m_process, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    connect(m_process, SIGNAL(readyReadStandardError()), SLOT(readyReadStandardError()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), SLOT(processErrored(QProcess::ProcessError)));
}

bool ValgrindControl::run(const KDevelop::IRun& run, int serial)
{
    m_serial = serial;

    Q_ASSERT(m_process->state() != QProcess::Running);

    /*int port = 38462;
    if (!m_server) {
        m_server = new QTcpServer(this);
        connect(m_server, SIGNAL(newConnection()), SLOT(readFromValgrind()));

        // Try an arbitrary port range for now
        while (!m_server->listen(QHostAddress::LocalHost, port) && port < 38482)
            ++port;

        if (!m_server->isListening())
            kWarning() << "Could not open TCP socket for communication with Valgrind." ;
        else
            kDebug() << "Opened TCP socket" << port << "for communication with Valgrind.";
    }*/

    QStringList arguments;
    arguments << QString("--tool=%1").arg(run.instrumentor());
    arguments << run.instrumentorArguments();
    arguments << "--xml=yes";
    //arguments << QString("--log-socket=localhost:%1").arg(port);
    arguments << run.executable().path();
    arguments << run.arguments();

    m_process->setReadChannel(QProcess::StandardError);
    m_process->setProgram(plugin()->valgrindExecutable().path(), arguments);
    m_process->start();

    return true;
}

void ValgrindControl::stop()
{
    m_process->kill();
}

void ValgrindControl::readFromValgrind( )
{
}

/*void ValgrindControl::newValgrindConnection( )
{
    QTcpSocket* sock = m_server->nextPendingConnection();
    kDebug() << sock;
    if (sock && !m_connection) {
        m_connection = sock;
        delete m_inputSource;
        m_inputSource = new QXmlInputSource(sock);
        m_xmlReader->parse(m_inputSource, true);
        connect(sock, SIGNAL(readyRead()), SLOT(slotReadFromValgrind()));
    }
}*/

ValgrindPlugin * ValgrindControl::plugin() const
{
    return static_cast<ValgrindPlugin*>(const_cast<QObject*>(parent()));
}

void ValgrindControl::processErrored(QProcess::ProcessError e)
{
    switch (e) {
        case QProcess::FailedToStart:
            KMessageBox::error(qApp->activeWindow(), i18n("Failed to start valgrind from \"%1.\"", plugin()->valgrindExecutable().path()), i18n("Failed to start Valgrind"));
            break;
        case QProcess::Crashed:
            KMessageBox::error(qApp->activeWindow(), i18n("Valgrind crashed."), i18n("Valgrind Error"));
            break;
        case QProcess::Timedout:
            KMessageBox::error(qApp->activeWindow(), i18n("Valgrind process timed out."), i18n("Valgrind Error"));
            break;
        case QProcess::WriteError:
            KMessageBox::error(qApp->activeWindow(), i18n("Write to Valgrind process failed."), i18n("Valgrind Error"));
            break;
        case QProcess::ReadError:
            KMessageBox::error(qApp->activeWindow(), i18n("Read from Valgrind process failed."), i18n("Valgrind Error"));
            break;
        case QProcess::UnknownError:
            KMessageBox::error(qApp->activeWindow(), i18n("Unknown Valgrind process error."), i18n("Valgrind Error"));
            break;
    }
}

void ValgrindControl::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    kDebug() << "Process Error " << exitCode << exitStatus;

    //core()->running( this, false );

    m_server->close();
    delete m_connection;
    m_connection = 0L;

    /*if (kcInfo.runKc)
    {
        KProcess kcProc;
//        kcProc.setWorkingDirectory(kcInfo.kcWorkDir);
        kcProc << kcInfo.kcPath;
        kcProc << QString("cachegrind.out.%1").arg(p->pid());
        kcProc.startDetached();
    }*/
}

void ValgrindControl::readyReadStandardError()
{
    m_model->parse();
}

void ValgrindControl::readyReadStandardOutput()
{
    m_applicationOutput->slotReceivedStdout(m_process->readAllStandardOutput());
}

void ValgrindControl::applicationOutput(const QStringList & lines)
{
    foreach (const QString& line, lines)
        emit plugin()->output(m_serial, line, KDevelop::IRunProvider::StandardOutput);
}

ValgrindModel * ValgrindControl::model() const
{
    return m_model;
}

#include "valgrindcontrol.moc"
