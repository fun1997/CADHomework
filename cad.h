#pragma once
#include <vector>
class Solid;
class Face;
class Loop;
class Halfedge;
class Vertex;
class Edge;
class Vec;
//定义全局变量FACE,UP用于存储面、点的指针
std::vector<Face*> FACE;
std::vector<Vertex*> UP;
class Solid {
public:
	Face* sfaces;
	Edge* sedges;
};
class Face {
public:
	Solid* fsolid;
	Loop* floops;
	Loop* finloops=nullptr;
};
class Loop {
public:
	Face* lface;
	Loop* nextl = nullptr;
	Halfedge* ledg;
};

class Halfedge {
public:
	Loop* wloop;
	Halfedge* nxt;
	Halfedge* prv;
	Halfedge* bro;
	Edge* edg;
	Vertex* startv;
	Vertex* endv;
};
class Edge {
public:
	Halfedge* he1;
	Halfedge* he2;
};
class Vertex {
public:
	Vertex* nextv;
	Vertex* prevv;
	double x, y, z;
	Vertex(double a = 0, double b = 0, double c = 0) :x(a), y(b), z(c) {}
};
class Vec {
public:
	double x, y, z;
	Vec(double a = 0, double b = 0, double c = 0) :x(a), y(b), z(c) {}
};