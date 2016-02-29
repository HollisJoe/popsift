/**
 * Copyright (c) 2012, Fixstars Corp.
 * All rights reserved.
 * 
 * See license at end of file.
 */

/**
 * @file   SIFT.hpp
 * @author Yuri Ardila <y_ardila@fixstars.com>
 * @date   Tue Oct 30 16:01:19 JST 2012
 * 
 * @brief  
 *    SIFT
 *    OpenCL Implementation
 *
 */

#pragma once

#include <vector>
#include <iostream>
#include <assert.h>

#include <cuda_runtime.h>

/* include utilities */
#include "keep_time.h"
#include "c_util_img.h"
#include "plane_2d.h"
#include "debug_macros.h"
#include "s_image.h"
#include "s_pyramid.h"

/* user parameters */
extern int verbose;

class PopSift
{
public:
    PopSift( int   octaves,
             int   levels,
             int   upscale,
             float threshold,
             float edgeThreshold,
             float sigma );

    ~PopSift();

	/**************************/
	/* @brief SIFT executions */
	/**************************/
public:
    void init( int w, int h );

    void execute( imgStream _inp );
    popart::Pyramid & pyramid() { return *_pyramid; };
    void log_to_file();
    void uninit( );

private:
    size_t           _upscaled_width;  // popart in use
    size_t           _upscaled_height; // popart in use
    popart::Image*   _baseImg;         // popart in use
    popart::Pyramid* _pyramid;         // popart in use

    cudaTextureObject_t _texture; // for upscale v5
    cudaTextureDesc     _texDesc; // for upscale v5
    cudaResourceDesc    _resDesc; // for upscale v5

    int              _octaves;         /* number of octaves */
    const int        _scales;          /* number of levels */
    const int        up;               /* upsampling times */
    const float      _sigma;           /* initial sigma */
    const float      _threshold;       /* DoG threshold */
    const float      _edgeLimit;       /* edge threshold */

    popart::Plane2D_uint8 _hst_input_image;
    popart::Plane2D_uint8 _dev_input_image;
    cudaStream_t          _stream;
    popart::KeepTime*     _initTime;
    popart::KeepTime*     _uploadTime;
    popart::KeepTime*     _pyramidTime;
    popart::KeepTime*     _extremaTime;
};

/**
 * Copyright (c) 2012, Fixstars Corp.
 * All rights reserved.
 * 
 * The following patent has been issued for methods embodied in this 
 * software: "Method and apparatus for identifying scale invariant features 
 * in an image and use of same for locating an object in an image," David 
 * G. Lowe, US Patent 6,711,293 (March 23, 2004). Provisional application 
 * filed March 8, 1999. Asignee: The University of British Columbia. For 
 * further details, contact David Lowe (lowe@cs.ubc.ca) or the 
 * University-Industry Liaison Office of the University of British 
 * Columbia.
 * 
 * Note that restrictions imposed by this patent (and possibly others) 
 * exist independently of and may be in conflict with the freedoms granted 
 * in this license, which refers to copyright of the program, not patents 
 * for any methods that it implements.  Both copyright and patent law must 
 * be obeyed to legally use and redistribute this program and it is not the 
 * purpose of this license to induce you to infringe any patents or other 
 * property right claims or to contest validity of any such claims.  If you 
 * redistribute or use the program, then this license merely protects you 
 * from committing copyright infringement.  It does not protect you from 
 * committing patent infringement.  So, before you do anything with this 
 * program, make sure that you have permission to do so not merely in terms 
 * of copyright, but also in terms of patent law.
 * 
 * Please note that this license is not to be understood as a guarantee 
 * either.  If you use the program according to this license, but in 
 * conflict with patent law, it does not mean that the licensor will refund 
 * you for any losses that you incur if you are sued for your patent 
 * infringement.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are 
 * met:
 *     * Redistributions of source code must retain the above copyright and 
 *       patent notices, this list of conditions and the following 
 *       disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Fixstars Corp. nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

