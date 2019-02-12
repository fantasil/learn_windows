#include"stdafx.h"

#include<iostream>


#define MyShapeAPI extern "C" _declspec(dllexport)

#include"list.h"





typedef struct rectange : public shape {
	void draw() override { std::cout << "rectange::draw" << std::endl; }
	~rectange() { std::cout << "~rectange" << std::endl; }
} *lp_rectange;

typedef struct triangle : public shape {
	void draw() override { std::cout << "triangle::draw" << std::endl; }
	~triangle() { std::cout << "~triangle" << std::endl; }
} *lp_triangle;


typedef struct square :public shape {
	void draw() override { std::cout << "square::draw" << std::endl; }
	~square() { std::cout << "~square" << std::endl; }
} *lp_square;

typedef struct shape_factory:public shape_factory_base{
	lp_shape make_rect() { return new rectange{}; }
	lp_shape make_squa() { return new square{}; }
	lp_shape make_tria() { return new triangle{}; }
	~shape_factory() { std::cout << "~shape_factory" << std::endl; }
} *lp_shape_factory;

lp_shape_factory_base get_shape_factory()
{
	return new shape_factory{};
}