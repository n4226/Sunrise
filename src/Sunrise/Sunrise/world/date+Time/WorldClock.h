#pragma once
#include "srpch.h"

#include "date/date.h"
#include <chrono>

namespace sunrise {

	class WorldClock
	{
	public:

		WorldClock();

		
		void setDate(date::sys_days date, std::chrono::seconds time);
		
		std::chrono::time_point<std::chrono::system_clock> utcTime;
		//std::chrono::time_point<std::chrono::system_clock> localTime;

		date::sys_days utcDays();
		std::chrono::time_point<std::chrono::system_clock>::duration utcTimeOfDay();
		date::hh_mm_ss<std::chrono::system_clock::duration> utcTimeOfDayHHMMSS();


		/// <summary>
		/// false if paused
		/// </summary>
		bool running = true;

		float runRate = 1;
		

		/// <summary>
		/// returns lat lon of sun above earth
		/// </summary>
		/// <param name="timePoint"></param>
		/// <returns></returns>
		glm::dvec2 getSunPosition(std::chrono::time_point<std::chrono::system_clock> timePoint);
		glm::dvec2 getCurrentSunPosition();

		

	protected:
		friend class WorldScene;

		void update(double dt);

	};

}