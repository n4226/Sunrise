#include "srpch.h"
#include "PlayerMovementSystem.h"

#include "../../core/Application.h"
#include "../WorldScene.h"
#include "Sunrise/Sunrise/world/simlink/SimlinkMessages.h"
#include "Sunrise/Sunrise/networking/networking.h"

namespace sunrise {

	using namespace math;

	PlayerMovementSystem::PlayerMovementSystem()
		: cameraPath(cameraPoints, true)
	{

	}


	void PlayerMovementSystem::setup()
	{
		sunrise::NetworkManager::CreateOptions netOptions{};

		netOptions.newThread = false;
		netOptions.type = sunrise::NetworkManager::Type::server;
		netOptions.udpBufferSize = sizeof(sunrise::SimlinkMessages::simpleUpdate);
		netOptions.deferServerStart = true;

		networkManager = new sunrise::NetworkManager(netOptions, *world->app.context);

		networkManager->registerUDPMessageCalback(std::function([this](const sunrise::SimlinkMessages::simpleUpdate& data) {
			//SR_INFO("got a simpleUpdate: position: ({}, {}, {})", data.lla.x, data.lla.y, data.lla.z);
			{
				auto handle = updateStreamed.lock();

				(*handle) = data;
				hasConnection = true;
			}
			}));

		networkManager->startServer();
	}

	void PlayerMovementSystem::update()
	{
		bool con;


		{
			auto handle = updateStreamed.try_lock();
			con = hasConnection;

			if (con) {
				if (handle && handle->lla != glm::dvec3(0)) {

					world->playerLLA = handle->lla;
					glm::vec3 simlinkRot = glm::eulerAngles(handle->rot);

					world->playerLLARotation =
						  glm::angleAxis(glm::radians(90.f), glm::vec3(-1, 0, 0))
						* glm::angleAxis(simlinkRot.z, glm::vec3(0, 1, 0))
						* glm::angleAxis(simlinkRot.y, glm::vec3(0, 0, -1))
						* glm::angleAxis(simlinkRot.x, glm::vec3(-1, 0, 0));
				}
				else {
					// do nothing
				}
			}
			else {
				movePlayerAlongCamPath();
				rotatePlayerAlongCamPath();
			}
		}


	}

	void PlayerMovementSystem::movePlayerAlongCamPath()
	{
		PROFILE_FUNCTION;
		//static const auto points = cameraPath.CalculateEvenlySpacedPoints(1000, 0.0001, true, Math::dEarthRad);



		auto segmentPoints = cameraPath.getPointsInSegment(currentSegment);

		if (totalSegmentTime == 0) {
			auto distance = math::llaDistance(segmentPoints[0], segmentPoints[2]);
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

	void PlayerMovementSystem::rotatePlayerAlongCamPath()
	{
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
		
		auto currentToNewLlaDelta = futurePlace - world->playerLLA;

		auto deltaAngle = atan2(currentToNewLlaDelta.x, currentToNewLlaDelta.y);

		world->playerLLARotation = 
			  glm::angleAxis(glm::radians(90.f), glm::vec3(-1, 0, 0))
			* glm::angleAxis(static_cast<float>(deltaAngle - M_PI_2), glm::vec3(0, -1, 0));

	}

}