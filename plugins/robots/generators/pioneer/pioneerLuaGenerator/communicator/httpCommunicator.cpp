/* Copyright 2017 QReal Research Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "communicator/httpCommunicator.h"

#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QTextCodec>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtWidgets/QApplication>

#include <qrkernel/settingsManager.h>
#include <qrgui/plugins/toolPluginInterface/usedInterfaces/errorReporterInterface.h>
#include <pioneerKit/constants.h>
#include <kitBase/robotModel/robotModelManagerInterface.h>

using namespace pioneer;
using namespace pioneer::lua;
using namespace qReal;

const QString apiLevel = "v0.1";

HttpCommunicator::HttpCommunicator(
		qReal::ErrorReporterInterface &errorReporter
		, const kitBase::robotModel::RobotModelManagerInterface &robotModelManager
		)
	: mCompileProcess(new QProcess)
	, mNetworkManager(new QNetworkAccessManager)
	, mErrorReporter(errorReporter)
	, mRobotModelManager(robotModelManager)
{
	connect(mCompileProcess.data()
			, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished)
			, this
			, &HttpCommunicator::onCompilationDone);

	connect(mNetworkManager.data(), &QNetworkAccessManager::finished, this, &HttpCommunicator::onPostRequestFinished);
}

HttpCommunicator::~HttpCommunicator()
{
	// Empty destructor to keep QScopedPointer happy.
}

void HttpCommunicator::uploadProgram(const QFileInfo &program)
{
	mCurrentProgram = program;

#ifdef Q_OS_WIN
	const QString processName = QApplication::applicationDirPath() + "/pioneerCompile.bat";
#else
	const QString processName = QApplication::applicationDirPath() + "/pioneerCompile.sh";
#endif

	const QString pathToLuac = mRobotModelManager.model().name() == modelNames::realCopter
			? QApplication::applicationDirPath()
					+ "/"
					+ SettingsManager::value(settings::pioneerRealCopterLuaPath, "").toString()
			: SettingsManager::value(settings::pioneerSimulatorLuaPath, "").toString();

	mCompileProcess->start(processName, { program.absoluteFilePath(), pathToLuac });

	mCompileProcess->waitForStarted();
	if (mCompileProcess->state() != QProcess::Running) {
		mErrorReporter.addError(tr("Unable to execute compilation script"));
		QStringList errors = QString(mCompileProcess->readAllStandardError()).split("\n", QString::SkipEmptyParts);
		for (const auto &error : errors) {
			mErrorReporter.addInformation(error);
		}

		done();
	} else {
		mErrorReporter.addInformation(tr("Uploading started, please wait..."));
	}
}

void HttpCommunicator::runProgram(const QFileInfo &program)
{
	mCurrentProgram = program;
	mIsStartNeeded = true;
	uploadProgram(program);
}

void HttpCommunicator::onCompilationDone()
{
	const QString ip = SettingsManager::value(settings::pioneerBaseStationIP).toString();
	if (ip.isEmpty()) {
		mErrorReporter.addError(tr("Pioneer base station IP address is not set. It can be set in Settings window."));
		return;
	}

	const QString port = SettingsManager::value(settings::pioneerBaseStationPort).toString();
	if (port.isEmpty()) {
		mErrorReporter.addError(tr("Pioneer base station port is not set. It can be set in Settings window."));
		return;
	}

	QFileInfo compiledProgram = mCurrentProgram.canonicalPath() + "/" + mCurrentProgram.baseName() + ".luac";
	QFile programFile(compiledProgram.canonicalFilePath());
	if (!programFile.open(QIODevice::ReadOnly)) {
		mErrorReporter.addError(tr("Generation or compilation failed, upload aborted."));
		done();
		return;
	}

	QByteArray program = programFile.readAll();
	programFile.close();

	if (program.isEmpty()) {
		mErrorReporter.addError(tr("Generation or compilation failed, upload aborted."));
		done();
		return;
	}

	QNetworkRequest request(QString("http://%1:%2/pioneer/%3/upload").arg(ip).arg(port).arg(apiLevel));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

	mNetworkManager->post(request, program);
}

void HttpCommunicator::onPostRequestFinished(QNetworkReply *reply)
{
	if (reply->url().toString().endsWith("/upload")) {
		if (reply->error() != QNetworkReply::NoError) {
			mErrorReporter.addError(reply->errorString());
			done();
			return;
		}

		if (mIsStartNeeded) {
			doRunProgram();
			mIsStartNeeded = false;
		} else {
			done();
		}
	} else if (reply->url().toString().endsWith("/start")) {
		if (reply->error() != QNetworkReply::NoError) {
			mErrorReporter.addError(reply->errorString());
		}

		done();
	}

	reply->deleteLater();
}

void HttpCommunicator::done()
{
	if (mIsStartNeeded) {
		emit runCompleted();
	} else {
		emit uploadCompleted();
	}
}

void HttpCommunicator::doRunProgram()
{
	const QString ip = SettingsManager::value(settings::pioneerBaseStationIP).toString();
	if (ip.isEmpty()) {
		mErrorReporter.addError(tr("Pioneer base station IP address is not set. It can be set in Settings window."));
		return;
	}

	const QString port = SettingsManager::value(settings::pioneerBaseStationPort).toString();
	if (port.isEmpty()) {
		mErrorReporter.addError(tr("Pioneer base station port is not set. It can be set in Settings window."));
		return;
	}

	QNetworkRequest request(QString("http://%1:%2/pioneer/%3/start").arg(ip).arg(port).arg(apiLevel));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

	mNetworkManager->post(request, QByteArray());
}