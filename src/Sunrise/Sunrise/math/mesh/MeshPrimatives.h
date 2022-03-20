#pragma once
#include "srpch.h"
#include "Mesh.h"

namespace sunrise {

	namespace MeshPrimatives
	{

		Mesh SUNRISE_API square();

		Mesh SUNRISE_API cube();

		namespace Basic2D {
			Basic2DMesh SUNRISE_API screenQuad();
		}

	}

}
