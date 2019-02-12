#pragma once


#include"stdafx.h"

#ifdef MyShapeAPI

#else
#define MyShapeAPI extern "C" _declspec(dllimport)
#endif


typedef struct shape {
	virtual void draw() = 0;
	virtual ~shape() {}
}*lp_shape;


typedef struct rectange *lp_rectange;
typedef struct triangle *lp_triangle;
typedef struct square *lp_square;
typedef struct shape_factory *lp_shape_factory;


typedef struct shape_factory_base {
	virtual lp_shape make_rect() = 0;
	virtual lp_shape make_squa() = 0;
	virtual lp_shape make_tria() = 0;
	virtual ~shape_factory_base() {}
} *lp_shape_factory_base;

MyShapeAPI lp_shape_factory_base get_shape_factory();





