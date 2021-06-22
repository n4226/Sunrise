#pragma once

/*
	hlepful reasourcdes on templates:



	https://www.youtube.com/watch?v=LMP_sxOaz6g
	https://www.youtube.com/watch?v=pzeYeh-lKI8

	https://crascit.com/2015/03/21/practical-uses-for-variadic-templates/

	// static asserts:
	https://stackoverflow.com/questions/44158904/static-assert-for-public-inheritance
	https://www.fluentcpp.com/2019/01/29/how-to-define-a-variadic-number-of-arguments-of-the-same-type-part-2/

*/


namespace testGPUStages {


	struct Resource {

	};

	struct Buffer : public Resource {
		size_t size;
	};

	struct Image : public Resource {
	};

	struct Attachment : public Image {
	};

	class TypeErasedStage {

	};

	template<typename Inputs, typename Outputs>
	class Node {
	public:

	};

	template<typename Inputs,typename Outputs>
	class Stage: public TypeErasedStage, public Node<Inputs,Outputs> {
	public:

		Stage() {
			
		}

	/*	Stage(Inputs* inputs, Outputs* outputs)
			: inputs(inputs), outputs(outputs)
		{

		}*/

		Inputs* inputs;
		Outputs* outputs;

		virtual void encode(Inputs in) {

		}

		//static_assert(std::is_convertible<input*, std::tuple<>*>::value, "Derived must inherit Resource as public");

	};

	/// <summary>
	/// stage holder inherets from node because it can have:
	/// 
	/// output(s) which start out an execution which stages can take in as input
	/// input(s) which end an exectui9on which stages can output to
	/// 
	/// </summary>
	/// <typeparam name="Inputs"></typeparam>
	/// <typeparam name="Outputs"></typeparam>
	template<typename Inputs, typename Outputs>
	class StageHolder: public Node<Inputs,Outputs> {

	public:

		template<typename ... Inputs,typename ... Outputs>
		void registerStageInputs(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>> stage,Inputs*... inputs) {

		}


		template<typename ... Inputs, typename ... Outputs>
		void registerStageOutputs(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>> stage, Outputs*... outputs) {

		}


		template<typename ... Inputs, typename ... Outputs>
		void registerStageDependancies(Stage<std::tuple<Inputs ...>, std::tuple<Outputs ...>> stage, std::vector<TypeErasedStage*> other) {

		}

		void run() {
			/*
				what needs to be done:
				
				for resources:

				for execution:



			*/
		}


	};

	
	class RenderCoordinator: public StageHolder<std::tuple<>,std::tuple<>> {

	};


	class MyGBufferStage : public Stage<std::tuple<>, std::tuple<Image>> {
	public:
		using Stage::Stage;

		using Inputs = std::tuple<>;


		void encode(Inputs inputs) override {



		}

	};

	class MyDeferredPassStage : public Stage<std::tuple<Image>, std::tuple<Image>> {
	public:
		using Stage::Stage;

		using Inputs = std::tuple<Image>;


		void encode(Inputs inputs) override {




		}

	};

	

	void fun() {
		/*auto holder = StageHolder();

		auto main = MyGBufferStage();
		auto deferred = MyDeferredPassStage();

		Image image{};


		Image swapImage{};


		holder.registerStageInputs(main);
		holder.registerStageOutputs(main,&image);

		holder.registerStageInputs(deferred,&image);
		holder.registerStageOutputs(deferred, &swapImage);*/



	}

}

