#pragma once
#include "Stage.h"

namespace sunrise::dependency {

	//https://stackoverflow.com/questions/36933176/how-do-you-static-assert-the-values-in-a-parameter-pack-of-a-variadic-template
	template<bool...> struct bool_pack;
	template<bool... bs>
	using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;

	/// <summary>
	/// stage holder inherets from node because it can have:
	/// 
	/// output(s) which start out an execution which stages can take in as input
	/// input(s) which end an exectui9on which stages can output to
	/// 
	/// </summary>
	/// <typeparam name="Inputs"></typeparam>
	/// <typeparam name="Outputs"></typeparam>
	template<typename End, typename Start, typename Options>
	class Dispatcher : public Node<End, Start> {
	public:
		//static_assert(std::is_same<End,tuple::value),"End and start must be tuples")

	

		//template<typename Inputs, typename Outputs>
		//void registerStageInputs(Stage<Inputs, Outputs, Options>* stage, Inputs inputs) {

		//}

		template<typename ... Inputs, typename ... Outputs>
		void registerStageInputs(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>, Options>* stage, Inputs... inputs) {
			//static_assert(all_true<std::is_pointer<>>::value)
		}

		template<typename ... Inputs, typename ... Outputs>
		void registerStageOutputs(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>, Options>* stage, Outputs... outputs) {

		}


		template<typename ... Inputs, typename ... Outputs>
		void registerStageDependancies(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>, Options>* stage, std::vector<TypeErasedStage*> others) {

		}

		// not sure how this part will work
		//void registerInputs(End inputs) {

		//}


		//void registerOutputs(Start outputs) {

		//}


		void run() {
			/*
				what needs to be done:

				for resources:

				for execution:



			*/



		}


	};

}