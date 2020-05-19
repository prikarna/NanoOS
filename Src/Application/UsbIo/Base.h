/*
 * File    : Base.h
 * Remark  : Base class of a class that need to override 'new' and 'delete' operator. 
 *
 */

#pragma once

class Base
{
public:
	void operator delete(void *) {};
};
