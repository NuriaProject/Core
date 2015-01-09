/* Copyright (c) 2014-2015, The Nuria Project
 * The NuriaProject Framework is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * The NuriaProject Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with The NuriaProject Framework.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NURIA_CONDITIONEVALUATOR_HPP
#define NURIA_CONDITIONEVALUATOR_HPP

#include "lazyevaluation.hpp"
#include "essentials.hpp"
#include "callback.hpp"

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
	
	void registerMethod (const QString &name, const Callback &method);
	
private:
	ConditionEvaluatorPrivate *d_ptr;
};

}

#endif // NURIA_CONDITIONEVALUATOR_HPP
