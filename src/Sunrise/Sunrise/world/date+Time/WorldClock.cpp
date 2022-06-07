#include "srpch.h"
#include "WorldClock.h"
#include "SolarPositionAlgorithm/spa.h"

sunrise::WorldClock::WorldClock()
{

	auto time = std::chrono::system_clock::now();

	utcTime = time;

}





void sunrise::WorldClock::setDate(date::sys_days date, std::chrono::seconds time)
{
	utcTime = date + time;
}

//sub solar impl
struct Date {
	double year, month, day, hour, minute, second;
};


/*
was converted to c++ with help of codex
originally from python shown in: https://python.plainenglish.io/find-the-suns-subsolar-point-42e9d4935b1c

*/
glm::dvec2 subsolar(Date date) {
	double ta = glm::pi<double>() * 2;
	double ut = date.hour + date.minute / 60 + date.second / 3600;
	double t = 367 * date.year - 7 * (date.year + (date.month + 9) / 12) / 4;
	double dn = t + 275 * date.month / 9 + date.day - 730531.5 + ut / 24;
	double sl = dn * 0.01720279239 + 4.894967873;
	double sa = dn * 0.01720197034 + 6.240040768;
	t = sl + 0.03342305518 * sin(sa);
	double ec = t + 0.0003490658504 * sin(2 * sa);
	double ob = 0.4090877234 - 0.000000006981317008 * dn;
	double st = 4.894961213 + 6.300388099 * dn;
	double ra = atan2(cos(ob) * sin(ec), cos(ec));
	double de = asin(sin(ob) * sin(ec));
	double la = glm::degrees(de);
	double lo = std::remainder(glm::degrees(ra - st), 360);

	if (lo > 180)
		lo = lo - 360;

	return { la, lo };
}


//write a function to calculate the current latitude and longitude of the sun's position on the earth's surface for a given std::chrono::timePoint. It must be acurate using proper known equations
//good source: https://faculty.eng.ufl.edu/jonathan-scheffe/wp-content/uploads/sites/100/2020/08/Solar-Time1419.pdf

//https://www.researchgate.net/post/How-can-we-compute-solar-position-at-a-given-place-on-a-given-day-and-time -- even better look at anser with links to SPA and c code 

date::sys_days sunrise::WorldClock::utcDays()
{
	return date::floor<date::days>(utcTime);
}

std::chrono::time_point<std::chrono::system_clock>::duration sunrise::WorldClock::utcTimeOfDay()
{
	return utcTime - utcDays();
}


date::hh_mm_ss<std::chrono::system_clock::duration> sunrise::WorldClock::utcTimeOfDayHHMMSS()
{
	return date::make_time(utcTime - utcDays());
}

/* code found

	https://github.com/xeqlol/SolarPositionAlgorithm
	https://midcdmz.nrel.gov/spa/
	
	
	https://astronomy.stackexchange.com/questions/7936/given-a-date-obtain-latitude-and-longitude-where-is-the-sun-zenith

	
	//what i'm tring to calculate is the sub solar point

*/

glm::dvec2 sunrise::WorldClock::getSunPosition(std::chrono::time_point<std::chrono::system_clock> timePoint)
{
	//convert timepoint to day point
	auto dayPoint = date::floor<date::days>(timePoint);
	

	auto ymd = date::year_month_day(dayPoint);
	auto delta = timePoint - dayPoint;
	auto time = date::make_time(delta);
	spa_data spa;

	spa.year =	(int)ymd.year();
	spa.month = (glm::uint)ymd.month();
	spa.day =	(glm::uint)ymd.day() - 1;
	spa.hour = time.hours().count();
	spa.minute = time.minutes().count();
	spa.second = time.seconds().count();
	spa.timezone = 0;
	spa.delta_ut1 = 0;
	spa.delta_t = 67;
	spa.longitude = 0;
	spa.latitude = 0;
	spa.elevation = 0;
	spa.pressure = 1010;
	spa.temperature = 10;
	spa.slope = 30;
	spa.azm_rotation = 0;
	spa.atmos_refract = 0.5667;
	spa.function = SPA_ALL;

	//int code = spa_calculate(&spa);
	
	// i believe answer is to use theta and beta from intermediate results of spa struct
	//return glm::dvec2(spa.alpha, spa.delta);
	
	Date date;
	date.year = spa.year;
	date.month = spa.month;
	date.day = spa.day;
	date.hour = spa.hour;
	date.minute = spa.minute;
	date.second = spa.second;
	
	return subsolar(date);
	

	
}

glm::dvec2 sunrise::WorldClock::getCurrentSunPosition()
{
	return getSunPosition(utcTime);
}

void sunrise::WorldClock::update(double dt)
{
	auto seconds = std::chrono::duration<double>(dt * runRate * running);
	auto nativeDur = std::chrono::duration_cast<std::chrono::system_clock::duration>(seconds);

	utcTime += nativeDur;
}

/*
double subsolar(vector<double> utc) {
	double ye, mo, da, ho, mi, se;
	ye = utc[0];
	mo = utc[1];
	da = utc[2];
	ho = utc[3];
	mi = utc[4];
	se = utc[5];
	double ta = pi * 2;
	double ut = ho + mi / 60 + se / 3600;
	double t = 367 * ye - 7 * (ye + (mo + 9) / 12) / 4;
	double dn = t + 275 * mo / 9 + da - 730531.5 + ut / 24;
	double sl = dn * 0.01720279239 + 4.894967873;
	double sa = dn * 0.01720197034 + 6.240040768;
	t = sl + 0.03342305518 * sin(sa);
	double ec = t + 0.0003490658504 * sin(2 * sa);
	double ob = 0.4090877234 - 0.000000006981317008 * dn;
	double st = 4.894961213 + 6.300388099 * dn;
	double ra = atan2(cos(ob) * sin(ec), cos(ec));
	double de = asin(sin(ob) * sin(ec));
	double la = degrees(de);
	double lo = degrees(ra - st) % 360;
	lo = lo - 360 if lo > 180 else lo;
	return [round(la, 6), round(lo, 6)];
}
*/