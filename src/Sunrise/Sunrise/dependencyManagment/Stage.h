#pragma once
#include "Node.h"

namespace sunrise::dependency {

	class TypeErasedStage {

	};

	template<typename Inputs, typename Outputs, typename Options>
	class Stage : public TypeErasedStage, public Node<Inputs, Outputs> {
	public:

		Stage() {

		}

		virtual Outputs run(Inputs in, Options options) {
			return Outputs();
		}

		//static_assert(std::is_convertible<input*, std::tuple<>*>::value, "Derived must inherit Resource as public");

	};

}