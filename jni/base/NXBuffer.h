/*
 * NXBuffer.h
 *
 *  Created on: 2015-9-16
 *      Author: yangyk
 */

#pragma once

#include "NXFmt.h"
#include "NXDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <android/log.h>
#define TAG "NXBuffer"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)

typedef enum tag_NXBufferType {
	NXBufferError  = 0, //invalid type
	NXBufferData   = 1, //binary type
	NXBufferAudio  = 2, //audio buffer
	NXBufferVideo  = 3, //video buffer
	NXBufferCanvas = 4, //render target buffer
} NXBufferType;


class NXBuffer : public NXObject
{
private:
	NXBufferType		_bufferType 	;
	NXSDKPixelFormat	_bufferPixel    ;
	uint32_t 			_bufferCont 	;
	uint8_t*			_bufferArry[4]  ;
	uint32_t 			_bufferLeng[4]  ;

private:
	NXBuffer();

public:
	//NXBufferData
	NXBuffer(uint32_t bytes);

	NXBuffer(NXBufferType type,uint32_t bytes);

	//NXBufferAudio
	NXBuffer(uint32_t sampleCount, uint32_t sampleBits, uint8_t channels);

	//NXBufferVideo
	NXBuffer(uint32_t width, uint32_t height, NXSDKPixelFormat pixfmt);

	//NXBufferCanvas
	NXBuffer(uint32_t width, uint32_t height);

	//Destructor
	virtual ~NXBuffer();

public:
	//copy
	NXBuffer* copy() ;

	//buffer type
	NXBufferType getBufferType() ;

	//pixel format
	NXSDKPixelFormat getPixelFormat();

	//lock this buffer
	bool lockBuffer();

	//unlock this buffer
	void unlockBuffer();

	//plane count
	uint32_t getPlaneCount() ;

	//plane length
	uint32_t getPlaneLength(uint32_t index) ;

	//plane pointer
	uint8_t* getPlanePtr(uint32_t index) ;
};

