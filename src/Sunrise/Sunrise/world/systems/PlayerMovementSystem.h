#pragma once
#include "srpch.h"
#include "../../graphics/vulkan/renderer/RenderSystem.h"
#include "../../Math.h"

#include "Sunrise/Sunrise/world/simlink/SimlinkMessages.h"
#include "Sunrise/Sunrise/networking/networking.h"

namespace sunrise {

	

	class PlayerMovementSystem: public System
	{
	public:
		PlayerMovementSystem();

		void update() override;
		void setup() override;
		void cleanup() override;

		enum class MovementMode
		{
			Path,Manual,Simlink
		};
	private:

		//TODO: set to simlink to sink to xplane
		MovementMode mode = 
			MovementMode::Simlink;
			//MovementMode::Manual;

		bool hasConnection = false;

		void movePlayerAlongCamPath();
		void rotatePlayerAlongCamPath();

		//manual constants
		glm::dvec2 manualMouseTranslation = glm::dvec2(0,0);

		//TODO: don't think this is correct
		const double speed = 300;//300; // in meters / second

		//TODO: reset this back to 0
		size_t currentSegment = 0;
		double currentSegmentTime = 0;
		double totalSegmentTime = 0;

		std::vector<glm::dvec3> cameraPoints = {

			//glm::dvec3(40.610319941413, -74.039182662964, 100);

			// reversed order
			//glm::dvec3(40.54709068849014 , -73.98451039585544,500),
			//glm::dvec3(40.60603999938215 , -74.02412689599031,500),
			//glm::dvec3(40.68617096089564 , -74.03331417746679,500),
			//glm::dvec3(40.73199901859223 , -74.01813813429706,500),
			//glm::dvec3(40.76734669031339 , -74.01368629698507,500),
			//glm::dvec3(40.79415639586417 , -73.98463708020655,500),
			//glm::dvec3(40.78492880489573 , -73.96389654744658,500),
			//glm::dvec3(40.76711927501298 , -73.97452691880592,500),
			//glm::dvec3(40.73273944708617 , -73.98185517766292,500),
			//glm::dvec3(40.71398903798578 , -73.99861573311571,500),
			//glm::dvec3(40.63009052816579 , -74.05752673164039,500),
			//glm::dvec3(40.59898784065786 , -74.04207906535621,500),
			//glm::dvec3(40.5273511630884  , -74.01884552005585,500),
			//glm::dvec3(40.51994075985466 , -73.96928018695641,500)


			glm::dvec3(40.51994075985466 , -73.96928018695641,50), //300), //500),
			glm::dvec3(40.54709068849014 , -73.98451039585544,50), //300), //500),
			glm::dvec3(40.60603999938215 , -74.02412689599031,50), //300), //500),
			glm::dvec3(40.68617096089564 , -74.03331417746679,50), //300), //500),
			glm::dvec3(40.73199901859223 , -74.01813813429706,50), //300), //500),
			glm::dvec3(40.76734669031339 , -74.01368629698507,50), //300), //500),
			glm::dvec3(40.79415639586417 , -73.98463708020655,50), //300), //500),
			glm::dvec3(40.78492880489573 , -73.96389654744658,50), //300), //500),
			glm::dvec3(40.76711927501298 , -73.97452691880592,50), //300), //500),
			glm::dvec3(40.73273944708617 , -73.98185517766292,50), //300), //500),
			glm::dvec3(40.71398903798578 , -73.99861573311571,50), //300), //500),
			glm::dvec3(40.63009052816579 , -74.05752673164039,50), //300), //500),
			glm::dvec3(40.59898784065786 , -74.04207906535621,50), //300), //500),
			glm::dvec3(40.5273511630884  , -74.01884552005585,50), //300), //500),

			//- this is the first point again 
			//glm::dvec3(40.51994075985466 , -73.96928018695641,100)

		};

		//std::vector<glm::dvec3> cameraPoints = {

		//	glm::dvec3(40.58247271163678, -74.03006951604831, 400),
		//	glm::dvec3(40.61167467092515, -74.02970854633186,400),
		//	glm::dvec3(40.6298066902089 , -74.03581033962581, 400),
		//	glm::dvec3(40.63660028914893, -74.07056592763701,400),
		//	glm::dvec3(40.61839198197045, -74.07515546810707,400 ),
		//	//glm::dvec3(-74.03006951604831, 40.58247271163678,400 )


		//};


		math::Path<glm::dvec3> cameraPath;

		libguarded::shared_guarded<SimlinkMessages::simpleUpdate> updateStreamed = libguarded::shared_guarded<SimlinkMessages::simpleUpdate>(SimlinkMessages::simpleUpdate());

		libguarded::plain_guarded<double> updateInterval = 0;

		sunrise::NetworkManager* networkManager = nullptr;
	};

}