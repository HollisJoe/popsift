/*
 * Copyright 2016, Simula Research Laboratory
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "sift_constants.h"
#include "sift_conf.h"

namespace popsift {

struct GaussInfo;

template<int LEVELS>
struct GaussTable
{
    float filter[ LEVELS * GAUSS_ALIGN ];

    /* The sigma used to generate the Gauss table for each level.
     * Meaning these are the differences between sigma0 and sigmaN.
     */
    float sigma [ LEVELS ];

    /* The span of the table that is generated for each level.
     */
    int   span  [ LEVELS ];

    __host__
    void clearTables( );

    __host__
    void computeBlurTable( const GaussInfo* info );

    __host__
    void transformBlurTable( const GaussInfo* info );
};

struct GaussInfo
{
    int required_filter_stages;

    /* These are the 1D Gauss tables for all levels of an octave.
     * The first row is special:
     * - in octave 0 if initial blur is non-zero, contains the
     *   remaining blur that is required to reach sigma0
     * - in octave 0 if initial blur is zero, contains the
     *   filter for sigma0
     * - in all other octaves, row 0 is unused
     */
    GaussTable<GAUSS_LEVELS> inc;

    GaussTable<GAUSS_LEVELS> inc_relative;

    /* Compute the 1D Gauss tables for all levels of octave 0.
     * For octave 0, all of these tables derive from the input
     * image.
     */
    GaussTable<GAUSS_LEVELS> abs_o0;

    /* Compute the 1D Gauss tables for all levels of octaves 1 and up.
     * Level 0 is empty, since it is created by other means.
     * All other levels blur from level 0, not considering any
     * initial blur.
     */
    GaussTable<GAUSS_LEVELS> abs_oN;

    /* In theory, level 0 of octave 2 contains the same information
     * whether it is constructed by downscaling and blurring the
     * input image with sigma or by blurring the input image with 2*sigma
     * and downscaling afterwards.
     */
    GaussTable<MAX_OCTAVES> dd;

    /* For a Gauss table with an arbitrary sigma (< 2) that the user may
     * want to apply to an input image.
     * Requires a separate initialization call.
     * Note that the user Gauss table works only for the "relative" kernels
     * that user the hardware interpolation kernel.
     */
    GaussTable<1> user;

    __host__
    void clearTables( );

    __host__
    void computeAbsBlurTable_o0( int level, int span, float sigma );

    __host__
    void computeAbsBlurTable_oN( int level, int span, float sigma );

public:
    __host__
    void setSpanMode( Config::GaussMode m );

    __host__
    int getSpan( float sigma ) const;

private:
    Config::GaussMode _span_mode;

    __host__
    static int vlFeatSpan( float sigma );

    __host__
    static int vlFeatRelativeSpan( float sigma );

    __host__
    static int openCVSpan( float sigma );
};

extern __device__ __constant__ GaussInfo d_gauss;
extern                         GaussInfo h_gauss;

/* init_filter must be called early to initialize the Gauss tables.
 */
void init_filter( const Config& conf,
                  float         sigma0,
                  int           levels );

/* pre_init_user_filter is only needed if the separate "user" Gauss
 * filter is requried.
 * If it is needed, pre_init_user_filter must be called before
 * init_filter.
 */
void pre_init_user_filter( const Config& conf,
                           float         sigma );

} // namespace popsift

