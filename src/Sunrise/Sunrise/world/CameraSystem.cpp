#include "srpch.h"
#include "CameraSystem.h"
#include "../core/Application.h"
#include "../world/WorldScene.h"

namespace sunrise {

	using namespace math;

	CameraSystem::CameraSystem()
		: cameraPath(cameraPoints, true)
	{

	}

	void CameraSystem::update()
	{
		PROFILE_FUNCTION;
			// move around the world fast
			//world->playerLLA = glm::dvec3(45 + sin(world->timef * 0.1f), 0, 10100 + -cos(world->timef * 0.5f) * 10'000);

			//world->playerLLA = glm::dvec3(sin(world->timef * 0.8f) * 6, sin(world->timef * 1.f) * 9, 1'000);
			//world->playerLLA = glm::dvec3(sin(world->timef * 0.8f) * 60, 0, 1'000);
			//world->playerLLA = glm::dvec3(0, sin(world->timef * 1.f) * 90, 1'000);

			// move up and down
			//world->playerLLA.z = -cos(world->timef) * 10000 + 10100;
			// the camera looks at -> +z
			//world->playerLLA.z = sin(world->timef * 0.1f) * 400 + 500;

			movePlayerAlongCamPath();

		world->playerTrans.position = LlatoGeo(world->playerLLA, world->origin, world->terrainSystem->getRadius());

		//world->playerTrans.rotation = glm::orientation(glm::vec3(0,0,1), glm::normalize(world->playerTrans.position));

		//world->playerTrans.rotation = glm::lookAt(glm::vec3(0), -glm::vec3(world->origin), glm::vec3(0, 0, -1));

		// sort of works - posible additnio to up = glm::normalize(world->playerTrans.position)
		//world->playerTrans.rotation = glm::lookAt(glm::normalize(world->playerTrans.position), glm::vec3(0), glm::vec3(0, 0, -1));

		// rotation ------



		// sin form [0,1] - * (sin(world->timef) * 0.5f + 1)

		auto N = glm::normalize(world->playerTrans.position + glm::vec3(world->origin));

		auto QuatAroundX = glm::angleAxis(glm::radians(static_cast<float>(world->playerLLA.x)), glm::vec3(1.0, 0.0, 0.0));
		// when at 0 lat and 0 lon to look directly at sphere the quataroundy must have an angle of .pi/2 rad
		auto QuatAroundY = glm::angleAxis(-glm::pi<float>() / 2 - glm::radians(static_cast<float>(world->playerLLA.y)), glm::vec3(0.0, 1.0, 0.0));
		auto QuatAroundZ = glm::angleAxis(0.f, glm::vec3(0.0, 0.0, 1.0));
		auto finalOrientation = QuatAroundZ * QuatAroundY * QuatAroundX;


		auto segment = currentSegment;

		auto segTime = (currentSegmentTime + 0.1);
		auto segtotalTime = totalSegmentTime;

		if (segTime > totalSegmentTime)
		{
			segment++;
			auto segmentPoints = cameraPath.getPointsInSegment(segment);
			auto distance = llaDistance(segmentPoints[0], segmentPoints[2]);
			segTime -= totalSegmentTime;
			segtotalTime = distance / speed;
		}
		auto segmentPoints = cameraPath.getPointsInSegment(segment);

		auto futurePlace = Bezier::evaluateCubic(segmentPoints[0], segmentPoints[1], segmentPoints[2], segmentPoints[3], glm::clamp(segTime / segtotalTime, 0.0, 1.0));
		//auto futureGeo = Math::LlatoGeo(futurePlace, world->origin, world->terrainSystem->getRadius());

		auto currentToNewLlaDelta = futurePlace - world->playerLLA;

		auto deltaAngle = atan2(currentToNewLlaDelta.x, currentToNewLlaDelta.y);

		//std::cout << glm::degrees(deltaAngle) << std::endl;

		world->playerTrans.rotation = finalOrientation *
			glm::angleAxis(glm::radians(90.f), glm::vec3(-1, 0, 0))
			// this is temporarry to rotate around to look
			//*glm::angleAxis(glm::radians(world->timef * 20.f), glm::vec3(0, 1, 0))


			// make cmaera look at future path
			* glm::angleAxis(static_cast<float>(deltaAngle - M_PI_2), glm::vec3(0, -1, 0))
			;

		//glm::
		//static_cast<glm::quat>(glm::lookAt(glm::vec3(0), glm::cross(N,glm::vec3(0,0,1)),-N));


		//glm::angleAxis(glm::radians(90.f), glm::vec3(0,-1, 0)) *
		//Math::fromToRotation(glm::vec3(0, -1, 0), glm::normalize(world->playerTrans.position + glm::vec3(world->origin)));
		//* glm::angleAxis(glm::radians(90.f), glm::vec3(-1, 0, 0));

		//glm::angleAxis(static_cast<float>(glm::radians(world->playerLLA.x)) ,glm::vec3(1,0,0));

		//glm::angleAxis(static_cast<float>(glm::radians(world->playerLLA.y)) ,glm::vec3(0,1,0));


	//world->playerTrans.rotation = Math::fromToRotation(glm::vec3(0, 0, -1), -glm::normalize(world->playerTrans.position + glm::vec3(world->origin)))
		//temporary
		//* glm::angleAxis(glm::radians(static_cast<float>(world->playerLLA.y)), glm::vec3(0, -1, 0))
		//* glm::angleAxis(glm::radians(90.f * world->timef), glm::vec3(-1, 0, 0));
		//;
	}


	void CameraSystem::movePlayerAlongCamPath()
	{
		PROFILE_FUNCTION;
			//static const auto points = cameraPath.CalculateEvenlySpacedPoints(1000, 0.0001, true, Math::dEarthRad);



			auto segmentPoints = cameraPath.getPointsInSegment(currentSegment);

		if (totalSegmentTime == 0) {
			auto distance = Math::llaDistance(segmentPoints[0], segmentPoints[2]);
			totalSegmentTime = distance / speed;
		}


		currentSegmentTime += world->deltaTime;

		//std::cout << cameraPath.getNumSegments() << std::endl;

		if (currentSegmentTime > totalSegmentTime) {
			currentSegmentTime -= totalSegmentTime;
			totalSegmentTime = 0;
			currentSegment = (currentSegment + 1) % cameraPath.getNumSegments();
		}


		world->playerLLA =
			//	// Bezier::lerp(segmentPoints[0], segmentPoints[2], t);
			Bezier::evaluateCubic(segmentPoints[0], segmentPoints[1], segmentPoints[2], segmentPoints[3], glm::clamp(currentSegmentTime / totalSegmentTime, 0.0, 1.0));



		//static size_t currentSegment = 0;
		//static const double loopTime = 20; //seconds
		//static double pathLength = cameraPath.lladirectLength();

		//auto segments = cameraPath.getNumSegments();

		//auto timePerSegment = loopTime / segments; 


		//auto t = fmod(world->time, loopTime); //sin(world->time * 0.1) * 0.5 + 0.5;

		////auto currentSegment = static_cast<int>(t / timePerSegment);
		//auto segmentPoints = cameraPath.getPointsInSegment(currentSegment);

		////  * glm::distance(segmentPoints[0],segmentPoints[2])
		//auto segmentTime = (t / timePerSegment) - currentSegment;

		//if (segmentTime > 1) {
		//	//currentSegment += 1;

		//	segmentPoints = cameraPath.getPointsInSegment(currentSegment);
		//	segmentTime = (t / timePerSegment * glm::distance(segmentPoints[0], segmentPoints[2])) - currentSegment;
		//}

		//std::cout << segmentTime << std::endl;
		//world->playerLLA =
		//	// Bezier::lerp(segmentPoints[0], segmentPoints[2], t);
		//	Bezier::evaluateCubic(segmentPoints[0], segmentPoints[3], segmentPoints[1], segmentPoints[2], glm::clamp(segmentTime,0.0,1.0));

	}

}