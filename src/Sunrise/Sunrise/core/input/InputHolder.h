#pragma once

#include "srpch.h"

namespace sunrise {
	class InputHolder
	{
	public:

		// will have both pulling and event quirring options

		/// <summary>
		/// get if a key is pressed 
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		virtual bool getKey(int key) = 0;


		// mouse

		bool mouseLeft = false;
		bool mouseRight = false;
		glm::dvec2 mousePos = glm::dvec2(0, 0);	

		/// <summary>
		/// delta over current frame
		/// </summary>
		glm::dvec2 mousePosFrameDelta = glm::dvec2(0, 0);

	protected:
		glm::dvec2 lastFrameMosPos = glm::dvec2(0, 0);
	};
}