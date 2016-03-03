#pragma once

#include "s_image.h"
#include "keep_time.h"
#include <vector>

#ifndef INF
#define INF               (1<<29)
#endif
#ifndef NINF
#define NINF              (-INF)
#endif
#ifndef M_PI
#define M_PI  3.1415926535897932384626433832F
#endif
#ifndef M_PI2
#define M_PI2 (2.0F * M_PI)
#endif

#define GAUSS_ONE_SIDE_RANGE 12
#define GAUSS_SPAN           (2*GAUSS_ONE_SIDE_RANGE+1)

#define USE_DOG_ARRAY

namespace popart {

extern __device__ __constant__ float d_sigma0;
extern __device__ __constant__ float d_sigma_k;

struct ExtremaMgmt
{
    uint32_t counter;
    uint32_t max1;     // initial max
    uint32_t max2;     // max after finding alternative angles
                       // Lowe says it happens to 15%, I reserve floor(25%)
    void init( uint32_t m1 ) {
        counter = 0;
        max1    = m1;
        max2    = m1 + m1/4;
    }
    ExtremaMgmt( ) { }

    ExtremaMgmt( uint32_t m1 ) {
        counter = 0;
        max1    = m1;
        max2    = m1 + m1/4;
    }
};

struct ExtremumCandidate
{
    float    xpos;
    float    ypos;
    float    sigma; // scale;
    float    angle_from_bemap;
};

struct Descriptor
{
    float features[128];
};

class Pyramid
{
public:
    class Octave
    {
        uint32_t  _debug_octave_id;
        uint32_t  _levels;

        Plane2D_float* _data;
        Plane2D_float  _intermediate_data;

        cudaArray_t           _dog_3d;
        cudaChannelFormatDesc _dog_3d_desc;
        cudaExtent            _dog_3d_ext;

        cudaSurfaceObject_t   _dog_3d_surf;

        cudaTextureObject_t   _dog_3d_tex;


    public:
        cudaTextureObject_t* _data_tex;
        cudaTextureObject_t  _interm_data_tex;

    private:
        /* It seems strange strange to collect extrema globally only.
         * Because of the global cut-off, features from the later
         * octave have a smaller chance of being accepted.
         * Besides, the management of computing gradiants and atans
         * must be handled per scale (level) of an octave.
         * There: one set of extrema per octave and level.
         */
        ExtremaMgmt*         _h_extrema_mgmt; // host side info
        ExtremaMgmt*         _d_extrema_mgmt; // device side info
        ExtremumCandidate**  _d_extrema;
        Descriptor**         _d_desc;
        Descriptor**         _h_desc;

    public:
        Octave( );
        ~Octave( ) { this->free(); }

        inline void debugSetOctave( uint32_t o ) { _debug_octave_id = o; }

        inline int getLevels() const { return _levels; }
        inline int getWidth() const  { return _data[0].getWidth(); }
        inline int getHeight() const { return _data[0].getHeight(); }

        inline Plane2D_float& getData( uint32_t level ) {
            return _data[level];
        }
        inline Plane2D_float& getIntermediateData( ) {
            return _intermediate_data;
        }
        
        inline cudaSurfaceObject_t& getDogSurface( ) {
            return _dog_3d_surf;
        }
        inline cudaTextureObject_t& getDogTexture( ) {
            return _dog_3d_tex;
        }

        inline uint32_t getFloatSizeData() const {
            return _data[0].getByteSize() / sizeof(float);
        }
        inline uint32_t getByteSizeData() const {
            return _data[0].getByteSize();
        }
        inline uint32_t getByteSizePitch() const {
            return _data[0].getPitch();
        }

        inline ExtremaMgmt* getExtremaMgmtH( uint32_t level ) {
            return &_h_extrema_mgmt[level];
        }

        inline ExtremaMgmt* getExtremaMgmtD( ) {
            return _d_extrema_mgmt;
        }

        inline ExtremumCandidate* getExtrema( uint32_t level ) {
            return _d_extrema[level];
        }
    
        void resetExtremaCount( );
        void readExtremaCount( );
        uint32_t getExtremaCount( ) const;
        uint32_t getExtremaCount( uint32_t level ) const;

        void        allocDescriptors( );
        Descriptor* getDescriptors( uint32_t level );
        void        downloadDescriptor( );
        void        writeDescriptor( ostream& ostr );

        void downloadToVector(uint32_t level, std::vector<ExtremumCandidate> &, std::vector<Descriptor> &);

        /**
         * alloc() - allocates all GPU memories for one octave
         * @param width in floats, not bytes!!!
         */
        void alloc( uint32_t width, uint32_t height, uint32_t levels, uint32_t layer_max_extrema );
        void free();

        /**
         * debug:
         * download a level and write to disk
         */
        void download_and_save_array( const char* basename, uint32_t octave, uint32_t level );

    private:
        void allocExtrema( uint32_t layer_max_extrema );
        void freeExtrema( );
    };
private:
    uint32_t     _num_octaves;
    uint32_t     _levels;
    Octave*      _octaves;

public:
    Pyramid( Image* base, uint32_t octaves, uint32_t levels );
    ~Pyramid( );

    static void init_filter( float sigma, uint32_t level );
    static void init_sigma(  float sigma, uint32_t level );
    void build( Image* base );

    void find_extrema( float edgeLimit, float threshold );

    void download_and_save_array( const char* basename, uint32_t octave, uint32_t level );

    void download_and_save_descriptors( const char* basename, uint32_t octave );

    void report_times();

    inline Octave & octave(size_t n){ return _octaves[n]; }

private:
    void build_v7  ( Image* base );
    void build_v8  ( Image* base );
    void build_v11 ( Image* base );
    void build_v12 ( Image* base );

    void reset_extremum_counter( );
    void find_extrema_v4( float edgeLimit, float threshold );
    void find_extrema_v5( float edgeLimit, float threshold );
    void find_extrema_v6( float edgeLimit, float threshold );

    template<int HEIGHT>
    void find_extrema_v4_sub( float edgeLimit, float threshold );
    template<int HEIGHT>
    void find_extrema_v5_sub( float edgeLimit, float threshold );
    template<int HEIGHT>
    void find_extrema_v6_sub( float edgeLimit, float threshold );


    void orientation_v1( );
    void orientation_v2( );

    void descriptors_v1( );

    void test_last_error( int line );
    void debug_out_floats  ( float* data, uint32_t pitch, uint32_t height );
    void debug_out_floats_t( float* data, uint32_t pitch, uint32_t height );

    KeepTime _keep_time_extrema_v4;
    KeepTime _keep_time_extrema_v5;
    KeepTime _keep_time_extrema_v6;

    KeepTime _keep_time_orient_v1;
    KeepTime _keep_time_orient_v2;

    KeepTime _keep_time_descr_v1;
};

} // namespace popart
