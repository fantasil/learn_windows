﻿// dll_test.cpp: 定义控制台应用程序的入口点。
//
#include"stdafx.h"
#include"dll_1.h"

#pragma comment(lib,"dll_1")




int main()
{
	using namespace std;
	
	using lp_shape_s = std::unique_ptr<shape>;
	using lp_factory_s = std::unique_ptr<shape_factory_base>;

	lp_factory_s factory{ get_shape_factory() };
	lp_shape_s rect{ factory->make_rect() };
	lp_shape_s squa{ factory->make_squa() };
	lp_shape_s tria{ factory->make_tria() };

	rect->draw();
	squa->draw();
	tria->draw();
	system("pause");
	return 0;
}

/*
关于dll导出c++类:
	1:在头文件(.h)中声明定义接口,声明需要导出的类
	2:在源文件(.cpp)中实现需要导出的类,这些类继承自该接口
	3:在需要使用声明类的地方,将这些类对象指针退化到接口指针使用
	4:在使用指针的时候,可以使用智能指针,免去手动delete的操作..
	5:总的来说,就是通过接口操纵指针
*/
