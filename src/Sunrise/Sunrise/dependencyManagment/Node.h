#pragma once

namespace sunrise::dependency {

	template<typename ... args>
	using tuple = std::tuple<args ...>;

	using emptyTuple = tuple<>;

	template<typename Inputs, typename Outputs>
	class Node {
	public:



	};

}