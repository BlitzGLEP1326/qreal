# Copyright 2013-2016 CyberTech Labs Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

QT += widgets network

includes(plugins/robots/core/kitBase \
		plugins/robots/core/twoDModel \
		plugins/robots/trik/trikCommon \
		plugins/robots/utils \
		qrtext \
		plugins/robots/trik/interpreters/trikInterpreterCommon \
		)

links(qrkernel qrutils qrtext qrgui-preferences-dialog qrgui-tool-plugin-interface \
		robots-utils robots-kit-base robots-2d-model robots-trik-kit qextserialport qslog robots-trik-kit-interpreter-common \
		)

HEADERS += \
	$$PWD/src/trikV62InterpreterPlugin.h \
	$$PWD/src/robotModel/real/trikV62RealRobotModel.h \
	$$PWD/src/robotModel/twoD/trikV62TwoDRobotModel.h \

SOURCES += \
	$$PWD/src/trikV62InterpreterPlugin.cpp \
	$$PWD/src/robotModel/real/trikV62RealRobotModel.cpp \
	$$PWD/src/robotModel/twoD/trikV62TwoDRobotModel.cpp \

TRANSLATIONS += \
	$$PWD/../../../../qrtranslations/ru/plugins/robots/trikV62Interpreter_ru.ts \
	$$PWD/../../../../qrtranslations/fr/plugins/robots/trikV62Interpreter_fr.ts \
