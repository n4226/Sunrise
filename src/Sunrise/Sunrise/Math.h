#pragma once

#include "srpch.h"

// all other math headers
#include "math/FrustrumMath.h"
#include "math/Box.h"
#include "math/paths/Bezier.h"
#include "math/paths/Path.h"
#include "math/mesh/Mesh.h"
#include "math/mesh/Triangulation.h"
#include "math/mesh/MeshRendering.h"

namespace sunrise::math {

	// constants

	constexpr auto dEarthRad = 6'378'137.0;
	constexpr auto fEarthRad = 6'378'137.f;
#pragma region Coordinate Convertions

	/// <summary>
	/// 
	/// </summary>
	/// <param name="lla">expected domain: [-90,90],[-180,180]</param>
	/// <param name="origin"></param>
	/// <param name="radius"></param>
	/// <returns></returns>
	glm::dvec3 SUNRISE_API LlatoGeo(glm::dvec3 lla, glm::dvec3 origin = glm::dvec3(), double radius = dEarthRad);
	/// <summary>
	/// 
	/// </summary>
	/// <param name="lla">expected domain: [-90,90],[-180,180]</param>
	/// <param name="origin"></param>
	/// <param name="radius"></param>
	/// <returns></returns>
	glm::vec3 SUNRISE_API LlatoGeo(glm::vec3 lla, glm::vec3 origin = glm::dvec3(),float radius = fEarthRad);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="geo"></param>
	/// <param name="radius">if left 0 rad will be found from the length of geo</param>
	/// <param name="origin"></param>
	/// <returns></returns>
	glm::dvec3 SUNRISE_API GeotoLla(glm::dvec3 geo, glm::float64 radius = 0, glm::dvec3 origin = glm::dvec3());
	/// <summary>
	/// 
	/// </summary>
	/// <param name="geo"></param>
	/// <param name="radius">if left 0 rad will be found from the length of geo</param>
	/// <param name="origin"></param>
	/// <returns></returns>
	glm::vec3 SUNRISE_API GeotoLla(glm::vec3 geo, glm::float32 radius = 0, glm::vec3 origin = glm::vec3());

#pragma endregion

	// LLA Distance
	double SUNRISE_API llaDistance(glm::dvec2 from,glm::dvec2 to,double radius = dEarthRad);

	glm::quat SUNRISE_API fromToRotation(glm::vec3 startingDirection, glm::vec3 endingDirection);
}

