/*
 * NXLock.h
 *
 *  Created on: 2015-9-16
 *      Author: yangyk
 */

#pragma once

#include <semaphore.h>

class NXLock
{
private:
	sem_t _semaphore ;
public:
	NXLock();
	virtual ~NXLock();
public:
	void lock() ;
	void unlock();
	bool trylock();
};

