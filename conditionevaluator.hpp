/* Copyright (c) 2014, The Nuria Project
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *    1. The origin of this software must not be misrepresented; you must not
 *       claim that you wrote the original software. If you use this software
 *       in a product, an acknowledgment in the product documentation would be
 *       appreciated but is not required.
 *    2. Altered source versions must be plainly marked as such, and must not be
 *       misrepresented as being the original software.
 *    3. This notice may not be removed or altered from any source
 *       distribution.
 */

#ifndef NURIA_CONDITIONEVALUATOR_HPP
#define NURIA_CONDITIONEVALUATOR_HPP

#include "lazyevaluation.hpp"
#include "essentials.hpp"

namespace Nuria {
class ConditionEvaluatorPrivate;

/**
 * \brief The AbstractConditionEvaluator class
 */
class NURIA_CORE_EXPORT AbstractConditionEvaluator {
public:
	
	AbstractConditionEvaluator ();
	virtual ~AbstractConditionEvaluator ();
	
	virtual bool compile (const LazyCondition &condition) = 0;
	virtual bool evaluate (const QVariantList &arguments, bool &error) = 0;
	
};


/**
 * \brief The ConditionEvaluator class implements a simple evaluator for
 * LazyCondition.
 * 
 */
class NURIA_CORE_EXPORT ConditionEvaluator : public AbstractConditionEvaluator {
public:
	ConditionEvaluator ();
	~ConditionEvaluator ();
	
	bool compile (const LazyCondition &condition) override;
	bool evaluate (const QVariantList &arguments, bool &error) override;
	
private:
	ConditionEvaluatorPrivate *d_ptr;
};

}

#endif // NURIA_CONDITIONEVALUATOR_HPP
