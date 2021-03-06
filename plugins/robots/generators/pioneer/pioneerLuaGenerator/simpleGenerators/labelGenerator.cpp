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

#include "labelGenerator.h"

#include <generatorBase/generatorCustomizer.h>
#include <qrutils/stringUtils.h>

#include "generators/gotoLabelManager.h"

using namespace pioneer::lua;
using namespace generatorBase::simple;

LabelGenerator::LabelGenerator(const qrRepo::RepoApi &repo
		, generatorBase::GeneratorCustomizer &customizer
		, const qReal::Id &id
		, QObject *parent
		, GotoLabelManager &gotoLabelManager)
	: BindingGenerator(repo, customizer, id, "label.t"
			, { Binding::createStaticConverting(
					"@@ID@@"
					, gotoLabelManager.labelFor(id)
					, customizer.factory()->nameNormalizerConverter()
				)
			}
			, parent)
{
}
