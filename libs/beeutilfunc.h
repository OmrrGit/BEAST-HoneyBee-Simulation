#ifndef _BEEUTILITYFUNCTIONS_H
#define _BEEUTILITYFUNCTIONS_H


namespace BEAST {

// Generates random double in range min-max
double randomDouble(double min, double max) {
    return min + randval<double>(max-min);
}

// Generates random double in range min-max
double randomInt(int min, int max) {
    return min + randval<int>(max-min);
}

// Sqaure a given number
double sqr(double a) {
    return a * a;
}

// If we consider wrapping as there being a grid of identical screens expanding outwards,
// from any one point A to another point B, there are multiple possible positions of B we could consider.
// shortestWrappedVec returns the shortest vector from A to B
Vector2D shortestWrappedVec(Vector2D from, Vector2D to, double width, double height, double& squareOut)
{
    int xCell;
    if (to.x < from.x - 0.5*width)
        xCell = 1;
    else if (to.x > from.x + 0.5*width)
        xCell = -1;
    else
        xCell = 0;

    int yCell;
    if (to.y < from.y - 0.5*height)
        yCell = 1;
    else if (to.y > from.y + 0.5*height)
        yCell = -1;
    else
        yCell = 0;

    Vector2D shortest = Vector2D(to.x + xCell*width, to.y + yCell*height) - from;
    squareOut = shortest.GetLengthSquared();

    return shortest;
}

}

#endif
