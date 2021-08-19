#include "srpch.h"
#include "Box.h"

namespace sunrise::math {

    constexpr Box::Box()
        : start(0), size(0)
    {
    }
    constexpr Box::Box(glm::dvec2 start, glm::dvec2 size)
        : start(start), size(size)
    {
    }

    constexpr glm::dvec2 Box::getEnd() const
    {
        return start + size;
    }

    constexpr glm::dvec2 Box::getCenter() const
    {
        return start + (size * 0.5);
    }

    constexpr bool Box::contains(glm::dvec2 point) const
    {
        if (point.x < start.x || point.y < start.y) {
            return false;
        }
        else if (point.x > getEnd().x || point.y > getEnd().y) {
            return false;
        }
        return true;
    }

    bool Box::containsAny(std::vector<glm::dvec2>& points) const
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            if (contains(points[i]))
                return true;
        }
        return false;
    }

    bool Box::containsAny(std::vector<glm::dvec2>&& points) const
    {
        for (size_t i = 0; i < points.size(); i++)
        {
            if (contains(points[i]))
                return true;
        }
        return false;
    }

    // see https://www.geeksforgeeks.org/find-two-rectangles-overlap/
    constexpr bool Box::overlaps(Box other) const
    {

        if (start.x == getEnd().x || start.y == getEnd().y || other.start.x == other.getEnd().x
            || other.start.y == other.getEnd().y) {
            // the line cannot have positive overlap
            return false;
        }

        if (start.x >= other.getEnd().x || other.start.x >= getEnd().x) return false;

        if (start.y >= other.getEnd().y || other.start.y >= getEnd().y) return false;

        return true;


    }


    //bool Box::contains(Box other)
    //{
    //    assert(false);
    //    return false;
    //}

    std::vector<glm::dvec2> Box::polygon() const
    {
        return {
            start,
            glm::dvec2(start.x,getEnd().y),
            getEnd(),
            glm::dvec2(getEnd().x,start.y),
        };
    }

    std::string Box::toString() const
    {
        return "(" + std::to_string(start.x) + "," + std::to_string(start.y) + ")" + "_origin--" + "(" + std::to_string(size.x) + "," + std::to_string(size.y) + ")" + "_size";
    }

    std::ostream& operator<<(std::ostream& strm, const Box& a)
    {
        return strm << a.toString();
    }

}