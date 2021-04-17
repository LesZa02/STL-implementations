#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

const double Precision = 1e-6;

struct Point
{
    double x = 0, y = 0;

    Point(){}
    explicit Point(double xx, double yy) : x(xx), y(yy) {}

    bool operator==(const Point& another) const
    {
        return (abs(x - another.x) < Precision && abs(y - another.y) < Precision);
    }
    bool operator!=(Point another) const
    {
        return !(*this == another);
    }
};

struct Vector
{
    double x = 0;
    double y = 0;

    Vector(){}
    explicit Vector(double xx, double yy) : x(xx), y(yy) {}
    explicit Vector(const Point& A, const Point& B) : x(B.x - A.x), y(B.y - A.y) {}
};

class Line
{
public:
    Line (){}
    explicit Line (double A, double B, double C) : a(A), b(B), c(C) {}
    explicit Line (Point A, Point B) : a(A.y - B.y), b(B.x - A.x), c(- a*A.x - b*A.y) {}

    explicit Line (double slope, double shift) : a(slope), b(-1), c(shift) {}
    explicit Line (Point point, double slope) : Line(slope, point.y - slope*point.x) {}

    bool operator==(Line another) const;
    bool operator!=(Line another) const
    {
        return !(*this == another);
    }

    double GetA() const
    {
        return a;
    }
    double GetB() const
    {
        return b;
    }
    double GetC() const
    {
        return c;
    }

private:
    double a = 0, b = 0, c = 0;
};

bool Line::operator==(Line another) const
{
    if (abs(b) > Precision && abs(another.GetB()) > Precision)
    {
        return ((abs(a/b -  another.GetA()/another.GetB()) < Precision) &&
                (abs(c/b - another.GetC()/another.GetB()) < Precision));
    }
    else
        return (abs(c/a - another.GetC()/another.GetA()) < Precision);
}

Line perpendicular(const Line& line, const Point& point)
{
    double a = line.GetA(), b = line.GetB();

    return Line(-b, a, b*point.x - a*point.y);
}

Point intersection(const Line& line, const Line& another)
{
    double a1 = line.GetA(), b1 = line.GetB(), c1 = line.GetC();
    double a2 = another.GetA(), b2 = another.GetB(), c2 = another.GetC();
    Point point;
    if (abs(b1) < Precision)
    {
        point = Point(-c1/a1, -c2/b2 + c1*a2/(a1*b2));
    }
    else
    {
        if (abs(a1) < Precision)
        {
            point = Point( -c2/a2 + c1*b2/(b1*a2), -c1/b1);
        }
        else
        {
            double x = (c1*b2 - b1*c2) / (b1*a2 - a1*b2);
            point = Point(x, -a1*x/b1 - c1/b1);
        }
    }
    return point;
}
///////////////////////////////
double Length(Point a, Point b)
{
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}

void RotatePoint(Point &point, Point center, double angle)
{
    angle = M_PI*angle/180;
    double xc = point.x - center.x;
    double yc = point.y - center.y;
    double length = Length(point, center);
    double oldAngle = atan (yc/xc);

    if(xc < 0)
    {
        oldAngle += M_PI;
    }
    xc = length*cos(oldAngle + angle);
    yc = length*sin(oldAngle + angle);

    point.x = xc + center.x;
    point.y = yc + center.y;
}

void ReflexPoint(Point &point, Point center)
{
    point.x += 2*(center.x - point.x);
    point.y += 2*(center.y - point.y);
}

void ReflexPoint(Point &point, Line axis)
{
    Line line = perpendicular(axis, point);
    Point center = intersection(axis, line);

    ReflexPoint(point, center);
}

void ScalePoint(Point &point, Point center, double coefficient)
{
    point.x = coefficient * (point.x - center.x) + center.x;
    point.y = coefficient * (point.y - center.y) + center.y;
}
///////////////////////////////
class Shape
{
public:
    virtual double perimeter() const = 0;
    virtual double area() const = 0;

    virtual bool operator==(const Shape& another) const = 0;
    virtual bool operator!=(const Shape& another) const = 0;
    virtual bool isCongruentTo(const Shape& another) const = 0;
    virtual bool isSimilarTo(const Shape& another) const = 0;
    virtual bool containsPoint(Point point) const = 0;

    virtual void rotate (Point center, double angle) = 0;
    virtual void reflex (Point center) = 0;
    virtual void reflex (Line axis) = 0;
    virtual void scale (Point center, double coefficient) = 0;

    virtual ~Shape() = default;
};
////////////////////////////
class Ellipse : public Shape
{
public:
    explicit Ellipse(Point focusA, Point focusB, double sum);
    ~Ellipse() = default;

    pair<Point, Point> focuses() const;
    pair<Line, Line> directrices() const;
    double eccentricity() const;
    Point center() const;

    double perimeter() const override;
    double area() const override;

    bool operator==(const Shape& another) const override;
    bool operator!=(const Shape& another) const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(Point point) const override;

    void rotate(Point center, double angle) override;
    void reflex(Point center) override;
    void reflex(Line axis) override;
    void scale(Point center, double coefficient) override;

protected:
    Point focus1, focus2;
    double SemiMajAxis = 0,
           SemiMinAxis = 0,
           FocalLength = 0;
};

Ellipse::Ellipse(Point focusA, Point focusB, double sum): focus1(focusA), focus2(focusB), SemiMajAxis(sum/2)
{
    double x1 = focusA.x,
           y1 = focusA.y,
           x2 = focusB.x,
           y2 = focusB.y;
    FocalLength = sqrt(pow((x1 - x2), 2) + pow((y1 - y2),2));
    SemiMinAxis = sqrt(pow(SemiMajAxis, 2) - pow(FocalLength/2, 2));
}

class Circle : public Ellipse
{
public:
    explicit Circle(Point center, double radius) : Ellipse (center, center, 2*radius)
    {
        Center = center;
        Radius = radius;
    }
    double radius() const
    {
        return Radius;
    }
private:
    Point Center;
    double Radius = 0;
};
////////////////////////////
class Polygon : public Shape
{
public:
    Polygon(){}
    Polygon(const Polygon& another)
    {
        points = another.points;
    }
    explicit Polygon(const vector<Point> &Points) : points(Points)  {}
    explicit Polygon(const initializer_list<Point> &list);
    ~Polygon() = default;

    size_t verticesCount() const
    {
        return points.size();
    }
    const vector<Point>& getVertices() const
    {
        return points;
    }
    bool isConvex() const;

    double perimeter() const override;
    double area() const override;

    bool operator==(const Shape& another) const override;
    bool operator!=(const Shape& another) const override;
    bool isCongruentTo(const Shape& another) const override;
    bool isSimilarTo(const Shape& another) const override;
    bool containsPoint(Point point) const override;

    void rotate(Point center, double angle) override;
    void reflex(Point center) override;
    void reflex(Line axis) override;
    void scale(Point center, double coefficient) override;

protected:
    vector <Point> points;
};

Polygon::Polygon(const initializer_list<Point> &list)
{
    for (Point elements : list)
    {
        points.push_back(elements);
    }
}

class Rectangle : public Polygon
{
public:
    explicit Rectangle(Point pointA, Point pointC, double ratioOfSides);
    Point center() const;
    pair<Line, Line> diagonals() const;

    double perimeter() const override
    {
        return 2*Length(points[0], points[1]) + 2*Length(points[1], points[2]);
    }
    double area() const override
    {
        return Length(points[0], points[1])*Length(points[1], points[2]);
    }
};

Rectangle::Rectangle(Point pointA, Point pointC, double ratioOfSides)
{
    Point Center((pointA.x + pointC.x)/2, (pointA.y + pointC.y)/2);
    if (ratioOfSides == 1)
    {
        points = {pointA, Point(Center.x - (pointC.y - Center.y), Center.y + (pointC.x - Center.x)),
                    pointC, Point(Center.x + (Center.y - pointA.y), Center.y - (Center.x - pointA.x))};
    }
    else
    {
        double ratio = max(ratioOfSides, 1/ratioOfSides);
        double angle = atan(ratio);
        Point D = pointA;
        Point B = pointC;
        RotatePoint(D, Center, 2*angle*180/M_PI);
        RotatePoint(B, Center, 2*angle*180/M_PI);
        points = {pointA, B, pointC, D};
    }
}

class Square : public Rectangle
{
public:
    explicit Square(Point pointA, Point pointB) : Rectangle(pointA, pointB, 1) {}

    double perimeter() const override
    {
        return 4*Length(points[0], points[1]);
    }

    bool isSimilarTo(const Square&)
    {
        return true;
    }
    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;
};
///////////////////////////////
class Triangle : public Polygon
{
public:
    explicit Triangle(Point a, Point b, Point c);

    Circle circumscribedCircle() const;
    Circle inscribedCircle() const;
    Point centroid() const;

    Point orthocenter() const;
    Line EulerLine() const;
    Circle ninePointsCircle() const;
private:
    double sideOne, sideTwo, sideThree;
};

Triangle::Triangle(Point a, Point b, Point c) : Polygon{a, b, c}
{
    sideOne = Length(a, b);
    sideTwo = Length(c, b);
    sideThree = Length(a, c);
}
///////////////////////////////////////////
pair<Point, Point> Ellipse::focuses() const
{
    pair<Point, Point> Focuses = {focus1, focus2};
    return Focuses;
}

double Ellipse::eccentricity() const
{
    return FocalLength / (2*SemiMajAxis);
}

Point Ellipse::center() const
{
    Point Center ((focus1.x + focus2.x)/2, (focus1.y + focus2.y)/2);
    return Center;
}

pair<Line, Line> Ellipse::directrices() const
{
    Line axis (focus1, focus2);
    Point Center = center();
    double x1 = pow(SemiMajAxis/FocalLength, 2)*(focus1.x - Center.x) + Center.x,
           x2 = pow(SemiMajAxis/FocalLength, 2)*(focus2.x - Center.x) + Center.x,
           y1 = pow(SemiMajAxis/FocalLength, 2)*(focus1.y - Center.y) + Center.y,
           y2 = pow(SemiMajAxis/FocalLength, 2)*(focus2.y - Center.y) + Center.y;
    Line directrice1 = perpendicular(axis, Point(x1, y1));
    Line directrice2 = perpendicular(axis, Point(x2, y2));

    pair<Line, Line> Directrices = {directrice1, directrice2};
    return Directrices;
}

double Ellipse::perimeter() const
{
    double Perimeter = M_PI*(3*(SemiMajAxis + SemiMinAxis) - sqrt((3*SemiMajAxis + SemiMinAxis)*(3*SemiMinAxis + SemiMajAxis)));
    return Perimeter;
}

double Ellipse::area() const
{
    return M_PI*SemiMajAxis*SemiMinAxis;
}

bool Ellipse::operator==(const Shape& another) const
{
    try
    {
        const Ellipse &Another = dynamic_cast<const Ellipse &>(another);
        if ((((Another.focus1 == focus1) && (Another.focus2 == focus2))
              || ((Another.focus2 == focus1) && (Another.focus2 == focus2))) &&
             (abs(Another.SemiMajAxis - SemiMajAxis) < Precision))
            return true;
        else
            return false;
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Ellipse::operator!=(const Shape& another) const
{
    return !(*this == another);
}

bool Ellipse::isCongruentTo(const Shape& another) const
{
    try
    {
        const Ellipse &Another = dynamic_cast<const Ellipse &>(another);
        if (abs(Another.SemiMinAxis - SemiMinAxis) < Precision &&
             abs(Another.SemiMajAxis - SemiMajAxis) < Precision)
            return true;
        else
            return false;
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Ellipse::isSimilarTo(const Shape& another) const
{
    try
    {
        const Ellipse &Another = dynamic_cast<const Ellipse &>(another);
        if (abs((Another.SemiMinAxis / SemiMinAxis) -
            (Another.SemiMajAxis / SemiMajAxis)) < Precision)
            return true;
        else
            return false;
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Ellipse::containsPoint(Point point) const
{
    double length = Length(point, focus1) + Length(point, focus2);
    if (length <= 2*SemiMajAxis + Precision)
    {
        return true;
    }
    return false;
}

void Ellipse::rotate(Point center, double angle)
{
    RotatePoint(focus1, center, angle);
    RotatePoint(focus2, center, angle);
}

void Ellipse::reflex(Point center)
{
    ReflexPoint(focus1, center);
    ReflexPoint(focus2, center);
}

void Ellipse::reflex(Line axis)
{
    ReflexPoint(focus1, axis);
    ReflexPoint(focus2, axis);
}

void Ellipse::scale(Point center, double coefficient)
{
    ScalePoint(focus1, center, coefficient);
    ScalePoint(focus2, center, coefficient);
    SemiMajAxis = SemiMajAxis*Length(focus1, focus2)/FocalLength;
    SemiMinAxis = SemiMinAxis*Length(focus1, focus2)/FocalLength;
    FocalLength = Length(focus1, focus2);
}
//////////////////////////////
double Polygon::perimeter() const
{
    double result = 0.0;
    for (size_t i = 0; i < points.size(); ++i)
    {
        result += Length(points[i], points[(i + 1) % points.size()]);
    }
    return result;
}

double Polygon::area() const
{
    double sum = 0.0;
    for (size_t i = 0; i < points.size(); ++i)
    {
        sum += 0.5*(points[(i + 1) % points.size()].x - points[i].x)*
                            (points[(i + 1) % points.size()].y + points[i].y);
    }
    return abs(sum);
}

bool Polygon::operator==(const Shape& another) const
{
    try
    {
        const Polygon& Another = dynamic_cast<const Polygon&>(another);

        if (points.size() == Another.points.size())
        {
            size_t length = points.size();
            size_t shift = 0;
            while (shift < length && points[shift] != Another.points[0])
            {
                ++shift;
            }
            if (points[shift] == Another.points[0])
            {
                bool result1 = true;
                bool result2 = true;
                for (size_t i = 0; i < length && result1; ++i)
                {
                    result1 = points[(shift + i) % length] == Another.points[i];
                }
                for (size_t i = 0; i < length && result2; ++i)
                {
                    result2 = points[(length + shift - i) % length] == Another.points[i];
                }
                return (result1||result2);
            }
        }
        return false;
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Polygon::operator!=(const Shape& another) const
{
    return !(*this == another);
}

bool Polygon::isSimilarTo(const Shape& another) const
{
    try
    {
        const Polygon& Another = dynamic_cast<const Polygon&>(another);

        if (points.size() == Another.points.size())
        {
            double k = Another.perimeter()/(*this).perimeter();
            size_t length = points.size();
            for (size_t shift = 0; shift < length; ++shift)
            {
                while (shift < length &&
                      (abs(k*Length(points[shift], points[(shift + 1)%length]) - Length(Another.points[0], Another.points[1])) > Precision))
                {
                    ++shift;
                }
                bool result1 = false;
                bool result2 = false;
                if (shift < length)
                {
                    result1 = true;
                    result2 = true;
                    for (size_t i = 0; (i < length) && result1; ++i)
                    {
                        result1 = (abs(k*Length(points[(shift + i) % length], points[(shift + i + 1) % length]) -
                                    Length(Another.points[i], Another.points[(i + 1) % length])) < Precision );
                    }
                    for (size_t i = 0; (i < length) && result2; ++i)
                    {
                        result2 = (abs(k*Length(points[(length + shift - i) % length], points[(length + shift - i + 1) % length]) -
                                    Length(Another.points[i], Another.points[(i + 1) % length])) < Precision);
                    }
                }
                return (result1||result2);
            }
        }
        return false;
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Polygon::isCongruentTo(const Shape& another) const
{
    try
    {
        const Polygon& Another = dynamic_cast<const Polygon&>(another);
        return (this -> isSimilarTo(another) && (abs(perimeter() - Another.perimeter())) < Precision*Another.points.size());
    }
    catch (const bad_cast& other)
    {
        return false;
    }
}

bool Polygon::containsPoint(const Point point) const
{
    for (size_t i = 0; i < points.size(); ++i)
        if (points[i] == point)
            return true;

    double sumAngle = 0;
    for (size_t i = 0; i < points.size(); ++i)
    {
        Vector v1(points[i], point), v2(points[(i + 1)%points.size()], point);
        double angle1 = atan2(v1.y, v1.x);
		double angle2 = atan2(v2.y, v2.x);
		double angle = abs(angle1 - angle2);
		angle = min(2*M_PI - angle, angle);

        if (v1.x*v2.y - v2.x*v1.y < 0)
            angle *= -1;
        sumAngle += angle;
    }
    return (abs(sumAngle) > Precision);
}

bool Polygon::isConvex() const
{
    size_t length = points.size();
    bool result1 = true;
    bool result2 = true;
    for (size_t i = 0; i < length && result1; ++i)
    {
        Vector v1(points[(i + 1) % length], points[i % length]),
               v2(points[(i + 2) % length], points[(i + 1) % length]);
        result1 = ((v1.x*v2.y - v2.x*v1.y) > 0);
    }
    for (size_t i = 0; i < length && result2; ++i)
    {
        Vector v1(points[(i + 1) % length], points[i % length]),
               v2(points[(i + 2) % length], points[(i + 1) % length]);
        result2 = ((v1.x*v2.y - v2.x*v1.y) < 0);
    }
    return (result1 || result2);
}

void Polygon::rotate(const Point center, const double angle)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        RotatePoint(points[i], center, angle);
    }
}

void Polygon::reflex(const Point center)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        ReflexPoint(points[i], center);
    }
}

void Polygon::reflex(const Line axis)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        ReflexPoint(points[i], axis);
    }
}

void Polygon::scale(Point center, double coefficient)
{
    for (size_t i = 0; i < points.size(); ++i)
    {
        ScalePoint(points[i], center, coefficient);
    }
}
///////////////////////////////
Point Rectangle::center() const
{
    return Point((points[0].x + points[2].x)/2, (points[0].y + points[2].y)/2);
}

pair<Line, Line> Rectangle::diagonals() const
{
    Line d1(points[0], points[2]);
    Line d2(points[1], points[3]);
    pair<Line, Line> Diagonals(d1, d2);
    return Diagonals;
}
//////////////////////////////////////////
Circle Square::circumscribedCircle() const
{
    Circle circle(Point((points[0].x + points[2].x)/2, (points[0].y + points[2].y)/2), Length(points[0], points[2])/2);
    return circle;
}

Circle Square::inscribedCircle() const
{
    double xCenter = (points[0].x + points[2].x)/2;
    double yCenter = (points[0].y + points[2].y)/2;
    double radius = Length(points[0], points[2])/(2*sqrt(2));
    Circle circle(Point(xCenter, yCenter), radius);
    return circle;
}
///////////////////////////////////////////
Circle Triangle::circumscribedCircle() const
{
    Point middleAB = Point((points[0].x + points[1].x)/2, (points[0].y + points[1].y)/2);
    Point middleBC = Point((points[1].x + points[2].x)/2, (points[1].y + points[2].y)/2);

    Line perpendicularAB = perpendicular(Line(points[0], points[1]), middleAB);
    Line perpendicularBC = perpendicular(Line(points[1], points[2]), middleBC);
    Point center = intersection(perpendicularAB, perpendicularBC);

    return Circle(center, Length(points[0], center));
}

Circle Triangle::inscribedCircle() const
{
    Line bis1(points[0], Point((sideThree*points[1].x + sideOne*points[2].x)/(sideOne + sideThree),
                           (sideThree*points[1].y + sideOne*points[2].y)/(sideOne + sideThree)));
    Line bis2(points[1], Point((sideTwo*points[0].x + sideOne*points[2].x)/(sideOne + sideTwo),
                           (sideTwo*points[0].y + sideOne*points[2].y)/(sideOne + sideTwo)));

    Point center = intersection(bis1, bis2);
    double radius = 2*area() / perimeter();
    return Circle(center, radius);
}
Point Triangle::centroid() const
{
    return Point((points[0].x + points[1].x + points[2].x) / 3,
                 (points[1].y + points[2].y + points[2].y) / 3);
}

Line Triangle::EulerLine() const
{
    return Line(ninePointsCircle().center(), centroid());
}

Circle Triangle::ninePointsCircle() const
{
    Point middleAB = Point((points[0].x + points[1].x)/2, (points[0].y + points[1].y)/2);
    Point middleBC = Point((points[1].x + points[2].x)/2, (points[1].y + points[2].y)/2);
    Point middleCA = Point((points[2].x + points[0].x)/2, (points[2].y + points[0].y)/2);
    return Triangle(middleAB, middleBC, middleCA).circumscribedCircle();
}

Point Triangle::orthocenter() const
{
    Line first = perpendicular(Line(points[0], points[1]), points[2]);
    Line second = perpendicular(Line(points[1], points[2]), (points[0]));
    return intersection(first, second);
}

