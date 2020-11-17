#pragma once
#include "glwidget_global.h"
#include "types.h"
namespace vrt {
	class PaintInfomation;
class Primitive : public OPENGLCLASS
{
public:
	Primitive();
	virtual ~Primitive();
	virtual void initialize() = 0;
	virtual void paint(PaintInfomation* info) = 0;
	virtual vrt::Bounds3f getBound() = 0;
	unsigned int id() const { return id_; }
	bool operator<(const Primitive& rhs) {
		return this->id_ < rhs.id_;
	}

private:
	unsigned int id_;
	static unsigned int globalId;
};

inline Primitive::Primitive()
	:id_(globalId++)
{
}

inline Primitive::~Primitive()
{
}

class GeometryPrimitive : public Primitive
{
public:
	void setColor(ColorType color) { color_ = color; };
	virtual ColorType color() { return color_; };
private:
	ColorType color_;
};
}
