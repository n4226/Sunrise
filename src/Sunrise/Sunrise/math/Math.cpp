#include "srpch.h"
#include "Math.h"


namespace sunrise {



    glm::dvec3 math::LlatoGeo(glm::dvec3 lla, glm::dvec3 origin, double radius)
    {
        lla.x += 90;
        lla.y += 180;

        radius += lla.z;
        glm::dvec3 xyz;

        xyz.x = (-radius * glm::sin(glm::radians(lla.x)) * glm::cos(glm::radians(lla.y)));
        xyz.y = (-radius * glm::cos(glm::radians(lla.x)));
        xyz.z = (-radius * glm::sin(glm::radians(lla.x)) * glm::sin(glm::radians(lla.y)));
        return (xyz - origin);
    }

    glm::vec3 math::LlatoGeo(glm::vec3 lla, glm::vec3 origin, float radius)
    {
        lla.x += 90;
        lla.y += 180;

        radius += lla.z;
        glm::vec3 xyz;

        xyz.x = (-radius * glm::sin(glm::radians(lla.x)) * glm::cos(glm::radians(lla.y)));
        xyz.y = (-radius * glm::cos(glm::radians(lla.x)));
        xyz.z = (-radius * glm::sin(glm::radians(lla.x)) * glm::sin(glm::radians(lla.y)));
        return (xyz - origin);
    }

    glm::dvec3 math::GeotoLla(glm::dvec3 geo, glm::float64 radius, glm::dvec3 origin)
    {
        geo += origin;
        if (radius == 0)
            radius = glm::length(geo);

        glm::dvec3 lla;

        lla.x = glm::degrees(M_PI - glm::acos(geo.y / radius));
        lla.y = glm::degrees(glm::atan(geo.z, geo.x) + M_PI);

        lla.x -= 90;
        lla.y -= 180;

        return lla;
    }

    glm::vec3 math::GeotoLla(glm::vec3 geo, glm::float32 radius, glm::vec3 origin)
    {
        geo += origin;
        if (radius == 0)
            radius = glm::length(geo);

        glm::vec3 lla;

        lla.x = glm::degrees(M_PI - glm::acos(geo.y / radius));
        lla.y = glm::degrees(glm::atan(geo.z, geo.x) + M_PI);

        lla.x -= 90;
        lla.y -= 180;

        return lla;
    }

    double math::llaDistance(glm::dvec2 from, glm::dvec2 to, double radius)
    {

        //       6371 // Radius of the earth in km
        auto dLat = glm::radians(to.x - from.x);  // deg2rad below
        auto dLon = glm::radians(to.y - from.y);

        auto line1 = glm::sin(dLat / 2) * glm::sin(dLat / 2);
        auto line2 = glm::cos(glm::radians(from.x)) * glm::cos(glm::radians(to.x));
        auto line3 = glm::sin(dLon / 2) * glm::sin(dLon / 2);
        auto a =
            line1 +
            line2 *
            line3;

        auto c = 2 * glm::atan(glm::sqrt(a), glm::sqrt(1 - a));
        auto d = radius * c; // Distance in |km|(maybe)
        return d;
    }

    glm::quat math::fromToRotation(glm::vec3 startingDirection, glm::vec3 endingDirection) {
        if (glm::length2(endingDirection + startingDirection) == 0)
        {
            endingDirection += glm::vec3(.0000001f, 0, 0);
        }
        auto xyz = glm::cross(endingDirection, startingDirection);
        auto w = glm::sqrt(glm::pow(glm::length(endingDirection), 2) * glm::pow(glm::length(startingDirection), 2)) + glm::dot(endingDirection, startingDirection);


        return glm::normalize(glm::quat(w, xyz.x, xyz.y, xyz.z));
    }

}