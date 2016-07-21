/* Copyright 2014-2016 CyberTech Labs Ltd.
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

#pragma once

#include <trikCommon/robotModel/parts/trikGamepadPadPressSensor.h>
#include <trikCommon/robotCommunication/tcpRobotCommunicator.h>

#include "trikInterpreterCommon/declSpec.h"

namespace trik {
namespace robotModel {
namespace real {
namespace parts {

/// Implementation of TRIK Android gamepad pad as button for interpretation mode on real robot.
class ROBOTS_TRIK_KIT_INTERPRETER_COMMON_EXPORT GamepadPadPressSensor
		: public robotModel::parts::TrikGamepadPadPressSensor
{
	Q_OBJECT

public:
	/// Constructor.
	/// @param info - device info for that device.
	/// @param port - port on which this device is configured.
	/// @param tcpRobotCommunicator - communicator object that is used to send/receive data to/from telemetry
	///        service on a robot.
	GamepadPadPressSensor(const kitBase::robotModel::DeviceInfo &info
		, const kitBase::robotModel::PortInfo &port
		, robotCommunication::TcpRobotCommunicator &tcpRobotCommunicator);

	void read() override;

private slots:
	/// Called when new data arrived from robot.
	void onIncomingData(const QString &portName, int value);

private:
	/// Communicator object that is used to send/receive data to/from telemetry service on a robot.
	robotCommunication::TcpRobotCommunicator &mRobotCommunicator;
};

}
}
}
}
