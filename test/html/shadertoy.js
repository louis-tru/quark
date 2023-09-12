"use strict"

//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piCamera
//
//==============================================================================

function piCamera()
{
    var mMatrix = setIdentity();
    var mMatrixInv = setIdentity();

    var mPosition = [0.0,0.0,0.0];
    var mXRotation = 0.0;
    var mYRotation = 0.0;

    var mPositionTarget = [0.0,0.0,0.0];
    var mXRotationTarget = 0.0;
    var mYRotationTarget = 0.0;

    var me = {};
/*
    me.Set = function( pos, dir, roll) 
             {
                 mMatrix    = setLookat( pos, add(pos,dir), [Math.sin(roll),Math.cos(roll),Math.sin(roll)] );
                 mMatrixInv = invertFast( mMatrix );
             };
*/
    me.SetPos = function(pos)
                {
                    mPosition = pos;
                    //mMatrix[ 3] = -(mMatrix[0] * pos[0] + mMatrix[1] * pos[1] + mMatrix[ 2] * pos[2]);
                    //mMatrix[ 7] = -(mMatrix[4] * pos[0] + mMatrix[5] * pos[1] + mMatrix[ 6] * pos[2]);
                    //mMatrix[11] = -(mMatrix[8] * pos[0] + mMatrix[9] * pos[1] + mMatrix[10] * pos[2]);
                };
/*
    me.GlobalMove = function(pos)
                    {
                        mMatrix[ 3] -= (mMatrix[0] * pos[0] + mMatrix[1] * pos[1] + mMatrix[ 2] * pos[2]);
                        mMatrix[ 7] -= (mMatrix[4] * pos[0] + mMatrix[5] * pos[1] + mMatrix[ 6] * pos[2]);
                        mMatrix[11] -= (mMatrix[8] * pos[0] + mMatrix[9] * pos[1] + mMatrix[10] * pos[2]);
                        mMatrixInv = invertFast(mMatrix);
                    };
*/
    me.LocalMove = function( dis )
                    {
                        dis = matMulvec( setRotationY(-mYRotation), dis);
                        mPositionTarget = sub(mPositionTarget,dis)
                    };

    me.RotateXY = function( x, y)
                  {
                    mXRotationTarget -= x;
                    mYRotationTarget -= y;
                    mXRotationTarget = Math.min( Math.max( mXRotationTarget, -Math.PI/2), Math.PI/2 );
                  };


    me.CameraExectue = function( dt )
    {
        // smooth position
        mXRotation += (mXRotationTarget-mXRotation) * 12.0*dt;
        mYRotation += (mYRotationTarget-mYRotation) * 12.0*dt;
        mPosition = add(mPosition, mul( sub(mPositionTarget, mPosition), 12.0*dt));

        // Make Camera matrix
        mMatrix = matMul( matMul(setRotationX(mXRotation), 
                                 setRotationY(mYRotation)),  
                                 setTranslation(mPosition));
        mMatrixInv = invertFast(mMatrix);

    }

    me.GetMatrix = function() { return mMatrix; };
    me.GetMatrixInverse = function() { return mMatrixInv; };
    me.SetMatrix = function( mat ) 
                    {
                        mMatrix = mat;
                        mMatrixInv = invertFast(mMatrix);

                        mPosition = getXYZ(matMulpoint(mat, [0.0,0.0,0.0]));
                        mPositionTarget = mPosition;
                    };

    me.GetPos = function() { return getXYZ(matMulpoint(mMatrixInv, [0.0,0.0,0.0])); }
    me.GetDir = function() { return getXYZ(normalize(matMulvec(mMatrixInv, [0.0,0.0,-1.0]))); }

    return me;
}
//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piFile
//
//==============================================================================

function piFile( binaryDataArrayBuffer )
{
    // private
    //var mDataView = new DataView( binaryDataArrayBuffer, 0 );
    var mDataView = binaryDataArrayBuffer;
    var mOffset = 0;

    // public members
    var me = {};
    me.mDummy = 0;
/*
    // public functions
    me.Seek = function( off ) { mOffset = off - 3; };
    me.ReadUInt16 = function()  { var res = mDataView.getUint16( mOffset ); mOffset+=2; return res; };
    me.ReadUInt32 = function()  { var res = mDataView.getUint32( mOffset ); mOffset+=4; return res; };
    me.ReadUInt64 = function()  { return me.ReadUInt32() + (me.ReadUInt32()<<32); };
    me.ReadFloat32 = function() { var res = mDataView.getFloat32( mOffset ); mOffset+=4; return res; };
*/

    me.Seek = function( off ) { mOffset = off; };
    me.ReadUInt8  = function()  { var res = (new Uint8Array(mDataView,mOffset))[0]; mOffset+=1; return res; };
    me.ReadUInt16 = function()  { var res = (new Uint16Array(mDataView,mOffset))[0]; mOffset+=2; return res; };
    me.ReadUInt32 = function()  { var res = (new Uint32Array(mDataView,mOffset))[0]; mOffset+=4; return res; };
    me.ReadUInt64 = function()  { return me.ReadUInt32() + (me.ReadUInt32()<<32); };
    me.ReadFloat32 = function() { var res = (new Float32Array(mDataView,mOffset))[0]; mOffset+=4; return res; };
    me.ReadFloat32Array = function(n) 
                          { 
                              var src = new Float32Array(mDataView, mOffset);
                              var res = [];  for( var i=0; i<n; i++ ) { res[i] = src[i]; }
                              mOffset += 4*n;
                              return res;
                          };
    me.ReadFloat32ArrayNative = function(n) { var src = new Float32Array(mDataView, mOffset); mOffset += 4*n; return src; };
    return me;
}
//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piMesh
//
//==============================================================================

function piMesh()
{
    this.mChunks = [];
    this.mPrimitiveType = 0;
    this.mVertexFormat = null;
}

/*
piMesh.prototype.normalize = function( ppos, npos )
{
    var numv = this.mVertexData.length;
    var numt = this.mIndices.length;

    for( var i=0; i<numv; i++ )
    {
        //float *v = (float*)piMesh_GetVertexData( me, i, npos );
        //v[0] = 0.0f;
        //v[1] = 0.0f;
        //v[2] = 0.0f;
        this.mVerts[8 * i + 3] = 0.0;
        this.mVerts[8 * i + 4] = 0.0;
        this.mVerts[8 * i + 5] = 0.0;
    }

    for( var i=0; i<numt; i++ )
    {
        piMeshFace *face = me->mFaceData.mIndexArray[0].mBuffer + i;

        const int ft = face->mNum;
        
        vec3 nor = vec3( 0.0f, 0.0f, 0.0f );
        for( int j=0; j<ft; j++ )
        {
            const vec3 va = *((vec3*)piMesh_GetVertexData( me, face->mIndex[ j      ], ppos ));
            const vec3 vb = *((vec3*)piMesh_GetVertexData( me, face->mIndex[(j+1)%ft], ppos ));

            nor += cross( va, vb );
        }

        for( int j=0; j<ft; j++ )
        {
            vec3 *n = (vec3*)piMesh_GetVertexData( me, face->mIndex[j], npos );
            n->x += nor.x;
            n->y += nor.y;
            n->z += nor.z;
        }
    }

    for( var i=0; i<numv; i++ )
    {
        vec3 *v = (vec3*)piMesh_GetVertexData( me, i, npos );
        *v = normalize( *v );
    }
}
*/

piMesh.prototype.createCube = function(renderer)
{
    this.mPrimitiveType = 0;

    this.mChunks[0] = { mVerts : new Float32Array([ -1.0, -1.0, -1.0,
                                       1.0, -1.0, -1.0,
                                      -1.0,  1.0, -1.0,
                                       1.0,  1.0, -1.0,
                                      -1.0, -1.0,  1.0,
                                       1.0, -1.0,  1.0,
                                      -1.0,  1.0,  1.0,
                                       1.0,  1.0,  1.0 ]),

                        mIndices : new Uint16Array([ 0, 2, 1,   1, 2, 3,   5, 1, 3,   5, 3, 7,   4, 5, 7,   4, 7, 6,   4, 6, 2,   4, 2, 0,   6, 7, 3,   6, 3, 2,   4, 0, 1,   4, 1, 5 ]),
                        mNumVertices : 8,
                        mNumFaces : 12, 
                        mTransform : setIdentity(),
                        mVBO : null, 
                        mIBO : null };

    this.mVertexFormat = { mStride:12, mChannels:[ { mNumComponents:3, mType: renderer.TYPE.FLOAT32, mNormalize: false } ] };

    return true;
}

piMesh.prototype.createCubeSharp = function (renderer)
{
    this.mPrimitiveType = 0;

    this.mChunks[0] = { mVerts : new Float32Array([-1.0, 1.0,-1.0,   0.0, 1.0, 0.0,   0.0, 0.0,
                                    -1.0, 1.0, 1.0,   0.0, 1.0, 0.0,   0.0, 1.0,
                                     1.0, 1.0, 1.0,   0.0, 1.0, 0.0,   1.0, 1.0,
                                     1.0, 1.0,-1.0,   0.0, 1.0, 0.0,   1.0, 0.0,

                                    -1.0,-1.0, 1.0,   0.0, 0.0, 1.0,   0.0, 0.0,
                                     1.0,-1.0, 1.0,   0.0, 0.0, 1.0,   0.0, 1.0,
                                     1.0, 1.0, 1.0,   0.0, 0.0, 1.0,   1.0, 1.0,
                                    -1.0, 1.0, 1.0,   0.0, 0.0, 1.0,   1.0, 0.0,

                                     1.0, 1.0, 1.0,   1.0, 0.0, 0.0,   0.0, 0.0,
                                     1.0,-1.0, 1.0,   1.0, 0.0, 0.0,   0.0, 1.0,
                                     1.0,-1.0,-1.0,   1.0, 0.0, 0.0,   1.0, 1.0,
                                     1.0, 1.0,-1.0,   1.0, 0.0, 0.0,   1.0, 0.0,

                                     1.0,-1.0,-1.0,   0.0, 0.0,-1.0,   0.0, 0.0,
                                    -1.0,-1.0,-1.0,   0.0, 0.0,-1.0,   0.0, 1.0,
                                    -1.0, 1.0,-1.0,   0.0, 0.0,-1.0,   1.0, 1.0,
                                     1.0, 1.0,-1.0,   0.0, 0.0,-1.0,   1.0, 0.0,

                                    -1.0,-1.0, 1.0,   0.0,-1.0, 0.0,   0.0, 0.0,
                                    -1.0,-1.0,-1.0,   0.0,-1.0, 0.0,   0.0, 1.0,
                                     1.0,-1.0,-1.0,   0.0,-1.0, 0.0,   1.0, 1.0,
                                     1.0,-1.0, 1.0,   0.0,-1.0, 0.0,   1.0, 0.0,

                                    -1.0, 1.0, 1.0,  -1.0, 0.0, 0.0,   0.0, 0.0,
                                    -1.0, 1.0,-1.0,  -1.0, 0.0, 0.0,   0.0, 1.0,
                                    -1.0,-1.0,-1.0,  -1.0, 0.0, 0.0,   1.0, 1.0,
                                    -1.0,-1.0, 1.0,  -1.0, 0.0, 0.0,   1.0, 0.0 ]),

                        mIndices : new Uint16Array([0,1,2, 0,2,3,   4,5,6, 4,6,7,    8,9,10,8,10,11,   12,13,14,12,14,15,   16,17,18,16,18,19,   20,21,22,20,22,23]),
                        mNumVertices : 24,
                        mNumFaces : 12, 
                        mTransform : setIdentity(),
                        mVBO : null, 
                        mIBO : null };


    this.mVertexFormat = { mStride:32, mChannels:[ { mNumComponents:3, mType: renderer.TYPE.FLOAT32, mNormalize: false },
                                                   { mNumComponents:3, mType: renderer.TYPE.FLOAT32, mNormalize: false },
                                                   { mNumComponents:2, mType: renderer.TYPE.FLOAT32, mNormalize: false } ] };

    return true;
}


piMesh.prototype.createUnitQuad = function (renderer)
{
    this.mPrimitiveType = 0;
    this.mVertexFormat = { mStride:8, mChannels: [ {mNumComponents:2, mType: renderer.TYPE.FLOAT32, mNormalize: false} ] };

    this.mChunks[0] = { mVerts : new Float32Array([-1.0, -1.0, 1.0, -1.0, -1.0,  1.0, 1.0,  1.0]),
                        mIndices : new Uint16Array([0, 2, 1, 1, 2, 3]),
                        mNumVertices : 4,
                        mNumFaces : 2, 
                        mTransform : setIdentity(),
                        mVBO : null, 
                        mIBO : null };
 
    return true;
}




piMesh.prototype.destroy = function()
{
    //delete this.mVerts;   this.mVerts = null;
    //delete this.mIndices; this.mIndices = null;
}

piMesh.prototype.scale = function (x, y, z)
{
    var stride = this.mVertexFormat.mStride/4;
    for( var j=0; j<this.mChunks.length; j++ )
    {
        var nv = this.mChunks[j].mNumVertices;
        for (var i = 0; i < nv; i++) 
        {
            this.mChunks[j].mVerts[stride * i + 0] *= x;
            this.mChunks[j].mVerts[stride * i + 1] *= y;
            this.mChunks[j].mVerts[stride * i + 2] *= z;
        }
    }
}

piMesh.prototype.translate = function (x, y, z) 
{
    var stride = this.mVertexFormat.mStride/4;
    for( var j=0; j<this.mChunks.length; j++ )
    {
        var nv = this.mChunks[j].mNumVertices;
        for (var i = 0; i < nv; i++) 
        {
            this.mChunks[j].mVerts[stride * i + 0] += x;
            this.mChunks[j].mVerts[stride * i + 1] += y;
            this.mChunks[j].mVerts[stride * i + 2] += z;
        }
    }
}

piMesh.prototype.GPULoad = function (renderer)
{
    for( var i=0; i<this.mChunks.length; i++ )
    {
        var vbo = renderer.CreateVertexArray(this.mChunks[i].mVerts, renderer.BUFTYPE.STATIC );
        if (vbo == null)
            return false;

        var ibo = renderer.CreateIndexArray(this.mChunks[i].mIndices, renderer.BUFTYPE.STATIC);
        if (ibo == null)
            return false;

        this.mChunks[i].mVBO = vbo;
        this.mChunks[i].mIBO = ibo;
    }
    return true;
}

piMesh.prototype.GPURender = function (renderer, positions )//, matPrj, matCam )
{
    //renderer.SetShaderConstantMat4F("unMPrj", matPrj, false );

    var num = this.mChunks.length;
    for( var i=0; i<num; i++ )
    {
        //var mat =  matMul( matCam, this.mChunks[i].mTransform );
        //renderer.SetShaderConstantMat4F("unMMod", mat, false);

        renderer.AttachVertexArray(this.mChunks[i].mVBO, this.mVertexFormat, positions );
        renderer.AttachIndexArray(this.mChunks[i].mIBO );
        renderer.DrawPrimitive(renderer.PRIMTYPE.TRIANGLES, this.mChunks[i].mNumFaces * 3, true, 1);
        renderer.DetachIndexArray(this.mChunks[i].mIBO);
        renderer.DetachVertexArray(this.mChunks[i].mVBO, this.mVertexFormat );
    }
}
//==============================================================================
//
// piLibs 2014-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piRenderer
//
//==============================================================================

function piRenderer()
{
    // private members

    var mGL = null;
    var mBindedShader = null;
    var mIs20 = false;
    var mFloat32Textures;
    var mFloat32Filter;
    var mFloat16Textures;
    var mDrawBuffers;
    var mDepthTextures;
    var mDerivatives;
    var mFloat16Filter;
    var mShaderTextureLOD;
    var mAnisotropic;
    var mRenderToFloat32F;
    var mDebugShader;
    var mAsynchCompile;

    var mVBO_Quad = null;
    var mVBO_Tri = null;
    var mVBO_CubePosNor = null;
    var mVBO_CubePos = null;
    var mShaderHeader = ["",""];
    var mShaderHeaderLines = [0,0];

    // public members
    var me = {};

    me.CLEAR      = { Color: 1, Zbuffer : 2, Stencil : 4 };
    me.TEXFMT     = { C4I8 : 0, C1I8 : 1, C1F16 : 2, C4F16 : 3, C1F32 : 4, C4F32 : 5, Z16 : 6, Z24 : 7, Z32 : 8, C3F32:9 };
    me.TEXWRP     = { CLAMP : 0, REPEAT : 1 };
    me.BUFTYPE    = { STATIC : 0, DYNAMIC : 1 };
    me.PRIMTYPE   = { POINTS : 0, LINES : 1, LINE_LOOP : 2, LINE_STRIP : 3, TRIANGLES : 4, TRIANGLE_STRIP : 5 };
    me.RENDSTGATE = { WIREFRAME : 0, FRONT_FACE : 1, CULL_FACE : 2, DEPTH_TEST : 3, ALPHA_TO_COVERAGE : 4 };
    me.TEXTYPE    = { T2D : 0, T3D : 1, CUBEMAP : 2 };
    me.FILTER     = { NONE : 0, LINEAR : 1, MIPMAP : 2, NONE_MIPMAP : 3 };
    me.TYPE       = { UINT8 : 0, UINT16 : 1, UINT32 : 2, FLOAT16: 3, FLOAT32 : 4, FLOAT64: 5 };

    // private functions

    var iFormatPI2GL = function( format )
    {
        if( mIs20 )
        {
             if (format === me.TEXFMT.C4I8)  return { mGLFormat: mGL.RGBA8,           mGLExternal: mGL.RGBA,             mGLType: mGL.UNSIGNED_BYTE }
        else if (format === me.TEXFMT.C1I8)  return { mGLFormat: mGL.R8,              mGLExternal: mGL.RED,              mGLType: mGL.UNSIGNED_BYTE }
        else if (format === me.TEXFMT.C1F16) return { mGLFormat: mGL.R16F,            mGLExternal: mGL.RED,              mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C4F16) return { mGLFormat: mGL.RGBA16F,         mGLExternal: mGL.RGBA,             mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C1F32) return { mGLFormat: mGL.R32F,            mGLExternal: mGL.RED,              mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C4F32) return { mGLFormat: mGL.RGBA32F,         mGLExternal: mGL.RGBA,             mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C3F32) return { mGLFormat: mGL.RGB32F,          mGLExternal: mGL.RGB,              mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.Z16)   return { mGLFormat: mGL.DEPTH_COMPONENT16, mGLExternal: mGL.DEPTH_COMPONENT,  mGLType: mGL.UNSIGNED_SHORT }
        else if (format === me.TEXFMT.Z24)   return { mGLFormat: mGL.DEPTH_COMPONENT24, mGLExternal: mGL.DEPTH_COMPONENT,  mGLType: mGL.UNSIGNED_SHORT }
        else if (format === me.TEXFMT.Z32)   return { mGLFormat: mGL.DEPTH_COMPONENT32F, mGLExternal: mGL.DEPTH_COMPONENT,  mGLType: mGL.UNSIGNED_SHORT }
        }
        else
        {
             if (format === me.TEXFMT.C4I8)  return { mGLFormat: mGL.RGBA,            mGLExternal: mGL.RGBA,            mGLType: mGL.UNSIGNED_BYTE }
        else if (format === me.TEXFMT.C1I8)  return { mGLFormat: mGL.LUMINANCE,       mGLExternal: mGL.LUMINANCE,       mGLType: mGL.UNSIGNED_BYTE }
        else if (format === me.TEXFMT.C1F16) return { mGLFormat: mGL.LUMINANCE,       mGLExternal: mGL.LUMINANCE,       mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C4F16) return { mGLFormat: mGL.RGBA,            mGLExternal: mGL.RGBA,            mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C1F32) return { mGLFormat: mGL.LUMINANCE,       mGLExternal: mGL.RED,             mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.C4F32) return { mGLFormat: mGL.RGBA,            mGLExternal: mGL.RGBA,            mGLType: mGL.FLOAT }
        else if (format === me.TEXFMT.Z16)   return { mGLFormat: mGL.DEPTH_COMPONENT, mGLExternal: mGL.DEPTH_COMPONENT, mGLType: mGL.UNSIGNED_SHORT }
        }

        return null;
     }

    // public functions

    me.Initialize = function( gl )
                    {
                        mGL = gl;

                        mIs20 = !(gl instanceof WebGLRenderingContext);

                        if( mIs20 )
                        {
                            mFloat32Textures  = true;
                            mFloat32Filter    = mGL.getExtension( 'OES_texture_float_linear');
                            mFloat16Textures  = true;
                            mFloat16Filter    = mGL.getExtension( 'OES_texture_half_float_linear' );
                            mDerivatives      = true;
                            mDrawBuffers      = true;
                            mDepthTextures    = true;
                            mShaderTextureLOD = true;
                            mAnisotropic = mGL.getExtension( 'EXT_texture_filter_anisotropic' );
                            mRenderToFloat32F = mGL.getExtension( 'EXT_color_buffer_float');
                            mDebugShader = mGL.getExtension('WEBGL_debug_shaders');
                            mAsynchCompile = mGL.getExtension('KHR_parallel_shader_compile');

							mGL.hint( mGL.FRAGMENT_SHADER_DERIVATIVE_HINT, mGL.NICEST);
                        }
                        else
                        {
                            mFloat32Textures  = mGL.getExtension( 'OES_texture_float' );
                            mFloat32Filter    = mGL.getExtension( 'OES_texture_float_linear');
                            mFloat16Textures  = mGL.getExtension( 'OES_texture_half_float' );
                            mFloat16Filter    = mGL.getExtension( 'OES_texture_half_float_linear' );
                            mDerivatives      = mGL.getExtension( 'OES_standard_derivatives' );
                            mDrawBuffers      = mGL.getExtension( 'WEBGL_draw_buffers' );
                            mDepthTextures    = mGL.getExtension( 'WEBGL_depth_texture' );
                            mShaderTextureLOD = mGL.getExtension( 'EXT_shader_texture_lod' );
                            mAnisotropic      = mGL.getExtension( 'EXT_texture_filter_anisotropic' );
                            mRenderToFloat32F = mFloat32Textures;
                            mDebugShader      = null;
                            mAsynchCompile    = null;
							
							if( mDerivatives !== null) mGL.hint( mDerivatives.FRAGMENT_SHADER_DERIVATIVE_HINT_OES, mGL.NICEST);
                        }
                        
                        
                        var maxTexSize = mGL.getParameter( mGL.MAX_TEXTURE_SIZE );
                        var maxCubeSize = mGL.getParameter( mGL.MAX_CUBE_MAP_TEXTURE_SIZE );
                        var maxRenderbufferSize = mGL.getParameter( mGL.MAX_RENDERBUFFER_SIZE );
                        var extensions = mGL.getSupportedExtensions();
                        var textureUnits = mGL.getParameter( mGL.MAX_TEXTURE_IMAGE_UNITS );
                        console.log("WebGL (2.0=" + mIs20 + "):" +
                                    " Asynch Compile: "  + ((mAsynchCompile !==null) ? "yes" : "no") +
                                    ", Textures: F32 ["   + ((mFloat32Textures !==null) ? "yes" : "no") +
                                    "], F16 ["   + ((mFloat16Textures !==null) ? "yes" : "no") +
                                    "], Depth [" + ((mDepthTextures   !==null) ? "yes" : "no") +
                                    "], LOD ["    + ((mShaderTextureLOD!==null) ? "yes" : "no") +
                                    "], Aniso ["   + ((mAnisotropic     !==null) ? "yes" : "no") +
                                    "], Units [" + textureUnits +
                                    "], Max Size [" + maxTexSize +
                                    "], Cube Max Size [" + maxCubeSize +
                                    "], Targets: MRT ["            + ((mDrawBuffers     !==null) ? "yes" : "no") +
                                    "], F32 ["     + ((mRenderToFloat32F!==null) ? "yes" : "no") +
                                    "], Max Size [" + maxRenderbufferSize + "]");

                        // create a 2D quad Vertex Buffer
                        var vertices = new Float32Array( [ -1.0, -1.0,   1.0, -1.0,    -1.0,  1.0,     1.0, -1.0,    1.0,  1.0,    -1.0,  1.0] );
                        mVBO_Quad = mGL.createBuffer();
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_Quad );
                        mGL.bufferData( mGL.ARRAY_BUFFER, vertices, mGL.STATIC_DRAW );
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, null );

                        // create a 2D triangle Vertex Buffer
                        mVBO_Tri = mGL.createBuffer();
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_Tri );
                        mGL.bufferData( mGL.ARRAY_BUFFER, new Float32Array( [ -1.0, -1.0,   3.0, -1.0,    -1.0,  3.0] ), mGL.STATIC_DRAW );
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, null );

                        // create a 3D cube Vertex Buffer
                        mVBO_CubePosNor = mGL.createBuffer();
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_CubePosNor );
                        mGL.bufferData( mGL.ARRAY_BUFFER, new Float32Array( [ -1.0, -1.0, -1.0,  -1.0,  0.0,  0.0,
                                                                              -1.0, -1.0,  1.0,  -1.0,  0.0,  0.0,
                                                                              -1.0,  1.0, -1.0,  -1.0,  0.0,  0.0,
                                                                              -1.0,  1.0,  1.0,  -1.0,  0.0,  0.0,
                                                                               1.0,  1.0, -1.0,   1.0,  0.0,  0.0,
                                                                               1.0,  1.0,  1.0,   1.0,  0.0,  0.0,
                                                                               1.0, -1.0, -1.0,   1.0,  0.0,  0.0,
                                                                               1.0, -1.0,  1.0,   1.0,  0.0,  0.0,
                                                                               1.0,  1.0,  1.0,   0.0,  1.0,  0.0,
                                                                               1.0,  1.0, -1.0,   0.0,  1.0,  0.0,
                                                                              -1.0,  1.0,  1.0,   0.0,  1.0,  0.0,
                                                                              -1.0,  1.0, -1.0,   0.0,  1.0,  0.0,
                                                                               1.0, -1.0, -1.0,   0.0, -1.0,  0.0,
                                                                               1.0, -1.0,  1.0,   0.0, -1.0,  0.0,
                                                                              -1.0, -1.0, -1.0,   0.0, -1.0,  0.0,
                                                                              -1.0, -1.0,  1.0,   0.0, -1.0,  0.0,
                                                                              -1.0,  1.0,  1.0,   0.0,  0.0,  1.0,
                                                                              -1.0, -1.0,  1.0,   0.0,  0.0,  1.0,
                                                                               1.0,  1.0,  1.0,   0.0,  0.0,  1.0,
                                                                               1.0, -1.0,  1.0,   0.0,  0.0,  1.0,
                                                                              -1.0, -1.0, -1.0,   0.0,  0.0, -1.0,
                                                                              -1.0,  1.0, -1.0,   0.0,  0.0, -1.0,
                                                                               1.0, -1.0, -1.0,   0.0,  0.0, -1.0,
                                                                               1.0,  1.0, -1.0,   0.0,  0.0, -1.0 ] ), mGL.STATIC_DRAW );
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, null );

                        // create a 3D cube Vertex Buffer
                        mVBO_CubePos = mGL.createBuffer();
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_CubePos );
                        mGL.bufferData( mGL.ARRAY_BUFFER, new Float32Array( [ -1.0, -1.0, -1.0,
                                                                              -1.0, -1.0,  1.0,
                                                                              -1.0,  1.0, -1.0,
                                                                              -1.0,  1.0,  1.0,
                                                                               1.0,  1.0, -1.0,
                                                                               1.0,  1.0,  1.0,
                                                                               1.0, -1.0, -1.0,
                                                                               1.0, -1.0,  1.0,
                                                                               1.0,  1.0,  1.0,
                                                                               1.0,  1.0, -1.0,
                                                                              -1.0,  1.0,  1.0,
                                                                              -1.0,  1.0, -1.0,
                                                                               1.0, -1.0, -1.0,
                                                                               1.0, -1.0,  1.0,
                                                                              -1.0, -1.0, -1.0,
                                                                              -1.0, -1.0,  1.0,
                                                                              -1.0,  1.0,  1.0,
                                                                              -1.0, -1.0,  1.0,
                                                                               1.0,  1.0,  1.0,
                                                                               1.0, -1.0,  1.0,
                                                                              -1.0, -1.0, -1.0,
                                                                              -1.0,  1.0, -1.0,
                                                                               1.0, -1.0, -1.0,
                                                                               1.0,  1.0, -1.0 ] ), mGL.STATIC_DRAW );
                        mGL.bindBuffer( mGL.ARRAY_BUFFER, null );

                        //-------------------------------------------------------------------

                        mShaderHeader[0] = "";
                        mShaderHeaderLines[0] = 0;
                        if( mIs20 ) 
                        { 
                            mShaderHeader[0] += "#version 300 es\n" +
                                                "#ifdef GL_ES\n"+
                                                "precision highp float;\n" +
                                                "precision highp int;\n"+
                                                "precision mediump sampler3D;\n"+
                                                "#endif\n"; 
                            mShaderHeaderLines[0] += 6;
                        }
                        else
                        {
                            mShaderHeader[0] += "#ifdef GL_ES\n"+
                                                "precision highp float;\n" +
                                                "precision highp int;\n"+
                                                "#endif\n"+
                                                "float round( float x ) { return floor(x+0.5); }\n"+
                                                "vec2 round(vec2 x) { return floor(x + 0.5); }\n"+
                                                "vec3 round(vec3 x) { return floor(x + 0.5); }\n"+
                                                "vec4 round(vec4 x) { return floor(x + 0.5); }\n"+
                                                "float trunc( float x, float n ) { return floor(x*n)/n; }\n"+
                                                "mat3 transpose(mat3 m) { return mat3(m[0].x, m[1].x, m[2].x, m[0].y, m[1].y, m[2].y, m[0].z, m[1].z, m[2].z); }\n"+
                                                "float determinant( in mat2 m ) { return m[0][0]*m[1][1] - m[0][1]*m[1][0]; }\n"+
                                                "float determinant( mat4 m ) { float b00 = m[0][0] * m[1][1] - m[0][1] * m[1][0], b01 = m[0][0] * m[1][2] - m[0][2] * m[1][0], b02 = m[0][0] * m[1][3] - m[0][3] * m[1][0], b03 = m[0][1] * m[1][2] - m[0][2] * m[1][1], b04 = m[0][1] * m[1][3] - m[0][3] * m[1][1], b05 = m[0][2] * m[1][3] - m[0][3] * m[1][2], b06 = m[2][0] * m[3][1] - m[2][1] * m[3][0], b07 = m[2][0] * m[3][2] - m[2][2] * m[3][0], b08 = m[2][0] * m[3][3] - m[2][3] * m[3][0], b09 = m[2][1] * m[3][2] - m[2][2] * m[3][1], b10 = m[2][1] * m[3][3] - m[2][3] * m[3][1], b11 = m[2][2] * m[3][3] - m[2][3] * m[3][2];  return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;}\n"+
                                                "mat2 inverse(mat2 m) { float det = determinant(m); return mat2(m[1][1], -m[0][1], -m[1][0], m[0][0]) / det; }\n"+
                                                "mat4 inverse(mat4 m ) { float inv0 = m[1].y*m[2].z*m[3].w - m[1].y*m[2].w*m[3].z - m[2].y*m[1].z*m[3].w + m[2].y*m[1].w*m[3].z + m[3].y*m[1].z*m[2].w - m[3].y*m[1].w*m[2].z; float inv4 = -m[1].x*m[2].z*m[3].w + m[1].x*m[2].w*m[3].z + m[2].x*m[1].z*m[3].w - m[2].x*m[1].w*m[3].z - m[3].x*m[1].z*m[2].w + m[3].x*m[1].w*m[2].z; float inv8 = m[1].x*m[2].y*m[3].w - m[1].x*m[2].w*m[3].y - m[2].x  * m[1].y * m[3].w + m[2].x  * m[1].w * m[3].y + m[3].x * m[1].y * m[2].w - m[3].x * m[1].w * m[2].y; float inv12 = -m[1].x  * m[2].y * m[3].z + m[1].x  * m[2].z * m[3].y +m[2].x  * m[1].y * m[3].z - m[2].x  * m[1].z * m[3].y - m[3].x * m[1].y * m[2].z + m[3].x * m[1].z * m[2].y; float inv1 = -m[0].y*m[2].z * m[3].w + m[0].y*m[2].w * m[3].z + m[2].y  * m[0].z * m[3].w - m[2].y  * m[0].w * m[3].z - m[3].y * m[0].z * m[2].w + m[3].y * m[0].w * m[2].z; float inv5 = m[0].x  * m[2].z * m[3].w - m[0].x  * m[2].w * m[3].z - m[2].x  * m[0].z * m[3].w + m[2].x  * m[0].w * m[3].z + m[3].x * m[0].z * m[2].w - m[3].x * m[0].w * m[2].z; float inv9 = -m[0].x  * m[2].y * m[3].w +  m[0].x  * m[2].w * m[3].y + m[2].x  * m[0].y * m[3].w - m[2].x  * m[0].w * m[3].y - m[3].x * m[0].y * m[2].w + m[3].x * m[0].w * m[2].y; float inv13 = m[0].x  * m[2].y * m[3].z - m[0].x  * m[2].z * m[3].y - m[2].x  * m[0].y * m[3].z + m[2].x  * m[0].z * m[3].y + m[3].x * m[0].y * m[2].z - m[3].x * m[0].z * m[2].y; float inv2 = m[0].y  * m[1].z * m[3].w - m[0].y  * m[1].w * m[3].z - m[1].y  * m[0].z * m[3].w + m[1].y  * m[0].w * m[3].z + m[3].y * m[0].z * m[1].w - m[3].y * m[0].w * m[1].z; float inv6 = -m[0].x  * m[1].z * m[3].w + m[0].x  * m[1].w * m[3].z + m[1].x  * m[0].z * m[3].w - m[1].x  * m[0].w * m[3].z - m[3].x * m[0].z * m[1].w + m[3].x * m[0].w * m[1].z; float inv10 = m[0].x  * m[1].y * m[3].w - m[0].x  * m[1].w * m[3].y - m[1].x  * m[0].y * m[3].w + m[1].x  * m[0].w * m[3].y + m[3].x * m[0].y * m[1].w - m[3].x * m[0].w * m[1].y; float inv14 = -m[0].x  * m[1].y * m[3].z + m[0].x  * m[1].z * m[3].y + m[1].x  * m[0].y * m[3].z - m[1].x  * m[0].z * m[3].y - m[3].x * m[0].y * m[1].z + m[3].x * m[0].z * m[1].y; float inv3 = -m[0].y * m[1].z * m[2].w + m[0].y * m[1].w * m[2].z + m[1].y * m[0].z * m[2].w - m[1].y * m[0].w * m[2].z - m[2].y * m[0].z * m[1].w + m[2].y * m[0].w * m[1].z; float inv7 = m[0].x * m[1].z * m[2].w - m[0].x * m[1].w * m[2].z - m[1].x * m[0].z * m[2].w + m[1].x * m[0].w * m[2].z + m[2].x * m[0].z * m[1].w - m[2].x * m[0].w * m[1].z; float inv11 = -m[0].x * m[1].y * m[2].w + m[0].x * m[1].w * m[2].y + m[1].x * m[0].y * m[2].w - m[1].x * m[0].w * m[2].y - m[2].x * m[0].y * m[1].w + m[2].x * m[0].w * m[1].y; float inv15 = m[0].x * m[1].y * m[2].z - m[0].x * m[1].z * m[2].y - m[1].x * m[0].y * m[2].z + m[1].x * m[0].z * m[2].y + m[2].x * m[0].y * m[1].z - m[2].x * m[0].z * m[1].y; float det = m[0].x * inv0 + m[0].y * inv4 + m[0].z * inv8 + m[0].w * inv12; det = 1.0 / det; return det*mat4( inv0, inv1, inv2, inv3,inv4, inv5, inv6, inv7,inv8, inv9, inv10, inv11,inv12, inv13, inv14, inv15);}\n"+                                                
                                                "float sinh(float x)  { return (exp(x)-exp(-x))/2.; }\n"+
                                                "float cosh(float x)  { return (exp(x)+exp(-x))/2.; }\n"+
                                                "float tanh(float x)  { return sinh(x)/cosh(x); }\n"+
                                                "float coth(float x)  { return cosh(x)/sinh(x); }\n"+
                                                "float sech(float x)  { return 1./cosh(x); }\n"+
                                                "float csch(float x)  { return 1./sinh(x); }\n"+
                                                "float asinh(float x) { return    log(x+sqrt(x*x+1.)); }\n"+
                                                "float acosh(float x) { return    log(x+sqrt(x*x-1.)); }\n"+
                                                "float atanh(float x) { return .5*log((1.+x)/(1.-x)); }\n"+
                                                "float acoth(float x) { return .5*log((x+1.)/(x-1.)); }\n"+
                                                "float asech(float x) { return    log((1.+sqrt(1.-x*x))/x); }\n"+
                                                "float acsch(float x) { return    log((1.+sqrt(1.+x*x))/x); }\n";
                            mShaderHeaderLines[0] += 26;
                        }

                        //-------------------------------------------------------

                        mShaderHeader[1] = "";
                        mShaderHeaderLines[1] = 0;
                        if( mIs20 ) 
                        { 
                            mShaderHeader[1] += "#version 300 es\n"+
                                                "#ifdef GL_ES\n"+
                                                "precision highp float;\n"+
                                                "precision highp int;\n"+
                                                "precision mediump sampler3D;\n"+
                                                "#endif\n"; 
                            mShaderHeaderLines[1] += 6;
                        }
                        else
                        {
                            if( mDerivatives ) { mShaderHeader[1] += "#ifdef GL_OES_standard_derivatives\n#extension GL_OES_standard_derivatives : enable\n#endif\n"; mShaderHeaderLines[1]+=3; }
                            if( mShaderTextureLOD  ) { mShaderHeader[1] += "#extension GL_EXT_shader_texture_lod : enable\n"; mShaderHeaderLines[1]++; }
                            mShaderHeader[1] += "#ifdef GL_ES\n"+
                                                "precision highp float;\n"+
                                                "precision highp int;\n"+
                                                "#endif\n"+ 
                                                "vec4 texture(     sampler2D   s, vec2 c)                   { return texture2D(s,c); }\n"+
                                                "vec4 texture(     sampler2D   s, vec2 c, float b)          { return texture2D(s,c,b); }\n"+
                                                "vec4 texture(     samplerCube s, vec3 c )                  { return textureCube(s,c); }\n"+
                                                "vec4 texture(     samplerCube s, vec3 c, float b)          { return textureCube(s,c,b); }\n"+
                                                "float round( float x ) { return floor(x+0.5); }\n"+
                                                "vec2 round(vec2 x) { return floor(x + 0.5); }\n"+
                                                "vec3 round(vec3 x) { return floor(x + 0.5); }\n"+
                                                "vec4 round(vec4 x) { return floor(x + 0.5); }\n"+
                                                "float trunc( float x, float n ) { return floor(x*n)/n; }\n"+
                                                "mat3 transpose(mat3 m) { return mat3(m[0].x, m[1].x, m[2].x, m[0].y, m[1].y, m[2].y, m[0].z, m[1].z, m[2].z); }\n"+
                                                "float determinant( in mat2 m ) { return m[0][0]*m[1][1] - m[0][1]*m[1][0]; }\n"+
                                                "float determinant( mat4 m ) { float b00 = m[0][0] * m[1][1] - m[0][1] * m[1][0], b01 = m[0][0] * m[1][2] - m[0][2] * m[1][0], b02 = m[0][0] * m[1][3] - m[0][3] * m[1][0], b03 = m[0][1] * m[1][2] - m[0][2] * m[1][1], b04 = m[0][1] * m[1][3] - m[0][3] * m[1][1], b05 = m[0][2] * m[1][3] - m[0][3] * m[1][2], b06 = m[2][0] * m[3][1] - m[2][1] * m[3][0], b07 = m[2][0] * m[3][2] - m[2][2] * m[3][0], b08 = m[2][0] * m[3][3] - m[2][3] * m[3][0], b09 = m[2][1] * m[3][2] - m[2][2] * m[3][1], b10 = m[2][1] * m[3][3] - m[2][3] * m[3][1], b11 = m[2][2] * m[3][3] - m[2][3] * m[3][2];  return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;}\n"+
                                                "mat2 inverse(mat2 m) { float det = determinant(m); return mat2(m[1][1], -m[0][1], -m[1][0], m[0][0]) / det; }\n"+
                                                "mat4 inverse(mat4 m ) { float inv0 = m[1].y*m[2].z*m[3].w - m[1].y*m[2].w*m[3].z - m[2].y*m[1].z*m[3].w + m[2].y*m[1].w*m[3].z + m[3].y*m[1].z*m[2].w - m[3].y*m[1].w*m[2].z; float inv4 = -m[1].x*m[2].z*m[3].w + m[1].x*m[2].w*m[3].z + m[2].x*m[1].z*m[3].w - m[2].x*m[1].w*m[3].z - m[3].x*m[1].z*m[2].w + m[3].x*m[1].w*m[2].z; float inv8 = m[1].x*m[2].y*m[3].w - m[1].x*m[2].w*m[3].y - m[2].x  * m[1].y * m[3].w + m[2].x  * m[1].w * m[3].y + m[3].x * m[1].y * m[2].w - m[3].x * m[1].w * m[2].y; float inv12 = -m[1].x  * m[2].y * m[3].z + m[1].x  * m[2].z * m[3].y +m[2].x  * m[1].y * m[3].z - m[2].x  * m[1].z * m[3].y - m[3].x * m[1].y * m[2].z + m[3].x * m[1].z * m[2].y; float inv1 = -m[0].y*m[2].z * m[3].w + m[0].y*m[2].w * m[3].z + m[2].y  * m[0].z * m[3].w - m[2].y  * m[0].w * m[3].z - m[3].y * m[0].z * m[2].w + m[3].y * m[0].w * m[2].z; float inv5 = m[0].x  * m[2].z * m[3].w - m[0].x  * m[2].w * m[3].z - m[2].x  * m[0].z * m[3].w + m[2].x  * m[0].w * m[3].z + m[3].x * m[0].z * m[2].w - m[3].x * m[0].w * m[2].z; float inv9 = -m[0].x  * m[2].y * m[3].w +  m[0].x  * m[2].w * m[3].y + m[2].x  * m[0].y * m[3].w - m[2].x  * m[0].w * m[3].y - m[3].x * m[0].y * m[2].w + m[3].x * m[0].w * m[2].y; float inv13 = m[0].x  * m[2].y * m[3].z - m[0].x  * m[2].z * m[3].y - m[2].x  * m[0].y * m[3].z + m[2].x  * m[0].z * m[3].y + m[3].x * m[0].y * m[2].z - m[3].x * m[0].z * m[2].y; float inv2 = m[0].y  * m[1].z * m[3].w - m[0].y  * m[1].w * m[3].z - m[1].y  * m[0].z * m[3].w + m[1].y  * m[0].w * m[3].z + m[3].y * m[0].z * m[1].w - m[3].y * m[0].w * m[1].z; float inv6 = -m[0].x  * m[1].z * m[3].w + m[0].x  * m[1].w * m[3].z + m[1].x  * m[0].z * m[3].w - m[1].x  * m[0].w * m[3].z - m[3].x * m[0].z * m[1].w + m[3].x * m[0].w * m[1].z; float inv10 = m[0].x  * m[1].y * m[3].w - m[0].x  * m[1].w * m[3].y - m[1].x  * m[0].y * m[3].w + m[1].x  * m[0].w * m[3].y + m[3].x * m[0].y * m[1].w - m[3].x * m[0].w * m[1].y; float inv14 = -m[0].x  * m[1].y * m[3].z + m[0].x  * m[1].z * m[3].y + m[1].x  * m[0].y * m[3].z - m[1].x  * m[0].z * m[3].y - m[3].x * m[0].y * m[1].z + m[3].x * m[0].z * m[1].y; float inv3 = -m[0].y * m[1].z * m[2].w + m[0].y * m[1].w * m[2].z + m[1].y * m[0].z * m[2].w - m[1].y * m[0].w * m[2].z - m[2].y * m[0].z * m[1].w + m[2].y * m[0].w * m[1].z; float inv7 = m[0].x * m[1].z * m[2].w - m[0].x * m[1].w * m[2].z - m[1].x * m[0].z * m[2].w + m[1].x * m[0].w * m[2].z + m[2].x * m[0].z * m[1].w - m[2].x * m[0].w * m[1].z; float inv11 = -m[0].x * m[1].y * m[2].w + m[0].x * m[1].w * m[2].y + m[1].x * m[0].y * m[2].w - m[1].x * m[0].w * m[2].y - m[2].x * m[0].y * m[1].w + m[2].x * m[0].w * m[1].y; float inv15 = m[0].x * m[1].y * m[2].z - m[0].x * m[1].z * m[2].y - m[1].x * m[0].y * m[2].z + m[1].x * m[0].z * m[2].y + m[2].x * m[0].y * m[1].z - m[2].x * m[0].z * m[1].y; float det = m[0].x * inv0 + m[0].y * inv4 + m[0].z * inv8 + m[0].w * inv12; det = 1.0 / det; return det*mat4( inv0, inv1, inv2, inv3,inv4, inv5, inv6, inv7,inv8, inv9, inv10, inv11,inv12, inv13, inv14, inv15);}\n"+
                                                "float sinh(float x)  { return (exp(x)-exp(-x))/2.; }\n"+
                                                "float cosh(float x)  { return (exp(x)+exp(-x))/2.; }\n"+
                                                "float tanh(float x)  { return sinh(x)/cosh(x); }\n"+
                                                "float coth(float x)  { return cosh(x)/sinh(x); }\n"+
                                                "float sech(float x)  { return 1./cosh(x); }\n"+
                                                "float csch(float x)  { return 1./sinh(x); }\n"+
                                                "float asinh(float x) { return    log(x+sqrt(x*x+1.)); }\n"+
                                                "float acosh(float x) { return    log(x+sqrt(x*x-1.)); }\n"+
                                                "float atanh(float x) { return .5*log((1.+x)/(1.-x)); }\n"+
                                                "float acoth(float x) { return .5*log((x+1.)/(x-1.)); }\n"+
                                                "float asech(float x) { return    log((1.+sqrt(1.-x*x))/x); }\n"+
                                                "float acsch(float x) { return    log((1.+sqrt(1.+x*x))/x); }\n";
                            mShaderHeaderLines[1] += 30;
                            if( mShaderTextureLOD )
                            {
                            mShaderHeader[1] += "vec4 textureLod(  sampler2D   s, vec2 c, float b)          { return texture2DLodEXT(s,c,b); }\n";
                            mShaderHeader[1] += "vec4 textureGrad( sampler2D   s, vec2 c, vec2 dx, vec2 dy) { return texture2DGradEXT(s,c,dx,dy); }\n";
                            mShaderHeaderLines[1] += 2;

                            //mShaderHeader[1] += "vec4 texelFetch( sampler2D s, ivec2 c, int l) { return texture2DLodEXT(s,(vec2(c)+0.5)/vec2(800,450),float(l)); }\n";
                            //mShaderHeaderLines[1] += 1;
                            }
                        }

                        return true;
                    };

        me.GetCaps =    function ()
                        {   
                            return { mIsGL20 : mIs20,
                                     mFloat32Textures: mFloat32Textures != null,
                                     mFloat16Textures: mFloat16Textures != null,
                                     mDrawBuffers: mDrawBuffers != null,
                                     mDepthTextures: mDepthTextures != null,
                                     mDerivatives: mDerivatives != null,
                                     mShaderTextureLOD: mShaderTextureLOD != null };
                        };

        me.GetShaderHeaderLines = function (shaderType)
                        {   
                            return mShaderHeaderLines[shaderType];
                        };

        me.CheckErrors = function()
                         {
                                var error = mGL.getError();
                                if( error != mGL.NO_ERROR ) 
                                { 
                                    for( var prop in mGL ) 
                                    {
                                        if( typeof mGL[prop] == 'number' ) 
                                        {
                                            if( mGL[prop] == error )
                                            {
                                                console.log( "GL Error " + error + ": " + prop );
                                                break;
                                            }
                                        }
                                    }
                                }
                         };

        me.Clear =  function( flags, ccolor, cdepth, cstencil )
                    {
                        var mode = 0;
                        if( flags & 1 ) { mode |= mGL.COLOR_BUFFER_BIT;   mGL.clearColor( ccolor[0], ccolor[1], ccolor[2], ccolor[3] ); }
                        if( flags & 2 ) { mode |= mGL.DEPTH_BUFFER_BIT;   mGL.clearDepth( cdepth ); }
                        if( flags & 4 ) { mode |= mGL.STENCIL_BUFFER_BIT; mGL.clearStencil( cstencil ); }
                        mGL.clear( mode );
                    };


        me.CreateTexture = function ( type, xres, yres, format, filter, wrap, buffer)
                           {
                                if( mGL===null ) return null;

                                var id = mGL.createTexture();

                                var glFoTy = iFormatPI2GL( format );
                                var glWrap = mGL.REPEAT; if (wrap === me.TEXWRP.CLAMP) glWrap = mGL.CLAMP_TO_EDGE;

                                if( type===me.TEXTYPE.T2D )
                                {
                                    mGL.bindTexture( mGL.TEXTURE_2D, id );
//if( buffer==null )
    //mGL.texStorage2D(mGL.TEXTURE_2D, 0, glFoTy.mGLFormat, xres, yres);
//else
                                    mGL.texImage2D( mGL.TEXTURE_2D, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texParameteri( mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_S, glWrap );
                                    mGL.texParameteri( mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_T, glWrap );

                                    if (filter === me.FILTER.NONE)
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                    }
                                    else if (filter === me.FILTER.LINEAR)
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                    }
                                    else if (filter === me.FILTER.MIPMAP)
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                        mGL.generateMipmap(mGL.TEXTURE_2D);
                                    }
                                    else
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                        mGL.generateMipmap(mGL.TEXTURE_2D);
                                    }

                                    mGL.bindTexture( mGL.TEXTURE_2D, null );
                                }
                                else if( type===me.TEXTYPE.T3D )
                                {
                                    if( mIs20 )
                                    {
                                        mGL.bindTexture( mGL.TEXTURE_3D, id );
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_BASE_LEVEL, 0);
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAX_LEVEL, Math.log2(xres));
                                        if (filter === me.FILTER.NONE)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                        }
                                        else if (filter === me.FILTER.LINEAR)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                        }
                                        else if (filter === me.FILTER.MIPMAP)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                        }
                                        else
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                            mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                            mGL.generateMipmap(mGL.TEXTURE_3D);
                                        }
                                        mGL.texImage3D( mGL.TEXTURE_3D, 0, glFoTy.mGLFormat, xres, yres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );

                                        mGL.texParameteri( mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_R, glWrap );
                                        mGL.texParameteri( mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_S, glWrap );
                                        mGL.texParameteri( mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_T, glWrap );

                                        if (filter === me.FILTER.MIPMAP)
                                            mGL.generateMipmap( mGL.TEXTURE_3D );
                                        mGL.bindTexture( mGL.TEXTURE_3D, null );
                                    }
                                    else 
                                    {
                                        return null;
                                    }
                                }
                                else 
                                {
                                    mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, id );

                                    // this works great if we know the number of required mipmaps in advance (1, or other)
                                   //mGL.texStorage2D( mGL.TEXTURE_CUBE_MAP, 1, glFoTy.mGLFormat, xres, yres );

                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_POSITIVE_X, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_NEGATIVE_X, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_POSITIVE_Y, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_POSITIVE_Z, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.texImage2D( mGL.TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, glFoTy.mGLFormat, xres, yres, 0, glFoTy.mGLExternal, glFoTy.mGLType, buffer );

                                    if( filter === me.FILTER.NONE)
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                    }
                                    else if (filter === me.FILTER.LINEAR)
                                    {
                                        mGL.texParameteri( mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR );
                                        mGL.texParameteri( mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR );
                                    }
                                    else if (filter === me.FILTER.MIPMAP)
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                    }

                                    if (filter === me.FILTER.MIPMAP)
                                        mGL.generateMipmap( mGL.TEXTURE_CUBE_MAP );

                                    mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, null );
                                }
                        return { mObjectID: id, mXres: xres, mYres: yres, mFormat: format, mType: type, mFilter: filter, mWrap: wrap, mVFlip:false };
                        };

        me.CreateTextureFromImage = function ( type, image, format, filter, wrap, flipY) 
                                {
                                    if( mGL===null ) return null;

                                    var id = mGL.createTexture();

                                    var glFoTy = iFormatPI2GL( format );

                                    var glWrap = mGL.REPEAT; if (wrap === me.TEXWRP.CLAMP) glWrap = mGL.CLAMP_TO_EDGE;

                                    if( type===me.TEXTYPE.T2D )
                                    {
                                        mGL.bindTexture(mGL.TEXTURE_2D, id);

                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, flipY);
                                        mGL.pixelStorei(mGL.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
                                        if( mIs20 ) mGL.pixelStorei(mGL.UNPACK_COLORSPACE_CONVERSION_WEBGL, mGL.NONE );

                                        mGL.texImage2D(mGL.TEXTURE_2D, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image);

                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_S, glWrap);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_T, glWrap);

                                        if (filter === me.FILTER.NONE)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                        }
                                        else if (filter === me.FILTER.LINEAR)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                        }
                                        else if( filter === me.FILTER.MIPMAP)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                            mGL.generateMipmap(mGL.TEXTURE_2D);
                                        }
                                        else
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                            mGL.generateMipmap(mGL.TEXTURE_2D);
                                        }
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                        mGL.bindTexture(mGL.TEXTURE_2D, null);
                                    }
                                    else if( type===me.TEXTYPE.T3D )
                                    {
                                        return null;
                                    }
                                    else 
                                    {
                                        mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, id );
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, flipY);
                                        mGL.activeTexture( mGL.TEXTURE0 );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_X, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[0] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_X, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[1] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_Y, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, (flipY ? image[3] : image[2]) );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, (flipY ? image[2] : image[3]) );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_Z, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[4] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[5] );

                                        if( filter === me.FILTER.NONE)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                            mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                        }
                                        else if (filter === me.FILTER.LINEAR)
                                        {
                                            mGL.texParameteri( mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR );
                                            mGL.texParameteri( mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR );
                                        }
                                        else if (filter === me.FILTER.MIPMAP)
                                        {
                                            mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                            mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                            mGL.generateMipmap( mGL.TEXTURE_CUBE_MAP );
                                        }
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                        mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, null );
                                    }
                                    return { mObjectID: id, mXres: image.width, mYres: image.height, mFormat: format, mType: type, mFilter: filter, mWrap:wrap, mVFlip:flipY };
                                };

        me.SetSamplerFilter = function (te, filter, doGenerateMipsIfNeeded) 
                            {
                                if (te.mFilter === filter) return;

                                if (te.mType === me.TEXTYPE.T2D) 
                                {
                                    mGL.bindTexture(mGL.TEXTURE_2D, te.mObjectID);

                                    if (filter === me.FILTER.NONE) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                    }
                                    else if (filter === me.FILTER.LINEAR) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                    }
                                    else if (filter === me.FILTER.MIPMAP) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_2D);
                                    }
                                    else
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_2D);
                                    }

                                    mGL.bindTexture(mGL.TEXTURE_2D, null);

                                }
                                else if (te.mType === me.TEXTYPE.T3D) 
                                {
                                    mGL.bindTexture(mGL.TEXTURE_3D, te.mObjectID);

                                    if (filter === me.FILTER.NONE) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                    }
                                    else if (filter === me.FILTER.LINEAR) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                    }
                                    else if (filter === me.FILTER.MIPMAP) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_3D);
                                    }
                                    else
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_3D);
                                    }

                                    mGL.bindTexture(mGL.TEXTURE_3D, null);

                                }
                                else
                                {
                                    mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, te.mObjectID);

                                    if (filter === me.FILTER.NONE) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST);
                                    }
                                    else if (filter === me.FILTER.LINEAR) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR);
                                    }
                                    else if (filter === me.FILTER.MIPMAP) 
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.LINEAR);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.LINEAR_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_CUBE_MAP);
                                    }
                                    else
                                    {
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MAG_FILTER, mGL.NEAREST);
                                        mGL.texParameteri(mGL.TEXTURE_CUBE_MAP, mGL.TEXTURE_MIN_FILTER, mGL.NEAREST_MIPMAP_LINEAR);
                                        if( doGenerateMipsIfNeeded ) mGL.generateMipmap(mGL.TEXTURE_CUBE_MAP);
                                    }

                                    mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);
                                }


                                te.mFilter = filter;
                            };

        me.SetSamplerWrap = function (te, wrap)
                            {
                                if (te.mWrap === wrap) return;

                                var glWrap = mGL.REPEAT; if (wrap === me.TEXWRP.CLAMP) glWrap = mGL.CLAMP_TO_EDGE;

                                var id = te.mObjectID;

                                if (te.mType === me.TEXTYPE.T2D)
                                {
                                    mGL.bindTexture(mGL.TEXTURE_2D, id);
                                    mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_S, glWrap);
                                    mGL.texParameteri(mGL.TEXTURE_2D, mGL.TEXTURE_WRAP_T, glWrap);
                                    mGL.bindTexture(mGL.TEXTURE_2D, null);

                                }
                                else if (te.mType === me.TEXTYPE.T3D)
                                {
                                    mGL.bindTexture(mGL.TEXTURE_3D, id);
                                    mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_R, glWrap);
                                    mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_S, glWrap);
                                    mGL.texParameteri(mGL.TEXTURE_3D, mGL.TEXTURE_WRAP_T, glWrap);
                                    mGL.bindTexture(mGL.TEXTURE_3D, null);
                                }

                                te.mWrap = wrap;
                            };

        me.SetSamplerVFlip = function (te, vflip, image)
                            {
                                if (te.mVFlip === vflip) return;

                                var id = te.mObjectID;

                                if (te.mType === me.TEXTYPE.T2D)
                                {
                                    if( image != null)
                                    {
                                        mGL.activeTexture( mGL.TEXTURE0 );
                                        mGL.bindTexture(mGL.TEXTURE_2D, id);
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, vflip);
                                        var glFoTy = iFormatPI2GL( te.mFormat );
                                        mGL.texImage2D(mGL.TEXTURE_2D, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image);
                                        mGL.bindTexture(mGL.TEXTURE_2D, null);
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                    }
                                }
                                else if (te.mType === me.TEXTYPE.CUBEMAP)
                                {
                                    if( image != null)
                                    {
                                        var glFoTy = iFormatPI2GL( te.mFormat );
                                        mGL.activeTexture( mGL.TEXTURE0 );
                                        mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, id );
                                        mGL.pixelStorei( mGL.UNPACK_FLIP_Y_WEBGL, vflip);
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_X, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[0] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_X, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[1] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_Y, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, (vflip ? image[3] : image[2]) );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, (vflip ? image[2] : image[3]) );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_POSITIVE_Z, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[4] );
                                        mGL.texImage2D(  mGL.TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image[5] );
                                        mGL.bindTexture( mGL.TEXTURE_CUBE_MAP, null );
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                    }
        
                                }

                                te.mVFlip = vflip;
                            };

        me.CreateMipmaps =  function (te)
                            {
                                if( te.mType===me.TEXTYPE.T2D )
                                {
                                    mGL.activeTexture(mGL.TEXTURE0);
                                    mGL.bindTexture(mGL.TEXTURE_2D, te.mObjectID);
                                    mGL.generateMipmap(mGL.TEXTURE_2D);
                                    mGL.bindTexture(mGL.TEXTURE_2D, null);
                                }
                                else if( te.mType===me.TEXTYPE.CUBEMAP )
                                {
                                    mGL.activeTexture(mGL.TEXTURE0);
                                    mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, te.mObjectID);
                                    mGL.generateMipmap( mGL.TEXTURE_CUBE_MAP );
                                    mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);
                                }
                            };

        me.UpdateTexture =  function( tex, x0, y0, xres, yres, buffer )
                            {
                                var glFoTy = iFormatPI2GL( tex.mFormat );
                                if( tex.mType===me.TEXTYPE.T2D )
                                {
                                    mGL.activeTexture( mGL.TEXTURE0);
                                    mGL.bindTexture( mGL.TEXTURE_2D, tex.mObjectID );
                                    mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, tex.mVFlip );
                                    mGL.texSubImage2D( mGL.TEXTURE_2D, 0, x0, y0, xres, yres, glFoTy.mGLExternal, glFoTy.mGLType, buffer );
                                    mGL.bindTexture( mGL.TEXTURE_2D, null );
                                    mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                }
                            };

        me.UpdateTextureFromImage = function( tex, image )
                                {
                                    var glFoTy = iFormatPI2GL( tex.mFormat );
                                    if( tex.mType===me.TEXTYPE.T2D )
                                    {
                                        mGL.activeTexture( mGL.TEXTURE0 );
                                        mGL.bindTexture( mGL.TEXTURE_2D, tex.mObjectID );
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, tex.mVFlip );
                                        mGL.texImage2D(  mGL.TEXTURE_2D, 0, glFoTy.mGLFormat, glFoTy.mGLExternal, glFoTy.mGLType, image );
                                        mGL.bindTexture( mGL.TEXTURE_2D, null );
                                        mGL.pixelStorei(mGL.UNPACK_FLIP_Y_WEBGL, false);
                                    }
                                };

        me.DestroyTexture = function( te )
                            {
                                 mGL.deleteTexture( te.mObjectID );
                            };

        me.AttachTextures = function (num, t0, t1, t2, t3) 
                            {
                                if (num > 0 && t0 != null) 
                                {
                                    mGL.activeTexture(mGL.TEXTURE0);
                                         if (t0.mType === me.TEXTYPE.T2D) mGL.bindTexture(mGL.TEXTURE_2D, t0.mObjectID);
                                    else if (t0.mType === me.TEXTYPE.T3D) mGL.bindTexture(mGL.TEXTURE_3D, t0.mObjectID);
                                    else if (t0.mType === me.TEXTYPE.CUBEMAP) mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, t0.mObjectID);
                                }

                                if (num > 1 && t1 != null) 
                                {
                                    mGL.activeTexture(mGL.TEXTURE1);
                                         if (t1.mType === me.TEXTYPE.T2D) mGL.bindTexture(mGL.TEXTURE_2D, t1.mObjectID);
                                    else if (t1.mType === me.TEXTYPE.T3D) mGL.bindTexture(mGL.TEXTURE_3D, t1.mObjectID);
                                    else if (t1.mType === me.TEXTYPE.CUBEMAP) mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, t1.mObjectID);
                                }

                                if (num > 2 && t2 != null) 
                                {
                                    mGL.activeTexture(mGL.TEXTURE2);
                                         if (t2.mType === me.TEXTYPE.T2D) mGL.bindTexture(mGL.TEXTURE_2D, t2.mObjectID);
                                    else if (t2.mType === me.TEXTYPE.T3D) mGL.bindTexture(mGL.TEXTURE_3D, t2.mObjectID);
                                    else if (t2.mType === me.TEXTYPE.CUBEMAP) mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, t2.mObjectID);
                                }

                                if (num > 3 && t3 != null) 
                                {
                                    mGL.activeTexture(mGL.TEXTURE3);
                                         if (t3.mType === me.TEXTYPE.T2D) mGL.bindTexture(mGL.TEXTURE_2D, t3.mObjectID);
                                    else if (t3.mType === me.TEXTYPE.T3D) mGL.bindTexture(mGL.TEXTURE_3D, t3.mObjectID);
                                    else if (t3.mType === me.TEXTYPE.CUBEMAP) mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, t3.mObjectID);
                                }
                            };

        me.DettachTextures = function()
                             {
                                mGL.activeTexture(mGL.TEXTURE0);
                                mGL.bindTexture(mGL.TEXTURE_2D, null);
                                mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);

                                mGL.activeTexture(mGL.TEXTURE1);
                                mGL.bindTexture(mGL.TEXTURE_2D, null);
                                mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);

                                mGL.activeTexture(mGL.TEXTURE2);
                                mGL.bindTexture(mGL.TEXTURE_2D, null);
                                mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);

                                mGL.activeTexture(mGL.TEXTURE3);
                                mGL.bindTexture(mGL.TEXTURE_2D, null);
                                mGL.bindTexture(mGL.TEXTURE_CUBE_MAP, null);
                             };

        me.CreateRenderTarget = function ( color0, color1, color2, color3, depth, wantZbuffer )
                                {
                                    var id =  mGL.createFramebuffer();
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, id);

                                    if (depth === null)
                                    {
                                        if( wantZbuffer===true )
                                        {
                                            var zb = mGL.createRenderbuffer();
                                            mGL.bindRenderbuffer(mGL.RENDERBUFFER, zb);
                                            mGL.renderbufferStorage(mGL.RENDERBUFFER, mGL.DEPTH_COMPONENT16, color0.mXres, color0.mYres);

                                            mGL.framebufferRenderbuffer(mGL.FRAMEBUFFER, mGL.DEPTH_ATTACHMENT, mGL.RENDERBUFFER, zb);
                                        }
                                    }
                                    else
                                    {
                                        mGL.framebufferTexture2D(mGL.FRAMEBUFFER, mGL.DEPTH_ATTACHMENT, mGL.TEXTURE_2D, depth.mObjectID, 0);
                                    }

                                    if( color0 !=null ) mGL.framebufferTexture2D(mGL.FRAMEBUFFER, mGL.COLOR_ATTACHMENT0, mGL.TEXTURE_2D, color0.mObjectID, 0);

                                    if (mGL.checkFramebufferStatus(mGL.FRAMEBUFFER) != mGL.FRAMEBUFFER_COMPLETE)
                                        return null;

                                    mGL.bindRenderbuffer(mGL.RENDERBUFFER, null);
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                                    return { mObjectID: id, mTex0: color0 };
                                };

        me.DestroyRenderTarget = function ( tex )
                                 {
                                     mGL.deleteFramebuffer(tex.mObjectID);
                                 };

        me.SetRenderTarget = function (tex)
                             {
                                if( tex===null )
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                                else
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, tex.mObjectID);

                                //mGL.drawBuffers([mGL.COLOR_ATTACHMENT0, mGL.COLOR_ATTACHMENT1]);
                             };

        me.CreateRenderTargetNew = function ( wantColor0, wantZbuffer, xres, yres, samples )
                                {
                                    var id =  mGL.createFramebuffer();
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, id);

                                    if( wantZbuffer===true )
                                    {
                                        var zb = mGL.createRenderbuffer();
                                        mGL.bindRenderbuffer(mGL.RENDERBUFFER, zb);

                                        if( samples==1 )
                                        mGL.renderbufferStorage(mGL.RENDERBUFFER, mGL.DEPTH_COMPONENT16, xres, yres);
                                        else
                                        mGL.renderbufferStorageMultisample(mGL.RENDERBUFFER, samples, mGL.DEPTH_COMPONENT16, xres, yres);
                                        mGL.framebufferRenderbuffer(mGL.FRAMEBUFFER, mGL.DEPTH_ATTACHMENT, mGL.RENDERBUFFER, zb);
                                    }

                                    if( wantColor0 )
                                    {
                                        var cb = mGL.createRenderbuffer();
                                        mGL.bindRenderbuffer(mGL.RENDERBUFFER, cb);
                                        if( samples==1 )
                                        mGL.renderbufferStorage(mGL.RENDERBUFFER, mGL.RGBA8, xres, yres);
                                        else
                                        mGL.renderbufferStorageMultisample(mGL.RENDERBUFFER, samples, mGL.RGBA8, xres, yres);
                                        mGL.framebufferRenderbuffer(mGL.FRAMEBUFFER, mGL.COLOR_ATTACHMENT0, mGL.RENDERBUFFER, cb);
                                    }

                                    if (mGL.checkFramebufferStatus(mGL.FRAMEBUFFER) != mGL.FRAMEBUFFER_COMPLETE)
                                    {
                                        return null;
                                    }
                                    mGL.bindRenderbuffer(mGL.RENDERBUFFER, null);
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                                    return { mObjectID: id, mXres: xres, mYres:yres, mTex0: color0 };
                                };

        me.CreateRenderTargetCubeMap = function ( color0, depth, wantZbuffer )
                                {
                                    var id =  mGL.createFramebuffer();
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, id);

                                    if (depth === null)
                                    {
                                        if( wantZbuffer===true )
                                        {
                                            var zb = mGL.createRenderbuffer();
                                            mGL.bindRenderbuffer(mGL.RENDERBUFFER, zb);
                                            mGL.renderbufferStorage(mGL.RENDERBUFFER, mGL.DEPTH_COMPONENT16, color0.mXres, color0.mYres);
                                            mGL.framebufferRenderbuffer(mGL.FRAMEBUFFER, mGL.DEPTH_ATTACHMENT, mGL.RENDERBUFFER, zb);
                                        }
                                    }
                                    else
                                    {
                                        mGL.framebufferTexture2D(mGL.FRAMEBUFFER, mGL.DEPTH_ATTACHMENT, mGL.TEXTURE_2D, depth.mObjectID, 0);
                                    }

                                    if( color0 !=null ) mGL.framebufferTexture2D(mGL.FRAMEBUFFER, mGL.COLOR_ATTACHMENT0, mGL.TEXTURE_CUBE_MAP_POSITIVE_X, color0.mObjectID, 0);

                                    if (mGL.checkFramebufferStatus(mGL.FRAMEBUFFER) != mGL.FRAMEBUFFER_COMPLETE)
                                        return null;

                                    mGL.bindRenderbuffer(mGL.RENDERBUFFER, null);
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                                    return { mObjectID: id, mTex0: color0 };
                                };

        me.SetRenderTargetCubeMap = function (fbo, face)
                             {
                                if( fbo===null )
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                                else
                                {
                                    mGL.bindFramebuffer(mGL.FRAMEBUFFER, fbo.mObjectID);
                                    mGL.framebufferTexture2D(mGL.FRAMEBUFFER, mGL.COLOR_ATTACHMENT0, mGL.TEXTURE_CUBE_MAP_POSITIVE_X+face, fbo.mTex0.mObjectID, 0);
                                }
                             };


        me.BlitRenderTarget = function( dst, src )
                                {
                                    mGL.bindFramebuffer(mGL.READ_FRAMEBUFFER, src.mObjectID);
                                    mGL.bindFramebuffer(mGL.DRAW_FRAMEBUFFER, dst.mObjectID);
                                    mGL.clearBufferfv(mGL.COLOR, 0, [0.0, 0.0, 0.0, 1.0]);
                                    mGL.blitFramebuffer( 0, 0, src.mXres, src.mYres,
                                                         0, 0, src.mXres, src.mYres,
                                                         mGL.COLOR_BUFFER_BIT, mGL.LINEAR
                                    );
                                };

        me.SetViewport = function( vp )
                         {
                              mGL.viewport( vp[0], vp[1], vp[2], vp[3] );
                         };

        me.SetWriteMask = function( c0, c1, c2, c3, z )
                          {
                              mGL.depthMask(z);
                              mGL.colorMask(c0,c0,c0,c0);
                          };

        me.SetState = function( stateName, stateValue )
                      {
                            if (stateName === me.RENDSTGATE.WIREFRAME)
                            {
                                if( stateValue ) mGL.polygonMode( mGL.FRONT_AND_BACK, mGL.LINE );
                                else             mGL.polygonMode( mGL.FRONT_AND_BACK, mGL.FILL );
                            }
                            else if (stateName === me.RENDSTGATE.FRONT_FACE)
                            {
                                if( stateValue ) mGL.cullFace( mGL.BACK );
                                else             mGL.cullFace( mGL.FRONT );
                            }
                            else if (stateName === me.RENDSTGATE.CULL_FACE)
                            {
                                if( stateValue ) mGL.enable( mGL.CULL_FACE );
                                else             mGL.disable( mGL.CULL_FACE );
                            }
                            else if (stateName === me.RENDSTGATE.DEPTH_TEST)
                            {
                                if( stateValue ) mGL.enable( mGL.DEPTH_TEST );
                                else             mGL.disable( mGL.DEPTH_TEST );
                            }
                            else if (stateName === me.RENDSTGATE.ALPHA_TO_COVERAGE)
                            {
                                if( stateValue ) { mGL.enable(  mGL.SAMPLE_ALPHA_TO_COVERAGE ); }
                                else             { mGL.disable( mGL.SAMPLE_ALPHA_TO_COVERAGE ); }
                            }
                      };

        me.SetMultisample = function( v)
                    {
                        if( v===true )
                        {
                            mGL.enable(mGL.SAMPLE_COVERAGE);
                            mGL.sampleCoverage(1.0, false);
                        }
                        else
                        {
                            mGL.disable(mGL.SAMPLE_COVERAGE);
                        }
                    };

        me.GetTranslatedShaderSource = function (shader) 
                          {
                            if( mGL===null ) return null;
                            if( mDebugShader===null ) return null;
                            let vfs = mGL.getAttachedShaders(shader.mProgram);
                            let str = mDebugShader.getTranslatedShaderSource(vfs[1]);
                            let parts = str.split("GLSL END"); str = (parts.length<2) ? str : parts[1];
                            return str;
                          };

        me.CreateShader = function (vsSource, fsSource, preventCache, forceSynch, onResolve) 
                          {
                            if( mGL===null ) return;

                            var vs = mGL.createShader( mGL.VERTEX_SHADER   );
                            var fs = mGL.createShader( mGL.FRAGMENT_SHADER );

                            vsSource = mShaderHeader[0] + vsSource;
                            fsSource = mShaderHeader[1] + fsSource;

                            if( preventCache )
                            {
                                let vran = Math.random().toString(36).substring(7);
                                let fran = Math.random().toString(36).substring(7);
                                vsSource += "\n#define K" + vran + "\n";
                                fsSource += "\n#define K" + fran + "\n";
                            }

                            var timeStart = getRealTime();

                            mGL.shaderSource(vs, vsSource);
                            mGL.shaderSource(fs, fsSource);
                            mGL.compileShader(vs);
                            mGL.compileShader(fs);

                            var pr = mGL.createProgram();
                            mGL.attachShader(pr, vs);
                            mGL.attachShader(pr, fs);
                            mGL.linkProgram(pr);

                            //-------------
                            let checkErrors = function()
                            {
                                if (!mGL.getProgramParameter(pr, mGL.LINK_STATUS))
                                {
                                    // vs error
                                    if (!mGL.getShaderParameter(vs, mGL.COMPILE_STATUS))
                                    {
                                        let vsLog = mGL.getShaderInfoLog(vs);
                                        onResolve(false, { mErrorType: 0, mErrorStr: vsLog });
                                        mGL.deleteProgram(pr);
                                    }
                                    // fs error
                                    else if (!mGL.getShaderParameter(fs, mGL.COMPILE_STATUS))
                                    {
                                        let fsLog = mGL.getShaderInfoLog(fs);
                                        onResolve(false, { mErrorType: 1, mErrorStr: fsLog });
                                        mGL.deleteProgram(pr);
                                    }
                                    // link error
                                    else
                                    {
                                        let infoLog = mGL.getProgramInfoLog(pr);
                                        onResolve(false, { mErrorType: 2, mErrorStr: infoLog });
                                        mGL.deleteProgram(pr);
                                    }
                                }
                                // no errors
                                else
                                {
                                    let compilationTime = getRealTime() - timeStart;
                                    onResolve(true, { mProgram: pr, mTime: compilationTime });
                                }
                            };

                            // check compilation
                            if (mAsynchCompile === null || forceSynch===true )
                            {
                                checkErrors();
                            }
                            else
                            {
                                let loopCheckCompletion = function ()
                                {
                                    if( mGL.getProgramParameter(pr, mAsynchCompile.COMPLETION_STATUS_KHR) === true )
                                        checkErrors();
                                    else
                                        setTimeout(loopCheckCompletion, 10);
                                };
                                setTimeout(loopCheckCompletion, 10);
                            }
                        };

        me.AttachShader = function( shader )
                          {
                                if( shader===null )
                                {
                                    mBindedShader = null;
                                    mGL.useProgram( null );
                                }
                                else
                                {
                                    mBindedShader = shader;
                                    mGL.useProgram(shader.mProgram);
                                }
                          };

        me.DetachShader = function ()
                        {
                            mGL.useProgram(null);
                        };

        me.DestroyShader = function( tex )
                        {
                            mGL.deleteProgram(tex.mProgram);
                        };

        me.GetAttribLocation = function (shader, name)
                        {
                            return mGL.getAttribLocation(shader.mProgram, name);
                        };

        me.SetShaderConstantLocation = function (shader, name)
                        {
                            return mGL.getUniformLocation(shader.mProgram, name);
                        };

        me.SetShaderConstantMat4F = function( uname, params, istranspose )
                        {
                            var program = mBindedShader;

                            let pos = mGL.getUniformLocation( program.mProgram, uname );
                            if( pos===null )
                                return false;

                            if( istranspose===false )
                            {
                                var tmp = new Float32Array( [ params[0], params[4], params[ 8], params[12],
                                                              params[1], params[5], params[ 9], params[13],
                                                              params[2], params[6], params[10], params[14],
                                                              params[3], params[7], params[11], params[15] ] );
	                            mGL.uniformMatrix4fv(pos,false,tmp);
                            }
                            else
                                mGL.uniformMatrix4fv(pos,false,new Float32Array(params) );
                            return true;
                        };

        me.SetShaderConstant1F_Pos = function(pos, x)
                        {
                            mGL.uniform1f(pos, x);
                            return true;
                        };

        me.SetShaderConstant1FV_Pos = function(pos, x)
                        {
                            mGL.uniform1fv(pos, x);
                            return true;
                        };

        me.SetShaderConstant1F = function( uname, x )
                        {
                            var pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null)
                                return false;
                            mGL.uniform1f(pos, x);
                            return true;
                        };

        me.SetShaderConstant1I = function(uname, x)
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null)
                                return false;
                            mGL.uniform1i(pos, x);
                            return true;
                        };
        me.SetShaderConstant1I_Pos = function(pos, x)
                        {
                            mGL.uniform1i(pos, x);
                            return true;
                        };


        me.SetShaderConstant2F = function(uname, x)
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null)
                                return false;
                            mGL.uniform2fv(pos, x);
                            return true;
                        };

        me.SetShaderConstant3F = function(uname, x, y, z)
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null)
                                return false;
                            mGL.uniform3f(pos, x, y, z);
                            return true;
                        };

        me.SetShaderConstant1FV = function(uname, x)
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null)
                                return false;
                            mGL.uniform1fv(pos, new Float32Array(x));
                            return true;
                        };

        me.SetShaderConstant3FV = function(uname, x) 
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null) return false;
                            mGL.uniform3fv(pos, new Float32Array(x) );
                            return true;
                        };

        me.SetShaderConstant4FV = function(uname, x) 
                        {
                            let pos = mGL.getUniformLocation(mBindedShader.mProgram, uname);
                            if (pos === null) return false;
                            mGL.uniform4fv(pos, new Float32Array(x) );
                            return true;
                        };

        me.SetShaderTextureUnit = function( uname, unit )
                        {
                            var program = mBindedShader;
                            let pos = mGL.getUniformLocation(program.mProgram, uname);
                            if (pos === null) return false;
                            mGL.uniform1i(pos, unit);
                            return true;
                        };

        me.CreateVertexArray = function( data, mode )
                        {
                            let id = mGL.createBuffer();
                            mGL.bindBuffer(mGL.ARRAY_BUFFER, id);
                            if (mode === me.BUFTYPE.STATIC)
                                mGL.bufferData(mGL.ARRAY_BUFFER, data, mGL.STATIC_DRAW);
                            else
                                mGL.bufferData(mGL.ARRAY_BUFFER, data, mGL.DYNAMIC_DRAW);
                            return { mObject: id };
                        };

        me.CreateIndexArray = function( data, mode )
                        {
                            let id = mGL.createBuffer();
                            mGL.bindBuffer(mGL.ELEMENT_ARRAY_BUFFER, id );
                            if (mode === me.BUFTYPE.STATIC)
                                mGL.bufferData(mGL.ELEMENT_ARRAY_BUFFER, data, mGL.STATIC_DRAW);
                            else
                                mGL.bufferData(mGL.ELEMENT_ARRAY_BUFFER, data, mGL.DYNAMIC_DRAW);
                            return { mObject: id };
                        };

        me.DestroyArray = function( tex )
                        {
                            mGL.destroyBuffer(tex.mObject);
                        };

        me.AttachVertexArray = function( tex, attribs, pos )
                        {
                            let shader = mBindedShader;

                            mGL.bindBuffer( mGL.ARRAY_BUFFER, tex.mObject);

                            var num = attribs.mChannels.length;
                            var stride = attribs.mStride;

                            var offset = 0;
                            for (var i = 0; i < num; i++)
                            {
                                var id = pos[i];
                                mGL.enableVertexAttribArray(id);
                                var dtype = mGL.FLOAT;
                                var dsize = 4;
                                     if( attribs.mChannels[i].mType === me.TYPE.UINT8   ) { dtype = mGL.UNSIGNED_BYTE;  dsize = 1; }
                                else if( attribs.mChannels[i].mType === me.TYPE.UINT16  ) { dtype = mGL.UNSIGNED_SHORT; dsize = 2; }
                                else if( attribs.mChannels[i].mType === me.TYPE.FLOAT32 ) { dtype = mGL.FLOAT;          dsize = 4; }
                                mGL.vertexAttribPointer(id, attribs.mChannels[i].mNumComponents, dtype, attribs.mChannels[i].mNormalize, stride, offset);
                                offset += attribs.mChannels[i].mNumComponents * dsize;
                            }
                        };

        me.AttachIndexArray = function( tex )
                        {
                            mGL.bindBuffer(mGL.ELEMENT_ARRAY_BUFFER, tex.mObject);
                        };

        me.DetachVertexArray = function (tex, attribs)
                        {
                            let num = attribs.mChannels.length;
                            for (let i = 0; i < num; i++)
                                mGL.disableVertexAttribArray(i);
                            mGL.bindBuffer(mGL.ARRAY_BUFFER, null);
                        };

        me.DetachIndexArray = function( tex )
                        {
                            mGL.bindBuffer(mGL.ELEMENT_ARRAY_BUFFER, null);
                        };

        me.DrawPrimitive = function( typeOfPrimitive, num, useIndexArray, numInstances )
                        {
                            let glType = mGL.POINTS;
                            if( typeOfPrimitive===me.PRIMTYPE.POINTS ) glType = mGL.POINTS;
                            if( typeOfPrimitive===me.PRIMTYPE.LINES ) glType = mGL.LINES;
                            if( typeOfPrimitive===me.PRIMTYPE.LINE_LOOP ) glType = mGL.LINE_LOOP;
                            if( typeOfPrimitive===me.PRIMTYPE.LINE_STRIP ) glType = mGL.LINE_STRIP;
                            if( typeOfPrimitive===me.PRIMTYPE.TRIANGLES ) glType = mGL.TRIANGLES;
                            if( typeOfPrimitive===me.PRIMTYPE.TRIANGLE_STRIP ) glType = mGL.TRIANGLE_STRIP;

                            if( numInstances<=1 )
                            {
  	                            if( useIndexArray ) mGL.drawElements( glType, num, mGL.UNSIGNED_SHORT, 0 );
	                            else                mGL.drawArrays( glType, 0, num );
                            }
                            else
                            {
                                mGL.drawArraysInstanced(glType, 0, num, numInstances);
                                mGL.drawElementsInstanced( glType, num, mGL.UNSIGNED_SHORT, 0, numInstances);
                            }
                        };


        me.DrawFullScreenTriangle_XY = function( vpos )
                        {
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_Tri );
                            mGL.vertexAttribPointer( vpos, 2, mGL.FLOAT, false, 0, 0 );
                            mGL.enableVertexAttribArray( vpos );
                            mGL.drawArrays( mGL.TRIANGLES, 0, 3 );
                            mGL.disableVertexAttribArray( vpos );
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, null );
                        };


        me.DrawUnitQuad_XY = function( vpos )
                        {
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_Quad );
                            mGL.vertexAttribPointer( vpos, 2, mGL.FLOAT, false, 0, 0 );
                            mGL.enableVertexAttribArray( vpos );
                            mGL.drawArrays( mGL.TRIANGLES, 0, 6 );
                            mGL.disableVertexAttribArray( vpos );
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, null );
                        };

        me.DrawUnitCube_XYZ_NOR = function( vpos )
                        {
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_CubePosNor );
                            mGL.vertexAttribPointer( vpos[0], 3, mGL.FLOAT, false, 0, 0 );
                            mGL.vertexAttribPointer( vpos[1], 3, mGL.FLOAT, false, 0, 0 );
                            mGL.enableVertexAttribArray( vpos[0] );
                            mGL.enableVertexAttribArray( vpos[1] );
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 0, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 4, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 8, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 12, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 16, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 20, 4);
                            mGL.disableVertexAttribArray( vpos[0] );
                            mGL.disableVertexAttribArray( vpos[1] );
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, null );
                        }

        me.DrawUnitCube_XYZ = function( vpos )
                        {
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, mVBO_CubePos );
                            mGL.vertexAttribPointer( vpos, 3, mGL.FLOAT, false, 0, 0 );
                            mGL.enableVertexAttribArray( vpos );
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 0, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 4, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 8, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 12, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 16, 4);
                            mGL.drawArrays(mGL.TRIANGLE_STRIP, 20, 4);
                            mGL.disableVertexAttribArray( vpos );
                            mGL.bindBuffer( mGL.ARRAY_BUFFER, null );
                        }

        me.SetBlend = function( enabled )
                    {
                        if( enabled )
                        {
                            mGL.enable( mGL.BLEND );
                            mGL.blendEquationSeparate( mGL.FUNC_ADD, mGL.FUNC_ADD );
                            mGL.blendFuncSeparate( mGL.SRC_ALPHA, mGL.ONE_MINUS_SRC_ALPHA, mGL.ONE, mGL.ONE_MINUS_SRC_ALPHA );
                        }
                        else
                        {
                            mGL.disable( mGL.BLEND );
                        }
                    };

        me.GetPixelData = function( data, offset, xres, yres )
                        {
                            mGL.readPixels(0, 0, xres, yres, mGL.RGBA, mGL.UNSIGNED_BYTE, data, offset);
                        };

        me.GetPixelDataRenderTarget = function( obj, data, xres, yres )
                        {
                            mGL.bindFramebuffer(mGL.FRAMEBUFFER, obj.mObjectID);
                            mGL.readBuffer(mGL.COLOR_ATTACHMENT0);
                            mGL.readPixels(0, 0, xres, yres, mGL.RGBA, mGL.FLOAT, data, 0);
                            mGL.bindFramebuffer(mGL.FRAMEBUFFER, null);
                        };
    return me;
}
//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piShading
//
//==============================================================================

function smoothstep(a, b, x)
{
    x = (x - a) / (b - a);
    if (x < 0) x = 0; else if (x > 1) x = 1;
    return x * x * (3.0 - 2.0 * x);
}

function clamp01(x)
{
    if( x < 0.0 ) x = 0.0;
    if( x > 1.0 ) x = 1.0;
    return x;
}

function clamp(x, a, b)
{
    if( x < a ) x = a;
    if( x > b ) x = b;
    return x;
}

function screen(a, b)
{
    return 1.0 - (1.0 - a) * (1.0 - b);
}

function parabola(x)
{
    return 4.0 * x * (1.0 - x);
}

function min(a, b)
{
    return (a < b) ? a : b;
}

function max(a, b)
{
    return (a > b) ? a : b;
}

function noise( x )
{
    function grad(i, j, x, y)
    {
        var h = 7 * i + 131 * j;
        h = (h << 13) ^ h;
        h = (h * (h * h * 15731 + 789221) + 1376312589);

        var rx = (h & 0x20000000) ? x : -x;
        var ry = (h & 0x10000000) ? y : -y;

        return rx + ry;
    }

    var i = [ Math.floor(x[0]), Math.floor(x[1]) ];
    var f = [ x[0] - i[0], x[1] - i[1] ];
    var w = [ f[0]*f[0]*(3.0-2.0*f[0]), f[1]*f[1]*(3.0-2.0*f[1]) ];

    var a = grad( i[0]+0, i[1]+0, f[0]+0.0, f[1]+0.0 );
    var b = grad( i[0]+1, i[1]+0, f[0]-1.0, f[1]+0.0 );
    var c = grad( i[0]+0, i[1]+1, f[0]+0.0, f[1]-1.0 );
    var d = grad( i[0]+1, i[1]+1, f[0]-1.0, f[1]-1.0 );

    return a + (b-a)*w[0] + (c-a)*w[1] + (a-b-c+d)*w[0]*w[1];
}//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piVecTypes
//
//==============================================================================


function vec3( a, b, c )
{
    return [ a, b, c ];
}

function add( a, b )
{
    return [ a[0]+b[0], a[1]+b[1], a[2]+b[2] ];
}

function sub( a, b )
{
    return [ a[0]-b[0], a[1]-b[1], a[2]-b[2] ];
}

function mul( a, s ) 
{
    return [ a[0]*s, a[1]*s, a[2]*s ];
}

function cross( a, b )
{
    return [ a[1]*b[2] - a[2]*b[1],
             a[2]*b[0] - a[0]*b[2],
             a[0]*b[1] - a[1]*b[0] ];
}

function dot( a, b ) 
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

function normalize( v ) 
{
    var is = 1.0 / Math.sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    return [ v[0]*is, v[1]*is, v[2]*is ];
}

function createCirclePoint( cen, uuu, vvv, rad, s, t )
{
    return [ cen[0] + rad*(uuu[0]*s + vvv[0]*t),
             cen[1] + rad*(uuu[1]*s + vvv[1]*t),
             cen[2] + rad*(uuu[2]*s + vvv[2]*t) ];
}

function createTangent( a, b, c )
{
    var cb = normalize( [ c[0]-b[0], c[1]-b[1], c[2]-b[2] ] );
    var ba = normalize( [ b[0]-a[0], b[1]-a[1], b[2]-a[2] ] );
    return normalize( [ ba[0]+cb[0], ba[1]+cb[1], ba[2]+cb[2] ] );

}

//===================================

function vec4( a, b, c, d )
{
    return [ a, b, c, d ];
}

function getXYZ( v )
{
    return [ v[0], v[1], v[2] ];
}

//===================================

function setIdentity()
{
    return [ 1.0, 0.0, 0.0, 0.0,
             0.0, 1.0, 0.0, 0.0,
             0.0, 0.0, 1.0, 0.0,
             0.0, 0.0, 0.0, 1.0 ];
}

function setRotationX( t )
{
    var sint = Math.sin(t);
    var cost = Math.cos(t);

    return [ 1.0,   0.0,   0.0, 0.0,
             0.0,  cost, -sint, 0.0, 
             0.0,  sint,  cost, 0.0,
             0.0,   0.0,   0.0, 1.0 ];
}

function setRotationY( t )
{
    var sint = Math.sin(t);
    var cost = Math.cos(t);

    return [ cost, 0.0, -sint, 0.0,
              0.0, 1.0,   0.0, 0.0, 
             sint, 0.0,  cost, 0.0,
              0.0, 0.0,   0.0, 1.0 ];
}

function extractRotationEuler( m)
{
    var res = [];
    if (m[0] == 1.0)
    {
        res[0] = Math.atan2(m[2], m[11]);
        res[1] = 0.0;
        res[2] = 0.0;

    }
    else if (m[0] == -1.0)
    {
        res[0] = Math.atan2( m[2], m[11]);
        res[1] = 0.0;
        res[2] = 0.0;
    }
    else
    {
        res[0] = Math.atan2( -m[9], m[10]);
        res[1] = Math.atan2( m[8], Math.sqrt(m[9]*m[9] + m[10]*m[10]));
        res[2] = Math.atan2( m[4], m[0]);
    }
    return res;
}

function setFromQuaternion( q )
{
    var ww = q[3]*q[3];
    var xx = q[0]*q[0];
    var yy = q[1]*q[1];
    var zz = q[2]*q[2];

    return [ ww+xx-yy-zz,                   2.0*(q[0]*q[1] - q[3]*q[2]), 2.0*(q[0]*q[2] + q[3]*q[1]), 0.0,
               2.0*(q[0]*q[1] + q[3]*q[2]),   ww-xx+yy-zz,                 2.0*(q[1]*q[2] - q[3]*q[0]), 0.0,
               2.0*(q[0]*q[2] - q[3]*q[1]),   2.0*(q[1]*q[2] + q[3]*q[0]), ww-xx-yy+zz,                 0.0,
               0.0,                           0.0,                         0.0,                         1.0 ];
}

function setPerspective( fovy, aspect, znear, zfar )
{
    var tan = Math.tan(fovy * Math.PI/180.0);
    var x = 1.0 / (tan*aspect);
    var y = 1.0 / (tan);
    var c = -(zfar + znear) / ( zfar - znear);
    var d = -(2.0 * zfar * znear) / (zfar - znear);

    return [ x,    0.0,  0.0,  0.0,
             0.0,  y,    0.0,  0.0,
             0.0,  0.0,  c,     d,
             0.0,  0.0, -1.0,  0.0 ];
}

function setLookAt( eye, tar, up )
{
    var dir = [ -tar[0]+eye[0], -tar[1]+eye[1], -tar[2]+eye[2] ];

	var m00 = dir[2]*up[1] - dir[1]*up[2]; 
    var m01 = dir[0]*up[2] - dir[2]*up[0];
    var m02 = dir[1]*up[0] - dir[0]*up[1];
    var im = 1.0/Math.sqrt( m00*m00 + m01*m01 + m02*m02 );
    m00 *= im;
    m01 *= im;
    m02 *= im;

	var m04 = m02*dir[1] - m01*dir[2]; 
    var m05 = m00*dir[2] - m02*dir[0];
    var m06 = m01*dir[0] - m00*dir[1];
    im = 1.0/Math.sqrt( m04*m04 + m05*m05 + m06*m06 );
    m04 *= im;
    m05 *= im;
    m06 *= im;

	var m08 = dir[0];
	var m09 = dir[1];
	var m10 = dir[2];
    im = 1.0/Math.sqrt( m08*m08 + m09*m09 + m10*m10 );
    m08 *= im;
    m09 *= im;
    m10 *= im;

	var m03 = -(m00*eye[0] + m01*eye[1] + m02*eye[2] );
	var m07 = -(m04*eye[0] + m05*eye[1] + m06*eye[2] );
	var m11 = -(m08*eye[0] + m09*eye[1] + m10*eye[2] );

    return [ m00, m01, m02, m03,
             m04, m05, m06, m07,
             m08, m09, m10, m11,
             0.0, 0.0, 0.0, 1.0 ];
}


function setOrtho( left, right, bottom, top, znear, zfar )
{
   var x = 2.0 / (right - left);
   var y = 2.0 / (top - bottom);
   var a = (right + left) / (right - left);
   var b = (top + bottom) / (top - bottom);
   var c = -2.0 / (zfar - znear);
   var d = -(zfar + znear) / ( zfar - znear);

   return [  x, 0.0, 0.0,   a,
           0.0,   y, 0.0,   b,
           0.0, 0.0,   c,   d,
           0.0, 0.0, 0.0, 1.0 ];
}

function setTranslation( p )
{
    return [ 1.0, 0.0, 0.0, p[0],
             0.0, 1.0, 0.0, p[1],
             0.0, 0.0, 1.0, p[2],
             0.0, 0.0, 0.0, 1.0 ];
}

function setScale( s )
{
    return [ s[0], 0.0,  0.0,  0.0,
             0.0,  s[1], 0.0,  0.0,
             0.0,  0.0,  s[2], 0.0,
             0.0,  0.0,  0.0,  1.0];
}

function setProjection( fov, znear, zfar )
{
    var x = 2.0 / (fov[3]+fov[2]);
    var y = 2.0 / (fov[0]+fov[1]);
    var a = (fov[3]-fov[2]) / (fov[3]+fov[2]);
    var b = (fov[0]-fov[1]) / (fov[0]+fov[1]);
    var c = -(zfar + znear) / ( zfar - znear);
    var d = -(2.0*zfar*znear) / (zfar - znear);
    return [   x, 0.0,    a, 0.0,
             0.0,   y,    b, 0.0,
             0.0, 0.0,    c,   d,
             0.0, 0.0, -1.0, 0.0 ];
   // inverse is:
   //return mat4x4( 1.0/x, 0.0f,  0.0f,   a/x,
   //               0.0f,  1.0/y, 0.0f,   b/x,
   //               0.0f,  0.0f,  0.0f,   -1.0,
   //               0.0f,  0.0f,  1.0f/d, c/d );
}


function invertFast( m )
{
    var inv = [
   
             m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10],

             -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10],

             m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6],

             -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6],

             -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10],

             m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10],

             -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6],


             m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6],


             m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9],



             -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9],

              m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5],

              -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5],

              -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9],

              m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9],

              -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5],

              m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5] ];

    var det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    det = 1.0/det;

    for( var i = 0; i<16; i++ ) inv[i] = inv[i] * det;

    return inv;
}

function matMul( a, b )
{
    var res = [];
    for( var i=0; i<4; i++ )
    {
        var x = a[4*i+0];
        var y = a[4*i+1];
        var z = a[4*i+2];
        var w = a[4*i+3];

        res[4*i+0] = x * b[ 0] + y * b[ 4] + z * b[ 8] + w * b[12];
        res[4*i+1] = x * b[ 1] + y * b[ 5] + z * b[ 9] + w * b[13];
        res[4*i+2] = x * b[ 2] + y * b[ 6] + z * b[10] + w * b[14];
        res[4*i+3] = x * b[ 3] + y * b[ 7] + z * b[11] + w * b[15];
    }

    return res;
}

function matMulpoint( m, v )
{
    return [ m[0]*v[0] + m[1]*v[1] + m[ 2]*v[2] + m[ 3],
             m[4]*v[0] + m[5]*v[1] + m[ 6]*v[2] + m[ 7],
             m[8]*v[0] + m[9]*v[1] + m[10]*v[2] + m[11] ];
}

function matMulvec( m, v )
{
    return [ m[0]*v[0] + m[1]*v[1] + m[ 2]*v[2],
             m[4]*v[0] + m[5]*v[1] + m[ 6]*v[2],
             m[8]*v[0] + m[9]*v[1] + m[10]*v[2] ];
}


function bound3( infi )
{
    return [ infi, -infi, infi, -infi, infi, -infi ];
}

function bound3_include( a, p )
{
    return [
        (p[0]<a[0]) ? p[0] : a[0],
        (p[0]>a[1]) ? p[0] : a[1],
        (p[1]<a[2]) ? p[1] : a[2],
        (p[1]>a[3]) ? p[1] : a[3],
        (p[2]<a[4]) ? p[2] : a[4],
        (p[2]>a[5]) ? p[2] : a[5] ];
}

function bound3_center( b )
{
    return [ 0.5*(b[0]+b[1]),
             0.5*(b[2]+b[3]),
             0.5*(b[4]+b[5]) ];
}

function bound3_radius( b )
{
    return [ 0.5*(b[1]-b[0]),
             0.5*(b[3]-b[2]),
             0.5*(b[5]-b[4]) ];
}
//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piWebVR
//
//==============================================================================

function WebVR( isVREnabledCallback, canvasElement )
{
    isVREnabledCallback(false);
    /*


    this.mSupportVR = false;
    this.mHMD = null;

    var me = this;
    var listVRDisplays = function (vrdevs) {
        for (var i = 0; i < vrdevs.length; i++)
        {
            if (vrdevs[i] instanceof VRDisplay)
            {
                me.mHMD = vrdevs[i];
                console.log("WebVR is available.");
                console.log(me.mHMD);
                break;
            }
        }

        isVREnabledCallback(true);
        me.mSupportVR = true;
    }

    if (navigator.getVRDisplays)
    {
        navigator.getVRDisplays().then(listVRDisplays);
    }

    isVREnabledCallback(false);
    this.mCanvas = canvasElement;
    */
}

WebVR.prototype.IsSupported = function()
{
    return false;
    //return this.mSupportVR;
}

WebVR.prototype.GetData = function( id )
{
    return {};
    /*
    var frameData = new VRFrameData();
    var s = this.mHMD.getFrameData(frameData);
    var ss = frameData.pose;
    
    var fovL = this.mHMD.getEyeParameters("left");
    var fovR = this.mHMD.getEyeParameters( "right" );

    // camera info
    var cPos = vec3(0.0, 0.0, 0.0);
    if (ss.position)
        cPos = vec3(-ss.position[0], -ss.position[1], -ss.position[2]);
    var rot = vec4(0.0, 0.0, 0.0, 0.0);
    if (ss.orientation)
        rot = vec4(ss.orientation[0], ss.orientation[1], ss.orientation[2], ss.orientation[3]);
    var cRot = setFromQuaternion(rot);
    var cTra = setTranslation(cPos);
    var cMat = matMul(invertFast(cRot), cTra);
    
    // per eye info
    var lTra = setTranslation( vec3(-fovL.offset[0], -fovL.offset[1], -fovL.offset[2]) );
    var lMat = matMul(lTra, cMat);
    var lPrj = [ Math.tan( fovL.fieldOfView.upDegrees * Math.PI/180.0),
                 Math.tan(fovL.fieldOfView.downDegrees * Math.PI / 180.0),
                 Math.tan(fovL.fieldOfView.leftDegrees * Math.PI / 180.0),
                 Math.tan(fovL.fieldOfView.rightDegrees * Math.PI / 180.0)];

    var rTra = setTranslation(vec3(-fovR.offset[0], -fovR.offset[1], -fovR.offset[2]));
    var rMat = matMul(rTra, cMat);
    var rPrj = [Math.tan(fovR.fieldOfView.upDegrees * Math.PI / 180.0),
                 Math.tan(fovR.fieldOfView.downDegrees * Math.PI / 180.0),
                 Math.tan(fovR.fieldOfView.leftDegrees * Math.PI / 180.0),
                 Math.tan(fovR.fieldOfView.rightDegrees * Math.PI / 180.0)];

    return {
        mCamera   : { mCamera: cMat },
        mLeftEye  : { mVP:[0,0,fovL.renderWidth,fovL.renderHeight], mProjection:lPrj, mCamera:lMat },
        mRightEye : { mVP:[fovR.renderWidth/2,0,fovR.renderWidth,fovR.renderHeight], mProjection:rPrj, mCamera:rMat }
           };
    */
}

WebVR.prototype.Enable = function( id )
{
    /*
    this.mHMD.requestPresent([{ source: this.mCanvas }]).then(
        function ()
        {
        },
        function (err)
        {
            console.log("webVR : requestPresent failed.");
        }
    );
    */
}

WebVR.prototype.Disable = function( id )
{
    /*
    if (!this.mHMD.isPresenting)
    {
        return;
    }

    this.mHMD.exitPresent().then(
        function ()
        {
        },
        function (err)
        {
            console.log("webVR : exitPresent failed.");
        }
    );
    */
}

WebVR.prototype.RequestAnimationFrame = function (id)
{
    //this.mHMD.requestAnimationFrame(id);
}

WebVR.prototype.IsPresenting = function (id)
{
    /*
    if (this.mHMD.isPresenting)
    {
        return true;
    }
    */
    return false;
}

WebVR.prototype.Finish = function (id)
{
    /*
    if (this.mHMD.isPresenting)
    {
        this.mHMD.submitFrame();
    }
    */
}

//==============================================================================
//
// piLibs 2015-2017 - http://www.iquilezles.org/www/material/piLibs/piLibs.htm
//
// piWebUtils
//
//==============================================================================


// RequestAnimationFrame
window.requestAnimFrame = ( function () { return window.requestAnimationFrame    || window.webkitRequestAnimationFrame ||
                                                 window.mozRequestAnimationFrame || window.oRequestAnimationFrame ||
                                                 window.msRequestAnimationFrame  || function( cb ) { window.setTimeout(cb,1000/60); };
                                        } )();

// performance.now
window.getRealTime = ( function() { if ("performance" in window ) return function() { return window.performance.now(); }
                                                                  return function() { return (new Date()).getTime(); }
                                  } )();

window.URL = window.URL || window.webkitURL;

navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;

function htmlEntities(str) 
{
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g,'&apos;');
}

function piDisableTouch()
{
    document.body.addEventListener('touchstart', function(e){ e.preventDefault(); });
}

var piGetTime = function ( timestamp )
{
    if (timestamp == 0)
        return "";
    return new Date(timestamp * 1000).toISOString().substr(0, 10);
}

function piGetCoords( obj )
{
    var x = 0;
    var y = 0; 
    do
    {
         x += obj.offsetLeft;
         y += obj.offsetTop;
    }while( obj = obj.offsetParent );

    return { mX:x, mY:y };
}

function piGetMouseCoords( ev, canvasElement )
{
    var pos = piGetCoords(canvasElement );
    var mcx =                        (ev.pageX - pos.mX) * canvasElement.width / canvasElement.offsetWidth;
    var mcy = canvasElement.height - (ev.pageY - pos.mY) * canvasElement.height / canvasElement.offsetHeight;

    return { mX: mcx, mY: mcy };

}

function piGetSourceElement( e )
{
    var ele = null;
    if( e.target )     ele = e.target;
    if( e.srcElement ) ele = e.srcElement;
    return ele;
}

function piRequestFullScreen( ele )
{
    if( ele==null ) ele =   document.documentElement;
         if( ele.requestFullscreen       ) ele.requestFullscreen();
    else if( ele.msRequestFullscreen     ) ele.msRequestFullscreen();
    else if( ele.mozRequestFullScreen    ) ele.mozRequestFullScreen();
    else if( ele.webkitRequestFullscreen ) ele.webkitRequestFullscreen( Element.ALLOW_KEYBOARD_INPUT );
}

function piIsFullScreen()
{
    return document.fullscreen || document.mozFullScreen || document.webkitIsFullScreen || document.msFullscreenElement || false;
}

function piExitFullScreen()
{
       if( document.exitFullscreen       ) document.exitFullscreen();
  else if( document.msExitFullscreen     ) document.msExitFullscreen();
  else if( document.mozCancelFullScreen  ) document.mozCancelFullScreen();
  else if( document.webkitExitFullscreen ) document.webkitExitFullscreen();
}

function piIsMobile()
{
    return (navigator.userAgent.match(/Android/i) ||
            navigator.userAgent.match(/webOS/i) ||
            navigator.userAgent.match(/iPhone/i) ||
            navigator.userAgent.match(/iPad/i) ||
            navigator.userAgent.match(/iPod/i) ||
            navigator.userAgent.match(/BlackBerry/i) ||
            navigator.userAgent.match(/Windows Phone/i)) ? true : false;
}

function piCreateGlContext( cv, useAlpha, useDepth, usePreserveBuffer, useSupersampling )
{
    var opts = { alpha: useAlpha, 
                 depth: useDepth, 
                 stencil: false, 
                 premultipliedAlpha: false, 
                 antialias: useSupersampling, 
                 preserveDrawingBuffer: usePreserveBuffer, 
                 powerPreference: "high-performance" }; // "low_power", "high_performance", "default"

    var gl = null;
    if( gl === null) gl = cv.getContext( "webgl2", opts );
    if( gl === null) gl = cv.getContext( "experimental-webgl2", opts );
    if( gl === null) gl = cv.getContext( "webgl", opts );
    if( gl === null) gl = cv.getContext( "experimental-webgl", opts );

    return gl;
}

function piCreateAudioContext()
{
    var res = null;
    try
    {
        if( window.AudioContext ) res = new AudioContext();
        if( res==null && window.webkitAudioContext ) res = new webkitAudioContext();
    }
    catch( e )
    {
        res = null;
    }
    return res;
}

function piHexColorToRGB(str) // "#ff3041"
{
    var rgb = parseInt(str.slice(1), 16);
    var r = (rgb >> 16) & 255;
    var g = (rgb >> 8) & 255;
    var b = (rgb >> 0) & 255;
    return [r, g, b];
}

function piCreateFPSCounter()
{
    var mFrame;
    var mTo;
    var mFPS;

    var iReset = function( time )
    {
        mFrame = 0;
        mTo = time;
        mFPS = 60.0;
    }

    var iCount = function( time )
    {
        mFrame++;

        if( (time-mTo)>500.0 )
        {
            mFPS = 1000.0*mFrame/(time-mTo);
            mFrame = 0;
            mTo = time;
            return true;
        }
        return false;
    }

    var iGetFPS = function()
    {
        return mFPS;
    }
    
    return { Reset : iReset, Count : iCount, GetFPS : iGetFPS };
}

function piCanMediaRecorded(canvas)
{
    if (typeof window.MediaRecorder !== 'function' || typeof canvas.captureStream !== 'function') {
        return false;
    }
    return true;
}

function piCreateMediaRecorder(isRecordingCallback, canvas) 
{
    if (piCanMediaRecorded(canvas) == false)
    {
        return null;
    }
    
    var options = { audioBitsPerSecond : 0, videoBitsPerSecond : 8000000 }; 
	     if (MediaRecorder.isTypeSupported('video/webm;codecs=h264')) options.mimeType = 'video/webm;codecs=h264';
    else if (MediaRecorder.isTypeSupported('video/webm;codecs=vp9' )) options.mimeType = 'video/webm;codecs=vp9';
    else if (MediaRecorder.isTypeSupported('video/webm;codecs=vp8' )) options.mimeType = 'video/webm;codecs=vp8';
    else                                                              options.mimeType = 'video/webm;';

    var mediaRecorder = new MediaRecorder(canvas.captureStream(), options);
    var chunks = [];
    
    mediaRecorder.ondataavailable = function(e) 
    {
        if (e.data.size > 0) 
        {
            chunks.push(e.data);
        }
    };
 
    mediaRecorder.onstart = function(){ 
        isRecordingCallback( true );
    };
    
    mediaRecorder.onstop = function()
    {
         isRecordingCallback( false );
         let blob     = new Blob(chunks, {type: "video/webm"});
         chunks       = [];
         let videoURL = window.URL.createObjectURL(blob);
         let url      = window.URL.createObjectURL(blob);
         let a        = document.createElement("a");
         document.body.appendChild(a);
         a.style      = "display: none";
         a.href       = url;
         a.download   = "capture.webm";
         a.click();
         window.URL.revokeObjectURL(url);
     };
    
    return mediaRecorder;
}

function piExportToEXR(width, height, numComponents, type, bytes)
{
    var bytesPerComponent = 0;
    if      (type=="Uint")   bytesPerComponent = 4; 
    else if (type=="Half")   bytesPerComponent = 2;
    else if (type=="Float")  bytesPerComponent = 4;

    var tHeader = 258 + (18 * numComponents + 1);
    var tTable = 8 * height;
    var tScanlines = height * (4 + 4 + (numComponents * bytesPerComponent * width));
    var tTotal = tHeader + tTable + tScanlines;

    //console.log("    header size = " + tHeader);
    //console.log("    table size = " + tTable);
    //console.log("    scanlines size = " + tScanlines);
    //console.log("    total = " + tTotal);

    var buffer = new ArrayBuffer(tTotal); 
    var data = new DataView(buffer);

    // Header
    {
        // Header : 4 bytes -> 0x76, 0x2f, 0x31, 0x01
        var c = 0;
        data.setUint8 (c++, 0x76);
        data.setUint8 (c++, 0x2f);
        data.setUint8 (c++, 0x31);
        data.setUint8 (c++, 0x01);

        // Version : 4 bytes -> 2, 0, 0, 0
        data.setUint8 (c++, 0x02);
        data.setUint8 (c++, 0x0);
        data.setUint8 (c++, 0x0);
        data.setUint8 (c++, 0x0);
        
        // Write channel info
        // Write attribute name : "channels"
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x68); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x73);
            data.setUint8 (c++, 0x0);

            // Write attribute type : "chlist"
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x68); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x00);

            // Write attribute size : 18 x 3 + 1 = 55
            var attribSize = 18 * numComponents + 1;
            data.setUint8 (c++, attribSize); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            var i;
            for (i = 0; i < numComponents; i++)
            {
                // Attribute : "B" (42) "G" (47) "R" (52)
                if (i==0)       data.setUint8 (c++, 0x42);
                else if (i==1)  data.setUint8 (c++, 0x47);
                else if (i==2)  data.setUint8 (c++, 0x52);
                data.setUint8 (c++, 0x00);
                
                // Value : Float (2), Half (1), Uint (0)
                if      (type=="Uint")   data.setUint8 (c++, 0x00); 
                else if (type=="Half")   data.setUint8 (c++, 0x01);
                else if (type=="Float")  data.setUint8 (c++, 0x02);
                data.setUint8 (c++, 0x00);
                data.setUint8 (c++, 0x00);
                data.setUint8 (c++, 0x00);

                // Plinear
                data.setUint8 (c++, 0x01);

                // Reserved
                data.setUint8 (c++, 0x00); 
                data.setUint8 (c++, 0x00); 
                data.setUint8 (c++, 0x00); 

                // X sampling
                data.setUint8 (c++, 0x01); 
                data.setUint8 (c++, 0x00); 
                data.setUint8 (c++, 0x00); 
                data.setUint8 (c++, 0x00); 
                
                // Y sampling
                data.setUint8 (c++, 0x01);
                data.setUint8 (c++, 0x00);
                data.setUint8 (c++, 0x00);
                data.setUint8 (c++, 0x00);
            }
            // End attribute
            data.setUint8 (c++, 0x00);
    
        // Write attribute name : "compression"
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x6d); 
            data.setUint8 (c++, 0x70); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x00);

            // Write attribute type : "compression"
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x6d); 
            data.setUint8 (c++, 0x70); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x00);

            // Write attribute size : "1"
            data.setUint8 (c++, 0x01); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            // Write attribute value : "0" (None)
            data.setUint8 (c++, 0x00);

        // datawindow
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x57); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x77); 
            data.setUint8 (c++, 0x00); 

            // box2i
            data.setUint8 (c++, 0x62); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x78); 
            data.setUint8 (c++, 0x32); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x00);

            // size 16
            data.setUint8 (c++, 0x10); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            // value 0 0 3 2
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 

            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            
            data.setUint32 (c, width-1, true);
            c += 4;
            
            data.setUint32 (c, height-1, true); 
            c += 4;

        // displayWindow
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x70); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x79); 
            data.setUint8 (c++, 0x57); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x77); 
            data.setUint8 (c++, 0x00);

            // box2i
            data.setUint8 (c++, 0x62); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x78); 
            data.setUint8 (c++, 0x32); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x00); 

            // size 16
            data.setUint8 (c++, 0x10); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            // value 0 0 3 2
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            
            data.setUint32 (c, width-1, true);
            c += 4;
            
            data.setUint32 (c, height-1, true); 
            c += 4;

        // lineOrder
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x4f); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x00); 
            
            // lineOrder
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x4f); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x00); 
            
            // size
            data.setUint8 (c++, 0x01);
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);
            
            // value 
            data.setUint8 (c++, 0x00);

        // PixelAspectRatio
            data.setUint8 (c++, 0x70); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x78); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x41); 
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x70); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x52); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x00); 

            // float
            data.setUint8 (c++, 0x66); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x00);
        
            // size 4
            data.setUint8 (c++, 0x04); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            // value 1.0
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x80); 
            data.setUint8 (c++, 0x3f);
        
        // screenWindowCenter
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x63);
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x57); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x77); 
            data.setUint8 (c++, 0x43); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x6e);
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x00);

            // v2f
            data.setUint8 (c++, 0x76); 
            data.setUint8 (c++, 0x32); 
            data.setUint8 (c++, 0x66); 
            data.setUint8 (c++, 0x00);

            // size 8
            data.setUint8 (c++, 0x08); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

            // value 0 0
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00);

        // screenWindowWidth
            data.setUint8 (c++, 0x73); 
            data.setUint8 (c++, 0x63); 
            data.setUint8 (c++, 0x72); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x65); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x57); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x6e); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x77); 
            data.setUint8 (c++, 0x57); 
            data.setUint8 (c++, 0x69); 
            data.setUint8 (c++, 0x64); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x68); 
            data.setUint8 (c++, 0x00); 
            
            // float
            data.setUint8 (c++, 0x66); 
            data.setUint8 (c++, 0x6c); 
            data.setUint8 (c++, 0x6f); 
            data.setUint8 (c++, 0x61); 
            data.setUint8 (c++, 0x74); 
            data.setUint8 (c++, 0x00); 

            // size
            data.setUint8 (c++, 0x04); 
            data.setUint8 (c++, 0x00);
            data.setUint8 (c++, 0x00);
            data.setUint8 (c++, 0x00);

            // value
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x00); 
            data.setUint8 (c++, 0x80); 
            data.setUint8 (c++, 0x3f);

        // End of header
        data.setUint8 (c++, 0x00);
    }
    //console.log("header size = " + c);

    // Scanline table
    var initc = c + height * 8;
    for (var scanline = 0 ; scanline < height ; scanline ++)
    {
        var jump = initc + scanline * (8 + width * bytesPerComponent * numComponents); 
        data.setUint32 (c, jump, true);
        c += 4; 

        data.setUint32 (c, 0x00, true);
        c += 4;
    }
    //console.log("header + scanlines table size = " + c);

    // Scanlines
    for (var scanline = 0 ; scanline < height ; scanline ++)
    {
        // Scanline
        data.setUint32(c, scanline, true);
        c += 4;

        // size 24
        var size = width * numComponents * bytesPerComponent; 
        data.setUint32(c, size, true);
        c += 4;

        var numComponentsSource = 4; // number of components in the SOURCE image
        for (var component = 0; component < numComponents ; component ++) 
        {
            for (var pixel = 0 ; pixel < width ; pixel ++) 
            {
                // flip vertical, so we read OpenGL buffers without JS image flipping
                var v = bytes[(height-1-scanline) * width *numComponentsSource + pixel * numComponentsSource + (2-component)];
                if      (type=="Float") data.setFloat32(c, v, true);
                else if (type=="Half")  data.setUint16(c, v, true);

                c += bytesPerComponent;
            }
        }
    }
    //console.log("total size = " + c);
    return new Blob([buffer], {type: 'application/octet-stream'});
}


function piExportToWAV(numSamples, rate, bits, numChannels, words)
{
    let numBytes = numSamples * numChannels * bits/8;

    let buffer = new ArrayBuffer(44 + numBytes); 
    let data = new DataView(buffer);

    {
        data.setUint32( 0, 0x46464952, true );  // RIFF
        data.setUint32( 4, numBytes + 36, true);
        {
            data.setUint32( 8, 0x45564157, true );  // WAV_WAVE
            data.setUint32( 12, 0x20746D66, true );  // WAV_FMT
            {
                data.setUint32( 16, 16, true);
                data.setUint16( 20, 1, true ); // WAV_FORMAT_PCM
                data.setUint16( 22, numChannels, true);
                data.setUint32( 24, rate, true);
                data.setUint32( 28, rate*numChannels*bits / 8, true);
                data.setUint16( 32, numChannels*bits / 8, true);
                data.setUint16( 34, bits, true);
            }

            data.setUint32( 36, 0x61746164, true);  // WAV_DATA
            {
                data.setUint32( 40, numBytes, true);
                let numWords = numSamples * numChannels;
                for(let i=0; i<numWords; i++ )
                {
                    data.setInt16( 44 + i*2, words[i], true );
                }
            }
        }
    }


    //console.log("total size = " + c);
    return new Blob([buffer], {type: 'application/octet-stream'});
}

function piTriggerDownload(name, blob)
{
    let url = URL.createObjectURL(blob);
    let aElement = document.createElement("a");
    aElement.href     = url;
    aElement.target   = "_self";
    aElement.download = name;
    document.body.appendChild(aElement);
    aElement.click();
    document.body.removeChild(aElement);
}//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

// This is CodeMirror (https://codemirror.net), a code editor
// implemented in JavaScript on top of the browser's DOM.
//
// You can find some technical background for some of the code below
// at http://marijnhaverbeke.nl/blog/#cm-internals .


(function (global, factory) {

  typeof exports === 'object' && typeof module !== 'undefined' ? module.exports = factory() :
  typeof define === 'function' && define.amd ? define(factory) :
  (global.CodeMirror = factory());
}(this, (function () { 'use strict';

  // Kludges for bugs and behavior differences that can't be feature
  // detected are enabled based on userAgent etc sniffing.
  var userAgent = navigator.userAgent;
  var platform = navigator.platform;

  var gecko = /gecko\/\d/i.test(userAgent);
  var ie_upto10 = /MSIE \d/.test(userAgent);
  var ie_11up = /Trident\/(?:[7-9]|\d{2,})\..*rv:(\d+)/.exec(userAgent);
  var edge = /Edge\/(\d+)/.exec(userAgent);
  var ie = ie_upto10 || ie_11up || edge;
  var ie_version = ie && (ie_upto10 ? document.documentMode || 6 : +(edge || ie_11up)[1]);
  var webkit = !edge && /WebKit\//.test(userAgent);
  var qtwebkit = webkit && /Qt\/\d+\.\d+/.test(userAgent);
  var chrome = !edge && /Chrome\//.test(userAgent);
  var presto = /Opera\//.test(userAgent);
  var safari = /Apple Computer/.test(navigator.vendor);
  var mac_geMountainLion = /Mac OS X 1\d\D([8-9]|\d\d)\D/.test(userAgent);
  var phantom = /PhantomJS/.test(userAgent);

  var ios = !edge && /AppleWebKit/.test(userAgent) && /Mobile\/\w+/.test(userAgent);
  var android = /Android/.test(userAgent);
  // This is woefully incomplete. Suggestions for alternative methods welcome.
  var mobile = ios || android || /webOS|BlackBerry|Opera Mini|Opera Mobi|IEMobile/i.test(userAgent);
  var mac = ios || /Mac/.test(platform);
  var chromeOS = /\bCrOS\b/.test(userAgent);
  var windows = /win/i.test(platform);

  var presto_version = presto && userAgent.match(/Version\/(\d*\.\d*)/);
  if (presto_version) { presto_version = Number(presto_version[1]); }
  if (presto_version && presto_version >= 15) { presto = false; webkit = true; }
  // Some browsers use the wrong event properties to signal cmd/ctrl on OS X
  var flipCtrlCmd = mac && (qtwebkit || presto && (presto_version == null || presto_version < 12.11));
  var captureRightClick = gecko || (ie && ie_version >= 9);

  function classTest(cls) { return new RegExp("(^|\\s)" + cls + "(?:$|\\s)\\s*") }

  var rmClass = function(node, cls) {
    var current = node.className;
    var match = classTest(cls).exec(current);
    if (match) {
      var after = current.slice(match.index + match[0].length);
      node.className = current.slice(0, match.index) + (after ? match[1] + after : "");
    }
  };

  function removeChildren(e) {
    for (var count = e.childNodes.length; count > 0; --count)
      { e.removeChild(e.firstChild); }
    return e
  }

  function removeChildrenAndAdd(parent, e) {
    return removeChildren(parent).appendChild(e)
  }

  function elt(tag, content, className, style) {
    var e = document.createElement(tag);
    if (className) { e.className = className; }
    if (style) { e.style.cssText = style; }
    if (typeof content == "string") { e.appendChild(document.createTextNode(content)); }
    else if (content) { for (var i = 0; i < content.length; ++i) { e.appendChild(content[i]); } }
    return e
  }
  // wrapper for elt, which removes the elt from the accessibility tree
  function eltP(tag, content, className, style) {
    var e = elt(tag, content, className, style);
    e.setAttribute("role", "presentation");
    return e
  }

  var range;
  if (document.createRange) { range = function(node, start, end, endNode) {
    var r = document.createRange();
    r.setEnd(endNode || node, end);
    r.setStart(node, start);
    return r
  }; }
  else { range = function(node, start, end) {
    var r = document.body.createTextRange();
    try { r.moveToElementText(node.parentNode); }
    catch(e) { return r }
    r.collapse(true);
    r.moveEnd("character", end);
    r.moveStart("character", start);
    return r
  }; }

  function contains(parent, child) {
    if (child.nodeType == 3) // Android browser always returns false when child is a textnode
      { child = child.parentNode; }
    if (parent.contains)
      { return parent.contains(child) }
    do {
      if (child.nodeType == 11) { child = child.host; }
      if (child == parent) { return true }
    } while (child = child.parentNode)
  }

  function activeElt() {
    // IE and Edge may throw an "Unspecified Error" when accessing document.activeElement.
    // IE < 10 will throw when accessed while the page is loading or in an iframe.
    // IE > 9 and Edge will throw when accessed in an iframe if document.body is unavailable.
    var activeElement;
    try {
      activeElement = document.activeElement;
    } catch(e) {
      activeElement = document.body || null;
    }
    while (activeElement && activeElement.shadowRoot && activeElement.shadowRoot.activeElement)
      { activeElement = activeElement.shadowRoot.activeElement; }
    return activeElement
  }

  function addClass(node, cls) {
    var current = node.className;
    if (!classTest(cls).test(current)) { node.className += (current ? " " : "") + cls; }
  }
  function joinClasses(a, b) {
    var as = a.split(" ");
    for (var i = 0; i < as.length; i++)
      { if (as[i] && !classTest(as[i]).test(b)) { b += " " + as[i]; } }
    return b
  }

  var selectInput = function(node) { node.select(); };
  if (ios) // Mobile Safari apparently has a bug where select() is broken.
    { selectInput = function(node) { node.selectionStart = 0; node.selectionEnd = node.value.length; }; }
  else if (ie) // Suppress mysterious IE10 errors
    { selectInput = function(node) { try { node.select(); } catch(_e) {} }; }

  function bind(f) {
    var args = Array.prototype.slice.call(arguments, 1);
    return function(){return f.apply(null, args)}
  }

  function copyObj(obj, target, overwrite) {
    if (!target) { target = {}; }
    for (var prop in obj)
      { if (obj.hasOwnProperty(prop) && (overwrite !== false || !target.hasOwnProperty(prop)))
        { target[prop] = obj[prop]; } }
    return target
  }

  // Counts the column offset in a string, taking tabs into account.
  // Used mostly to find indentation.
  function countColumn(string, end, tabSize, startIndex, startValue) {
    if (end == null) {
      end = string.search(/[^\s\u00a0]/);
      if (end == -1) { end = string.length; }
    }
    for (var i = startIndex || 0, n = startValue || 0;;) {
      var nextTab = string.indexOf("\t", i);
      if (nextTab < 0 || nextTab >= end)
        { return n + (end - i) }
      n += nextTab - i;
      n += tabSize - (n % tabSize);
      i = nextTab + 1;
    }
  }

  var Delayed = function() {this.id = null;};
  Delayed.prototype.set = function (ms, f) {
    clearTimeout(this.id);
    this.id = setTimeout(f, ms);
  };

  function indexOf(array, elt) {
    for (var i = 0; i < array.length; ++i)
      { if (array[i] == elt) { return i } }
    return -1
  }

  // Number of pixels added to scroller and sizer to hide scrollbar
  var scrollerGap = 30;

  // Returned or thrown by various protocols to signal 'I'm not
  // handling this'.
  var Pass = {toString: function(){return "CodeMirror.Pass"}};

  // Reused option objects for setSelection & friends
  var sel_dontScroll = {scroll: false}, sel_mouse = {origin: "*mouse"}, sel_move = {origin: "+move"};

  // The inverse of countColumn -- find the offset that corresponds to
  // a particular column.
  function findColumn(string, goal, tabSize) {
    for (var pos = 0, col = 0;;) {
      var nextTab = string.indexOf("\t", pos);
      if (nextTab == -1) { nextTab = string.length; }
      var skipped = nextTab - pos;
      if (nextTab == string.length || col + skipped >= goal)
        { return pos + Math.min(skipped, goal - col) }
      col += nextTab - pos;
      col += tabSize - (col % tabSize);
      pos = nextTab + 1;
      if (col >= goal) { return pos }
    }
  }

  var spaceStrs = [""];
  function spaceStr(n) {
    while (spaceStrs.length <= n)
      { spaceStrs.push(lst(spaceStrs) + " "); }
    return spaceStrs[n]
  }

  function lst(arr) { return arr[arr.length-1] }

  function map(array, f) {
    var out = [];
    for (var i = 0; i < array.length; i++) { out[i] = f(array[i], i); }
    return out
  }

  function insertSorted(array, value, score) {
    var pos = 0, priority = score(value);
    while (pos < array.length && score(array[pos]) <= priority) { pos++; }
    array.splice(pos, 0, value);
  }

  function nothing() {}

  function createObj(base, props) {
    var inst;
    if (Object.create) {
      inst = Object.create(base);
    } else {
      nothing.prototype = base;
      inst = new nothing();
    }
    if (props) { copyObj(props, inst); }
    return inst
  }

  var nonASCIISingleCaseWordChar = /[\u00df\u0587\u0590-\u05f4\u0600-\u06ff\u3040-\u309f\u30a0-\u30ff\u3400-\u4db5\u4e00-\u9fcc\uac00-\ud7af]/;
  function isWordCharBasic(ch) {
    return /\w/.test(ch) || ch > "\x80" &&
      (ch.toUpperCase() != ch.toLowerCase() || nonASCIISingleCaseWordChar.test(ch))
  }
  function isWordChar(ch, helper) {
    if (!helper) { return isWordCharBasic(ch) }
    if (helper.source.indexOf("\\w") > -1 && isWordCharBasic(ch)) { return true }
    return helper.test(ch)
  }

  function isEmpty(obj) {
    for (var n in obj) { if (obj.hasOwnProperty(n) && obj[n]) { return false } }
    return true
  }

  // Extending unicode characters. A series of a non-extending char +
  // any number of extending chars is treated as a single unit as far
  // as editing and measuring is concerned. This is not fully correct,
  // since some scripts/fonts/browsers also treat other configurations
  // of code points as a group.
  var extendingChars = /[\u0300-\u036f\u0483-\u0489\u0591-\u05bd\u05bf\u05c1\u05c2\u05c4\u05c5\u05c7\u0610-\u061a\u064b-\u065e\u0670\u06d6-\u06dc\u06de-\u06e4\u06e7\u06e8\u06ea-\u06ed\u0711\u0730-\u074a\u07a6-\u07b0\u07eb-\u07f3\u0816-\u0819\u081b-\u0823\u0825-\u0827\u0829-\u082d\u0900-\u0902\u093c\u0941-\u0948\u094d\u0951-\u0955\u0962\u0963\u0981\u09bc\u09be\u09c1-\u09c4\u09cd\u09d7\u09e2\u09e3\u0a01\u0a02\u0a3c\u0a41\u0a42\u0a47\u0a48\u0a4b-\u0a4d\u0a51\u0a70\u0a71\u0a75\u0a81\u0a82\u0abc\u0ac1-\u0ac5\u0ac7\u0ac8\u0acd\u0ae2\u0ae3\u0b01\u0b3c\u0b3e\u0b3f\u0b41-\u0b44\u0b4d\u0b56\u0b57\u0b62\u0b63\u0b82\u0bbe\u0bc0\u0bcd\u0bd7\u0c3e-\u0c40\u0c46-\u0c48\u0c4a-\u0c4d\u0c55\u0c56\u0c62\u0c63\u0cbc\u0cbf\u0cc2\u0cc6\u0ccc\u0ccd\u0cd5\u0cd6\u0ce2\u0ce3\u0d3e\u0d41-\u0d44\u0d4d\u0d57\u0d62\u0d63\u0dca\u0dcf\u0dd2-\u0dd4\u0dd6\u0ddf\u0e31\u0e34-\u0e3a\u0e47-\u0e4e\u0eb1\u0eb4-\u0eb9\u0ebb\u0ebc\u0ec8-\u0ecd\u0f18\u0f19\u0f35\u0f37\u0f39\u0f71-\u0f7e\u0f80-\u0f84\u0f86\u0f87\u0f90-\u0f97\u0f99-\u0fbc\u0fc6\u102d-\u1030\u1032-\u1037\u1039\u103a\u103d\u103e\u1058\u1059\u105e-\u1060\u1071-\u1074\u1082\u1085\u1086\u108d\u109d\u135f\u1712-\u1714\u1732-\u1734\u1752\u1753\u1772\u1773\u17b7-\u17bd\u17c6\u17c9-\u17d3\u17dd\u180b-\u180d\u18a9\u1920-\u1922\u1927\u1928\u1932\u1939-\u193b\u1a17\u1a18\u1a56\u1a58-\u1a5e\u1a60\u1a62\u1a65-\u1a6c\u1a73-\u1a7c\u1a7f\u1b00-\u1b03\u1b34\u1b36-\u1b3a\u1b3c\u1b42\u1b6b-\u1b73\u1b80\u1b81\u1ba2-\u1ba5\u1ba8\u1ba9\u1c2c-\u1c33\u1c36\u1c37\u1cd0-\u1cd2\u1cd4-\u1ce0\u1ce2-\u1ce8\u1ced\u1dc0-\u1de6\u1dfd-\u1dff\u200c\u200d\u20d0-\u20f0\u2cef-\u2cf1\u2de0-\u2dff\u302a-\u302f\u3099\u309a\ua66f-\ua672\ua67c\ua67d\ua6f0\ua6f1\ua802\ua806\ua80b\ua825\ua826\ua8c4\ua8e0-\ua8f1\ua926-\ua92d\ua947-\ua951\ua980-\ua982\ua9b3\ua9b6-\ua9b9\ua9bc\uaa29-\uaa2e\uaa31\uaa32\uaa35\uaa36\uaa43\uaa4c\uaab0\uaab2-\uaab4\uaab7\uaab8\uaabe\uaabf\uaac1\uabe5\uabe8\uabed\udc00-\udfff\ufb1e\ufe00-\ufe0f\ufe20-\ufe26\uff9e\uff9f]/;
  function isExtendingChar(ch) { return ch.charCodeAt(0) >= 768 && extendingChars.test(ch) }

  // Returns a number from the range [`0`; `str.length`] unless `pos` is outside that range.
  function skipExtendingChars(str, pos, dir) {
    while ((dir < 0 ? pos > 0 : pos < str.length) && isExtendingChar(str.charAt(pos))) { pos += dir; }
    return pos
  }

  // Returns the value from the range [`from`; `to`] that satisfies
  // `pred` and is closest to `from`. Assumes that at least `to`
  // satisfies `pred`. Supports `from` being greater than `to`.
  function findFirst(pred, from, to) {
    // At any point we are certain `to` satisfies `pred`, don't know
    // whether `from` does.
    var dir = from > to ? -1 : 1;
    for (;;) {
      if (from == to) { return from }
      var midF = (from + to) / 2, mid = dir < 0 ? Math.ceil(midF) : Math.floor(midF);
      if (mid == from) { return pred(mid) ? from : to }
      if (pred(mid)) { to = mid; }
      else { from = mid + dir; }
    }
  }

  // BIDI HELPERS

  function iterateBidiSections(order, from, to, f) {
    if (!order) { return f(from, to, "ltr", 0) }
    var found = false;
    for (var i = 0; i < order.length; ++i) {
      var part = order[i];
      if (part.from < to && part.to > from || from == to && part.to == from) {
        f(Math.max(part.from, from), Math.min(part.to, to), part.level == 1 ? "rtl" : "ltr", i);
        found = true;
      }
    }
    if (!found) { f(from, to, "ltr"); }
  }

  var bidiOther = null;
  function getBidiPartAt(order, ch, sticky) {
    var found;
    bidiOther = null;
    for (var i = 0; i < order.length; ++i) {
      var cur = order[i];
      if (cur.from < ch && cur.to > ch) { return i }
      if (cur.to == ch) {
        if (cur.from != cur.to && sticky == "before") { found = i; }
        else { bidiOther = i; }
      }
      if (cur.from == ch) {
        if (cur.from != cur.to && sticky != "before") { found = i; }
        else { bidiOther = i; }
      }
    }
    return found != null ? found : bidiOther
  }

  // Bidirectional ordering algorithm
  // See http://unicode.org/reports/tr9/tr9-13.html for the algorithm
  // that this (partially) implements.

  // One-char codes used for character types:
  // L (L):   Left-to-Right
  // R (R):   Right-to-Left
  // r (AL):  Right-to-Left Arabic
  // 1 (EN):  European Number
  // + (ES):  European Number Separator
  // % (ET):  European Number Terminator
  // n (AN):  Arabic Number
  // , (CS):  Common Number Separator
  // m (NSM): Non-Spacing Mark
  // b (BN):  Boundary Neutral
  // s (B):   Paragraph Separator
  // t (S):   Segment Separator
  // w (WS):  Whitespace
  // N (ON):  Other Neutrals

  // Returns null if characters are ordered as they appear
  // (left-to-right), or an array of sections ({from, to, level}
  // objects) in the order in which they occur visually.
  var bidiOrdering = (function() {
    // Character types for codepoints 0 to 0xff
    var lowTypes = "bbbbbbbbbtstwsbbbbbbbbbbbbbbssstwNN%%%NNNNNN,N,N1111111111NNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNNNLLLLLLLLLLLLLLLLLLLLLLLLLLNNNNbbbbbbsbbbbbbbbbbbbbbbbbbbbbbbbbb,N%%%%NNNNLNNNNN%%11NLNNN1LNNNNNLLLLLLLLLLLLLLLLLLLLLLLNLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLN";
    // Character types for codepoints 0x600 to 0x6f9
    var arabicTypes = "nnnnnnNNr%%r,rNNmmmmmmmmmmmrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrmmmmmmmmmmmmmmmmmmmmmnnnnnnnnnn%nnrrrmrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrmmmmmmmnNmmmmmmrrmmNmmmmrr1111111111";
    function charType(code) {
      if (code <= 0xf7) { return lowTypes.charAt(code) }
      else if (0x590 <= code && code <= 0x5f4) { return "R" }
      else if (0x600 <= code && code <= 0x6f9) { return arabicTypes.charAt(code - 0x600) }
      else if (0x6ee <= code && code <= 0x8ac) { return "r" }
      else if (0x2000 <= code && code <= 0x200b) { return "w" }
      else if (code == 0x200c) { return "b" }
      else { return "L" }
    }

    var bidiRE = /[\u0590-\u05f4\u0600-\u06ff\u0700-\u08ac]/;
    var isNeutral = /[stwN]/, isStrong = /[LRr]/, countsAsLeft = /[Lb1n]/, countsAsNum = /[1n]/;

    function BidiSpan(level, from, to) {
      this.level = level;
      this.from = from; this.to = to;
    }

    return function(str, direction) {
      var outerType = direction == "ltr" ? "L" : "R";

      if (str.length == 0 || direction == "ltr" && !bidiRE.test(str)) { return false }
      var len = str.length, types = [];
      for (var i = 0; i < len; ++i)
        { types.push(charType(str.charCodeAt(i))); }

      // W1. Examine each non-spacing mark (NSM) in the level run, and
      // change the type of the NSM to the type of the previous
      // character. If the NSM is at the start of the level run, it will
      // get the type of sor.
      for (var i$1 = 0, prev = outerType; i$1 < len; ++i$1) {
        var type = types[i$1];
        if (type == "m") { types[i$1] = prev; }
        else { prev = type; }
      }

      // W2. Search backwards from each instance of a European number
      // until the first strong type (R, L, AL, or sor) is found. If an
      // AL is found, change the type of the European number to Arabic
      // number.
      // W3. Change all ALs to R.
      for (var i$2 = 0, cur = outerType; i$2 < len; ++i$2) {
        var type$1 = types[i$2];
        if (type$1 == "1" && cur == "r") { types[i$2] = "n"; }
        else if (isStrong.test(type$1)) { cur = type$1; if (type$1 == "r") { types[i$2] = "R"; } }
      }

      // W4. A single European separator between two European numbers
      // changes to a European number. A single common separator between
      // two numbers of the same type changes to that type.
      for (var i$3 = 1, prev$1 = types[0]; i$3 < len - 1; ++i$3) {
        var type$2 = types[i$3];
        if (type$2 == "+" && prev$1 == "1" && types[i$3+1] == "1") { types[i$3] = "1"; }
        else if (type$2 == "," && prev$1 == types[i$3+1] &&
                 (prev$1 == "1" || prev$1 == "n")) { types[i$3] = prev$1; }
        prev$1 = type$2;
      }

      // W5. A sequence of European terminators adjacent to European
      // numbers changes to all European numbers.
      // W6. Otherwise, separators and terminators change to Other
      // Neutral.
      for (var i$4 = 0; i$4 < len; ++i$4) {
        var type$3 = types[i$4];
        if (type$3 == ",") { types[i$4] = "N"; }
        else if (type$3 == "%") {
          var end = (void 0);
          for (end = i$4 + 1; end < len && types[end] == "%"; ++end) {}
          var replace = (i$4 && types[i$4-1] == "!") || (end < len && types[end] == "1") ? "1" : "N";
          for (var j = i$4; j < end; ++j) { types[j] = replace; }
          i$4 = end - 1;
        }
      }

      // W7. Search backwards from each instance of a European number
      // until the first strong type (R, L, or sor) is found. If an L is
      // found, then change the type of the European number to L.
      for (var i$5 = 0, cur$1 = outerType; i$5 < len; ++i$5) {
        var type$4 = types[i$5];
        if (cur$1 == "L" && type$4 == "1") { types[i$5] = "L"; }
        else if (isStrong.test(type$4)) { cur$1 = type$4; }
      }

      // N1. A sequence of neutrals takes the direction of the
      // surrounding strong text if the text on both sides has the same
      // direction. European and Arabic numbers act as if they were R in
      // terms of their influence on neutrals. Start-of-level-run (sor)
      // and end-of-level-run (eor) are used at level run boundaries.
      // N2. Any remaining neutrals take the embedding direction.
      for (var i$6 = 0; i$6 < len; ++i$6) {
        if (isNeutral.test(types[i$6])) {
          var end$1 = (void 0);
          for (end$1 = i$6 + 1; end$1 < len && isNeutral.test(types[end$1]); ++end$1) {}
          var before = (i$6 ? types[i$6-1] : outerType) == "L";
          var after = (end$1 < len ? types[end$1] : outerType) == "L";
          var replace$1 = before == after ? (before ? "L" : "R") : outerType;
          for (var j$1 = i$6; j$1 < end$1; ++j$1) { types[j$1] = replace$1; }
          i$6 = end$1 - 1;
        }
      }

      // Here we depart from the documented algorithm, in order to avoid
      // building up an actual levels array. Since there are only three
      // levels (0, 1, 2) in an implementation that doesn't take
      // explicit embedding into account, we can build up the order on
      // the fly, without following the level-based algorithm.
      var order = [], m;
      for (var i$7 = 0; i$7 < len;) {
        if (countsAsLeft.test(types[i$7])) {
          var start = i$7;
          for (++i$7; i$7 < len && countsAsLeft.test(types[i$7]); ++i$7) {}
          order.push(new BidiSpan(0, start, i$7));
        } else {
          var pos = i$7, at = order.length;
          for (++i$7; i$7 < len && types[i$7] != "L"; ++i$7) {}
          for (var j$2 = pos; j$2 < i$7;) {
            if (countsAsNum.test(types[j$2])) {
              if (pos < j$2) { order.splice(at, 0, new BidiSpan(1, pos, j$2)); }
              var nstart = j$2;
              for (++j$2; j$2 < i$7 && countsAsNum.test(types[j$2]); ++j$2) {}
              order.splice(at, 0, new BidiSpan(2, nstart, j$2));
              pos = j$2;
            } else { ++j$2; }
          }
          if (pos < i$7) { order.splice(at, 0, new BidiSpan(1, pos, i$7)); }
        }
      }
      if (direction == "ltr") {
        if (order[0].level == 1 && (m = str.match(/^\s+/))) {
          order[0].from = m[0].length;
          order.unshift(new BidiSpan(0, 0, m[0].length));
        }
        if (lst(order).level == 1 && (m = str.match(/\s+$/))) {
          lst(order).to -= m[0].length;
          order.push(new BidiSpan(0, len - m[0].length, len));
        }
      }

      return direction == "rtl" ? order.reverse() : order
    }
  })();

  // Get the bidi ordering for the given line (and cache it). Returns
  // false for lines that are fully left-to-right, and an array of
  // BidiSpan objects otherwise.
  function getOrder(line, direction) {
    var order = line.order;
    if (order == null) { order = line.order = bidiOrdering(line.text, direction); }
    return order
  }

  // EVENT HANDLING

  // Lightweight event framework. on/off also work on DOM nodes,
  // registering native DOM handlers.

  var noHandlers = [];

  var on = function(emitter, type, f) {
    if (emitter.addEventListener) {
      emitter.addEventListener(type, f, false);
    } else if (emitter.attachEvent) {
      emitter.attachEvent("on" + type, f);
    } else {
      var map$$1 = emitter._handlers || (emitter._handlers = {});
      map$$1[type] = (map$$1[type] || noHandlers).concat(f);
    }
  };

  function getHandlers(emitter, type) {
    return emitter._handlers && emitter._handlers[type] || noHandlers
  }

  function off(emitter, type, f) {
    if (emitter.removeEventListener) {
      emitter.removeEventListener(type, f, false);
    } else if (emitter.detachEvent) {
      emitter.detachEvent("on" + type, f);
    } else {
      var map$$1 = emitter._handlers, arr = map$$1 && map$$1[type];
      if (arr) {
        var index = indexOf(arr, f);
        if (index > -1)
          { map$$1[type] = arr.slice(0, index).concat(arr.slice(index + 1)); }
      }
    }
  }

  function signal(emitter, type /*, values...*/) {
    var handlers = getHandlers(emitter, type);
    if (!handlers.length) { return }
    var args = Array.prototype.slice.call(arguments, 2);
    for (var i = 0; i < handlers.length; ++i) { handlers[i].apply(null, args); }
  }

  // The DOM events that CodeMirror handles can be overridden by
  // registering a (non-DOM) handler on the editor for the event name,
  // and preventDefault-ing the event in that handler.
  function signalDOMEvent(cm, e, override) {
    if (typeof e == "string")
      { e = {type: e, preventDefault: function() { this.defaultPrevented = true; }}; }
    signal(cm, override || e.type, cm, e);
    return e_defaultPrevented(e) || e.codemirrorIgnore
  }

  function signalCursorActivity(cm) {
    var arr = cm._handlers && cm._handlers.cursorActivity;
    if (!arr) { return }
    var set = cm.curOp.cursorActivityHandlers || (cm.curOp.cursorActivityHandlers = []);
    for (var i = 0; i < arr.length; ++i) { if (indexOf(set, arr[i]) == -1)
      { set.push(arr[i]); } }
  }

  function hasHandler(emitter, type) {
    return getHandlers(emitter, type).length > 0
  }

  // Add on and off methods to a constructor's prototype, to make
  // registering events on such objects more convenient.
  function eventMixin(ctor) {
    ctor.prototype.on = function(type, f) {on(this, type, f);};
    ctor.prototype.off = function(type, f) {off(this, type, f);};
  }

  // Due to the fact that we still support jurassic IE versions, some
  // compatibility wrappers are needed.

  function e_preventDefault(e) {
    if (e.preventDefault) { e.preventDefault(); }
    else { e.returnValue = false; }
  }
  function e_stopPropagation(e) {
    if (e.stopPropagation) { e.stopPropagation(); }
    else { e.cancelBubble = true; }
  }
  function e_defaultPrevented(e) {
    return e.defaultPrevented != null ? e.defaultPrevented : e.returnValue == false
  }
  function e_stop(e) {e_preventDefault(e); e_stopPropagation(e);}

  function e_target(e) {return e.target || e.srcElement}
  function e_button(e) {
    var b = e.which;
    if (b == null) {
      if (e.button & 1) { b = 1; }
      else if (e.button & 2) { b = 3; }
      else if (e.button & 4) { b = 2; }
    }
    if (mac && e.ctrlKey && b == 1) { b = 3; }
    return b
  }

  // Detect drag-and-drop
  var dragAndDrop = function() {
    // There is *some* kind of drag-and-drop support in IE6-8, but I
    // couldn't get it to work yet.
    if (ie && ie_version < 9) { return false }
    var div = elt('div');
    return "draggable" in div || "dragDrop" in div
  }();

  var zwspSupported;
  function zeroWidthElement(measure) {
    if (zwspSupported == null) {
      var test = elt("span", "\u200b");
      removeChildrenAndAdd(measure, elt("span", [test, document.createTextNode("x")]));
      if (measure.firstChild.offsetHeight != 0)
        { zwspSupported = test.offsetWidth <= 1 && test.offsetHeight > 2 && !(ie && ie_version < 8); }
    }
    var node = zwspSupported ? elt("span", "\u200b") :
      elt("span", "\u00a0", null, "display: inline-block; width: 1px; margin-right: -1px");
    node.setAttribute("cm-text", "");
    return node
  }

  // Feature-detect IE's crummy client rect reporting for bidi text
  var badBidiRects;
  function hasBadBidiRects(measure) {
    if (badBidiRects != null) { return badBidiRects }
    var txt = removeChildrenAndAdd(measure, document.createTextNode("A\u062eA"));
    var r0 = range(txt, 0, 1).getBoundingClientRect();
    var r1 = range(txt, 1, 2).getBoundingClientRect();
    removeChildren(measure);
    if (!r0 || r0.left == r0.right) { return false } // Safari returns null in some cases (#2780)
    return badBidiRects = (r1.right - r0.right < 3)
  }

  // See if "".split is the broken IE version, if so, provide an
  // alternative way to split lines.
  var splitLinesAuto = "\n\nb".split(/\n/).length != 3 ? function (string) {
    var pos = 0, result = [], l = string.length;
    while (pos <= l) {
      var nl = string.indexOf("\n", pos);
      if (nl == -1) { nl = string.length; }
      var line = string.slice(pos, string.charAt(nl - 1) == "\r" ? nl - 1 : nl);
      var rt = line.indexOf("\r");
      if (rt != -1) {
        result.push(line.slice(0, rt));
        pos += rt + 1;
      } else {
        result.push(line);
        pos = nl + 1;
      }
    }
    return result
  } : function (string) { return string.split(/\r\n?|\n/); };

  var hasSelection = window.getSelection ? function (te) {
    try { return te.selectionStart != te.selectionEnd }
    catch(e) { return false }
  } : function (te) {
    var range$$1;
    try {range$$1 = te.ownerDocument.selection.createRange();}
    catch(e) {}
    if (!range$$1 || range$$1.parentElement() != te) { return false }
    return range$$1.compareEndPoints("StartToEnd", range$$1) != 0
  };

  var hasCopyEvent = (function () {
    var e = elt("div");
    if ("oncopy" in e) { return true }
    e.setAttribute("oncopy", "return;");
    return typeof e.oncopy == "function"
  })();

  var badZoomedRects = null;
  function hasBadZoomedRects(measure) {
    if (badZoomedRects != null) { return badZoomedRects }
    var node = removeChildrenAndAdd(measure, elt("span", "x"));
    var normal = node.getBoundingClientRect();
    var fromRange = range(node, 0, 1).getBoundingClientRect();
    return badZoomedRects = Math.abs(normal.left - fromRange.left) > 1
  }

  // Known modes, by name and by MIME
  var modes = {}, mimeModes = {};

  // Extra arguments are stored as the mode's dependencies, which is
  // used by (legacy) mechanisms like loadmode.js to automatically
  // load a mode. (Preferred mechanism is the require/define calls.)
  function defineMode(name, mode) {
    if (arguments.length > 2)
      { mode.dependencies = Array.prototype.slice.call(arguments, 2); }
    modes[name] = mode;
  }

  function defineMIME(mime, spec) {
    mimeModes[mime] = spec;
  }

  // Given a MIME type, a {name, ...options} config object, or a name
  // string, return a mode config object.
  function resolveMode(spec) {
    if (typeof spec == "string" && mimeModes.hasOwnProperty(spec)) {
      spec = mimeModes[spec];
    } else if (spec && typeof spec.name == "string" && mimeModes.hasOwnProperty(spec.name)) {
      var found = mimeModes[spec.name];
      if (typeof found == "string") { found = {name: found}; }
      spec = createObj(found, spec);
      spec.name = found.name;
    } else if (typeof spec == "string" && /^[\w\-]+\/[\w\-]+\+xml$/.test(spec)) {
      return resolveMode("application/xml")
    } else if (typeof spec == "string" && /^[\w\-]+\/[\w\-]+\+json$/.test(spec)) {
      return resolveMode("application/json")
    }
    if (typeof spec == "string") { return {name: spec} }
    else { return spec || {name: "null"} }
  }

  // Given a mode spec (anything that resolveMode accepts), find and
  // initialize an actual mode object.
  function getMode(options, spec) {
    spec = resolveMode(spec);
    var mfactory = modes[spec.name];
    if (!mfactory) { return getMode(options, "text/plain") }
    var modeObj = mfactory(options, spec);
    if (modeExtensions.hasOwnProperty(spec.name)) {
      var exts = modeExtensions[spec.name];
      for (var prop in exts) {
        if (!exts.hasOwnProperty(prop)) { continue }
        if (modeObj.hasOwnProperty(prop)) { modeObj["_" + prop] = modeObj[prop]; }
        modeObj[prop] = exts[prop];
      }
    }
    modeObj.name = spec.name;
    if (spec.helperType) { modeObj.helperType = spec.helperType; }
    if (spec.modeProps) { for (var prop$1 in spec.modeProps)
      { modeObj[prop$1] = spec.modeProps[prop$1]; } }

    return modeObj
  }

  // This can be used to attach properties to mode objects from
  // outside the actual mode definition.
  var modeExtensions = {};
  function extendMode(mode, properties) {
    var exts = modeExtensions.hasOwnProperty(mode) ? modeExtensions[mode] : (modeExtensions[mode] = {});
    copyObj(properties, exts);
  }

  function copyState(mode, state) {
    if (state === true) { return state }
    if (mode.copyState) { return mode.copyState(state) }
    var nstate = {};
    for (var n in state) {
      var val = state[n];
      if (val instanceof Array) { val = val.concat([]); }
      nstate[n] = val;
    }
    return nstate
  }

  // Given a mode and a state (for that mode), find the inner mode and
  // state at the position that the state refers to.
  function innerMode(mode, state) {
    var info;
    while (mode.innerMode) {
      info = mode.innerMode(state);
      if (!info || info.mode == mode) { break }
      state = info.state;
      mode = info.mode;
    }
    return info || {mode: mode, state: state}
  }

  function startState(mode, a1, a2) {
    return mode.startState ? mode.startState(a1, a2) : true
  }

  // STRING STREAM

  // Fed to the mode parsers, provides helper functions to make
  // parsers more succinct.

  var StringStream = function(string, tabSize, lineOracle) {
    this.pos = this.start = 0;
    this.string = string;
    this.tabSize = tabSize || 8;
    this.lastColumnPos = this.lastColumnValue = 0;
    this.lineStart = 0;
    this.lineOracle = lineOracle;
  };

  StringStream.prototype.eol = function () {return this.pos >= this.string.length};
  StringStream.prototype.sol = function () {return this.pos == this.lineStart};
  StringStream.prototype.peek = function () {return this.string.charAt(this.pos) || undefined};
  StringStream.prototype.next = function () {
    if (this.pos < this.string.length)
      { return this.string.charAt(this.pos++) }
  };
  StringStream.prototype.eat = function (match) {
    var ch = this.string.charAt(this.pos);
    var ok;
    if (typeof match == "string") { ok = ch == match; }
    else { ok = ch && (match.test ? match.test(ch) : match(ch)); }
    if (ok) {++this.pos; return ch}
  };
  StringStream.prototype.eatWhile = function (match) {
    var start = this.pos;
    while (this.eat(match)){}
    return this.pos > start
  };
  StringStream.prototype.eatSpace = function () {
      var this$1 = this;

    var start = this.pos;
    while (/[\s\u00a0]/.test(this.string.charAt(this.pos))) { ++this$1.pos; }
    return this.pos > start
  };
  StringStream.prototype.skipToEnd = function () {this.pos = this.string.length;};
  StringStream.prototype.skipTo = function (ch) {
    var found = this.string.indexOf(ch, this.pos);
    if (found > -1) {this.pos = found; return true}
  };
  StringStream.prototype.backUp = function (n) {this.pos -= n;};
  StringStream.prototype.column = function () {
    if (this.lastColumnPos < this.start) {
      this.lastColumnValue = countColumn(this.string, this.start, this.tabSize, this.lastColumnPos, this.lastColumnValue);
      this.lastColumnPos = this.start;
    }
    return this.lastColumnValue - (this.lineStart ? countColumn(this.string, this.lineStart, this.tabSize) : 0)
  };
  StringStream.prototype.indentation = function () {
    return countColumn(this.string, null, this.tabSize) -
      (this.lineStart ? countColumn(this.string, this.lineStart, this.tabSize) : 0)
  };
  StringStream.prototype.match = function (pattern, consume, caseInsensitive) {
    if (typeof pattern == "string") {
      var cased = function (str) { return caseInsensitive ? str.toLowerCase() : str; };
      var substr = this.string.substr(this.pos, pattern.length);
      if (cased(substr) == cased(pattern)) {
        if (consume !== false) { this.pos += pattern.length; }
        return true
      }
    } else {
      var match = this.string.slice(this.pos).match(pattern);
      if (match && match.index > 0) { return null }
      if (match && consume !== false) { this.pos += match[0].length; }
      return match
    }
  };
  StringStream.prototype.current = function (){return this.string.slice(this.start, this.pos)};
  StringStream.prototype.hideFirstChars = function (n, inner) {
    this.lineStart += n;
    try { return inner() }
    finally { this.lineStart -= n; }
  };
  StringStream.prototype.lookAhead = function (n) {
    var oracle = this.lineOracle;
    return oracle && oracle.lookAhead(n)
  };
  StringStream.prototype.baseToken = function () {
    var oracle = this.lineOracle;
    return oracle && oracle.baseToken(this.pos)
  };

  // Find the line object corresponding to the given line number.
  function getLine(doc, n) {
    n -= doc.first;
    if (n < 0 || n >= doc.size) { throw new Error("There is no line " + (n + doc.first) + " in the document.") }
    var chunk = doc;
    while (!chunk.lines) {
      for (var i = 0;; ++i) {
        var child = chunk.children[i], sz = child.chunkSize();
        if (n < sz) { chunk = child; break }
        n -= sz;
      }
    }
    return chunk.lines[n]
  }

  // Get the part of a document between two positions, as an array of
  // strings.
  function getBetween(doc, start, end) {
    var out = [], n = start.line;
    doc.iter(start.line, end.line + 1, function (line) {
      var text = line.text;
      if (n == end.line) { text = text.slice(0, end.ch); }
      if (n == start.line) { text = text.slice(start.ch); }
      out.push(text);
      ++n;
    });
    return out
  }
  // Get the lines between from and to, as array of strings.
  function getLines(doc, from, to) {
    var out = [];
    doc.iter(from, to, function (line) { out.push(line.text); }); // iter aborts when callback returns truthy value
    return out
  }

  // Update the height of a line, propagating the height change
  // upwards to parent nodes.
  function updateLineHeight(line, height) {
    var diff = height - line.height;
    if (diff) { for (var n = line; n; n = n.parent) { n.height += diff; } }
  }

  // Given a line object, find its line number by walking up through
  // its parent links.
  function lineNo(line) {
    if (line.parent == null) { return null }
    var cur = line.parent, no = indexOf(cur.lines, line);
    for (var chunk = cur.parent; chunk; cur = chunk, chunk = chunk.parent) {
      for (var i = 0;; ++i) {
        if (chunk.children[i] == cur) { break }
        no += chunk.children[i].chunkSize();
      }
    }
    return no + cur.first
  }

  // Find the line at the given vertical position, using the height
  // information in the document tree.
  function lineAtHeight(chunk, h) {
    var n = chunk.first;
    outer: do {
      for (var i$1 = 0; i$1 < chunk.children.length; ++i$1) {
        var child = chunk.children[i$1], ch = child.height;
        if (h < ch) { chunk = child; continue outer }
        h -= ch;
        n += child.chunkSize();
      }
      return n
    } while (!chunk.lines)
    var i = 0;
    for (; i < chunk.lines.length; ++i) {
      var line = chunk.lines[i], lh = line.height;
      if (h < lh) { break }
      h -= lh;
    }
    return n + i
  }

  function isLine(doc, l) {return l >= doc.first && l < doc.first + doc.size}

  function lineNumberFor(options, i) {
    return String(options.lineNumberFormatter(i + options.firstLineNumber))
  }

  // A Pos instance represents a position within the text.
  function Pos(line, ch, sticky) {
    if ( sticky === void 0 ) sticky = null;

    if (!(this instanceof Pos)) { return new Pos(line, ch, sticky) }
    this.line = line;
    this.ch = ch;
    this.sticky = sticky;
  }

  // Compare two positions, return 0 if they are the same, a negative
  // number when a is less, and a positive number otherwise.
  function cmp(a, b) { return a.line - b.line || a.ch - b.ch }

  function equalCursorPos(a, b) { return a.sticky == b.sticky && cmp(a, b) == 0 }

  function copyPos(x) {return Pos(x.line, x.ch)}
  function maxPos(a, b) { return cmp(a, b) < 0 ? b : a }
  function minPos(a, b) { return cmp(a, b) < 0 ? a : b }

  // Most of the external API clips given positions to make sure they
  // actually exist within the document.
  function clipLine(doc, n) {return Math.max(doc.first, Math.min(n, doc.first + doc.size - 1))}
  function clipPos(doc, pos) {
    if (pos.line < doc.first) { return Pos(doc.first, 0) }
    var last = doc.first + doc.size - 1;
    if (pos.line > last) { return Pos(last, getLine(doc, last).text.length) }
    return clipToLen(pos, getLine(doc, pos.line).text.length)
  }
  function clipToLen(pos, linelen) {
    var ch = pos.ch;
    if (ch == null || ch > linelen) { return Pos(pos.line, linelen) }
    else if (ch < 0) { return Pos(pos.line, 0) }
    else { return pos }
  }
  function clipPosArray(doc, array) {
    var out = [];
    for (var i = 0; i < array.length; i++) { out[i] = clipPos(doc, array[i]); }
    return out
  }

  var SavedContext = function(state, lookAhead) {
    this.state = state;
    this.lookAhead = lookAhead;
  };

  var Context = function(doc, state, line, lookAhead) {
    this.state = state;
    this.doc = doc;
    this.line = line;
    this.maxLookAhead = lookAhead || 0;
    this.baseTokens = null;
    this.baseTokenPos = 1;
  };

  Context.prototype.lookAhead = function (n) {
    var line = this.doc.getLine(this.line + n);
    if (line != null && n > this.maxLookAhead) { this.maxLookAhead = n; }
    return line
  };

  Context.prototype.baseToken = function (n) {
      var this$1 = this;

    if (!this.baseTokens) { return null }
    while (this.baseTokens[this.baseTokenPos] <= n)
      { this$1.baseTokenPos += 2; }
    var type = this.baseTokens[this.baseTokenPos + 1];
    return {type: type && type.replace(/( |^)overlay .*/, ""),
            size: this.baseTokens[this.baseTokenPos] - n}
  };

  Context.prototype.nextLine = function () {
    this.line++;
    if (this.maxLookAhead > 0) { this.maxLookAhead--; }
  };

  Context.fromSaved = function (doc, saved, line) {
    if (saved instanceof SavedContext)
      { return new Context(doc, copyState(doc.mode, saved.state), line, saved.lookAhead) }
    else
      { return new Context(doc, copyState(doc.mode, saved), line) }
  };

  Context.prototype.save = function (copy) {
    var state = copy !== false ? copyState(this.doc.mode, this.state) : this.state;
    return this.maxLookAhead > 0 ? new SavedContext(state, this.maxLookAhead) : state
  };


  // Compute a style array (an array starting with a mode generation
  // -- for invalidation -- followed by pairs of end positions and
  // style strings), which is used to highlight the tokens on the
  // line.
  function highlightLine(cm, line, context, forceToEnd) {
    // A styles array always starts with a number identifying the
    // mode/overlays that it is based on (for easy invalidation).
    var st = [cm.state.modeGen], lineClasses = {};
    // Compute the base array of styles
    runMode(cm, line.text, cm.doc.mode, context, function (end, style) { return st.push(end, style); },
            lineClasses, forceToEnd);
    var state = context.state;

    // Run overlays, adjust style array.
    var loop = function ( o ) {
      context.baseTokens = st;
      var overlay = cm.state.overlays[o], i = 1, at = 0;
      context.state = true;
      runMode(cm, line.text, overlay.mode, context, function (end, style) {
        var start = i;
        // Ensure there's a token end at the current position, and that i points at it
        while (at < end) {
          var i_end = st[i];
          if (i_end > end)
            { st.splice(i, 1, end, st[i+1], i_end); }
          i += 2;
          at = Math.min(end, i_end);
        }
        if (!style) { return }
        if (overlay.opaque) {
          st.splice(start, i - start, end, "overlay " + style);
          i = start + 2;
        } else {
          for (; start < i; start += 2) {
            var cur = st[start+1];
            st[start+1] = (cur ? cur + " " : "") + "overlay " + style;
          }
        }
      }, lineClasses);
      context.state = state;
      context.baseTokens = null;
      context.baseTokenPos = 1;
    };

    for (var o = 0; o < cm.state.overlays.length; ++o) loop( o );

    return {styles: st, classes: lineClasses.bgClass || lineClasses.textClass ? lineClasses : null}
  }

  function getLineStyles(cm, line, updateFrontier) {
    if (!line.styles || line.styles[0] != cm.state.modeGen) {
      var context = getContextBefore(cm, lineNo(line));
      var resetState = line.text.length > cm.options.maxHighlightLength && copyState(cm.doc.mode, context.state);
      var result = highlightLine(cm, line, context);
      if (resetState) { context.state = resetState; }
      line.stateAfter = context.save(!resetState);
      line.styles = result.styles;
      if (result.classes) { line.styleClasses = result.classes; }
      else if (line.styleClasses) { line.styleClasses = null; }
      if (updateFrontier === cm.doc.highlightFrontier)
        { cm.doc.modeFrontier = Math.max(cm.doc.modeFrontier, ++cm.doc.highlightFrontier); }
    }
    return line.styles
  }

  function getContextBefore(cm, n, precise) {
    var doc = cm.doc, display = cm.display;
    if (!doc.mode.startState) { return new Context(doc, true, n) }
    var start = findStartLine(cm, n, precise);
    var saved = start > doc.first && getLine(doc, start - 1).stateAfter;
    var context = saved ? Context.fromSaved(doc, saved, start) : new Context(doc, startState(doc.mode), start);

    doc.iter(start, n, function (line) {
      processLine(cm, line.text, context);
      var pos = context.line;
      line.stateAfter = pos == n - 1 || pos % 5 == 0 || pos >= display.viewFrom && pos < display.viewTo ? context.save() : null;
      context.nextLine();
    });
    if (precise) { doc.modeFrontier = context.line; }
    return context
  }

  // Lightweight form of highlight -- proceed over this line and
  // update state, but don't save a style array. Used for lines that
  // aren't currently visible.
  function processLine(cm, text, context, startAt) {
    var mode = cm.doc.mode;
    var stream = new StringStream(text, cm.options.tabSize, context);
    stream.start = stream.pos = startAt || 0;
    if (text == "") { callBlankLine(mode, context.state); }
    while (!stream.eol()) {
      readToken(mode, stream, context.state);
      stream.start = stream.pos;
    }
  }

  function callBlankLine(mode, state) {
    if (mode.blankLine) { return mode.blankLine(state) }
    if (!mode.innerMode) { return }
    var inner = innerMode(mode, state);
    if (inner.mode.blankLine) { return inner.mode.blankLine(inner.state) }
  }

  function readToken(mode, stream, state, inner) {
    for (var i = 0; i < 10; i++) {
      if (inner) { inner[0] = innerMode(mode, state).mode; }
      var style = mode.token(stream, state);
      if (stream.pos > stream.start) { return style }
    }
    throw new Error("Mode " + mode.name + " failed to advance stream.")
  }

  var Token = function(stream, type, state) {
    this.start = stream.start; this.end = stream.pos;
    this.string = stream.current();
    this.type = type || null;
    this.state = state;
  };

  // Utility for getTokenAt and getLineTokens
  function takeToken(cm, pos, precise, asArray) {
    var doc = cm.doc, mode = doc.mode, style;
    pos = clipPos(doc, pos);
    var line = getLine(doc, pos.line), context = getContextBefore(cm, pos.line, precise);
    var stream = new StringStream(line.text, cm.options.tabSize, context), tokens;
    if (asArray) { tokens = []; }
    while ((asArray || stream.pos < pos.ch) && !stream.eol()) {
      stream.start = stream.pos;
      style = readToken(mode, stream, context.state);
      if (asArray) { tokens.push(new Token(stream, style, copyState(doc.mode, context.state))); }
    }
    return asArray ? tokens : new Token(stream, style, context.state)
  }

  function extractLineClasses(type, output) {
    if (type) { for (;;) {
      var lineClass = type.match(/(?:^|\s+)line-(background-)?(\S+)/);
      if (!lineClass) { break }
      type = type.slice(0, lineClass.index) + type.slice(lineClass.index + lineClass[0].length);
      var prop = lineClass[1] ? "bgClass" : "textClass";
      if (output[prop] == null)
        { output[prop] = lineClass[2]; }
      else if (!(new RegExp("(?:^|\s)" + lineClass[2] + "(?:$|\s)")).test(output[prop]))
        { output[prop] += " " + lineClass[2]; }
    } }
    return type
  }

  // Run the given mode's parser over a line, calling f for each token.
  function runMode(cm, text, mode, context, f, lineClasses, forceToEnd) {
    var flattenSpans = mode.flattenSpans;
    if (flattenSpans == null) { flattenSpans = cm.options.flattenSpans; }
    var curStart = 0, curStyle = null;
    var stream = new StringStream(text, cm.options.tabSize, context), style;
    var inner = cm.options.addModeClass && [null];
    if (text == "") { extractLineClasses(callBlankLine(mode, context.state), lineClasses); }
    while (!stream.eol()) {
      if (stream.pos > cm.options.maxHighlightLength) {
        flattenSpans = false;
        if (forceToEnd) { processLine(cm, text, context, stream.pos); }
        stream.pos = text.length;
        style = null;
      } else {
        style = extractLineClasses(readToken(mode, stream, context.state, inner), lineClasses);
      }
      if (inner) {
        var mName = inner[0].name;
        if (mName) { style = "m-" + (style ? mName + " " + style : mName); }
      }
      if (!flattenSpans || curStyle != style) {
        while (curStart < stream.start) {
          curStart = Math.min(stream.start, curStart + 5000);
          f(curStart, curStyle);
        }
        curStyle = style;
      }
      stream.start = stream.pos;
    }
    while (curStart < stream.pos) {
      // Webkit seems to refuse to render text nodes longer than 57444
      // characters, and returns inaccurate measurements in nodes
      // starting around 5000 chars.
      var pos = Math.min(stream.pos, curStart + 5000);
      f(pos, curStyle);
      curStart = pos;
    }
  }

  // Finds the line to start with when starting a parse. Tries to
  // find a line with a stateAfter, so that it can start with a
  // valid state. If that fails, it returns the line with the
  // smallest indentation, which tends to need the least context to
  // parse correctly.
  function findStartLine(cm, n, precise) {
    var minindent, minline, doc = cm.doc;
    var lim = precise ? -1 : n - (cm.doc.mode.innerMode ? 1000 : 100);
    for (var search = n; search > lim; --search) {
      if (search <= doc.first) { return doc.first }
      var line = getLine(doc, search - 1), after = line.stateAfter;
      if (after && (!precise || search + (after instanceof SavedContext ? after.lookAhead : 0) <= doc.modeFrontier))
        { return search }
      var indented = countColumn(line.text, null, cm.options.tabSize);
      if (minline == null || minindent > indented) {
        minline = search - 1;
        minindent = indented;
      }
    }
    return minline
  }

  function retreatFrontier(doc, n) {
    doc.modeFrontier = Math.min(doc.modeFrontier, n);
    if (doc.highlightFrontier < n - 10) { return }
    var start = doc.first;
    for (var line = n - 1; line > start; line--) {
      var saved = getLine(doc, line).stateAfter;
      // change is on 3
      // state on line 1 looked ahead 2 -- so saw 3
      // test 1 + 2 < 3 should cover this
      if (saved && (!(saved instanceof SavedContext) || line + saved.lookAhead < n)) {
        start = line + 1;
        break
      }
    }
    doc.highlightFrontier = Math.min(doc.highlightFrontier, start);
  }

  // Optimize some code when these features are not used.
  var sawReadOnlySpans = false, sawCollapsedSpans = false;

  function seeReadOnlySpans() {
    sawReadOnlySpans = true;
  }

  function seeCollapsedSpans() {
    sawCollapsedSpans = true;
  }

  // TEXTMARKER SPANS

  function MarkedSpan(marker, from, to) {
    this.marker = marker;
    this.from = from; this.to = to;
  }

  // Search an array of spans for a span matching the given marker.
  function getMarkedSpanFor(spans, marker) {
    if (spans) { for (var i = 0; i < spans.length; ++i) {
      var span = spans[i];
      if (span.marker == marker) { return span }
    } }
  }
  // Remove a span from an array, returning undefined if no spans are
  // left (we don't store arrays for lines without spans).
  function removeMarkedSpan(spans, span) {
    var r;
    for (var i = 0; i < spans.length; ++i)
      { if (spans[i] != span) { (r || (r = [])).push(spans[i]); } }
    return r
  }
  // Add a span to a line.
  function addMarkedSpan(line, span) {
    line.markedSpans = line.markedSpans ? line.markedSpans.concat([span]) : [span];
    span.marker.attachLine(line);
  }

  // Used for the algorithm that adjusts markers for a change in the
  // document. These functions cut an array of spans at a given
  // character position, returning an array of remaining chunks (or
  // undefined if nothing remains).
  function markedSpansBefore(old, startCh, isInsert) {
    var nw;
    if (old) { for (var i = 0; i < old.length; ++i) {
      var span = old[i], marker = span.marker;
      var startsBefore = span.from == null || (marker.inclusiveLeft ? span.from <= startCh : span.from < startCh);
      if (startsBefore || span.from == startCh && marker.type == "bookmark" && (!isInsert || !span.marker.insertLeft)) {
        var endsAfter = span.to == null || (marker.inclusiveRight ? span.to >= startCh : span.to > startCh)
        ;(nw || (nw = [])).push(new MarkedSpan(marker, span.from, endsAfter ? null : span.to));
      }
    } }
    return nw
  }
  function markedSpansAfter(old, endCh, isInsert) {
    var nw;
    if (old) { for (var i = 0; i < old.length; ++i) {
      var span = old[i], marker = span.marker;
      var endsAfter = span.to == null || (marker.inclusiveRight ? span.to >= endCh : span.to > endCh);
      if (endsAfter || span.from == endCh && marker.type == "bookmark" && (!isInsert || span.marker.insertLeft)) {
        var startsBefore = span.from == null || (marker.inclusiveLeft ? span.from <= endCh : span.from < endCh)
        ;(nw || (nw = [])).push(new MarkedSpan(marker, startsBefore ? null : span.from - endCh,
                                              span.to == null ? null : span.to - endCh));
      }
    } }
    return nw
  }

  // Given a change object, compute the new set of marker spans that
  // cover the line in which the change took place. Removes spans
  // entirely within the change, reconnects spans belonging to the
  // same marker that appear on both sides of the change, and cuts off
  // spans partially within the change. Returns an array of span
  // arrays with one element for each line in (after) the change.
  function stretchSpansOverChange(doc, change) {
    if (change.full) { return null }
    var oldFirst = isLine(doc, change.from.line) && getLine(doc, change.from.line).markedSpans;
    var oldLast = isLine(doc, change.to.line) && getLine(doc, change.to.line).markedSpans;
    if (!oldFirst && !oldLast) { return null }

    var startCh = change.from.ch, endCh = change.to.ch, isInsert = cmp(change.from, change.to) == 0;
    // Get the spans that 'stick out' on both sides
    var first = markedSpansBefore(oldFirst, startCh, isInsert);
    var last = markedSpansAfter(oldLast, endCh, isInsert);

    // Next, merge those two ends
    var sameLine = change.text.length == 1, offset = lst(change.text).length + (sameLine ? startCh : 0);
    if (first) {
      // Fix up .to properties of first
      for (var i = 0; i < first.length; ++i) {
        var span = first[i];
        if (span.to == null) {
          var found = getMarkedSpanFor(last, span.marker);
          if (!found) { span.to = startCh; }
          else if (sameLine) { span.to = found.to == null ? null : found.to + offset; }
        }
      }
    }
    if (last) {
      // Fix up .from in last (or move them into first in case of sameLine)
      for (var i$1 = 0; i$1 < last.length; ++i$1) {
        var span$1 = last[i$1];
        if (span$1.to != null) { span$1.to += offset; }
        if (span$1.from == null) {
          var found$1 = getMarkedSpanFor(first, span$1.marker);
          if (!found$1) {
            span$1.from = offset;
            if (sameLine) { (first || (first = [])).push(span$1); }
          }
        } else {
          span$1.from += offset;
          if (sameLine) { (first || (first = [])).push(span$1); }
        }
      }
    }
    // Make sure we didn't create any zero-length spans
    if (first) { first = clearEmptySpans(first); }
    if (last && last != first) { last = clearEmptySpans(last); }

    var newMarkers = [first];
    if (!sameLine) {
      // Fill gap with whole-line-spans
      var gap = change.text.length - 2, gapMarkers;
      if (gap > 0 && first)
        { for (var i$2 = 0; i$2 < first.length; ++i$2)
          { if (first[i$2].to == null)
            { (gapMarkers || (gapMarkers = [])).push(new MarkedSpan(first[i$2].marker, null, null)); } } }
      for (var i$3 = 0; i$3 < gap; ++i$3)
        { newMarkers.push(gapMarkers); }
      newMarkers.push(last);
    }
    return newMarkers
  }

  // Remove spans that are empty and don't have a clearWhenEmpty
  // option of false.
  function clearEmptySpans(spans) {
    for (var i = 0; i < spans.length; ++i) {
      var span = spans[i];
      if (span.from != null && span.from == span.to && span.marker.clearWhenEmpty !== false)
        { spans.splice(i--, 1); }
    }
    if (!spans.length) { return null }
    return spans
  }

  // Used to 'clip' out readOnly ranges when making a change.
  function removeReadOnlyRanges(doc, from, to) {
    var markers = null;
    doc.iter(from.line, to.line + 1, function (line) {
      if (line.markedSpans) { for (var i = 0; i < line.markedSpans.length; ++i) {
        var mark = line.markedSpans[i].marker;
        if (mark.readOnly && (!markers || indexOf(markers, mark) == -1))
          { (markers || (markers = [])).push(mark); }
      } }
    });
    if (!markers) { return null }
    var parts = [{from: from, to: to}];
    for (var i = 0; i < markers.length; ++i) {
      var mk = markers[i], m = mk.find(0);
      for (var j = 0; j < parts.length; ++j) {
        var p = parts[j];
        if (cmp(p.to, m.from) < 0 || cmp(p.from, m.to) > 0) { continue }
        var newParts = [j, 1], dfrom = cmp(p.from, m.from), dto = cmp(p.to, m.to);
        if (dfrom < 0 || !mk.inclusiveLeft && !dfrom)
          { newParts.push({from: p.from, to: m.from}); }
        if (dto > 0 || !mk.inclusiveRight && !dto)
          { newParts.push({from: m.to, to: p.to}); }
        parts.splice.apply(parts, newParts);
        j += newParts.length - 3;
      }
    }
    return parts
  }

  // Connect or disconnect spans from a line.
  function detachMarkedSpans(line) {
    var spans = line.markedSpans;
    if (!spans) { return }
    for (var i = 0; i < spans.length; ++i)
      { spans[i].marker.detachLine(line); }
    line.markedSpans = null;
  }
  function attachMarkedSpans(line, spans) {
    if (!spans) { return }
    for (var i = 0; i < spans.length; ++i)
      { spans[i].marker.attachLine(line); }
    line.markedSpans = spans;
  }

  // Helpers used when computing which overlapping collapsed span
  // counts as the larger one.
  function extraLeft(marker) { return marker.inclusiveLeft ? -1 : 0 }
  function extraRight(marker) { return marker.inclusiveRight ? 1 : 0 }

  // Returns a number indicating which of two overlapping collapsed
  // spans is larger (and thus includes the other). Falls back to
  // comparing ids when the spans cover exactly the same range.
  function compareCollapsedMarkers(a, b) {
    var lenDiff = a.lines.length - b.lines.length;
    if (lenDiff != 0) { return lenDiff }
    var aPos = a.find(), bPos = b.find();
    var fromCmp = cmp(aPos.from, bPos.from) || extraLeft(a) - extraLeft(b);
    if (fromCmp) { return -fromCmp }
    var toCmp = cmp(aPos.to, bPos.to) || extraRight(a) - extraRight(b);
    if (toCmp) { return toCmp }
    return b.id - a.id
  }

  // Find out whether a line ends or starts in a collapsed span. If
  // so, return the marker for that span.
  function collapsedSpanAtSide(line, start) {
    var sps = sawCollapsedSpans && line.markedSpans, found;
    if (sps) { for (var sp = (void 0), i = 0; i < sps.length; ++i) {
      sp = sps[i];
      if (sp.marker.collapsed && (start ? sp.from : sp.to) == null &&
          (!found || compareCollapsedMarkers(found, sp.marker) < 0))
        { found = sp.marker; }
    } }
    return found
  }
  function collapsedSpanAtStart(line) { return collapsedSpanAtSide(line, true) }
  function collapsedSpanAtEnd(line) { return collapsedSpanAtSide(line, false) }

  function collapsedSpanAround(line, ch) {
    var sps = sawCollapsedSpans && line.markedSpans, found;
    if (sps) { for (var i = 0; i < sps.length; ++i) {
      var sp = sps[i];
      if (sp.marker.collapsed && (sp.from == null || sp.from < ch) && (sp.to == null || sp.to > ch) &&
          (!found || compareCollapsedMarkers(found, sp.marker) < 0)) { found = sp.marker; }
    } }
    return found
  }

  // Test whether there exists a collapsed span that partially
  // overlaps (covers the start or end, but not both) of a new span.
  // Such overlap is not allowed.
  function conflictingCollapsedRange(doc, lineNo$$1, from, to, marker) {
    var line = getLine(doc, lineNo$$1);
    var sps = sawCollapsedSpans && line.markedSpans;
    if (sps) { for (var i = 0; i < sps.length; ++i) {
      var sp = sps[i];
      if (!sp.marker.collapsed) { continue }
      var found = sp.marker.find(0);
      var fromCmp = cmp(found.from, from) || extraLeft(sp.marker) - extraLeft(marker);
      var toCmp = cmp(found.to, to) || extraRight(sp.marker) - extraRight(marker);
      if (fromCmp >= 0 && toCmp <= 0 || fromCmp <= 0 && toCmp >= 0) { continue }
      if (fromCmp <= 0 && (sp.marker.inclusiveRight && marker.inclusiveLeft ? cmp(found.to, from) >= 0 : cmp(found.to, from) > 0) ||
          fromCmp >= 0 && (sp.marker.inclusiveRight && marker.inclusiveLeft ? cmp(found.from, to) <= 0 : cmp(found.from, to) < 0))
        { return true }
    } }
  }

  // A visual line is a line as drawn on the screen. Folding, for
  // example, can cause multiple logical lines to appear on the same
  // visual line. This finds the start of the visual line that the
  // given line is part of (usually that is the line itself).
  function visualLine(line) {
    var merged;
    while (merged = collapsedSpanAtStart(line))
      { line = merged.find(-1, true).line; }
    return line
  }

  function visualLineEnd(line) {
    var merged;
    while (merged = collapsedSpanAtEnd(line))
      { line = merged.find(1, true).line; }
    return line
  }

  // Returns an array of logical lines that continue the visual line
  // started by the argument, or undefined if there are no such lines.
  function visualLineContinued(line) {
    var merged, lines;
    while (merged = collapsedSpanAtEnd(line)) {
      line = merged.find(1, true).line
      ;(lines || (lines = [])).push(line);
    }
    return lines
  }

  // Get the line number of the start of the visual line that the
  // given line number is part of.
  function visualLineNo(doc, lineN) {
    var line = getLine(doc, lineN), vis = visualLine(line);
    if (line == vis) { return lineN }
    return lineNo(vis)
  }

  // Get the line number of the start of the next visual line after
  // the given line.
  function visualLineEndNo(doc, lineN) {
    if (lineN > doc.lastLine()) { return lineN }
    var line = getLine(doc, lineN), merged;
    if (!lineIsHidden(doc, line)) { return lineN }
    while (merged = collapsedSpanAtEnd(line))
      { line = merged.find(1, true).line; }
    return lineNo(line) + 1
  }

  // Compute whether a line is hidden. Lines count as hidden when they
  // are part of a visual line that starts with another line, or when
  // they are entirely covered by collapsed, non-widget span.
  function lineIsHidden(doc, line) {
    var sps = sawCollapsedSpans && line.markedSpans;
    if (sps) { for (var sp = (void 0), i = 0; i < sps.length; ++i) {
      sp = sps[i];
      if (!sp.marker.collapsed) { continue }
      if (sp.from == null) { return true }
      if (sp.marker.widgetNode) { continue }
      if (sp.from == 0 && sp.marker.inclusiveLeft && lineIsHiddenInner(doc, line, sp))
        { return true }
    } }
  }
  function lineIsHiddenInner(doc, line, span) {
    if (span.to == null) {
      var end = span.marker.find(1, true);
      return lineIsHiddenInner(doc, end.line, getMarkedSpanFor(end.line.markedSpans, span.marker))
    }
    if (span.marker.inclusiveRight && span.to == line.text.length)
      { return true }
    for (var sp = (void 0), i = 0; i < line.markedSpans.length; ++i) {
      sp = line.markedSpans[i];
      if (sp.marker.collapsed && !sp.marker.widgetNode && sp.from == span.to &&
          (sp.to == null || sp.to != span.from) &&
          (sp.marker.inclusiveLeft || span.marker.inclusiveRight) &&
          lineIsHiddenInner(doc, line, sp)) { return true }
    }
  }

  // Find the height above the given line.
  function heightAtLine(lineObj) {
    lineObj = visualLine(lineObj);

    var h = 0, chunk = lineObj.parent;
    for (var i = 0; i < chunk.lines.length; ++i) {
      var line = chunk.lines[i];
      if (line == lineObj) { break }
      else { h += line.height; }
    }
    for (var p = chunk.parent; p; chunk = p, p = chunk.parent) {
      for (var i$1 = 0; i$1 < p.children.length; ++i$1) {
        var cur = p.children[i$1];
        if (cur == chunk) { break }
        else { h += cur.height; }
      }
    }
    return h
  }

  // Compute the character length of a line, taking into account
  // collapsed ranges (see markText) that might hide parts, and join
  // other lines onto it.
  function lineLength(line) {
    if (line.height == 0) { return 0 }
    var len = line.text.length, merged, cur = line;
    while (merged = collapsedSpanAtStart(cur)) {
      var found = merged.find(0, true);
      cur = found.from.line;
      len += found.from.ch - found.to.ch;
    }
    cur = line;
    while (merged = collapsedSpanAtEnd(cur)) {
      var found$1 = merged.find(0, true);
      len -= cur.text.length - found$1.from.ch;
      cur = found$1.to.line;
      len += cur.text.length - found$1.to.ch;
    }
    return len
  }

  // Find the longest line in the document.
  function findMaxLine(cm) {
    var d = cm.display, doc = cm.doc;
    d.maxLine = getLine(doc, doc.first);
    d.maxLineLength = lineLength(d.maxLine);
    d.maxLineChanged = true;
    doc.iter(function (line) {
      var len = lineLength(line);
      if (len > d.maxLineLength) {
        d.maxLineLength = len;
        d.maxLine = line;
      }
    });
  }

  // LINE DATA STRUCTURE

  // Line objects. These hold state related to a line, including
  // highlighting info (the styles array).
  var Line = function(text, markedSpans, estimateHeight) {
    this.text = text;
    attachMarkedSpans(this, markedSpans);
    this.height = estimateHeight ? estimateHeight(this) : 1;
  };

  Line.prototype.lineNo = function () { return lineNo(this) };
  eventMixin(Line);

  // Change the content (text, markers) of a line. Automatically
  // invalidates cached information and tries to re-estimate the
  // line's height.
  function updateLine(line, text, markedSpans, estimateHeight) {
    line.text = text;
    if (line.stateAfter) { line.stateAfter = null; }
    if (line.styles) { line.styles = null; }
    if (line.order != null) { line.order = null; }
    detachMarkedSpans(line);
    attachMarkedSpans(line, markedSpans);
    var estHeight = estimateHeight ? estimateHeight(line) : 1;
    if (estHeight != line.height) { updateLineHeight(line, estHeight); }
  }

  // Detach a line from the document tree and its markers.
  function cleanUpLine(line) {
    line.parent = null;
    detachMarkedSpans(line);
  }

  // Convert a style as returned by a mode (either null, or a string
  // containing one or more styles) to a CSS style. This is cached,
  // and also looks for line-wide styles.
  var styleToClassCache = {}, styleToClassCacheWithMode = {};
  function interpretTokenStyle(style, options) {
    if (!style || /^\s*$/.test(style)) { return null }
    var cache = options.addModeClass ? styleToClassCacheWithMode : styleToClassCache;
    return cache[style] ||
      (cache[style] = style.replace(/\S+/g, "cm-$&"))
  }

  // Render the DOM representation of the text of a line. Also builds
  // up a 'line map', which points at the DOM nodes that represent
  // specific stretches of text, and is used by the measuring code.
  // The returned object contains the DOM node, this map, and
  // information about line-wide styles that were set by the mode.
  function buildLineContent(cm, lineView) {
    // The padding-right forces the element to have a 'border', which
    // is needed on Webkit to be able to get line-level bounding
    // rectangles for it (in measureChar).
    var content = eltP("span", null, null, webkit ? "padding-right: .1px" : null);
    var builder = {pre: eltP("pre", [content], "CodeMirror-line"), content: content,
                   col: 0, pos: 0, cm: cm,
                   trailingSpace: false,
                   splitSpaces: cm.getOption("lineWrapping")};
    lineView.measure = {};

    // Iterate over the logical lines that make up this visual line.
    for (var i = 0; i <= (lineView.rest ? lineView.rest.length : 0); i++) {
      var line = i ? lineView.rest[i - 1] : lineView.line, order = (void 0);
      builder.pos = 0;
      builder.addToken = buildToken;
      // Optionally wire in some hacks into the token-rendering
      // algorithm, to deal with browser quirks.
      if (hasBadBidiRects(cm.display.measure) && (order = getOrder(line, cm.doc.direction)))
        { builder.addToken = buildTokenBadBidi(builder.addToken, order); }
      builder.map = [];
      var allowFrontierUpdate = lineView != cm.display.externalMeasured && lineNo(line);
      insertLineContent(line, builder, getLineStyles(cm, line, allowFrontierUpdate));
      if (line.styleClasses) {
        if (line.styleClasses.bgClass)
          { builder.bgClass = joinClasses(line.styleClasses.bgClass, builder.bgClass || ""); }
        if (line.styleClasses.textClass)
          { builder.textClass = joinClasses(line.styleClasses.textClass, builder.textClass || ""); }
      }

      // Ensure at least a single node is present, for measuring.
      if (builder.map.length == 0)
        { builder.map.push(0, 0, builder.content.appendChild(zeroWidthElement(cm.display.measure))); }

      // Store the map and a cache object for the current logical line
      if (i == 0) {
        lineView.measure.map = builder.map;
        lineView.measure.cache = {};
      } else {
  (lineView.measure.maps || (lineView.measure.maps = [])).push(builder.map)
        ;(lineView.measure.caches || (lineView.measure.caches = [])).push({});
      }
    }

    // See issue #2901
    if (webkit) {
      var last = builder.content.lastChild;
      if (/\bcm-tab\b/.test(last.className) || (last.querySelector && last.querySelector(".cm-tab")))
        { builder.content.className = "cm-tab-wrap-hack"; }
    }

    signal(cm, "renderLine", cm, lineView.line, builder.pre);
    if (builder.pre.className)
      { builder.textClass = joinClasses(builder.pre.className, builder.textClass || ""); }

    return builder
  }

  function defaultSpecialCharPlaceholder(ch) {
    var token = elt("span", "\u2022", "cm-invalidchar");
    token.title = "\\u" + ch.charCodeAt(0).toString(16);
    token.setAttribute("aria-label", token.title);
    return token
  }

  // Build up the DOM representation for a single token, and add it to
  // the line map. Takes care to render special characters separately.
  function buildToken(builder, text, style, startStyle, endStyle, css, attributes) {
    if (!text) { return }
    var displayText = builder.splitSpaces ? splitSpaces(text, builder.trailingSpace) : text;
    var special = builder.cm.state.specialChars, mustWrap = false;
    var content;
    if (!special.test(text)) {
      builder.col += text.length;
      content = document.createTextNode(displayText);
      builder.map.push(builder.pos, builder.pos + text.length, content);
      if (ie && ie_version < 9) { mustWrap = true; }
      builder.pos += text.length;
    } else {
      content = document.createDocumentFragment();
      var pos = 0;
      while (true) {
        special.lastIndex = pos;
        var m = special.exec(text);
        var skipped = m ? m.index - pos : text.length - pos;
        if (skipped) {
          var txt = document.createTextNode(displayText.slice(pos, pos + skipped));
          if (ie && ie_version < 9) { content.appendChild(elt("span", [txt])); }
          else { content.appendChild(txt); }
          builder.map.push(builder.pos, builder.pos + skipped, txt);
          builder.col += skipped;
          builder.pos += skipped;
        }
        if (!m) { break }
        pos += skipped + 1;
        var txt$1 = (void 0);
        if (m[0] == "\t") {
          var tabSize = builder.cm.options.tabSize, tabWidth = tabSize - builder.col % tabSize;
          txt$1 = content.appendChild(elt("span", spaceStr(tabWidth), "cm-tab"));
          txt$1.setAttribute("role", "presentation");
          txt$1.setAttribute("cm-text", "\t");
          builder.col += tabWidth;
        } else if (m[0] == "\r" || m[0] == "\n") {
          txt$1 = content.appendChild(elt("span", m[0] == "\r" ? "\u240d" : "\u2424", "cm-invalidchar"));
          txt$1.setAttribute("cm-text", m[0]);
          builder.col += 1;
        } else {
          txt$1 = builder.cm.options.specialCharPlaceholder(m[0]);
          txt$1.setAttribute("cm-text", m[0]);
          if (ie && ie_version < 9) { content.appendChild(elt("span", [txt$1])); }
          else { content.appendChild(txt$1); }
          builder.col += 1;
        }
        builder.map.push(builder.pos, builder.pos + 1, txt$1);
        builder.pos++;
      }
    }
    builder.trailingSpace = displayText.charCodeAt(text.length - 1) == 32;
    if (style || startStyle || endStyle || mustWrap || css) {
      var fullStyle = style || "";
      if (startStyle) { fullStyle += startStyle; }
      if (endStyle) { fullStyle += endStyle; }
      var token = elt("span", [content], fullStyle, css);
      if (attributes) {
        for (var attr in attributes) { if (attributes.hasOwnProperty(attr) && attr != "style" && attr != "class")
          { token.setAttribute(attr, attributes[attr]); } }
      }
      return builder.content.appendChild(token)
    }
    builder.content.appendChild(content);
  }

  // Change some spaces to NBSP to prevent the browser from collapsing
  // trailing spaces at the end of a line when rendering text (issue #1362).
  function splitSpaces(text, trailingBefore) {
    if (text.length > 1 && !/  /.test(text)) { return text }
    var spaceBefore = trailingBefore, result = "";
    for (var i = 0; i < text.length; i++) {
      var ch = text.charAt(i);
      if (ch == " " && spaceBefore && (i == text.length - 1 || text.charCodeAt(i + 1) == 32))
        { ch = "\u00a0"; }
      result += ch;
      spaceBefore = ch == " ";
    }
    return result
  }

  // Work around nonsense dimensions being reported for stretches of
  // right-to-left text.
  function buildTokenBadBidi(inner, order) {
    return function (builder, text, style, startStyle, endStyle, css, attributes) {
      style = style ? style + " cm-force-border" : "cm-force-border";
      var start = builder.pos, end = start + text.length;
      for (;;) {
        // Find the part that overlaps with the start of this text
        var part = (void 0);
        for (var i = 0; i < order.length; i++) {
          part = order[i];
          if (part.to > start && part.from <= start) { break }
        }
        if (part.to >= end) { return inner(builder, text, style, startStyle, endStyle, css, attributes) }
        inner(builder, text.slice(0, part.to - start), style, startStyle, null, css, attributes);
        startStyle = null;
        text = text.slice(part.to - start);
        start = part.to;
      }
    }
  }

  function buildCollapsedSpan(builder, size, marker, ignoreWidget) {
    var widget = !ignoreWidget && marker.widgetNode;
    if (widget) { builder.map.push(builder.pos, builder.pos + size, widget); }
    if (!ignoreWidget && builder.cm.display.input.needsContentAttribute) {
      if (!widget)
        { widget = builder.content.appendChild(document.createElement("span")); }
      widget.setAttribute("cm-marker", marker.id);
    }
    if (widget) {
      builder.cm.display.input.setUneditable(widget);
      builder.content.appendChild(widget);
    }
    builder.pos += size;
    builder.trailingSpace = false;
  }

  // Outputs a number of spans to make up a line, taking highlighting
  // and marked text into account.
  function insertLineContent(line, builder, styles) {
    var spans = line.markedSpans, allText = line.text, at = 0;
    if (!spans) {
      for (var i$1 = 1; i$1 < styles.length; i$1+=2)
        { builder.addToken(builder, allText.slice(at, at = styles[i$1]), interpretTokenStyle(styles[i$1+1], builder.cm.options)); }
      return
    }

    var len = allText.length, pos = 0, i = 1, text = "", style, css;
    var nextChange = 0, spanStyle, spanEndStyle, spanStartStyle, collapsed, attributes;
    for (;;) {
      if (nextChange == pos) { // Update current marker set
        spanStyle = spanEndStyle = spanStartStyle = css = "";
        attributes = null;
        collapsed = null; nextChange = Infinity;
        var foundBookmarks = [], endStyles = (void 0);
        for (var j = 0; j < spans.length; ++j) {
          var sp = spans[j], m = sp.marker;
          if (m.type == "bookmark" && sp.from == pos && m.widgetNode) {
            foundBookmarks.push(m);
          } else if (sp.from <= pos && (sp.to == null || sp.to > pos || m.collapsed && sp.to == pos && sp.from == pos)) {
            if (sp.to != null && sp.to != pos && nextChange > sp.to) {
              nextChange = sp.to;
              spanEndStyle = "";
            }
            if (m.className) { spanStyle += " " + m.className; }
            if (m.css) { css = (css ? css + ";" : "") + m.css; }
            if (m.startStyle && sp.from == pos) { spanStartStyle += " " + m.startStyle; }
            if (m.endStyle && sp.to == nextChange) { (endStyles || (endStyles = [])).push(m.endStyle, sp.to); }
            // support for the old title property
            // https://github.com/codemirror/CodeMirror/pull/5673
            if (m.title) { (attributes || (attributes = {})).title = m.title; }
            if (m.attributes) {
              for (var attr in m.attributes)
                { (attributes || (attributes = {}))[attr] = m.attributes[attr]; }
            }
            if (m.collapsed && (!collapsed || compareCollapsedMarkers(collapsed.marker, m) < 0))
              { collapsed = sp; }
          } else if (sp.from > pos && nextChange > sp.from) {
            nextChange = sp.from;
          }
        }
        if (endStyles) { for (var j$1 = 0; j$1 < endStyles.length; j$1 += 2)
          { if (endStyles[j$1 + 1] == nextChange) { spanEndStyle += " " + endStyles[j$1]; } } }

        if (!collapsed || collapsed.from == pos) { for (var j$2 = 0; j$2 < foundBookmarks.length; ++j$2)
          { buildCollapsedSpan(builder, 0, foundBookmarks[j$2]); } }
        if (collapsed && (collapsed.from || 0) == pos) {
          buildCollapsedSpan(builder, (collapsed.to == null ? len + 1 : collapsed.to) - pos,
                             collapsed.marker, collapsed.from == null);
          if (collapsed.to == null) { return }
          if (collapsed.to == pos) { collapsed = false; }
        }
      }
      if (pos >= len) { break }

      var upto = Math.min(len, nextChange);
      while (true) {
        if (text) {
          var end = pos + text.length;
          if (!collapsed) {
            var tokenText = end > upto ? text.slice(0, upto - pos) : text;
            builder.addToken(builder, tokenText, style ? style + spanStyle : spanStyle,
                             spanStartStyle, pos + tokenText.length == nextChange ? spanEndStyle : "", css, attributes);
          }
          if (end >= upto) {text = text.slice(upto - pos); pos = upto; break}
          pos = end;
          spanStartStyle = "";
        }
        text = allText.slice(at, at = styles[i++]);
        style = interpretTokenStyle(styles[i++], builder.cm.options);
      }
    }
  }


  // These objects are used to represent the visible (currently drawn)
  // part of the document. A LineView may correspond to multiple
  // logical lines, if those are connected by collapsed ranges.
  function LineView(doc, line, lineN) {
    // The starting line
    this.line = line;
    // Continuing lines, if any
    this.rest = visualLineContinued(line);
    // Number of logical lines in this visual line
    this.size = this.rest ? lineNo(lst(this.rest)) - lineN + 1 : 1;
    this.node = this.text = null;
    this.hidden = lineIsHidden(doc, line);
  }

  // Create a range of LineView objects for the given lines.
  function buildViewArray(cm, from, to) {
    var array = [], nextPos;
    for (var pos = from; pos < to; pos = nextPos) {
      var view = new LineView(cm.doc, getLine(cm.doc, pos), pos);
      nextPos = pos + view.size;
      array.push(view);
    }
    return array
  }

  var operationGroup = null;

  function pushOperation(op) {
    if (operationGroup) {
      operationGroup.ops.push(op);
    } else {
      op.ownsGroup = operationGroup = {
        ops: [op],
        delayedCallbacks: []
      };
    }
  }

  function fireCallbacksForOps(group) {
    // Calls delayed callbacks and cursorActivity handlers until no
    // new ones appear
    var callbacks = group.delayedCallbacks, i = 0;
    do {
      for (; i < callbacks.length; i++)
        { callbacks[i].call(null); }
      for (var j = 0; j < group.ops.length; j++) {
        var op = group.ops[j];
        if (op.cursorActivityHandlers)
          { while (op.cursorActivityCalled < op.cursorActivityHandlers.length)
            { op.cursorActivityHandlers[op.cursorActivityCalled++].call(null, op.cm); } }
      }
    } while (i < callbacks.length)
  }

  function finishOperation(op, endCb) {
    var group = op.ownsGroup;
    if (!group) { return }

    try { fireCallbacksForOps(group); }
    finally {
      operationGroup = null;
      endCb(group);
    }
  }

  var orphanDelayedCallbacks = null;

  // Often, we want to signal events at a point where we are in the
  // middle of some work, but don't want the handler to start calling
  // other methods on the editor, which might be in an inconsistent
  // state or simply not expect any other events to happen.
  // signalLater looks whether there are any handlers, and schedules
  // them to be executed when the last operation ends, or, if no
  // operation is active, when a timeout fires.
  function signalLater(emitter, type /*, values...*/) {
    var arr = getHandlers(emitter, type);
    if (!arr.length) { return }
    var args = Array.prototype.slice.call(arguments, 2), list;
    if (operationGroup) {
      list = operationGroup.delayedCallbacks;
    } else if (orphanDelayedCallbacks) {
      list = orphanDelayedCallbacks;
    } else {
      list = orphanDelayedCallbacks = [];
      setTimeout(fireOrphanDelayed, 0);
    }
    var loop = function ( i ) {
      list.push(function () { return arr[i].apply(null, args); });
    };

    for (var i = 0; i < arr.length; ++i)
      loop( i );
  }

  function fireOrphanDelayed() {
    var delayed = orphanDelayedCallbacks;
    orphanDelayedCallbacks = null;
    for (var i = 0; i < delayed.length; ++i) { delayed[i](); }
  }

  // When an aspect of a line changes, a string is added to
  // lineView.changes. This updates the relevant part of the line's
  // DOM structure.
  function updateLineForChanges(cm, lineView, lineN, dims) {
    for (var j = 0; j < lineView.changes.length; j++) {
      var type = lineView.changes[j];
      if (type == "text") { updateLineText(cm, lineView); }
      else if (type == "gutter") { updateLineGutter(cm, lineView, lineN, dims); }
      else if (type == "class") { updateLineClasses(cm, lineView); }
      else if (type == "widget") { updateLineWidgets(cm, lineView, dims); }
    }
    lineView.changes = null;
  }

  // Lines with gutter elements, widgets or a background class need to
  // be wrapped, and have the extra elements added to the wrapper div
  function ensureLineWrapped(lineView) {
    if (lineView.node == lineView.text) {
      lineView.node = elt("div", null, null, "position: relative");
      if (lineView.text.parentNode)
        { lineView.text.parentNode.replaceChild(lineView.node, lineView.text); }
      lineView.node.appendChild(lineView.text);
      if (ie && ie_version < 8) { lineView.node.style.zIndex = 2; }
    }
    return lineView.node
  }

  function updateLineBackground(cm, lineView) {
    var cls = lineView.bgClass ? lineView.bgClass + " " + (lineView.line.bgClass || "") : lineView.line.bgClass;
    if (cls) { cls += " CodeMirror-linebackground"; }
    if (lineView.background) {
      if (cls) { lineView.background.className = cls; }
      else { lineView.background.parentNode.removeChild(lineView.background); lineView.background = null; }
    } else if (cls) {
      var wrap = ensureLineWrapped(lineView);
      lineView.background = wrap.insertBefore(elt("div", null, cls), wrap.firstChild);
      cm.display.input.setUneditable(lineView.background);
    }
  }

  // Wrapper around buildLineContent which will reuse the structure
  // in display.externalMeasured when possible.
  function getLineContent(cm, lineView) {
    var ext = cm.display.externalMeasured;
    if (ext && ext.line == lineView.line) {
      cm.display.externalMeasured = null;
      lineView.measure = ext.measure;
      return ext.built
    }
    return buildLineContent(cm, lineView)
  }

  // Redraw the line's text. Interacts with the background and text
  // classes because the mode may output tokens that influence these
  // classes.
  function updateLineText(cm, lineView) {
    var cls = lineView.text.className;
    var built = getLineContent(cm, lineView);
    if (lineView.text == lineView.node) { lineView.node = built.pre; }
    lineView.text.parentNode.replaceChild(built.pre, lineView.text);
    lineView.text = built.pre;
    if (built.bgClass != lineView.bgClass || built.textClass != lineView.textClass) {
      lineView.bgClass = built.bgClass;
      lineView.textClass = built.textClass;
      updateLineClasses(cm, lineView);
    } else if (cls) {
      lineView.text.className = cls;
    }
  }

  function updateLineClasses(cm, lineView) {
    updateLineBackground(cm, lineView);
    if (lineView.line.wrapClass)
      { ensureLineWrapped(lineView).className = lineView.line.wrapClass; }
    else if (lineView.node != lineView.text)
      { lineView.node.className = ""; }
    var textClass = lineView.textClass ? lineView.textClass + " " + (lineView.line.textClass || "") : lineView.line.textClass;
    lineView.text.className = textClass || "";
  }

  function updateLineGutter(cm, lineView, lineN, dims) {
    if (lineView.gutter) {
      lineView.node.removeChild(lineView.gutter);
      lineView.gutter = null;
    }
    if (lineView.gutterBackground) {
      lineView.node.removeChild(lineView.gutterBackground);
      lineView.gutterBackground = null;
    }
    if (lineView.line.gutterClass) {
      var wrap = ensureLineWrapped(lineView);
      lineView.gutterBackground = elt("div", null, "CodeMirror-gutter-background " + lineView.line.gutterClass,
                                      ("left: " + (cm.options.fixedGutter ? dims.fixedPos : -dims.gutterTotalWidth) + "px; width: " + (dims.gutterTotalWidth) + "px"));
      cm.display.input.setUneditable(lineView.gutterBackground);
      wrap.insertBefore(lineView.gutterBackground, lineView.text);
    }
    var markers = lineView.line.gutterMarkers;
    if (cm.options.lineNumbers || markers) {
      var wrap$1 = ensureLineWrapped(lineView);
      var gutterWrap = lineView.gutter = elt("div", null, "CodeMirror-gutter-wrapper", ("left: " + (cm.options.fixedGutter ? dims.fixedPos : -dims.gutterTotalWidth) + "px"));
      cm.display.input.setUneditable(gutterWrap);
      wrap$1.insertBefore(gutterWrap, lineView.text);
      if (lineView.line.gutterClass)
        { gutterWrap.className += " " + lineView.line.gutterClass; }
      if (cm.options.lineNumbers && (!markers || !markers["CodeMirror-linenumbers"]))
        { lineView.lineNumber = gutterWrap.appendChild(
          elt("div", lineNumberFor(cm.options, lineN),
              "CodeMirror-linenumber CodeMirror-gutter-elt",
              ("left: " + (dims.gutterLeft["CodeMirror-linenumbers"]) + "px; width: " + (cm.display.lineNumInnerWidth) + "px"))); }
      if (markers) { for (var k = 0; k < cm.display.gutterSpecs.length; ++k) {
        var id = cm.display.gutterSpecs[k].className, found = markers.hasOwnProperty(id) && markers[id];
        if (found)
          { gutterWrap.appendChild(elt("div", [found], "CodeMirror-gutter-elt",
                                     ("left: " + (dims.gutterLeft[id]) + "px; width: " + (dims.gutterWidth[id]) + "px"))); }
      } }
    }
  }

  function updateLineWidgets(cm, lineView, dims) {
    if (lineView.alignable) { lineView.alignable = null; }
    for (var node = lineView.node.firstChild, next = (void 0); node; node = next) {
      next = node.nextSibling;
      if (node.className == "CodeMirror-linewidget")
        { lineView.node.removeChild(node); }
    }
    insertLineWidgets(cm, lineView, dims);
  }

  // Build a line's DOM representation from scratch
  function buildLineElement(cm, lineView, lineN, dims) {
    var built = getLineContent(cm, lineView);
    lineView.text = lineView.node = built.pre;
    if (built.bgClass) { lineView.bgClass = built.bgClass; }
    if (built.textClass) { lineView.textClass = built.textClass; }

    updateLineClasses(cm, lineView);
    updateLineGutter(cm, lineView, lineN, dims);
    insertLineWidgets(cm, lineView, dims);
    return lineView.node
  }

  // A lineView may contain multiple logical lines (when merged by
  // collapsed spans). The widgets for all of them need to be drawn.
  function insertLineWidgets(cm, lineView, dims) {
    insertLineWidgetsFor(cm, lineView.line, lineView, dims, true);
    if (lineView.rest) { for (var i = 0; i < lineView.rest.length; i++)
      { insertLineWidgetsFor(cm, lineView.rest[i], lineView, dims, false); } }
  }

  function insertLineWidgetsFor(cm, line, lineView, dims, allowAbove) {
    if (!line.widgets) { return }
    var wrap = ensureLineWrapped(lineView);
    for (var i = 0, ws = line.widgets; i < ws.length; ++i) {
      var widget = ws[i], node = elt("div", [widget.node], "CodeMirror-linewidget");
      if (!widget.handleMouseEvents) { node.setAttribute("cm-ignore-events", "true"); }
      positionLineWidget(widget, node, lineView, dims);
      cm.display.input.setUneditable(node);
      if (allowAbove && widget.above)
        { wrap.insertBefore(node, lineView.gutter || lineView.text); }
      else
        { wrap.appendChild(node); }
      signalLater(widget, "redraw");
    }
  }

  function positionLineWidget(widget, node, lineView, dims) {
    if (widget.noHScroll) {
  (lineView.alignable || (lineView.alignable = [])).push(node);
      var width = dims.wrapperWidth;
      node.style.left = dims.fixedPos + "px";
      if (!widget.coverGutter) {
        width -= dims.gutterTotalWidth;
        node.style.paddingLeft = dims.gutterTotalWidth + "px";
      }
      node.style.width = width + "px";
    }
    if (widget.coverGutter) {
      node.style.zIndex = 5;
      node.style.position = "relative";
      if (!widget.noHScroll) { node.style.marginLeft = -dims.gutterTotalWidth + "px"; }
    }
  }

  function widgetHeight(widget) {
    if (widget.height != null) { return widget.height }
    var cm = widget.doc.cm;
    if (!cm) { return 0 }
    if (!contains(document.body, widget.node)) {
      var parentStyle = "position: relative;";
      if (widget.coverGutter)
        { parentStyle += "margin-left: -" + cm.display.gutters.offsetWidth + "px;"; }
      if (widget.noHScroll)
        { parentStyle += "width: " + cm.display.wrapper.clientWidth + "px;"; }
      removeChildrenAndAdd(cm.display.measure, elt("div", [widget.node], null, parentStyle));
    }
    return widget.height = widget.node.parentNode.offsetHeight
  }

  // Return true when the given mouse event happened in a widget
  function eventInWidget(display, e) {
    for (var n = e_target(e); n != display.wrapper; n = n.parentNode) {
      if (!n || (n.nodeType == 1 && n.getAttribute("cm-ignore-events") == "true") ||
          (n.parentNode == display.sizer && n != display.mover))
        { return true }
    }
  }

  // POSITION MEASUREMENT

  function paddingTop(display) {return display.lineSpace.offsetTop}
  function paddingVert(display) {return display.mover.offsetHeight - display.lineSpace.offsetHeight}
  function paddingH(display) {
    if (display.cachedPaddingH) { return display.cachedPaddingH }
    var e = removeChildrenAndAdd(display.measure, elt("pre", "x", "CodeMirror-line-like"));
    var style = window.getComputedStyle ? window.getComputedStyle(e) : e.currentStyle;
    var data = {left: parseInt(style.paddingLeft), right: parseInt(style.paddingRight)};
    if (!isNaN(data.left) && !isNaN(data.right)) { display.cachedPaddingH = data; }
    return data
  }

  function scrollGap(cm) { return scrollerGap - cm.display.nativeBarWidth }
  function displayWidth(cm) {
    return cm.display.scroller.clientWidth - scrollGap(cm) - cm.display.barWidth
  }
  function displayHeight(cm) {
    return cm.display.scroller.clientHeight - scrollGap(cm) - cm.display.barHeight
  }

  // Ensure the lineView.wrapping.heights array is populated. This is
  // an array of bottom offsets for the lines that make up a drawn
  // line. When lineWrapping is on, there might be more than one
  // height.
  function ensureLineHeights(cm, lineView, rect) {
    var wrapping = cm.options.lineWrapping;
    var curWidth = wrapping && displayWidth(cm);
    if (!lineView.measure.heights || wrapping && lineView.measure.width != curWidth) {
      var heights = lineView.measure.heights = [];
      if (wrapping) {
        lineView.measure.width = curWidth;
        var rects = lineView.text.firstChild.getClientRects();
        for (var i = 0; i < rects.length - 1; i++) {
          var cur = rects[i], next = rects[i + 1];
          if (Math.abs(cur.bottom - next.bottom) > 2)
            { heights.push((cur.bottom + next.top) / 2 - rect.top); }
        }
      }
      heights.push(rect.bottom - rect.top);
    }
  }

  // Find a line map (mapping character offsets to text nodes) and a
  // measurement cache for the given line number. (A line view might
  // contain multiple lines when collapsed ranges are present.)
  function mapFromLineView(lineView, line, lineN) {
    if (lineView.line == line)
      { return {map: lineView.measure.map, cache: lineView.measure.cache} }
    for (var i = 0; i < lineView.rest.length; i++)
      { if (lineView.rest[i] == line)
        { return {map: lineView.measure.maps[i], cache: lineView.measure.caches[i]} } }
    for (var i$1 = 0; i$1 < lineView.rest.length; i$1++)
      { if (lineNo(lineView.rest[i$1]) > lineN)
        { return {map: lineView.measure.maps[i$1], cache: lineView.measure.caches[i$1], before: true} } }
  }

  // Render a line into the hidden node display.externalMeasured. Used
  // when measurement is needed for a line that's not in the viewport.
  function updateExternalMeasurement(cm, line) {
    line = visualLine(line);
    var lineN = lineNo(line);
    var view = cm.display.externalMeasured = new LineView(cm.doc, line, lineN);
    view.lineN = lineN;
    var built = view.built = buildLineContent(cm, view);
    view.text = built.pre;
    removeChildrenAndAdd(cm.display.lineMeasure, built.pre);
    return view
  }

  // Get a {top, bottom, left, right} box (in line-local coordinates)
  // for a given character.
  function measureChar(cm, line, ch, bias) {
    return measureCharPrepared(cm, prepareMeasureForLine(cm, line), ch, bias)
  }

  // Find a line view that corresponds to the given line number.
  function findViewForLine(cm, lineN) {
    if (lineN >= cm.display.viewFrom && lineN < cm.display.viewTo)
      { return cm.display.view[findViewIndex(cm, lineN)] }
    var ext = cm.display.externalMeasured;
    if (ext && lineN >= ext.lineN && lineN < ext.lineN + ext.size)
      { return ext }
  }

  // Measurement can be split in two steps, the set-up work that
  // applies to the whole line, and the measurement of the actual
  // character. Functions like coordsChar, that need to do a lot of
  // measurements in a row, can thus ensure that the set-up work is
  // only done once.
  function prepareMeasureForLine(cm, line) {
    var lineN = lineNo(line);
    var view = findViewForLine(cm, lineN);
    if (view && !view.text) {
      view = null;
    } else if (view && view.changes) {
      updateLineForChanges(cm, view, lineN, getDimensions(cm));
      cm.curOp.forceUpdate = true;
    }
    if (!view)
      { view = updateExternalMeasurement(cm, line); }

    var info = mapFromLineView(view, line, lineN);
    return {
      line: line, view: view, rect: null,
      map: info.map, cache: info.cache, before: info.before,
      hasHeights: false
    }
  }

  // Given a prepared measurement object, measures the position of an
  // actual character (or fetches it from the cache).
  function measureCharPrepared(cm, prepared, ch, bias, varHeight) {
    if (prepared.before) { ch = -1; }
    var key = ch + (bias || ""), found;
    if (prepared.cache.hasOwnProperty(key)) {
      found = prepared.cache[key];
    } else {
      if (!prepared.rect)
        { prepared.rect = prepared.view.text.getBoundingClientRect(); }
      if (!prepared.hasHeights) {
        ensureLineHeights(cm, prepared.view, prepared.rect);
        prepared.hasHeights = true;
      }
      found = measureCharInner(cm, prepared, ch, bias);
      if (!found.bogus) { prepared.cache[key] = found; }
    }
    return {left: found.left, right: found.right,
            top: varHeight ? found.rtop : found.top,
            bottom: varHeight ? found.rbottom : found.bottom}
  }

  var nullRect = {left: 0, right: 0, top: 0, bottom: 0};

  function nodeAndOffsetInLineMap(map$$1, ch, bias) {
    var node, start, end, collapse, mStart, mEnd;
    // First, search the line map for the text node corresponding to,
    // or closest to, the target character.
    for (var i = 0; i < map$$1.length; i += 3) {
      mStart = map$$1[i];
      mEnd = map$$1[i + 1];
      if (ch < mStart) {
        start = 0; end = 1;
        collapse = "left";
      } else if (ch < mEnd) {
        start = ch - mStart;
        end = start + 1;
      } else if (i == map$$1.length - 3 || ch == mEnd && map$$1[i + 3] > ch) {
        end = mEnd - mStart;
        start = end - 1;
        if (ch >= mEnd) { collapse = "right"; }
      }
      if (start != null) {
        node = map$$1[i + 2];
        if (mStart == mEnd && bias == (node.insertLeft ? "left" : "right"))
          { collapse = bias; }
        if (bias == "left" && start == 0)
          { while (i && map$$1[i - 2] == map$$1[i - 3] && map$$1[i - 1].insertLeft) {
            node = map$$1[(i -= 3) + 2];
            collapse = "left";
          } }
        if (bias == "right" && start == mEnd - mStart)
          { while (i < map$$1.length - 3 && map$$1[i + 3] == map$$1[i + 4] && !map$$1[i + 5].insertLeft) {
            node = map$$1[(i += 3) + 2];
            collapse = "right";
          } }
        break
      }
    }
    return {node: node, start: start, end: end, collapse: collapse, coverStart: mStart, coverEnd: mEnd}
  }

  function getUsefulRect(rects, bias) {
    var rect = nullRect;
    if (bias == "left") { for (var i = 0; i < rects.length; i++) {
      if ((rect = rects[i]).left != rect.right) { break }
    } } else { for (var i$1 = rects.length - 1; i$1 >= 0; i$1--) {
      if ((rect = rects[i$1]).left != rect.right) { break }
    } }
    return rect
  }

  function measureCharInner(cm, prepared, ch, bias) {
    var place = nodeAndOffsetInLineMap(prepared.map, ch, bias);
    var node = place.node, start = place.start, end = place.end, collapse = place.collapse;

    var rect;
    if (node.nodeType == 3) { // If it is a text node, use a range to retrieve the coordinates.
      for (var i$1 = 0; i$1 < 4; i$1++) { // Retry a maximum of 4 times when nonsense rectangles are returned
        while (start && isExtendingChar(prepared.line.text.charAt(place.coverStart + start))) { --start; }
        while (place.coverStart + end < place.coverEnd && isExtendingChar(prepared.line.text.charAt(place.coverStart + end))) { ++end; }
        if (ie && ie_version < 9 && start == 0 && end == place.coverEnd - place.coverStart)
          { rect = node.parentNode.getBoundingClientRect(); }
        else
          { rect = getUsefulRect(range(node, start, end).getClientRects(), bias); }
        if (rect.left || rect.right || start == 0) { break }
        end = start;
        start = start - 1;
        collapse = "right";
      }
      if (ie && ie_version < 11) { rect = maybeUpdateRectForZooming(cm.display.measure, rect); }
    } else { // If it is a widget, simply get the box for the whole widget.
      if (start > 0) { collapse = bias = "right"; }
      var rects;
      if (cm.options.lineWrapping && (rects = node.getClientRects()).length > 1)
        { rect = rects[bias == "right" ? rects.length - 1 : 0]; }
      else
        { rect = node.getBoundingClientRect(); }
    }
    if (ie && ie_version < 9 && !start && (!rect || !rect.left && !rect.right)) {
      var rSpan = node.parentNode.getClientRects()[0];
      if (rSpan)
        { rect = {left: rSpan.left, right: rSpan.left + charWidth(cm.display), top: rSpan.top, bottom: rSpan.bottom}; }
      else
        { rect = nullRect; }
    }

    var rtop = rect.top - prepared.rect.top, rbot = rect.bottom - prepared.rect.top;
    var mid = (rtop + rbot) / 2;
    var heights = prepared.view.measure.heights;
    var i = 0;
    for (; i < heights.length - 1; i++)
      { if (mid < heights[i]) { break } }
    var top = i ? heights[i - 1] : 0, bot = heights[i];
    var result = {left: (collapse == "right" ? rect.right : rect.left) - prepared.rect.left,
                  right: (collapse == "left" ? rect.left : rect.right) - prepared.rect.left,
                  top: top, bottom: bot};
    if (!rect.left && !rect.right) { result.bogus = true; }
    if (!cm.options.singleCursorHeightPerLine) { result.rtop = rtop; result.rbottom = rbot; }

    return result
  }

  // Work around problem with bounding client rects on ranges being
  // returned incorrectly when zoomed on IE10 and below.
  function maybeUpdateRectForZooming(measure, rect) {
    if (!window.screen || screen.logicalXDPI == null ||
        screen.logicalXDPI == screen.deviceXDPI || !hasBadZoomedRects(measure))
      { return rect }
    var scaleX = screen.logicalXDPI / screen.deviceXDPI;
    var scaleY = screen.logicalYDPI / screen.deviceYDPI;
    return {left: rect.left * scaleX, right: rect.right * scaleX,
            top: rect.top * scaleY, bottom: rect.bottom * scaleY}
  }

  function clearLineMeasurementCacheFor(lineView) {
    if (lineView.measure) {
      lineView.measure.cache = {};
      lineView.measure.heights = null;
      if (lineView.rest) { for (var i = 0; i < lineView.rest.length; i++)
        { lineView.measure.caches[i] = {}; } }
    }
  }

  function clearLineMeasurementCache(cm) {
    cm.display.externalMeasure = null;
    removeChildren(cm.display.lineMeasure);
    for (var i = 0; i < cm.display.view.length; i++)
      { clearLineMeasurementCacheFor(cm.display.view[i]); }
  }

  function clearCaches(cm) {
    clearLineMeasurementCache(cm);
    cm.display.cachedCharWidth = cm.display.cachedTextHeight = cm.display.cachedPaddingH = null;
    if (!cm.options.lineWrapping) { cm.display.maxLineChanged = true; }
    cm.display.lineNumChars = null;
  }

  function pageScrollX() {
    // Work around https://bugs.chromium.org/p/chromium/issues/detail?id=489206
    // which causes page_Offset and bounding client rects to use
    // different reference viewports and invalidate our calculations.
    if (chrome && android) { return -(document.body.getBoundingClientRect().left - parseInt(getComputedStyle(document.body).marginLeft)) }
    return window.pageXOffset || (document.documentElement || document.body).scrollLeft
  }
  function pageScrollY() {
    if (chrome && android) { return -(document.body.getBoundingClientRect().top - parseInt(getComputedStyle(document.body).marginTop)) }
    return window.pageYOffset || (document.documentElement || document.body).scrollTop
  }

  function widgetTopHeight(lineObj) {
    var height = 0;
    if (lineObj.widgets) { for (var i = 0; i < lineObj.widgets.length; ++i) { if (lineObj.widgets[i].above)
      { height += widgetHeight(lineObj.widgets[i]); } } }
    return height
  }

  // Converts a {top, bottom, left, right} box from line-local
  // coordinates into another coordinate system. Context may be one of
  // "line", "div" (display.lineDiv), "local"./null (editor), "window",
  // or "page".
  function intoCoordSystem(cm, lineObj, rect, context, includeWidgets) {
    if (!includeWidgets) {
      var height = widgetTopHeight(lineObj);
      rect.top += height; rect.bottom += height;
    }
    if (context == "line") { return rect }
    if (!context) { context = "local"; }
    var yOff = heightAtLine(lineObj);
    if (context == "local") { yOff += paddingTop(cm.display); }
    else { yOff -= cm.display.viewOffset; }
    if (context == "page" || context == "window") {
      var lOff = cm.display.lineSpace.getBoundingClientRect();
      yOff += lOff.top + (context == "window" ? 0 : pageScrollY());
      var xOff = lOff.left + (context == "window" ? 0 : pageScrollX());
      rect.left += xOff; rect.right += xOff;
    }
    rect.top += yOff; rect.bottom += yOff;
    return rect
  }

  // Coverts a box from "div" coords to another coordinate system.
  // Context may be "window", "page", "div", or "local"./null.
  function fromCoordSystem(cm, coords, context) {
    if (context == "div") { return coords }
    var left = coords.left, top = coords.top;
    // First move into "page" coordinate system
    if (context == "page") {
      left -= pageScrollX();
      top -= pageScrollY();
    } else if (context == "local" || !context) {
      var localBox = cm.display.sizer.getBoundingClientRect();
      left += localBox.left;
      top += localBox.top;
    }

    var lineSpaceBox = cm.display.lineSpace.getBoundingClientRect();
    return {left: left - lineSpaceBox.left, top: top - lineSpaceBox.top}
  }

  function charCoords(cm, pos, context, lineObj, bias) {
    if (!lineObj) { lineObj = getLine(cm.doc, pos.line); }
    return intoCoordSystem(cm, lineObj, measureChar(cm, lineObj, pos.ch, bias), context)
  }

  // Returns a box for a given cursor position, which may have an
  // 'other' property containing the position of the secondary cursor
  // on a bidi boundary.
  // A cursor Pos(line, char, "before") is on the same visual line as `char - 1`
  // and after `char - 1` in writing order of `char - 1`
  // A cursor Pos(line, char, "after") is on the same visual line as `char`
  // and before `char` in writing order of `char`
  // Examples (upper-case letters are RTL, lower-case are LTR):
  //     Pos(0, 1, ...)
  //     before   after
  // ab     a|b     a|b
  // aB     a|B     aB|
  // Ab     |Ab     A|b
  // AB     B|A     B|A
  // Every position after the last character on a line is considered to stick
  // to the last character on the line.
  function cursorCoords(cm, pos, context, lineObj, preparedMeasure, varHeight) {
    lineObj = lineObj || getLine(cm.doc, pos.line);
    if (!preparedMeasure) { preparedMeasure = prepareMeasureForLine(cm, lineObj); }
    function get(ch, right) {
      var m = measureCharPrepared(cm, preparedMeasure, ch, right ? "right" : "left", varHeight);
      if (right) { m.left = m.right; } else { m.right = m.left; }
      return intoCoordSystem(cm, lineObj, m, context)
    }
    var order = getOrder(lineObj, cm.doc.direction), ch = pos.ch, sticky = pos.sticky;
    if (ch >= lineObj.text.length) {
      ch = lineObj.text.length;
      sticky = "before";
    } else if (ch <= 0) {
      ch = 0;
      sticky = "after";
    }
    if (!order) { return get(sticky == "before" ? ch - 1 : ch, sticky == "before") }

    function getBidi(ch, partPos, invert) {
      var part = order[partPos], right = part.level == 1;
      return get(invert ? ch - 1 : ch, right != invert)
    }
    var partPos = getBidiPartAt(order, ch, sticky);
    var other = bidiOther;
    var val = getBidi(ch, partPos, sticky == "before");
    if (other != null) { val.other = getBidi(ch, other, sticky != "before"); }
    return val
  }

  // Used to cheaply estimate the coordinates for a position. Used for
  // intermediate scroll updates.
  function estimateCoords(cm, pos) {
    var left = 0;
    pos = clipPos(cm.doc, pos);
    if (!cm.options.lineWrapping) { left = charWidth(cm.display) * pos.ch; }
    var lineObj = getLine(cm.doc, pos.line);
    var top = heightAtLine(lineObj) + paddingTop(cm.display);
    return {left: left, right: left, top: top, bottom: top + lineObj.height}
  }

  // Positions returned by coordsChar contain some extra information.
  // xRel is the relative x position of the input coordinates compared
  // to the found position (so xRel > 0 means the coordinates are to
  // the right of the character position, for example). When outside
  // is true, that means the coordinates lie outside the line's
  // vertical range.
  function PosWithInfo(line, ch, sticky, outside, xRel) {
    var pos = Pos(line, ch, sticky);
    pos.xRel = xRel;
    if (outside) { pos.outside = outside; }
    return pos
  }

  // Compute the character position closest to the given coordinates.
  // Input must be lineSpace-local ("div" coordinate system).
  function coordsChar(cm, x, y) {
    var doc = cm.doc;
    y += cm.display.viewOffset;
    if (y < 0) { return PosWithInfo(doc.first, 0, null, -1, -1) }
    var lineN = lineAtHeight(doc, y), last = doc.first + doc.size - 1;
    if (lineN > last)
      { return PosWithInfo(doc.first + doc.size - 1, getLine(doc, last).text.length, null, 1, 1) }
    if (x < 0) { x = 0; }

    var lineObj = getLine(doc, lineN);
    for (;;) {
      var found = coordsCharInner(cm, lineObj, lineN, x, y);
      var collapsed = collapsedSpanAround(lineObj, found.ch + (found.xRel > 0 || found.outside > 0 ? 1 : 0));
      if (!collapsed) { return found }
      var rangeEnd = collapsed.find(1);
      if (rangeEnd.line == lineN) { return rangeEnd }
      lineObj = getLine(doc, lineN = rangeEnd.line);
    }
  }

  function wrappedLineExtent(cm, lineObj, preparedMeasure, y) {
    y -= widgetTopHeight(lineObj);
    var end = lineObj.text.length;
    var begin = findFirst(function (ch) { return measureCharPrepared(cm, preparedMeasure, ch - 1).bottom <= y; }, end, 0);
    end = findFirst(function (ch) { return measureCharPrepared(cm, preparedMeasure, ch).top > y; }, begin, end);
    return {begin: begin, end: end}
  }

  function wrappedLineExtentChar(cm, lineObj, preparedMeasure, target) {
    if (!preparedMeasure) { preparedMeasure = prepareMeasureForLine(cm, lineObj); }
    var targetTop = intoCoordSystem(cm, lineObj, measureCharPrepared(cm, preparedMeasure, target), "line").top;
    return wrappedLineExtent(cm, lineObj, preparedMeasure, targetTop)
  }

  // Returns true if the given side of a box is after the given
  // coordinates, in top-to-bottom, left-to-right order.
  function boxIsAfter(box, x, y, left) {
    return box.bottom <= y ? false : box.top > y ? true : (left ? box.left : box.right) > x
  }

  function coordsCharInner(cm, lineObj, lineNo$$1, x, y) {
    // Move y into line-local coordinate space
    y -= heightAtLine(lineObj);
    var preparedMeasure = prepareMeasureForLine(cm, lineObj);
    // When directly calling `measureCharPrepared`, we have to adjust
    // for the widgets at this line.
    var widgetHeight$$1 = widgetTopHeight(lineObj);
    var begin = 0, end = lineObj.text.length, ltr = true;

    var order = getOrder(lineObj, cm.doc.direction);
    // If the line isn't plain left-to-right text, first figure out
    // which bidi section the coordinates fall into.
    if (order) {
      var part = (cm.options.lineWrapping ? coordsBidiPartWrapped : coordsBidiPart)
                   (cm, lineObj, lineNo$$1, preparedMeasure, order, x, y);
      ltr = part.level != 1;
      // The awkward -1 offsets are needed because findFirst (called
      // on these below) will treat its first bound as inclusive,
      // second as exclusive, but we want to actually address the
      // characters in the part's range
      begin = ltr ? part.from : part.to - 1;
      end = ltr ? part.to : part.from - 1;
    }

    // A binary search to find the first character whose bounding box
    // starts after the coordinates. If we run across any whose box wrap
    // the coordinates, store that.
    var chAround = null, boxAround = null;
    var ch = findFirst(function (ch) {
      var box = measureCharPrepared(cm, preparedMeasure, ch);
      box.top += widgetHeight$$1; box.bottom += widgetHeight$$1;
      if (!boxIsAfter(box, x, y, false)) { return false }
      if (box.top <= y && box.left <= x) {
        chAround = ch;
        boxAround = box;
      }
      return true
    }, begin, end);

    var baseX, sticky, outside = false;
    // If a box around the coordinates was found, use that
    if (boxAround) {
      // Distinguish coordinates nearer to the left or right side of the box
      var atLeft = x - boxAround.left < boxAround.right - x, atStart = atLeft == ltr;
      ch = chAround + (atStart ? 0 : 1);
      sticky = atStart ? "after" : "before";
      baseX = atLeft ? boxAround.left : boxAround.right;
    } else {
      // (Adjust for extended bound, if necessary.)
      if (!ltr && (ch == end || ch == begin)) { ch++; }
      // To determine which side to associate with, get the box to the
      // left of the character and compare it's vertical position to the
      // coordinates
      sticky = ch == 0 ? "after" : ch == lineObj.text.length ? "before" :
        (measureCharPrepared(cm, preparedMeasure, ch - (ltr ? 1 : 0)).bottom + widgetHeight$$1 <= y) == ltr ?
        "after" : "before";
      // Now get accurate coordinates for this place, in order to get a
      // base X position
      var coords = cursorCoords(cm, Pos(lineNo$$1, ch, sticky), "line", lineObj, preparedMeasure);
      baseX = coords.left;
      outside = y < coords.top ? -1 : y >= coords.bottom ? 1 : 0;
    }

    ch = skipExtendingChars(lineObj.text, ch, 1);
    return PosWithInfo(lineNo$$1, ch, sticky, outside, x - baseX)
  }

  function coordsBidiPart(cm, lineObj, lineNo$$1, preparedMeasure, order, x, y) {
    // Bidi parts are sorted left-to-right, and in a non-line-wrapping
    // situation, we can take this ordering to correspond to the visual
    // ordering. This finds the first part whose end is after the given
    // coordinates.
    var index = findFirst(function (i) {
      var part = order[i], ltr = part.level != 1;
      return boxIsAfter(cursorCoords(cm, Pos(lineNo$$1, ltr ? part.to : part.from, ltr ? "before" : "after"),
                                     "line", lineObj, preparedMeasure), x, y, true)
    }, 0, order.length - 1);
    var part = order[index];
    // If this isn't the first part, the part's start is also after
    // the coordinates, and the coordinates aren't on the same line as
    // that start, move one part back.
    if (index > 0) {
      var ltr = part.level != 1;
      var start = cursorCoords(cm, Pos(lineNo$$1, ltr ? part.from : part.to, ltr ? "after" : "before"),
                               "line", lineObj, preparedMeasure);
      if (boxIsAfter(start, x, y, true) && start.top > y)
        { part = order[index - 1]; }
    }
    return part
  }

  function coordsBidiPartWrapped(cm, lineObj, _lineNo, preparedMeasure, order, x, y) {
    // In a wrapped line, rtl text on wrapping boundaries can do things
    // that don't correspond to the ordering in our `order` array at
    // all, so a binary search doesn't work, and we want to return a
    // part that only spans one line so that the binary search in
    // coordsCharInner is safe. As such, we first find the extent of the
    // wrapped line, and then do a flat search in which we discard any
    // spans that aren't on the line.
    var ref = wrappedLineExtent(cm, lineObj, preparedMeasure, y);
    var begin = ref.begin;
    var end = ref.end;
    if (/\s/.test(lineObj.text.charAt(end - 1))) { end--; }
    var part = null, closestDist = null;
    for (var i = 0; i < order.length; i++) {
      var p = order[i];
      if (p.from >= end || p.to <= begin) { continue }
      var ltr = p.level != 1;
      var endX = measureCharPrepared(cm, preparedMeasure, ltr ? Math.min(end, p.to) - 1 : Math.max(begin, p.from)).right;
      // Weigh against spans ending before this, so that they are only
      // picked if nothing ends after
      var dist = endX < x ? x - endX + 1e9 : endX - x;
      if (!part || closestDist > dist) {
        part = p;
        closestDist = dist;
      }
    }
    if (!part) { part = order[order.length - 1]; }
    // Clip the part to the wrapped line.
    if (part.from < begin) { part = {from: begin, to: part.to, level: part.level}; }
    if (part.to > end) { part = {from: part.from, to: end, level: part.level}; }
    return part
  }

  var measureText;
  // Compute the default text height.
  function textHeight(display) {
    if (display.cachedTextHeight != null) { return display.cachedTextHeight }
    if (measureText == null) {
      measureText = elt("pre", null, "CodeMirror-line-like");
      // Measure a bunch of lines, for browsers that compute
      // fractional heights.
      for (var i = 0; i < 49; ++i) {
        measureText.appendChild(document.createTextNode("x"));
        measureText.appendChild(elt("br"));
      }
      measureText.appendChild(document.createTextNode("x"));
    }
    removeChildrenAndAdd(display.measure, measureText);
    var height = measureText.offsetHeight / 50;
    if (height > 3) { display.cachedTextHeight = height; }
    removeChildren(display.measure);
    return height || 1
  }

  // Compute the default character width.
  function charWidth(display) {
    if (display.cachedCharWidth != null) { return display.cachedCharWidth }
    var anchor = elt("span", "xxxxxxxxxx");
    var pre = elt("pre", [anchor], "CodeMirror-line-like");
    removeChildrenAndAdd(display.measure, pre);
    var rect = anchor.getBoundingClientRect(), width = (rect.right - rect.left) / 10;
    if (width > 2) { display.cachedCharWidth = width; }
    return width || 10
  }

  // Do a bulk-read of the DOM positions and sizes needed to draw the
  // view, so that we don't interleave reading and writing to the DOM.
  function getDimensions(cm) {
    var d = cm.display, left = {}, width = {};
    var gutterLeft = d.gutters.clientLeft;
    for (var n = d.gutters.firstChild, i = 0; n; n = n.nextSibling, ++i) {
      var id = cm.display.gutterSpecs[i].className;
      left[id] = n.offsetLeft + n.clientLeft + gutterLeft;
      width[id] = n.clientWidth;
    }
    return {fixedPos: compensateForHScroll(d),
            gutterTotalWidth: d.gutters.offsetWidth,
            gutterLeft: left,
            gutterWidth: width,
            wrapperWidth: d.wrapper.clientWidth}
  }

  // Computes display.scroller.scrollLeft + display.gutters.offsetWidth,
  // but using getBoundingClientRect to get a sub-pixel-accurate
  // result.
  function compensateForHScroll(display) {
    return display.scroller.getBoundingClientRect().left - display.sizer.getBoundingClientRect().left
  }

  // Returns a function that estimates the height of a line, to use as
  // first approximation until the line becomes visible (and is thus
  // properly measurable).
  function estimateHeight(cm) {
    var th = textHeight(cm.display), wrapping = cm.options.lineWrapping;
    var perLine = wrapping && Math.max(5, cm.display.scroller.clientWidth / charWidth(cm.display) - 3);
    return function (line) {
      if (lineIsHidden(cm.doc, line)) { return 0 }

      var widgetsHeight = 0;
      if (line.widgets) { for (var i = 0; i < line.widgets.length; i++) {
        if (line.widgets[i].height) { widgetsHeight += line.widgets[i].height; }
      } }

      if (wrapping)
        { return widgetsHeight + (Math.ceil(line.text.length / perLine) || 1) * th }
      else
        { return widgetsHeight + th }
    }
  }

  function estimateLineHeights(cm) {
    var doc = cm.doc, est = estimateHeight(cm);
    doc.iter(function (line) {
      var estHeight = est(line);
      if (estHeight != line.height) { updateLineHeight(line, estHeight); }
    });
  }

  // Given a mouse event, find the corresponding position. If liberal
  // is false, it checks whether a gutter or scrollbar was clicked,
  // and returns null if it was. forRect is used by rectangular
  // selections, and tries to estimate a character position even for
  // coordinates beyond the right of the text.
  function posFromMouse(cm, e, liberal, forRect) {
    var display = cm.display;
    if (!liberal && e_target(e).getAttribute("cm-not-content") == "true") { return null }

    var x, y, space = display.lineSpace.getBoundingClientRect();
    // Fails unpredictably on IE[67] when mouse is dragged around quickly.
    try { x = e.clientX - space.left; y = e.clientY - space.top; }
    catch (e) { return null }
    var coords = coordsChar(cm, x, y), line;
    if (forRect && coords.xRel == 1 && (line = getLine(cm.doc, coords.line).text).length == coords.ch) {
      var colDiff = countColumn(line, line.length, cm.options.tabSize) - line.length;
      coords = Pos(coords.line, Math.max(0, Math.round((x - paddingH(cm.display).left) / charWidth(cm.display)) - colDiff));
    }
    return coords
  }

  // Find the view element corresponding to a given line. Return null
  // when the line isn't visible.
  function findViewIndex(cm, n) {
    if (n >= cm.display.viewTo) { return null }
    n -= cm.display.viewFrom;
    if (n < 0) { return null }
    var view = cm.display.view;
    for (var i = 0; i < view.length; i++) {
      n -= view[i].size;
      if (n < 0) { return i }
    }
  }

  // Updates the display.view data structure for a given change to the
  // document. From and to are in pre-change coordinates. Lendiff is
  // the amount of lines added or subtracted by the change. This is
  // used for changes that span multiple lines, or change the way
  // lines are divided into visual lines. regLineChange (below)
  // registers single-line changes.
  function regChange(cm, from, to, lendiff) {
    if (from == null) { from = cm.doc.first; }
    if (to == null) { to = cm.doc.first + cm.doc.size; }
    if (!lendiff) { lendiff = 0; }

    var display = cm.display;
    if (lendiff && to < display.viewTo &&
        (display.updateLineNumbers == null || display.updateLineNumbers > from))
      { display.updateLineNumbers = from; }

    cm.curOp.viewChanged = true;

    if (from >= display.viewTo) { // Change after
      if (sawCollapsedSpans && visualLineNo(cm.doc, from) < display.viewTo)
        { resetView(cm); }
    } else if (to <= display.viewFrom) { // Change before
      if (sawCollapsedSpans && visualLineEndNo(cm.doc, to + lendiff) > display.viewFrom) {
        resetView(cm);
      } else {
        display.viewFrom += lendiff;
        display.viewTo += lendiff;
      }
    } else if (from <= display.viewFrom && to >= display.viewTo) { // Full overlap
      resetView(cm);
    } else if (from <= display.viewFrom) { // Top overlap
      var cut = viewCuttingPoint(cm, to, to + lendiff, 1);
      if (cut) {
        display.view = display.view.slice(cut.index);
        display.viewFrom = cut.lineN;
        display.viewTo += lendiff;
      } else {
        resetView(cm);
      }
    } else if (to >= display.viewTo) { // Bottom overlap
      var cut$1 = viewCuttingPoint(cm, from, from, -1);
      if (cut$1) {
        display.view = display.view.slice(0, cut$1.index);
        display.viewTo = cut$1.lineN;
      } else {
        resetView(cm);
      }
    } else { // Gap in the middle
      var cutTop = viewCuttingPoint(cm, from, from, -1);
      var cutBot = viewCuttingPoint(cm, to, to + lendiff, 1);
      if (cutTop && cutBot) {
        display.view = display.view.slice(0, cutTop.index)
          .concat(buildViewArray(cm, cutTop.lineN, cutBot.lineN))
          .concat(display.view.slice(cutBot.index));
        display.viewTo += lendiff;
      } else {
        resetView(cm);
      }
    }

    var ext = display.externalMeasured;
    if (ext) {
      if (to < ext.lineN)
        { ext.lineN += lendiff; }
      else if (from < ext.lineN + ext.size)
        { display.externalMeasured = null; }
    }
  }

  // Register a change to a single line. Type must be one of "text",
  // "gutter", "class", "widget"
  function regLineChange(cm, line, type) {
    cm.curOp.viewChanged = true;
    var display = cm.display, ext = cm.display.externalMeasured;
    if (ext && line >= ext.lineN && line < ext.lineN + ext.size)
      { display.externalMeasured = null; }

    if (line < display.viewFrom || line >= display.viewTo) { return }
    var lineView = display.view[findViewIndex(cm, line)];
    if (lineView.node == null) { return }
    var arr = lineView.changes || (lineView.changes = []);
    if (indexOf(arr, type) == -1) { arr.push(type); }
  }

  // Clear the view.
  function resetView(cm) {
    cm.display.viewFrom = cm.display.viewTo = cm.doc.first;
    cm.display.view = [];
    cm.display.viewOffset = 0;
  }

  function viewCuttingPoint(cm, oldN, newN, dir) {
    var index = findViewIndex(cm, oldN), diff, view = cm.display.view;
    if (!sawCollapsedSpans || newN == cm.doc.first + cm.doc.size)
      { return {index: index, lineN: newN} }
    var n = cm.display.viewFrom;
    for (var i = 0; i < index; i++)
      { n += view[i].size; }
    if (n != oldN) {
      if (dir > 0) {
        if (index == view.length - 1) { return null }
        diff = (n + view[index].size) - oldN;
        index++;
      } else {
        diff = n - oldN;
      }
      oldN += diff; newN += diff;
    }
    while (visualLineNo(cm.doc, newN) != newN) {
      if (index == (dir < 0 ? 0 : view.length - 1)) { return null }
      newN += dir * view[index - (dir < 0 ? 1 : 0)].size;
      index += dir;
    }
    return {index: index, lineN: newN}
  }

  // Force the view to cover a given range, adding empty view element
  // or clipping off existing ones as needed.
  function adjustView(cm, from, to) {
    var display = cm.display, view = display.view;
    if (view.length == 0 || from >= display.viewTo || to <= display.viewFrom) {
      display.view = buildViewArray(cm, from, to);
      display.viewFrom = from;
    } else {
      if (display.viewFrom > from)
        { display.view = buildViewArray(cm, from, display.viewFrom).concat(display.view); }
      else if (display.viewFrom < from)
        { display.view = display.view.slice(findViewIndex(cm, from)); }
      display.viewFrom = from;
      if (display.viewTo < to)
        { display.view = display.view.concat(buildViewArray(cm, display.viewTo, to)); }
      else if (display.viewTo > to)
        { display.view = display.view.slice(0, findViewIndex(cm, to)); }
    }
    display.viewTo = to;
  }

  // Count the number of lines in the view whose DOM representation is
  // out of date (or nonexistent).
  function countDirtyView(cm) {
    var view = cm.display.view, dirty = 0;
    for (var i = 0; i < view.length; i++) {
      var lineView = view[i];
      if (!lineView.hidden && (!lineView.node || lineView.changes)) { ++dirty; }
    }
    return dirty
  }

  function updateSelection(cm) {
    cm.display.input.showSelection(cm.display.input.prepareSelection());
  }

  function prepareSelection(cm, primary) {
    if ( primary === void 0 ) primary = true;

    var doc = cm.doc, result = {};
    var curFragment = result.cursors = document.createDocumentFragment();
    var selFragment = result.selection = document.createDocumentFragment();

    for (var i = 0; i < doc.sel.ranges.length; i++) {
      if (!primary && i == doc.sel.primIndex) { continue }
      var range$$1 = doc.sel.ranges[i];
      if (range$$1.from().line >= cm.display.viewTo || range$$1.to().line < cm.display.viewFrom) { continue }
      var collapsed = range$$1.empty();
      if (collapsed || cm.options.showCursorWhenSelecting)
        { drawSelectionCursor(cm, range$$1.head, curFragment); }
      if (!collapsed)
        { drawSelectionRange(cm, range$$1, selFragment); }
    }
    return result
  }

  // Draws a cursor for the given range
  function drawSelectionCursor(cm, head, output) {
    var pos = cursorCoords(cm, head, "div", null, null, !cm.options.singleCursorHeightPerLine);

    var cursor = output.appendChild(elt("div", "\u00a0", "CodeMirror-cursor"));
    cursor.style.left = pos.left + "px";
    cursor.style.top = pos.top + "px";
    cursor.style.height = Math.max(0, pos.bottom - pos.top) * cm.options.cursorHeight + "px";

    if (pos.other) {
      // Secondary cursor, shown when on a 'jump' in bi-directional text
      var otherCursor = output.appendChild(elt("div", "\u00a0", "CodeMirror-cursor CodeMirror-secondarycursor"));
      otherCursor.style.display = "";
      otherCursor.style.left = pos.other.left + "px";
      otherCursor.style.top = pos.other.top + "px";
      otherCursor.style.height = (pos.other.bottom - pos.other.top) * .85 + "px";
    }
  }

  function cmpCoords(a, b) { return a.top - b.top || a.left - b.left }

  // Draws the given range as a highlighted selection
  function drawSelectionRange(cm, range$$1, output) {
    var display = cm.display, doc = cm.doc;
    var fragment = document.createDocumentFragment();
    var padding = paddingH(cm.display), leftSide = padding.left;
    var rightSide = Math.max(display.sizerWidth, displayWidth(cm) - display.sizer.offsetLeft) - padding.right;
    var docLTR = doc.direction == "ltr";

    function add(left, top, width, bottom) {
      if (top < 0) { top = 0; }
      top = Math.round(top);
      bottom = Math.round(bottom);
      fragment.appendChild(elt("div", null, "CodeMirror-selected", ("position: absolute; left: " + left + "px;\n                             top: " + top + "px; width: " + (width == null ? rightSide - left : width) + "px;\n                             height: " + (bottom - top) + "px")));
    }

    function drawForLine(line, fromArg, toArg) {
      var lineObj = getLine(doc, line);
      var lineLen = lineObj.text.length;
      var start, end;
      function coords(ch, bias) {
        return charCoords(cm, Pos(line, ch), "div", lineObj, bias)
      }

      function wrapX(pos, dir, side) {
        var extent = wrappedLineExtentChar(cm, lineObj, null, pos);
        var prop = (dir == "ltr") == (side == "after") ? "left" : "right";
        var ch = side == "after" ? extent.begin : extent.end - (/\s/.test(lineObj.text.charAt(extent.end - 1)) ? 2 : 1);
        return coords(ch, prop)[prop]
      }

      var order = getOrder(lineObj, doc.direction);
      iterateBidiSections(order, fromArg || 0, toArg == null ? lineLen : toArg, function (from, to, dir, i) {
        var ltr = dir == "ltr";
        var fromPos = coords(from, ltr ? "left" : "right");
        var toPos = coords(to - 1, ltr ? "right" : "left");

        var openStart = fromArg == null && from == 0, openEnd = toArg == null && to == lineLen;
        var first = i == 0, last = !order || i == order.length - 1;
        if (toPos.top - fromPos.top <= 3) { // Single line
          var openLeft = (docLTR ? openStart : openEnd) && first;
          var openRight = (docLTR ? openEnd : openStart) && last;
          var left = openLeft ? leftSide : (ltr ? fromPos : toPos).left;
          var right = openRight ? rightSide : (ltr ? toPos : fromPos).right;
          add(left, fromPos.top, right - left, fromPos.bottom);
        } else { // Multiple lines
          var topLeft, topRight, botLeft, botRight;
          if (ltr) {
            topLeft = docLTR && openStart && first ? leftSide : fromPos.left;
            topRight = docLTR ? rightSide : wrapX(from, dir, "before");
            botLeft = docLTR ? leftSide : wrapX(to, dir, "after");
            botRight = docLTR && openEnd && last ? rightSide : toPos.right;
          } else {
            topLeft = !docLTR ? leftSide : wrapX(from, dir, "before");
            topRight = !docLTR && openStart && first ? rightSide : fromPos.right;
            botLeft = !docLTR && openEnd && last ? leftSide : toPos.left;
            botRight = !docLTR ? rightSide : wrapX(to, dir, "after");
          }
          add(topLeft, fromPos.top, topRight - topLeft, fromPos.bottom);
          if (fromPos.bottom < toPos.top) { add(leftSide, fromPos.bottom, null, toPos.top); }
          add(botLeft, toPos.top, botRight - botLeft, toPos.bottom);
        }

        if (!start || cmpCoords(fromPos, start) < 0) { start = fromPos; }
        if (cmpCoords(toPos, start) < 0) { start = toPos; }
        if (!end || cmpCoords(fromPos, end) < 0) { end = fromPos; }
        if (cmpCoords(toPos, end) < 0) { end = toPos; }
      });
      return {start: start, end: end}
    }

    var sFrom = range$$1.from(), sTo = range$$1.to();
    if (sFrom.line == sTo.line) {
      drawForLine(sFrom.line, sFrom.ch, sTo.ch);
    } else {
      var fromLine = getLine(doc, sFrom.line), toLine = getLine(doc, sTo.line);
      var singleVLine = visualLine(fromLine) == visualLine(toLine);
      var leftEnd = drawForLine(sFrom.line, sFrom.ch, singleVLine ? fromLine.text.length + 1 : null).end;
      var rightStart = drawForLine(sTo.line, singleVLine ? 0 : null, sTo.ch).start;
      if (singleVLine) {
        if (leftEnd.top < rightStart.top - 2) {
          add(leftEnd.right, leftEnd.top, null, leftEnd.bottom);
          add(leftSide, rightStart.top, rightStart.left, rightStart.bottom);
        } else {
          add(leftEnd.right, leftEnd.top, rightStart.left - leftEnd.right, leftEnd.bottom);
        }
      }
      if (leftEnd.bottom < rightStart.top)
        { add(leftSide, leftEnd.bottom, null, rightStart.top); }
    }

    output.appendChild(fragment);
  }

  // Cursor-blinking
  function restartBlink(cm) {
    if (!cm.state.focused) { return }
    var display = cm.display;
    clearInterval(display.blinker);
    var on = true;
    display.cursorDiv.style.visibility = "";
    if (cm.options.cursorBlinkRate > 0)
      { display.blinker = setInterval(function () { return display.cursorDiv.style.visibility = (on = !on) ? "" : "hidden"; },
        cm.options.cursorBlinkRate); }
    else if (cm.options.cursorBlinkRate < 0)
      { display.cursorDiv.style.visibility = "hidden"; }
  }

  function ensureFocus(cm) {
    if (!cm.state.focused) { cm.display.input.focus(); onFocus(cm); }
  }

  function delayBlurEvent(cm) {
    cm.state.delayingBlurEvent = true;
    setTimeout(function () { if (cm.state.delayingBlurEvent) {
      cm.state.delayingBlurEvent = false;
      onBlur(cm);
    } }, 100);
  }

  function onFocus(cm, e) {
    if (cm.state.delayingBlurEvent) { cm.state.delayingBlurEvent = false; }

    if (cm.options.readOnly == "nocursor") { return }
    if (!cm.state.focused) {
      signal(cm, "focus", cm, e);
      cm.state.focused = true;
      addClass(cm.display.wrapper, "CodeMirror-focused");
      // This test prevents this from firing when a context
      // menu is closed (since the input reset would kill the
      // select-all detection hack)
      if (!cm.curOp && cm.display.selForContextMenu != cm.doc.sel) {
        cm.display.input.reset();
        if (webkit) { setTimeout(function () { return cm.display.input.reset(true); }, 20); } // Issue #1730
      }
      cm.display.input.receivedFocus();
    }
    restartBlink(cm);
  }
  function onBlur(cm, e) {
    if (cm.state.delayingBlurEvent) { return }

    if (cm.state.focused) {
      signal(cm, "blur", cm, e);
      cm.state.focused = false;
      rmClass(cm.display.wrapper, "CodeMirror-focused");
    }
    clearInterval(cm.display.blinker);
    setTimeout(function () { if (!cm.state.focused) { cm.display.shift = false; } }, 150);
  }

  // Read the actual heights of the rendered lines, and update their
  // stored heights to match.
  function updateHeightsInViewport(cm) {
    var display = cm.display;
    var prevBottom = display.lineDiv.offsetTop;
    for (var i = 0; i < display.view.length; i++) {
      var cur = display.view[i], wrapping = cm.options.lineWrapping;
      var height = (void 0), width = 0;
      if (cur.hidden) { continue }
      if (ie && ie_version < 8) {
        var bot = cur.node.offsetTop + cur.node.offsetHeight;
        height = bot - prevBottom;
        prevBottom = bot;
      } else {
        var box = cur.node.getBoundingClientRect();
        height = box.bottom - box.top;
        // Check that lines don't extend past the right of the current
        // editor width
        if (!wrapping && cur.text.firstChild)
          { width = cur.text.firstChild.getBoundingClientRect().right - box.left - 1; }
      }
      var diff = cur.line.height - height;
      if (diff > .005 || diff < -.005) {
        updateLineHeight(cur.line, height);
        updateWidgetHeight(cur.line);
        if (cur.rest) { for (var j = 0; j < cur.rest.length; j++)
          { updateWidgetHeight(cur.rest[j]); } }
      }
      if (width > cm.display.sizerWidth) {
        var chWidth = Math.ceil(width / charWidth(cm.display));
        if (chWidth > cm.display.maxLineLength) {
          cm.display.maxLineLength = chWidth;
          cm.display.maxLine = cur.line;
          cm.display.maxLineChanged = true;
        }
      }
    }
  }

  // Read and store the height of line widgets associated with the
  // given line.
  function updateWidgetHeight(line) {
    if (line.widgets) { for (var i = 0; i < line.widgets.length; ++i) {
      var w = line.widgets[i], parent = w.node.parentNode;
      if (parent) { w.height = parent.offsetHeight; }
    } }
  }

  // Compute the lines that are visible in a given viewport (defaults
  // the the current scroll position). viewport may contain top,
  // height, and ensure (see op.scrollToPos) properties.
  function visibleLines(display, doc, viewport) {
    var top = viewport && viewport.top != null ? Math.max(0, viewport.top) : display.scroller.scrollTop;
    top = Math.floor(top - paddingTop(display));
    var bottom = viewport && viewport.bottom != null ? viewport.bottom : top + display.wrapper.clientHeight;

    var from = lineAtHeight(doc, top), to = lineAtHeight(doc, bottom);
    // Ensure is a {from: {line, ch}, to: {line, ch}} object, and
    // forces those lines into the viewport (if possible).
    if (viewport && viewport.ensure) {
      var ensureFrom = viewport.ensure.from.line, ensureTo = viewport.ensure.to.line;
      if (ensureFrom < from) {
        from = ensureFrom;
        to = lineAtHeight(doc, heightAtLine(getLine(doc, ensureFrom)) + display.wrapper.clientHeight);
      } else if (Math.min(ensureTo, doc.lastLine()) >= to) {
        from = lineAtHeight(doc, heightAtLine(getLine(doc, ensureTo)) - display.wrapper.clientHeight);
        to = ensureTo;
      }
    }
    return {from: from, to: Math.max(to, from + 1)}
  }

  // SCROLLING THINGS INTO VIEW

  // If an editor sits on the top or bottom of the window, partially
  // scrolled out of view, this ensures that the cursor is visible.
  function maybeScrollWindow(cm, rect) {
    if (signalDOMEvent(cm, "scrollCursorIntoView")) { return }

    var display = cm.display, box = display.sizer.getBoundingClientRect(), doScroll = null;
    if (rect.top + box.top < 0) { doScroll = true; }
    else if (rect.bottom + box.top > (window.innerHeight || document.documentElement.clientHeight)) { doScroll = false; }
    if (doScroll != null && !phantom) {
      var scrollNode = elt("div", "\u200b", null, ("position: absolute;\n                         top: " + (rect.top - display.viewOffset - paddingTop(cm.display)) + "px;\n                         height: " + (rect.bottom - rect.top + scrollGap(cm) + display.barHeight) + "px;\n                         left: " + (rect.left) + "px; width: " + (Math.max(2, rect.right - rect.left)) + "px;"));
      cm.display.lineSpace.appendChild(scrollNode);
      scrollNode.scrollIntoView(doScroll);
      cm.display.lineSpace.removeChild(scrollNode);
    }
  }

  // Scroll a given position into view (immediately), verifying that
  // it actually became visible (as line heights are accurately
  // measured, the position of something may 'drift' during drawing).
  function scrollPosIntoView(cm, pos, end, margin) {
    if (margin == null) { margin = 0; }
    var rect;
    if (!cm.options.lineWrapping && pos == end) {
      // Set pos and end to the cursor positions around the character pos sticks to
      // If pos.sticky == "before", that is around pos.ch - 1, otherwise around pos.ch
      // If pos == Pos(_, 0, "before"), pos and end are unchanged
      pos = pos.ch ? Pos(pos.line, pos.sticky == "before" ? pos.ch - 1 : pos.ch, "after") : pos;
      end = pos.sticky == "before" ? Pos(pos.line, pos.ch + 1, "before") : pos;
    }
    for (var limit = 0; limit < 5; limit++) {
      var changed = false;
      var coords = cursorCoords(cm, pos);
      var endCoords = !end || end == pos ? coords : cursorCoords(cm, end);
      rect = {left: Math.min(coords.left, endCoords.left),
              top: Math.min(coords.top, endCoords.top) - margin,
              right: Math.max(coords.left, endCoords.left),
              bottom: Math.max(coords.bottom, endCoords.bottom) + margin};
      var scrollPos = calculateScrollPos(cm, rect);
      var startTop = cm.doc.scrollTop, startLeft = cm.doc.scrollLeft;
      if (scrollPos.scrollTop != null) {
        updateScrollTop(cm, scrollPos.scrollTop);
        if (Math.abs(cm.doc.scrollTop - startTop) > 1) { changed = true; }
      }
      if (scrollPos.scrollLeft != null) {
        setScrollLeft(cm, scrollPos.scrollLeft);
        if (Math.abs(cm.doc.scrollLeft - startLeft) > 1) { changed = true; }
      }
      if (!changed) { break }
    }
    return rect
  }

  // Scroll a given set of coordinates into view (immediately).
  function scrollIntoView(cm, rect) {
    var scrollPos = calculateScrollPos(cm, rect);
    if (scrollPos.scrollTop != null) { updateScrollTop(cm, scrollPos.scrollTop); }
    if (scrollPos.scrollLeft != null) { setScrollLeft(cm, scrollPos.scrollLeft); }
  }

  // Calculate a new scroll position needed to scroll the given
  // rectangle into view. Returns an object with scrollTop and
  // scrollLeft properties. When these are undefined, the
  // vertical/horizontal position does not need to be adjusted.
  function calculateScrollPos(cm, rect) {
    var display = cm.display, snapMargin = textHeight(cm.display);
    if (rect.top < 0) { rect.top = 0; }
    var screentop = cm.curOp && cm.curOp.scrollTop != null ? cm.curOp.scrollTop : display.scroller.scrollTop;
    var screen = displayHeight(cm), result = {};
    if (rect.bottom - rect.top > screen) { rect.bottom = rect.top + screen; }
    var docBottom = cm.doc.height + paddingVert(display);
    var atTop = rect.top < snapMargin, atBottom = rect.bottom > docBottom - snapMargin;
    if (rect.top < screentop) {
      result.scrollTop = atTop ? 0 : rect.top;
    } else if (rect.bottom > screentop + screen) {
      var newTop = Math.min(rect.top, (atBottom ? docBottom : rect.bottom) - screen);
      if (newTop != screentop) { result.scrollTop = newTop; }
    }

    var screenleft = cm.curOp && cm.curOp.scrollLeft != null ? cm.curOp.scrollLeft : display.scroller.scrollLeft;
    var screenw = displayWidth(cm) - (cm.options.fixedGutter ? display.gutters.offsetWidth : 0);
    var tooWide = rect.right - rect.left > screenw;
    if (tooWide) { rect.right = rect.left + screenw; }
    if (rect.left < 10)
      { result.scrollLeft = 0; }
    else if (rect.left < screenleft)
      { result.scrollLeft = Math.max(0, rect.left - (tooWide ? 0 : 10)); }
    else if (rect.right > screenw + screenleft - 3)
      { result.scrollLeft = rect.right + (tooWide ? 0 : 10) - screenw; }
    return result
  }

  // Store a relative adjustment to the scroll position in the current
  // operation (to be applied when the operation finishes).
  function addToScrollTop(cm, top) {
    if (top == null) { return }
    resolveScrollToPos(cm);
    cm.curOp.scrollTop = (cm.curOp.scrollTop == null ? cm.doc.scrollTop : cm.curOp.scrollTop) + top;
  }

  // Make sure that at the end of the operation the current cursor is
  // shown.
  function ensureCursorVisible(cm) {
    resolveScrollToPos(cm);
    var cur = cm.getCursor();
    cm.curOp.scrollToPos = {from: cur, to: cur, margin: cm.options.cursorScrollMargin};
  }

  function scrollToCoords(cm, x, y) {
    if (x != null || y != null) { resolveScrollToPos(cm); }
    if (x != null) { cm.curOp.scrollLeft = x; }
    if (y != null) { cm.curOp.scrollTop = y; }
  }

  function scrollToRange(cm, range$$1) {
    resolveScrollToPos(cm);
    cm.curOp.scrollToPos = range$$1;
  }

  // When an operation has its scrollToPos property set, and another
  // scroll action is applied before the end of the operation, this
  // 'simulates' scrolling that position into view in a cheap way, so
  // that the effect of intermediate scroll commands is not ignored.
  function resolveScrollToPos(cm) {
    var range$$1 = cm.curOp.scrollToPos;
    if (range$$1) {
      cm.curOp.scrollToPos = null;
      var from = estimateCoords(cm, range$$1.from), to = estimateCoords(cm, range$$1.to);
      scrollToCoordsRange(cm, from, to, range$$1.margin);
    }
  }

  function scrollToCoordsRange(cm, from, to, margin) {
    var sPos = calculateScrollPos(cm, {
      left: Math.min(from.left, to.left),
      top: Math.min(from.top, to.top) - margin,
      right: Math.max(from.right, to.right),
      bottom: Math.max(from.bottom, to.bottom) + margin
    });
    scrollToCoords(cm, sPos.scrollLeft, sPos.scrollTop);
  }

  // Sync the scrollable area and scrollbars, ensure the viewport
  // covers the visible area.
  function updateScrollTop(cm, val) {
    if (Math.abs(cm.doc.scrollTop - val) < 2) { return }
    if (!gecko) { updateDisplaySimple(cm, {top: val}); }
    setScrollTop(cm, val, true);
    if (gecko) { updateDisplaySimple(cm); }
    startWorker(cm, 100);
  }

  function setScrollTop(cm, val, forceScroll) {
    val = Math.min(cm.display.scroller.scrollHeight - cm.display.scroller.clientHeight, val);
    if (cm.display.scroller.scrollTop == val && !forceScroll) { return }
    cm.doc.scrollTop = val;
    cm.display.scrollbars.setScrollTop(val);
    if (cm.display.scroller.scrollTop != val) { cm.display.scroller.scrollTop = val; }
  }

  // Sync scroller and scrollbar, ensure the gutter elements are
  // aligned.
  function setScrollLeft(cm, val, isScroller, forceScroll) {
    val = Math.min(val, cm.display.scroller.scrollWidth - cm.display.scroller.clientWidth);
    if ((isScroller ? val == cm.doc.scrollLeft : Math.abs(cm.doc.scrollLeft - val) < 2) && !forceScroll) { return }
    cm.doc.scrollLeft = val;
    alignHorizontally(cm);
    if (cm.display.scroller.scrollLeft != val) { cm.display.scroller.scrollLeft = val; }
    cm.display.scrollbars.setScrollLeft(val);
  }

  // SCROLLBARS

  // Prepare DOM reads needed to update the scrollbars. Done in one
  // shot to minimize update/measure roundtrips.
  function measureForScrollbars(cm) {
    var d = cm.display, gutterW = d.gutters.offsetWidth;
    var docH = Math.round(cm.doc.height + paddingVert(cm.display));
    return {
      clientHeight: d.scroller.clientHeight,
      viewHeight: d.wrapper.clientHeight,
      scrollWidth: d.scroller.scrollWidth, clientWidth: d.scroller.clientWidth,
      viewWidth: d.wrapper.clientWidth,
      barLeft: cm.options.fixedGutter ? gutterW : 0,
      docHeight: docH,
      scrollHeight: docH + scrollGap(cm) + d.barHeight,
      nativeBarWidth: d.nativeBarWidth,
      gutterWidth: gutterW
    }
  }

  var NativeScrollbars = function(place, scroll, cm) {
    this.cm = cm;
    var vert = this.vert = elt("div", [elt("div", null, null, "min-width: 1px")], "CodeMirror-vscrollbar");
    var horiz = this.horiz = elt("div", [elt("div", null, null, "height: 100%; min-height: 1px")], "CodeMirror-hscrollbar");
    vert.tabIndex = horiz.tabIndex = -1;
    place(vert); place(horiz);

    on(vert, "scroll", function () {
      if (vert.clientHeight) { scroll(vert.scrollTop, "vertical"); }
    });
    on(horiz, "scroll", function () {
      if (horiz.clientWidth) { scroll(horiz.scrollLeft, "horizontal"); }
    });

    this.checkedZeroWidth = false;
    // Need to set a minimum width to see the scrollbar on IE7 (but must not set it on IE8).
    if (ie && ie_version < 8) { this.horiz.style.minHeight = this.vert.style.minWidth = "18px"; }
  };

  NativeScrollbars.prototype.update = function (measure) {
    var needsH = measure.scrollWidth > measure.clientWidth + 1;
    var needsV = measure.scrollHeight > measure.clientHeight + 1;
    var sWidth = measure.nativeBarWidth;

    if (needsV) {
      this.vert.style.display = "block";
      this.vert.style.bottom = needsH ? sWidth + "px" : "0";
      var totalHeight = measure.viewHeight - (needsH ? sWidth : 0);
      // A bug in IE8 can cause this value to be negative, so guard it.
      this.vert.firstChild.style.height =
        Math.max(0, measure.scrollHeight - measure.clientHeight + totalHeight) + "px";
    } else {
      this.vert.style.display = "";
      this.vert.firstChild.style.height = "0";
    }

    if (needsH) {
      this.horiz.style.display = "block";
      this.horiz.style.right = needsV ? sWidth + "px" : "0";
      this.horiz.style.left = measure.barLeft + "px";
      var totalWidth = measure.viewWidth - measure.barLeft - (needsV ? sWidth : 0);
      this.horiz.firstChild.style.width =
        Math.max(0, measure.scrollWidth - measure.clientWidth + totalWidth) + "px";
    } else {
      this.horiz.style.display = "";
      this.horiz.firstChild.style.width = "0";
    }

    if (!this.checkedZeroWidth && measure.clientHeight > 0) {
      if (sWidth == 0) { this.zeroWidthHack(); }
      this.checkedZeroWidth = true;
    }

    return {right: needsV ? sWidth : 0, bottom: needsH ? sWidth : 0}
  };

  NativeScrollbars.prototype.setScrollLeft = function (pos) {
    if (this.horiz.scrollLeft != pos) { this.horiz.scrollLeft = pos; }
    if (this.disableHoriz) { this.enableZeroWidthBar(this.horiz, this.disableHoriz, "horiz"); }
  };

  NativeScrollbars.prototype.setScrollTop = function (pos) {
    if (this.vert.scrollTop != pos) { this.vert.scrollTop = pos; }
    if (this.disableVert) { this.enableZeroWidthBar(this.vert, this.disableVert, "vert"); }
  };

  NativeScrollbars.prototype.zeroWidthHack = function () {
    var w = mac && !mac_geMountainLion ? "12px" : "18px";
    this.horiz.style.height = this.vert.style.width = w;
    this.horiz.style.pointerEvents = this.vert.style.pointerEvents = "none";
    this.disableHoriz = new Delayed;
    this.disableVert = new Delayed;
  };

  NativeScrollbars.prototype.enableZeroWidthBar = function (bar, delay, type) {
    bar.style.pointerEvents = "auto";
    function maybeDisable() {
      // To find out whether the scrollbar is still visible, we
      // check whether the element under the pixel in the bottom
      // right corner of the scrollbar box is the scrollbar box
      // itself (when the bar is still visible) or its filler child
      // (when the bar is hidden). If it is still visible, we keep
      // it enabled, if it's hidden, we disable pointer events.
      var box = bar.getBoundingClientRect();
      var elt$$1 = type == "vert" ? document.elementFromPoint(box.right - 1, (box.top + box.bottom) / 2)
          : document.elementFromPoint((box.right + box.left) / 2, box.bottom - 1);
      if (elt$$1 != bar) { bar.style.pointerEvents = "none"; }
      else { delay.set(1000, maybeDisable); }
    }
    delay.set(1000, maybeDisable);
  };

  NativeScrollbars.prototype.clear = function () {
    var parent = this.horiz.parentNode;
    parent.removeChild(this.horiz);
    parent.removeChild(this.vert);
  };

  var NullScrollbars = function () {};

  NullScrollbars.prototype.update = function () { return {bottom: 0, right: 0} };
  NullScrollbars.prototype.setScrollLeft = function () {};
  NullScrollbars.prototype.setScrollTop = function () {};
  NullScrollbars.prototype.clear = function () {};

  function updateScrollbars(cm, measure) {
    if (!measure) { measure = measureForScrollbars(cm); }
    var startWidth = cm.display.barWidth, startHeight = cm.display.barHeight;
    updateScrollbarsInner(cm, measure);
    for (var i = 0; i < 4 && startWidth != cm.display.barWidth || startHeight != cm.display.barHeight; i++) {
      if (startWidth != cm.display.barWidth && cm.options.lineWrapping)
        { updateHeightsInViewport(cm); }
      updateScrollbarsInner(cm, measureForScrollbars(cm));
      startWidth = cm.display.barWidth; startHeight = cm.display.barHeight;
    }
  }

  // Re-synchronize the fake scrollbars with the actual size of the
  // content.
  function updateScrollbarsInner(cm, measure) {
    var d = cm.display;
    var sizes = d.scrollbars.update(measure);

    d.sizer.style.paddingRight = (d.barWidth = sizes.right) + "px";
    d.sizer.style.paddingBottom = (d.barHeight = sizes.bottom) + "px";
    d.heightForcer.style.borderBottom = sizes.bottom + "px solid transparent";

    if (sizes.right && sizes.bottom) {
      d.scrollbarFiller.style.display = "block";
      d.scrollbarFiller.style.height = sizes.bottom + "px";
      d.scrollbarFiller.style.width = sizes.right + "px";
    } else { d.scrollbarFiller.style.display = ""; }
    if (sizes.bottom && cm.options.coverGutterNextToScrollbar && cm.options.fixedGutter) {
      d.gutterFiller.style.display = "block";
      d.gutterFiller.style.height = sizes.bottom + "px";
      d.gutterFiller.style.width = measure.gutterWidth + "px";
    } else { d.gutterFiller.style.display = ""; }
  }

  var scrollbarModel = {"native": NativeScrollbars, "null": NullScrollbars};

  function initScrollbars(cm) {
    if (cm.display.scrollbars) {
      cm.display.scrollbars.clear();
      if (cm.display.scrollbars.addClass)
        { rmClass(cm.display.wrapper, cm.display.scrollbars.addClass); }
    }

    cm.display.scrollbars = new scrollbarModel[cm.options.scrollbarStyle](function (node) {
      cm.display.wrapper.insertBefore(node, cm.display.scrollbarFiller);
      // Prevent clicks in the scrollbars from killing focus
      on(node, "mousedown", function () {
        if (cm.state.focused) { setTimeout(function () { return cm.display.input.focus(); }, 0); }
      });
      node.setAttribute("cm-not-content", "true");
    }, function (pos, axis) {
      if (axis == "horizontal") { setScrollLeft(cm, pos); }
      else { updateScrollTop(cm, pos); }
    }, cm);
    if (cm.display.scrollbars.addClass)
      { addClass(cm.display.wrapper, cm.display.scrollbars.addClass); }
  }

  // Operations are used to wrap a series of changes to the editor
  // state in such a way that each change won't have to update the
  // cursor and display (which would be awkward, slow, and
  // error-prone). Instead, display updates are batched and then all
  // combined and executed at once.

  var nextOpId = 0;
  // Start a new operation.
  function startOperation(cm) {
    cm.curOp = {
      cm: cm,
      viewChanged: false,      // Flag that indicates that lines might need to be redrawn
      startHeight: cm.doc.height, // Used to detect need to update scrollbar
      forceUpdate: false,      // Used to force a redraw
      updateInput: 0,       // Whether to reset the input textarea
      typing: false,           // Whether this reset should be careful to leave existing text (for compositing)
      changeObjs: null,        // Accumulated changes, for firing change events
      cursorActivityHandlers: null, // Set of handlers to fire cursorActivity on
      cursorActivityCalled: 0, // Tracks which cursorActivity handlers have been called already
      selectionChanged: false, // Whether the selection needs to be redrawn
      updateMaxLine: false,    // Set when the widest line needs to be determined anew
      scrollLeft: null, scrollTop: null, // Intermediate scroll position, not pushed to DOM yet
      scrollToPos: null,       // Used to scroll to a specific position
      focus: false,
      id: ++nextOpId           // Unique ID
    };
    pushOperation(cm.curOp);
  }

  // Finish an operation, updating the display and signalling delayed events
  function endOperation(cm) {
    var op = cm.curOp;
    if (op) { finishOperation(op, function (group) {
      for (var i = 0; i < group.ops.length; i++)
        { group.ops[i].cm.curOp = null; }
      endOperations(group);
    }); }
  }

  // The DOM updates done when an operation finishes are batched so
  // that the minimum number of relayouts are required.
  function endOperations(group) {
    var ops = group.ops;
    for (var i = 0; i < ops.length; i++) // Read DOM
      { endOperation_R1(ops[i]); }
    for (var i$1 = 0; i$1 < ops.length; i$1++) // Write DOM (maybe)
      { endOperation_W1(ops[i$1]); }
    for (var i$2 = 0; i$2 < ops.length; i$2++) // Read DOM
      { endOperation_R2(ops[i$2]); }
    for (var i$3 = 0; i$3 < ops.length; i$3++) // Write DOM (maybe)
      { endOperation_W2(ops[i$3]); }
    for (var i$4 = 0; i$4 < ops.length; i$4++) // Read DOM
      { endOperation_finish(ops[i$4]); }
  }

  function endOperation_R1(op) {
    var cm = op.cm, display = cm.display;
    maybeClipScrollbars(cm);
    if (op.updateMaxLine) { findMaxLine(cm); }

    op.mustUpdate = op.viewChanged || op.forceUpdate || op.scrollTop != null ||
      op.scrollToPos && (op.scrollToPos.from.line < display.viewFrom ||
                         op.scrollToPos.to.line >= display.viewTo) ||
      display.maxLineChanged && cm.options.lineWrapping;
    op.update = op.mustUpdate &&
      new DisplayUpdate(cm, op.mustUpdate && {top: op.scrollTop, ensure: op.scrollToPos}, op.forceUpdate);
  }

  function endOperation_W1(op) {
    op.updatedDisplay = op.mustUpdate && updateDisplayIfNeeded(op.cm, op.update);
  }

  function endOperation_R2(op) {
    var cm = op.cm, display = cm.display;
    if (op.updatedDisplay) { updateHeightsInViewport(cm); }

    op.barMeasure = measureForScrollbars(cm);

    // If the max line changed since it was last measured, measure it,
    // and ensure the document's width matches it.
    // updateDisplay_W2 will use these properties to do the actual resizing
    if (display.maxLineChanged && !cm.options.lineWrapping) {
      op.adjustWidthTo = measureChar(cm, display.maxLine, display.maxLine.text.length).left + 3;
      cm.display.sizerWidth = op.adjustWidthTo;
      op.barMeasure.scrollWidth =
        Math.max(display.scroller.clientWidth, display.sizer.offsetLeft + op.adjustWidthTo + scrollGap(cm) + cm.display.barWidth);
      op.maxScrollLeft = Math.max(0, display.sizer.offsetLeft + op.adjustWidthTo - displayWidth(cm));
    }

    if (op.updatedDisplay || op.selectionChanged)
      { op.preparedSelection = display.input.prepareSelection(); }
  }

  function endOperation_W2(op) {
    var cm = op.cm;

    if (op.adjustWidthTo != null) {
      cm.display.sizer.style.minWidth = op.adjustWidthTo + "px";
      if (op.maxScrollLeft < cm.doc.scrollLeft)
        { setScrollLeft(cm, Math.min(cm.display.scroller.scrollLeft, op.maxScrollLeft), true); }
      cm.display.maxLineChanged = false;
    }

    var takeFocus = op.focus && op.focus == activeElt();
    if (op.preparedSelection)
      { cm.display.input.showSelection(op.preparedSelection, takeFocus); }
    if (op.updatedDisplay || op.startHeight != cm.doc.height)
      { updateScrollbars(cm, op.barMeasure); }
    if (op.updatedDisplay)
      { setDocumentHeight(cm, op.barMeasure); }

    if (op.selectionChanged) { restartBlink(cm); }

    if (cm.state.focused && op.updateInput)
      { cm.display.input.reset(op.typing); }
    if (takeFocus) { ensureFocus(op.cm); }
  }

  function endOperation_finish(op) {
    var cm = op.cm, display = cm.display, doc = cm.doc;

    if (op.updatedDisplay) { postUpdateDisplay(cm, op.update); }

    // Abort mouse wheel delta measurement, when scrolling explicitly
    if (display.wheelStartX != null && (op.scrollTop != null || op.scrollLeft != null || op.scrollToPos))
      { display.wheelStartX = display.wheelStartY = null; }

    // Propagate the scroll position to the actual DOM scroller
    if (op.scrollTop != null) { setScrollTop(cm, op.scrollTop, op.forceScroll); }

    if (op.scrollLeft != null) { setScrollLeft(cm, op.scrollLeft, true, true); }
    // If we need to scroll a specific position into view, do so.
    if (op.scrollToPos) {
      var rect = scrollPosIntoView(cm, clipPos(doc, op.scrollToPos.from),
                                   clipPos(doc, op.scrollToPos.to), op.scrollToPos.margin);
      maybeScrollWindow(cm, rect);
    }

    // Fire events for markers that are hidden/unidden by editing or
    // undoing
    var hidden = op.maybeHiddenMarkers, unhidden = op.maybeUnhiddenMarkers;
    if (hidden) { for (var i = 0; i < hidden.length; ++i)
      { if (!hidden[i].lines.length) { signal(hidden[i], "hide"); } } }
    if (unhidden) { for (var i$1 = 0; i$1 < unhidden.length; ++i$1)
      { if (unhidden[i$1].lines.length) { signal(unhidden[i$1], "unhide"); } } }

    if (display.wrapper.offsetHeight)
      { doc.scrollTop = cm.display.scroller.scrollTop; }

    // Fire change events, and delayed event handlers
    if (op.changeObjs)
      { signal(cm, "changes", cm, op.changeObjs); }
    if (op.update)
      { op.update.finish(); }
  }

  // Run the given function in an operation
  function runInOp(cm, f) {
    if (cm.curOp) { return f() }
    startOperation(cm);
    try { return f() }
    finally { endOperation(cm); }
  }
  // Wraps a function in an operation. Returns the wrapped function.
  function operation(cm, f) {
    return function() {
      if (cm.curOp) { return f.apply(cm, arguments) }
      startOperation(cm);
      try { return f.apply(cm, arguments) }
      finally { endOperation(cm); }
    }
  }
  // Used to add methods to editor and doc instances, wrapping them in
  // operations.
  function methodOp(f) {
    return function() {
      if (this.curOp) { return f.apply(this, arguments) }
      startOperation(this);
      try { return f.apply(this, arguments) }
      finally { endOperation(this); }
    }
  }
  function docMethodOp(f) {
    return function() {
      var cm = this.cm;
      if (!cm || cm.curOp) { return f.apply(this, arguments) }
      startOperation(cm);
      try { return f.apply(this, arguments) }
      finally { endOperation(cm); }
    }
  }

  // HIGHLIGHT WORKER

  function startWorker(cm, time) {
    if (cm.doc.highlightFrontier < cm.display.viewTo)
      { cm.state.highlight.set(time, bind(highlightWorker, cm)); }
  }

  function highlightWorker(cm) {
    var doc = cm.doc;
    if (doc.highlightFrontier >= cm.display.viewTo) { return }
    var end = +new Date + cm.options.workTime;
    var context = getContextBefore(cm, doc.highlightFrontier);
    var changedLines = [];

    doc.iter(context.line, Math.min(doc.first + doc.size, cm.display.viewTo + 500), function (line) {
      if (context.line >= cm.display.viewFrom) { // Visible
        var oldStyles = line.styles;
        var resetState = line.text.length > cm.options.maxHighlightLength ? copyState(doc.mode, context.state) : null;
        var highlighted = highlightLine(cm, line, context, true);
        if (resetState) { context.state = resetState; }
        line.styles = highlighted.styles;
        var oldCls = line.styleClasses, newCls = highlighted.classes;
        if (newCls) { line.styleClasses = newCls; }
        else if (oldCls) { line.styleClasses = null; }
        var ischange = !oldStyles || oldStyles.length != line.styles.length ||
          oldCls != newCls && (!oldCls || !newCls || oldCls.bgClass != newCls.bgClass || oldCls.textClass != newCls.textClass);
        for (var i = 0; !ischange && i < oldStyles.length; ++i) { ischange = oldStyles[i] != line.styles[i]; }
        if (ischange) { changedLines.push(context.line); }
        line.stateAfter = context.save();
        context.nextLine();
      } else {
        if (line.text.length <= cm.options.maxHighlightLength)
          { processLine(cm, line.text, context); }
        line.stateAfter = context.line % 5 == 0 ? context.save() : null;
        context.nextLine();
      }
      if (+new Date > end) {
        startWorker(cm, cm.options.workDelay);
        return true
      }
    });
    doc.highlightFrontier = context.line;
    doc.modeFrontier = Math.max(doc.modeFrontier, context.line);
    if (changedLines.length) { runInOp(cm, function () {
      for (var i = 0; i < changedLines.length; i++)
        { regLineChange(cm, changedLines[i], "text"); }
    }); }
  }

  // DISPLAY DRAWING

  var DisplayUpdate = function(cm, viewport, force) {
    var display = cm.display;

    this.viewport = viewport;
    // Store some values that we'll need later (but don't want to force a relayout for)
    this.visible = visibleLines(display, cm.doc, viewport);
    this.editorIsHidden = !display.wrapper.offsetWidth;
    this.wrapperHeight = display.wrapper.clientHeight;
    this.wrapperWidth = display.wrapper.clientWidth;
    this.oldDisplayWidth = displayWidth(cm);
    this.force = force;
    this.dims = getDimensions(cm);
    this.events = [];
  };

  DisplayUpdate.prototype.signal = function (emitter, type) {
    if (hasHandler(emitter, type))
      { this.events.push(arguments); }
  };
  DisplayUpdate.prototype.finish = function () {
      var this$1 = this;

    for (var i = 0; i < this.events.length; i++)
      { signal.apply(null, this$1.events[i]); }
  };

  function maybeClipScrollbars(cm) {
    var display = cm.display;
    if (!display.scrollbarsClipped && display.scroller.offsetWidth) {
      display.nativeBarWidth = display.scroller.offsetWidth - display.scroller.clientWidth;
      display.heightForcer.style.height = scrollGap(cm) + "px";
      display.sizer.style.marginBottom = -display.nativeBarWidth + "px";
      display.sizer.style.borderRightWidth = scrollGap(cm) + "px";
      display.scrollbarsClipped = true;
    }
  }

  function selectionSnapshot(cm) {
    if (cm.hasFocus()) { return null }
    var active = activeElt();
    if (!active || !contains(cm.display.lineDiv, active)) { return null }
    var result = {activeElt: active};
    if (window.getSelection) {
      var sel = window.getSelection();
      if (sel.anchorNode && sel.extend && contains(cm.display.lineDiv, sel.anchorNode)) {
        result.anchorNode = sel.anchorNode;
        result.anchorOffset = sel.anchorOffset;
        result.focusNode = sel.focusNode;
        result.focusOffset = sel.focusOffset;
      }
    }
    return result
  }

  function restoreSelection(snapshot) {
    if (!snapshot || !snapshot.activeElt || snapshot.activeElt == activeElt()) { return }
    snapshot.activeElt.focus();
    if (snapshot.anchorNode && contains(document.body, snapshot.anchorNode) && contains(document.body, snapshot.focusNode)) {
      var sel = window.getSelection(), range$$1 = document.createRange();
      range$$1.setEnd(snapshot.anchorNode, snapshot.anchorOffset);
      range$$1.collapse(false);
      sel.removeAllRanges();
      sel.addRange(range$$1);
      sel.extend(snapshot.focusNode, snapshot.focusOffset);
    }
  }

  // Does the actual updating of the line display. Bails out
  // (returning false) when there is nothing to be done and forced is
  // false.
  function updateDisplayIfNeeded(cm, update) {
    var display = cm.display, doc = cm.doc;

    if (update.editorIsHidden) {
      resetView(cm);
      return false
    }

    // Bail out if the visible area is already rendered and nothing changed.
    if (!update.force &&
        update.visible.from >= display.viewFrom && update.visible.to <= display.viewTo &&
        (display.updateLineNumbers == null || display.updateLineNumbers >= display.viewTo) &&
        display.renderedView == display.view && countDirtyView(cm) == 0)
      { return false }

    if (maybeUpdateLineNumberWidth(cm)) {
      resetView(cm);
      update.dims = getDimensions(cm);
    }

    // Compute a suitable new viewport (from & to)
    var end = doc.first + doc.size;
    var from = Math.max(update.visible.from - cm.options.viewportMargin, doc.first);
    var to = Math.min(end, update.visible.to + cm.options.viewportMargin);
    if (display.viewFrom < from && from - display.viewFrom < 20) { from = Math.max(doc.first, display.viewFrom); }
    if (display.viewTo > to && display.viewTo - to < 20) { to = Math.min(end, display.viewTo); }
    if (sawCollapsedSpans) {
      from = visualLineNo(cm.doc, from);
      to = visualLineEndNo(cm.doc, to);
    }

    var different = from != display.viewFrom || to != display.viewTo ||
      display.lastWrapHeight != update.wrapperHeight || display.lastWrapWidth != update.wrapperWidth;
    adjustView(cm, from, to);

    display.viewOffset = heightAtLine(getLine(cm.doc, display.viewFrom));
    // Position the mover div to align with the current scroll position
    cm.display.mover.style.top = display.viewOffset + "px";

    var toUpdate = countDirtyView(cm);
    if (!different && toUpdate == 0 && !update.force && display.renderedView == display.view &&
        (display.updateLineNumbers == null || display.updateLineNumbers >= display.viewTo))
      { return false }

    // For big changes, we hide the enclosing element during the
    // update, since that speeds up the operations on most browsers.
    var selSnapshot = selectionSnapshot(cm);
    if (toUpdate > 4) { display.lineDiv.style.display = "none"; }
    patchDisplay(cm, display.updateLineNumbers, update.dims);
    if (toUpdate > 4) { display.lineDiv.style.display = ""; }
    display.renderedView = display.view;
    // There might have been a widget with a focused element that got
    // hidden or updated, if so re-focus it.
    restoreSelection(selSnapshot);

    // Prevent selection and cursors from interfering with the scroll
    // width and height.
    removeChildren(display.cursorDiv);
    removeChildren(display.selectionDiv);
    display.gutters.style.height = display.sizer.style.minHeight = 0;

    if (different) {
      display.lastWrapHeight = update.wrapperHeight;
      display.lastWrapWidth = update.wrapperWidth;
      startWorker(cm, 400);
    }

    display.updateLineNumbers = null;

    return true
  }

  function postUpdateDisplay(cm, update) {
    var viewport = update.viewport;

    for (var first = true;; first = false) {
      if (!first || !cm.options.lineWrapping || update.oldDisplayWidth == displayWidth(cm)) {
        // Clip forced viewport to actual scrollable area.
        if (viewport && viewport.top != null)
          { viewport = {top: Math.min(cm.doc.height + paddingVert(cm.display) - displayHeight(cm), viewport.top)}; }
        // Updated line heights might result in the drawn area not
        // actually covering the viewport. Keep looping until it does.
        update.visible = visibleLines(cm.display, cm.doc, viewport);
        if (update.visible.from >= cm.display.viewFrom && update.visible.to <= cm.display.viewTo)
          { break }
      }
      if (!updateDisplayIfNeeded(cm, update)) { break }
      updateHeightsInViewport(cm);
      var barMeasure = measureForScrollbars(cm);
      updateSelection(cm);
      updateScrollbars(cm, barMeasure);
      setDocumentHeight(cm, barMeasure);
      update.force = false;
    }

    update.signal(cm, "update", cm);
    if (cm.display.viewFrom != cm.display.reportedViewFrom || cm.display.viewTo != cm.display.reportedViewTo) {
      update.signal(cm, "viewportChange", cm, cm.display.viewFrom, cm.display.viewTo);
      cm.display.reportedViewFrom = cm.display.viewFrom; cm.display.reportedViewTo = cm.display.viewTo;
    }
  }

  function updateDisplaySimple(cm, viewport) {
    var update = new DisplayUpdate(cm, viewport);
    if (updateDisplayIfNeeded(cm, update)) {
      updateHeightsInViewport(cm);
      postUpdateDisplay(cm, update);
      var barMeasure = measureForScrollbars(cm);
      updateSelection(cm);
      updateScrollbars(cm, barMeasure);
      setDocumentHeight(cm, barMeasure);
      update.finish();
    }
  }

  // Sync the actual display DOM structure with display.view, removing
  // nodes for lines that are no longer in view, and creating the ones
  // that are not there yet, and updating the ones that are out of
  // date.
  function patchDisplay(cm, updateNumbersFrom, dims) {
    var display = cm.display, lineNumbers = cm.options.lineNumbers;
    var container = display.lineDiv, cur = container.firstChild;

    function rm(node) {
      var next = node.nextSibling;
      // Works around a throw-scroll bug in OS X Webkit
      if (webkit && mac && cm.display.currentWheelTarget == node)
        { node.style.display = "none"; }
      else
        { node.parentNode.removeChild(node); }
      return next
    }

    var view = display.view, lineN = display.viewFrom;
    // Loop over the elements in the view, syncing cur (the DOM nodes
    // in display.lineDiv) with the view as we go.
    for (var i = 0; i < view.length; i++) {
      var lineView = view[i];
      if (lineView.hidden) ; else if (!lineView.node || lineView.node.parentNode != container) { // Not drawn yet
        var node = buildLineElement(cm, lineView, lineN, dims);
        container.insertBefore(node, cur);
      } else { // Already drawn
        while (cur != lineView.node) { cur = rm(cur); }
        var updateNumber = lineNumbers && updateNumbersFrom != null &&
          updateNumbersFrom <= lineN && lineView.lineNumber;
        if (lineView.changes) {
          if (indexOf(lineView.changes, "gutter") > -1) { updateNumber = false; }
          updateLineForChanges(cm, lineView, lineN, dims);
        }
        if (updateNumber) {
          removeChildren(lineView.lineNumber);
          lineView.lineNumber.appendChild(document.createTextNode(lineNumberFor(cm.options, lineN)));
        }
        cur = lineView.node.nextSibling;
      }
      lineN += lineView.size;
    }
    while (cur) { cur = rm(cur); }
  }

  function updateGutterSpace(display) {
    var width = display.gutters.offsetWidth;
    display.sizer.style.marginLeft = width + "px";
  }

  function setDocumentHeight(cm, measure) {
    cm.display.sizer.style.minHeight = measure.docHeight + "px";
    cm.display.heightForcer.style.top = measure.docHeight + "px";
    cm.display.gutters.style.height = (measure.docHeight + cm.display.barHeight + scrollGap(cm)) + "px";
  }

  // Re-align line numbers and gutter marks to compensate for
  // horizontal scrolling.
  function alignHorizontally(cm) {
    var display = cm.display, view = display.view;
    if (!display.alignWidgets && (!display.gutters.firstChild || !cm.options.fixedGutter)) { return }
    var comp = compensateForHScroll(display) - display.scroller.scrollLeft + cm.doc.scrollLeft;
    var gutterW = display.gutters.offsetWidth, left = comp + "px";
    for (var i = 0; i < view.length; i++) { if (!view[i].hidden) {
      if (cm.options.fixedGutter) {
        if (view[i].gutter)
          { view[i].gutter.style.left = left; }
        if (view[i].gutterBackground)
          { view[i].gutterBackground.style.left = left; }
      }
      var align = view[i].alignable;
      if (align) { for (var j = 0; j < align.length; j++)
        { align[j].style.left = left; } }
    } }
    if (cm.options.fixedGutter)
      { display.gutters.style.left = (comp + gutterW) + "px"; }
  }

  // Used to ensure that the line number gutter is still the right
  // size for the current document size. Returns true when an update
  // is needed.
  function maybeUpdateLineNumberWidth(cm) {
    if (!cm.options.lineNumbers) { return false }
    var doc = cm.doc, last = lineNumberFor(cm.options, doc.first + doc.size - 1), display = cm.display;
    if (last.length != display.lineNumChars) {
      var test = display.measure.appendChild(elt("div", [elt("div", last)],
                                                 "CodeMirror-linenumber CodeMirror-gutter-elt"));
      var innerW = test.firstChild.offsetWidth, padding = test.offsetWidth - innerW;
      display.lineGutter.style.width = "";
      display.lineNumInnerWidth = Math.max(innerW, display.lineGutter.offsetWidth - padding) + 1;
      display.lineNumWidth = display.lineNumInnerWidth + padding;
      display.lineNumChars = display.lineNumInnerWidth ? last.length : -1;
      display.lineGutter.style.width = display.lineNumWidth + "px";
      updateGutterSpace(cm.display);
      return true
    }
    return false
  }

  function getGutters(gutters, lineNumbers) {
    var result = [], sawLineNumbers = false;
    for (var i = 0; i < gutters.length; i++) {
      var name = gutters[i], style = null;
      if (typeof name != "string") { style = name.style; name = name.className; }
      if (name == "CodeMirror-linenumbers") {
        if (!lineNumbers) { continue }
        else { sawLineNumbers = true; }
      }
      result.push({className: name, style: style});
    }
    if (lineNumbers && !sawLineNumbers) { result.push({className: "CodeMirror-linenumbers", style: null}); }
    return result
  }

  // Rebuild the gutter elements, ensure the margin to the left of the
  // code matches their width.
  function renderGutters(display) {
    var gutters = display.gutters, specs = display.gutterSpecs;
    removeChildren(gutters);
    display.lineGutter = null;
    for (var i = 0; i < specs.length; ++i) {
      var ref = specs[i];
      var className = ref.className;
      var style = ref.style;
      var gElt = gutters.appendChild(elt("div", null, "CodeMirror-gutter " + className));
      if (style) { gElt.style.cssText = style; }
      if (className == "CodeMirror-linenumbers") {
        display.lineGutter = gElt;
        gElt.style.width = (display.lineNumWidth || 1) + "px";
      }
    }
    gutters.style.display = specs.length ? "" : "none";
    updateGutterSpace(display);
  }

  function updateGutters(cm) {
    renderGutters(cm.display);
    regChange(cm);
    alignHorizontally(cm);
  }

  // The display handles the DOM integration, both for input reading
  // and content drawing. It holds references to DOM nodes and
  // display-related state.

  function Display(place, doc, input, options) {
    var d = this;
    this.input = input;

    // Covers bottom-right square when both scrollbars are present.
    d.scrollbarFiller = elt("div", null, "CodeMirror-scrollbar-filler");
    d.scrollbarFiller.setAttribute("cm-not-content", "true");
    // Covers bottom of gutter when coverGutterNextToScrollbar is on
    // and h scrollbar is present.
    d.gutterFiller = elt("div", null, "CodeMirror-gutter-filler");
    d.gutterFiller.setAttribute("cm-not-content", "true");
    // Will contain the actual code, positioned to cover the viewport.
    d.lineDiv = eltP("div", null, "CodeMirror-code");
    // Elements are added to these to represent selection and cursors.
    d.selectionDiv = elt("div", null, null, "position: relative; z-index: 1");
    d.cursorDiv = elt("div", null, "CodeMirror-cursors");
    // A visibility: hidden element used to find the size of things.
    d.measure = elt("div", null, "CodeMirror-measure");
    // When lines outside of the viewport are measured, they are drawn in this.
    d.lineMeasure = elt("div", null, "CodeMirror-measure");
    // Wraps everything that needs to exist inside the vertically-padded coordinate system
    d.lineSpace = eltP("div", [d.measure, d.lineMeasure, d.selectionDiv, d.cursorDiv, d.lineDiv],
                      null, "position: relative; outline: none");
    var lines = eltP("div", [d.lineSpace], "CodeMirror-lines");
    // Moved around its parent to cover visible view.
    d.mover = elt("div", [lines], null, "position: relative");
    // Set to the height of the document, allowing scrolling.
    d.sizer = elt("div", [d.mover], "CodeMirror-sizer");
    d.sizerWidth = null;
    // Behavior of elts with overflow: auto and padding is
    // inconsistent across browsers. This is used to ensure the
    // scrollable area is big enough.
    d.heightForcer = elt("div", null, null, "position: absolute; height: " + scrollerGap + "px; width: 1px;");
    // Will contain the gutters, if any.
    d.gutters = elt("div", null, "CodeMirror-gutters");
    d.lineGutter = null;
    // Actual scrollable element.
    d.scroller = elt("div", [d.sizer, d.heightForcer, d.gutters], "CodeMirror-scroll");
    d.scroller.setAttribute("tabIndex", "-1");
    // The element in which the editor lives.
    d.wrapper = elt("div", [d.scrollbarFiller, d.gutterFiller, d.scroller], "CodeMirror");

    // Work around IE7 z-index bug (not perfect, hence IE7 not really being supported)
    if (ie && ie_version < 8) { d.gutters.style.zIndex = -1; d.scroller.style.paddingRight = 0; }
    if (!webkit && !(gecko && mobile)) { d.scroller.draggable = true; }

    if (place) {
      if (place.appendChild) { place.appendChild(d.wrapper); }
      else { place(d.wrapper); }
    }

    // Current rendered range (may be bigger than the view window).
    d.viewFrom = d.viewTo = doc.first;
    d.reportedViewFrom = d.reportedViewTo = doc.first;
    // Information about the rendered lines.
    d.view = [];
    d.renderedView = null;
    // Holds info about a single rendered line when it was rendered
    // for measurement, while not in view.
    d.externalMeasured = null;
    // Empty space (in pixels) above the view
    d.viewOffset = 0;
    d.lastWrapHeight = d.lastWrapWidth = 0;
    d.updateLineNumbers = null;

    d.nativeBarWidth = d.barHeight = d.barWidth = 0;
    d.scrollbarsClipped = false;

    // Used to only resize the line number gutter when necessary (when
    // the amount of lines crosses a boundary that makes its width change)
    d.lineNumWidth = d.lineNumInnerWidth = d.lineNumChars = null;
    // Set to true when a non-horizontal-scrolling line widget is
    // added. As an optimization, line widget aligning is skipped when
    // this is false.
    d.alignWidgets = false;

    d.cachedCharWidth = d.cachedTextHeight = d.cachedPaddingH = null;

    // Tracks the maximum line length so that the horizontal scrollbar
    // can be kept static when scrolling.
    d.maxLine = null;
    d.maxLineLength = 0;
    d.maxLineChanged = false;

    // Used for measuring wheel scrolling granularity
    d.wheelDX = d.wheelDY = d.wheelStartX = d.wheelStartY = null;

    // True when shift is held down.
    d.shift = false;

    // Used to track whether anything happened since the context menu
    // was opened.
    d.selForContextMenu = null;

    d.activeTouch = null;

    d.gutterSpecs = getGutters(options.gutters, options.lineNumbers);
    renderGutters(d);

    input.init(d);
  }

  // Since the delta values reported on mouse wheel events are
  // unstandardized between browsers and even browser versions, and
  // generally horribly unpredictable, this code starts by measuring
  // the scroll effect that the first few mouse wheel events have,
  // and, from that, detects the way it can convert deltas to pixel
  // offsets afterwards.
  //
  // The reason we want to know the amount a wheel event will scroll
  // is that it gives us a chance to update the display before the
  // actual scrolling happens, reducing flickering.

  var wheelSamples = 0, wheelPixelsPerUnit = null;
  // Fill in a browser-detected starting value on browsers where we
  // know one. These don't have to be accurate -- the result of them
  // being wrong would just be a slight flicker on the first wheel
  // scroll (if it is large enough).
  if (ie) { wheelPixelsPerUnit = -.53; }
  else if (gecko) { wheelPixelsPerUnit = 15; }
  else if (chrome) { wheelPixelsPerUnit = -.7; }
  else if (safari) { wheelPixelsPerUnit = -1/3; }

  function wheelEventDelta(e) {
    var dx = e.wheelDeltaX, dy = e.wheelDeltaY;
    if (dx == null && e.detail && e.axis == e.HORIZONTAL_AXIS) { dx = e.detail; }
    if (dy == null && e.detail && e.axis == e.VERTICAL_AXIS) { dy = e.detail; }
    else if (dy == null) { dy = e.wheelDelta; }
    return {x: dx, y: dy}
  }
  function wheelEventPixels(e) {
    var delta = wheelEventDelta(e);
    delta.x *= wheelPixelsPerUnit;
    delta.y *= wheelPixelsPerUnit;
    return delta
  }

  function onScrollWheel(cm, e) {
    var delta = wheelEventDelta(e), dx = delta.x, dy = delta.y;

    var display = cm.display, scroll = display.scroller;
    // Quit if there's nothing to scroll here
    var canScrollX = scroll.scrollWidth > scroll.clientWidth;
    var canScrollY = scroll.scrollHeight > scroll.clientHeight;
    if (!(dx && canScrollX || dy && canScrollY)) { return }

    // Webkit browsers on OS X abort momentum scrolls when the target
    // of the scroll event is removed from the scrollable element.
    // This hack (see related code in patchDisplay) makes sure the
    // element is kept around.
    if (dy && mac && webkit) {
      outer: for (var cur = e.target, view = display.view; cur != scroll; cur = cur.parentNode) {
        for (var i = 0; i < view.length; i++) {
          if (view[i].node == cur) {
            cm.display.currentWheelTarget = cur;
            break outer
          }
        }
      }
    }

    // On some browsers, horizontal scrolling will cause redraws to
    // happen before the gutter has been realigned, causing it to
    // wriggle around in a most unseemly way. When we have an
    // estimated pixels/delta value, we just handle horizontal
    // scrolling entirely here. It'll be slightly off from native, but
    // better than glitching out.
    if (dx && !gecko && !presto && wheelPixelsPerUnit != null) {
      if (dy && canScrollY)
        { updateScrollTop(cm, Math.max(0, scroll.scrollTop + dy * wheelPixelsPerUnit)); }
      setScrollLeft(cm, Math.max(0, scroll.scrollLeft + dx * wheelPixelsPerUnit));
      // Only prevent default scrolling if vertical scrolling is
      // actually possible. Otherwise, it causes vertical scroll
      // jitter on OSX trackpads when deltaX is small and deltaY
      // is large (issue #3579)
      if (!dy || (dy && canScrollY))
        { e_preventDefault(e); }
      display.wheelStartX = null; // Abort measurement, if in progress
      return
    }

    // 'Project' the visible viewport to cover the area that is being
    // scrolled into view (if we know enough to estimate it).
    if (dy && wheelPixelsPerUnit != null) {
      var pixels = dy * wheelPixelsPerUnit;
      var top = cm.doc.scrollTop, bot = top + display.wrapper.clientHeight;
      if (pixels < 0) { top = Math.max(0, top + pixels - 50); }
      else { bot = Math.min(cm.doc.height, bot + pixels + 50); }
      updateDisplaySimple(cm, {top: top, bottom: bot});
    }

    if (wheelSamples < 20) {
      if (display.wheelStartX == null) {
        display.wheelStartX = scroll.scrollLeft; display.wheelStartY = scroll.scrollTop;
        display.wheelDX = dx; display.wheelDY = dy;
        setTimeout(function () {
          if (display.wheelStartX == null) { return }
          var movedX = scroll.scrollLeft - display.wheelStartX;
          var movedY = scroll.scrollTop - display.wheelStartY;
          var sample = (movedY && display.wheelDY && movedY / display.wheelDY) ||
            (movedX && display.wheelDX && movedX / display.wheelDX);
          display.wheelStartX = display.wheelStartY = null;
          if (!sample) { return }
          wheelPixelsPerUnit = (wheelPixelsPerUnit * wheelSamples + sample) / (wheelSamples + 1);
          ++wheelSamples;
        }, 200);
      } else {
        display.wheelDX += dx; display.wheelDY += dy;
      }
    }
  }

  // Selection objects are immutable. A new one is created every time
  // the selection changes. A selection is one or more non-overlapping
  // (and non-touching) ranges, sorted, and an integer that indicates
  // which one is the primary selection (the one that's scrolled into
  // view, that getCursor returns, etc).
  var Selection = function(ranges, primIndex) {
    this.ranges = ranges;
    this.primIndex = primIndex;
  };

  Selection.prototype.primary = function () { return this.ranges[this.primIndex] };

  Selection.prototype.equals = function (other) {
      var this$1 = this;

    if (other == this) { return true }
    if (other.primIndex != this.primIndex || other.ranges.length != this.ranges.length) { return false }
    for (var i = 0; i < this.ranges.length; i++) {
      var here = this$1.ranges[i], there = other.ranges[i];
      if (!equalCursorPos(here.anchor, there.anchor) || !equalCursorPos(here.head, there.head)) { return false }
    }
    return true
  };

  Selection.prototype.deepCopy = function () {
      var this$1 = this;

    var out = [];
    for (var i = 0; i < this.ranges.length; i++)
      { out[i] = new Range(copyPos(this$1.ranges[i].anchor), copyPos(this$1.ranges[i].head)); }
    return new Selection(out, this.primIndex)
  };

  Selection.prototype.somethingSelected = function () {
      var this$1 = this;

    for (var i = 0; i < this.ranges.length; i++)
      { if (!this$1.ranges[i].empty()) { return true } }
    return false
  };

  Selection.prototype.contains = function (pos, end) {
      var this$1 = this;

    if (!end) { end = pos; }
    for (var i = 0; i < this.ranges.length; i++) {
      var range = this$1.ranges[i];
      if (cmp(end, range.from()) >= 0 && cmp(pos, range.to()) <= 0)
        { return i }
    }
    return -1
  };

  var Range = function(anchor, head) {
    this.anchor = anchor; this.head = head;
  };

  Range.prototype.from = function () { return minPos(this.anchor, this.head) };
  Range.prototype.to = function () { return maxPos(this.anchor, this.head) };
  Range.prototype.empty = function () { return this.head.line == this.anchor.line && this.head.ch == this.anchor.ch };

  // Take an unsorted, potentially overlapping set of ranges, and
  // build a selection out of it. 'Consumes' ranges array (modifying
  // it).
  function normalizeSelection(cm, ranges, primIndex) {
    var mayTouch = cm && cm.options.selectionsMayTouch;
    var prim = ranges[primIndex];
    ranges.sort(function (a, b) { return cmp(a.from(), b.from()); });
    primIndex = indexOf(ranges, prim);
    for (var i = 1; i < ranges.length; i++) {
      var cur = ranges[i], prev = ranges[i - 1];
      var diff = cmp(prev.to(), cur.from());
      if (mayTouch && !cur.empty() ? diff > 0 : diff >= 0) {
        var from = minPos(prev.from(), cur.from()), to = maxPos(prev.to(), cur.to());
        var inv = prev.empty() ? cur.from() == cur.head : prev.from() == prev.head;
        if (i <= primIndex) { --primIndex; }
        ranges.splice(--i, 2, new Range(inv ? to : from, inv ? from : to));
      }
    }
    return new Selection(ranges, primIndex)
  }

  function simpleSelection(anchor, head) {
    return new Selection([new Range(anchor, head || anchor)], 0)
  }

  // Compute the position of the end of a change (its 'to' property
  // refers to the pre-change end).
  function changeEnd(change) {
    if (!change.text) { return change.to }
    return Pos(change.from.line + change.text.length - 1,
               lst(change.text).length + (change.text.length == 1 ? change.from.ch : 0))
  }

  // Adjust a position to refer to the post-change position of the
  // same text, or the end of the change if the change covers it.
  function adjustForChange(pos, change) {
    if (cmp(pos, change.from) < 0) { return pos }
    if (cmp(pos, change.to) <= 0) { return changeEnd(change) }

    var line = pos.line + change.text.length - (change.to.line - change.from.line) - 1, ch = pos.ch;
    if (pos.line == change.to.line) { ch += changeEnd(change).ch - change.to.ch; }
    return Pos(line, ch)
  }

  function computeSelAfterChange(doc, change) {
    var out = [];
    for (var i = 0; i < doc.sel.ranges.length; i++) {
      var range = doc.sel.ranges[i];
      out.push(new Range(adjustForChange(range.anchor, change),
                         adjustForChange(range.head, change)));
    }
    return normalizeSelection(doc.cm, out, doc.sel.primIndex)
  }

  function offsetPos(pos, old, nw) {
    if (pos.line == old.line)
      { return Pos(nw.line, pos.ch - old.ch + nw.ch) }
    else
      { return Pos(nw.line + (pos.line - old.line), pos.ch) }
  }

  // Used by replaceSelections to allow moving the selection to the
  // start or around the replaced test. Hint may be "start" or "around".
  function computeReplacedSel(doc, changes, hint) {
    var out = [];
    var oldPrev = Pos(doc.first, 0), newPrev = oldPrev;
    for (var i = 0; i < changes.length; i++) {
      var change = changes[i];
      var from = offsetPos(change.from, oldPrev, newPrev);
      var to = offsetPos(changeEnd(change), oldPrev, newPrev);
      oldPrev = change.to;
      newPrev = to;
      if (hint == "around") {
        var range = doc.sel.ranges[i], inv = cmp(range.head, range.anchor) < 0;
        out[i] = new Range(inv ? to : from, inv ? from : to);
      } else {
        out[i] = new Range(from, from);
      }
    }
    return new Selection(out, doc.sel.primIndex)
  }

  // Used to get the editor into a consistent state again when options change.

  function loadMode(cm) {
    cm.doc.mode = getMode(cm.options, cm.doc.modeOption);
    resetModeState(cm);
  }

  function resetModeState(cm) {
    cm.doc.iter(function (line) {
      if (line.stateAfter) { line.stateAfter = null; }
      if (line.styles) { line.styles = null; }
    });
    cm.doc.modeFrontier = cm.doc.highlightFrontier = cm.doc.first;
    startWorker(cm, 100);
    cm.state.modeGen++;
    if (cm.curOp) { regChange(cm); }
  }

  // DOCUMENT DATA STRUCTURE

  // By default, updates that start and end at the beginning of a line
  // are treated specially, in order to make the association of line
  // widgets and marker elements with the text behave more intuitive.
  function isWholeLineUpdate(doc, change) {
    return change.from.ch == 0 && change.to.ch == 0 && lst(change.text) == "" &&
      (!doc.cm || doc.cm.options.wholeLineUpdateBefore)
  }

  // Perform a change on the document data structure.
  function updateDoc(doc, change, markedSpans, estimateHeight$$1) {
    function spansFor(n) {return markedSpans ? markedSpans[n] : null}
    function update(line, text, spans) {
      updateLine(line, text, spans, estimateHeight$$1);
      signalLater(line, "change", line, change);
    }
    function linesFor(start, end) {
      var result = [];
      for (var i = start; i < end; ++i)
        { result.push(new Line(text[i], spansFor(i), estimateHeight$$1)); }
      return result
    }

    var from = change.from, to = change.to, text = change.text;
    var firstLine = getLine(doc, from.line), lastLine = getLine(doc, to.line);
    var lastText = lst(text), lastSpans = spansFor(text.length - 1), nlines = to.line - from.line;

    // Adjust the line structure
    if (change.full) {
      doc.insert(0, linesFor(0, text.length));
      doc.remove(text.length, doc.size - text.length);
    } else if (isWholeLineUpdate(doc, change)) {
      // This is a whole-line replace. Treated specially to make
      // sure line objects move the way they are supposed to.
      var added = linesFor(0, text.length - 1);
      update(lastLine, lastLine.text, lastSpans);
      if (nlines) { doc.remove(from.line, nlines); }
      if (added.length) { doc.insert(from.line, added); }
    } else if (firstLine == lastLine) {
      if (text.length == 1) {
        update(firstLine, firstLine.text.slice(0, from.ch) + lastText + firstLine.text.slice(to.ch), lastSpans);
      } else {
        var added$1 = linesFor(1, text.length - 1);
        added$1.push(new Line(lastText + firstLine.text.slice(to.ch), lastSpans, estimateHeight$$1));
        update(firstLine, firstLine.text.slice(0, from.ch) + text[0], spansFor(0));
        doc.insert(from.line + 1, added$1);
      }
    } else if (text.length == 1) {
      update(firstLine, firstLine.text.slice(0, from.ch) + text[0] + lastLine.text.slice(to.ch), spansFor(0));
      doc.remove(from.line + 1, nlines);
    } else {
      update(firstLine, firstLine.text.slice(0, from.ch) + text[0], spansFor(0));
      update(lastLine, lastText + lastLine.text.slice(to.ch), lastSpans);
      var added$2 = linesFor(1, text.length - 1);
      if (nlines > 1) { doc.remove(from.line + 1, nlines - 1); }
      doc.insert(from.line + 1, added$2);
    }

    signalLater(doc, "change", doc, change);
  }

  // Call f for all linked documents.
  function linkedDocs(doc, f, sharedHistOnly) {
    function propagate(doc, skip, sharedHist) {
      if (doc.linked) { for (var i = 0; i < doc.linked.length; ++i) {
        var rel = doc.linked[i];
        if (rel.doc == skip) { continue }
        var shared = sharedHist && rel.sharedHist;
        if (sharedHistOnly && !shared) { continue }
        f(rel.doc, shared);
        propagate(rel.doc, doc, shared);
      } }
    }
    propagate(doc, null, true);
  }

  // Attach a document to an editor.
  function attachDoc(cm, doc) {
    if (doc.cm) { throw new Error("This document is already in use.") }
    cm.doc = doc;
    doc.cm = cm;
    estimateLineHeights(cm);
    loadMode(cm);
    setDirectionClass(cm);
    if (!cm.options.lineWrapping) { findMaxLine(cm); }
    cm.options.mode = doc.modeOption;
    regChange(cm);
  }

  function setDirectionClass(cm) {
  (cm.doc.direction == "rtl" ? addClass : rmClass)(cm.display.lineDiv, "CodeMirror-rtl");
  }

  function directionChanged(cm) {
    runInOp(cm, function () {
      setDirectionClass(cm);
      regChange(cm);
    });
  }

  function History(startGen) {
    // Arrays of change events and selections. Doing something adds an
    // event to done and clears undo. Undoing moves events from done
    // to undone, redoing moves them in the other direction.
    this.done = []; this.undone = [];
    this.undoDepth = Infinity;
    // Used to track when changes can be merged into a single undo
    // event
    this.lastModTime = this.lastSelTime = 0;
    this.lastOp = this.lastSelOp = null;
    this.lastOrigin = this.lastSelOrigin = null;
    // Used by the isClean() method
    this.generation = this.maxGeneration = startGen || 1;
  }

  // Create a history change event from an updateDoc-style change
  // object.
  function historyChangeFromChange(doc, change) {
    var histChange = {from: copyPos(change.from), to: changeEnd(change), text: getBetween(doc, change.from, change.to)};
    attachLocalSpans(doc, histChange, change.from.line, change.to.line + 1);
    linkedDocs(doc, function (doc) { return attachLocalSpans(doc, histChange, change.from.line, change.to.line + 1); }, true);
    return histChange
  }

  // Pop all selection events off the end of a history array. Stop at
  // a change event.
  function clearSelectionEvents(array) {
    while (array.length) {
      var last = lst(array);
      if (last.ranges) { array.pop(); }
      else { break }
    }
  }

  // Find the top change event in the history. Pop off selection
  // events that are in the way.
  function lastChangeEvent(hist, force) {
    if (force) {
      clearSelectionEvents(hist.done);
      return lst(hist.done)
    } else if (hist.done.length && !lst(hist.done).ranges) {
      return lst(hist.done)
    } else if (hist.done.length > 1 && !hist.done[hist.done.length - 2].ranges) {
      hist.done.pop();
      return lst(hist.done)
    }
  }

  // Register a change in the history. Merges changes that are within
  // a single operation, or are close together with an origin that
  // allows merging (starting with "+") into a single event.
  function addChangeToHistory(doc, change, selAfter, opId) {
    var hist = doc.history;
    hist.undone.length = 0;
    var time = +new Date, cur;
    var last;

    if ((hist.lastOp == opId ||
         hist.lastOrigin == change.origin && change.origin &&
         ((change.origin.charAt(0) == "+" && hist.lastModTime > time - (doc.cm ? doc.cm.options.historyEventDelay : 500)) ||
          change.origin.charAt(0) == "*")) &&
        (cur = lastChangeEvent(hist, hist.lastOp == opId))) {
      // Merge this change into the last event
      last = lst(cur.changes);
      if (cmp(change.from, change.to) == 0 && cmp(change.from, last.to) == 0) {
        // Optimized case for simple insertion -- don't want to add
        // new changesets for every character typed
        last.to = changeEnd(change);
      } else {
        // Add new sub-event
        cur.changes.push(historyChangeFromChange(doc, change));
      }
    } else {
      // Can not be merged, start a new event.
      var before = lst(hist.done);
      if (!before || !before.ranges)
        { pushSelectionToHistory(doc.sel, hist.done); }
      cur = {changes: [historyChangeFromChange(doc, change)],
             generation: hist.generation};
      hist.done.push(cur);
      while (hist.done.length > hist.undoDepth) {
        hist.done.shift();
        if (!hist.done[0].ranges) { hist.done.shift(); }
      }
    }
    hist.done.push(selAfter);
    hist.generation = ++hist.maxGeneration;
    hist.lastModTime = hist.lastSelTime = time;
    hist.lastOp = hist.lastSelOp = opId;
    hist.lastOrigin = hist.lastSelOrigin = change.origin;

    if (!last) { signal(doc, "historyAdded"); }
  }

  function selectionEventCanBeMerged(doc, origin, prev, sel) {
    var ch = origin.charAt(0);
    return ch == "*" ||
      ch == "+" &&
      prev.ranges.length == sel.ranges.length &&
      prev.somethingSelected() == sel.somethingSelected() &&
      new Date - doc.history.lastSelTime <= (doc.cm ? doc.cm.options.historyEventDelay : 500)
  }

  // Called whenever the selection changes, sets the new selection as
  // the pending selection in the history, and pushes the old pending
  // selection into the 'done' array when it was significantly
  // different (in number of selected ranges, emptiness, or time).
  function addSelectionToHistory(doc, sel, opId, options) {
    var hist = doc.history, origin = options && options.origin;

    // A new event is started when the previous origin does not match
    // the current, or the origins don't allow matching. Origins
    // starting with * are always merged, those starting with + are
    // merged when similar and close together in time.
    if (opId == hist.lastSelOp ||
        (origin && hist.lastSelOrigin == origin &&
         (hist.lastModTime == hist.lastSelTime && hist.lastOrigin == origin ||
          selectionEventCanBeMerged(doc, origin, lst(hist.done), sel))))
      { hist.done[hist.done.length - 1] = sel; }
    else
      { pushSelectionToHistory(sel, hist.done); }

    hist.lastSelTime = +new Date;
    hist.lastSelOrigin = origin;
    hist.lastSelOp = opId;
    if (options && options.clearRedo !== false)
      { clearSelectionEvents(hist.undone); }
  }

  function pushSelectionToHistory(sel, dest) {
    var top = lst(dest);
    if (!(top && top.ranges && top.equals(sel)))
      { dest.push(sel); }
  }

  // Used to store marked span information in the history.
  function attachLocalSpans(doc, change, from, to) {
    var existing = change["spans_" + doc.id], n = 0;
    doc.iter(Math.max(doc.first, from), Math.min(doc.first + doc.size, to), function (line) {
      if (line.markedSpans)
        { (existing || (existing = change["spans_" + doc.id] = {}))[n] = line.markedSpans; }
      ++n;
    });
  }

  // When un/re-doing restores text containing marked spans, those
  // that have been explicitly cleared should not be restored.
  function removeClearedSpans(spans) {
    if (!spans) { return null }
    var out;
    for (var i = 0; i < spans.length; ++i) {
      if (spans[i].marker.explicitlyCleared) { if (!out) { out = spans.slice(0, i); } }
      else if (out) { out.push(spans[i]); }
    }
    return !out ? spans : out.length ? out : null
  }

  // Retrieve and filter the old marked spans stored in a change event.
  function getOldSpans(doc, change) {
    var found = change["spans_" + doc.id];
    if (!found) { return null }
    var nw = [];
    for (var i = 0; i < change.text.length; ++i)
      { nw.push(removeClearedSpans(found[i])); }
    return nw
  }

  // Used for un/re-doing changes from the history. Combines the
  // result of computing the existing spans with the set of spans that
  // existed in the history (so that deleting around a span and then
  // undoing brings back the span).
  function mergeOldSpans(doc, change) {
    var old = getOldSpans(doc, change);
    var stretched = stretchSpansOverChange(doc, change);
    if (!old) { return stretched }
    if (!stretched) { return old }

    for (var i = 0; i < old.length; ++i) {
      var oldCur = old[i], stretchCur = stretched[i];
      if (oldCur && stretchCur) {
        spans: for (var j = 0; j < stretchCur.length; ++j) {
          var span = stretchCur[j];
          for (var k = 0; k < oldCur.length; ++k)
            { if (oldCur[k].marker == span.marker) { continue spans } }
          oldCur.push(span);
        }
      } else if (stretchCur) {
        old[i] = stretchCur;
      }
    }
    return old
  }

  // Used both to provide a JSON-safe object in .getHistory, and, when
  // detaching a document, to split the history in two
  function copyHistoryArray(events, newGroup, instantiateSel) {
    var copy = [];
    for (var i = 0; i < events.length; ++i) {
      var event = events[i];
      if (event.ranges) {
        copy.push(instantiateSel ? Selection.prototype.deepCopy.call(event) : event);
        continue
      }
      var changes = event.changes, newChanges = [];
      copy.push({changes: newChanges});
      for (var j = 0; j < changes.length; ++j) {
        var change = changes[j], m = (void 0);
        newChanges.push({from: change.from, to: change.to, text: change.text});
        if (newGroup) { for (var prop in change) { if (m = prop.match(/^spans_(\d+)$/)) {
          if (indexOf(newGroup, Number(m[1])) > -1) {
            lst(newChanges)[prop] = change[prop];
            delete change[prop];
          }
        } } }
      }
    }
    return copy
  }

  // The 'scroll' parameter given to many of these indicated whether
  // the new cursor position should be scrolled into view after
  // modifying the selection.

  // If shift is held or the extend flag is set, extends a range to
  // include a given position (and optionally a second position).
  // Otherwise, simply returns the range between the given positions.
  // Used for cursor motion and such.
  function extendRange(range, head, other, extend) {
    if (extend) {
      var anchor = range.anchor;
      if (other) {
        var posBefore = cmp(head, anchor) < 0;
        if (posBefore != (cmp(other, anchor) < 0)) {
          anchor = head;
          head = other;
        } else if (posBefore != (cmp(head, other) < 0)) {
          head = other;
        }
      }
      return new Range(anchor, head)
    } else {
      return new Range(other || head, head)
    }
  }

  // Extend the primary selection range, discard the rest.
  function extendSelection(doc, head, other, options, extend) {
    if (extend == null) { extend = doc.cm && (doc.cm.display.shift || doc.extend); }
    setSelection(doc, new Selection([extendRange(doc.sel.primary(), head, other, extend)], 0), options);
  }

  // Extend all selections (pos is an array of selections with length
  // equal the number of selections)
  function extendSelections(doc, heads, options) {
    var out = [];
    var extend = doc.cm && (doc.cm.display.shift || doc.extend);
    for (var i = 0; i < doc.sel.ranges.length; i++)
      { out[i] = extendRange(doc.sel.ranges[i], heads[i], null, extend); }
    var newSel = normalizeSelection(doc.cm, out, doc.sel.primIndex);
    setSelection(doc, newSel, options);
  }

  // Updates a single range in the selection.
  function replaceOneSelection(doc, i, range, options) {
    var ranges = doc.sel.ranges.slice(0);
    ranges[i] = range;
    setSelection(doc, normalizeSelection(doc.cm, ranges, doc.sel.primIndex), options);
  }

  // Reset the selection to a single range.
  function setSimpleSelection(doc, anchor, head, options) {
    setSelection(doc, simpleSelection(anchor, head), options);
  }

  // Give beforeSelectionChange handlers a change to influence a
  // selection update.
  function filterSelectionChange(doc, sel, options) {
    var obj = {
      ranges: sel.ranges,
      update: function(ranges) {
        var this$1 = this;

        this.ranges = [];
        for (var i = 0; i < ranges.length; i++)
          { this$1.ranges[i] = new Range(clipPos(doc, ranges[i].anchor),
                                     clipPos(doc, ranges[i].head)); }
      },
      origin: options && options.origin
    };
    signal(doc, "beforeSelectionChange", doc, obj);
    if (doc.cm) { signal(doc.cm, "beforeSelectionChange", doc.cm, obj); }
    if (obj.ranges != sel.ranges) { return normalizeSelection(doc.cm, obj.ranges, obj.ranges.length - 1) }
    else { return sel }
  }

  function setSelectionReplaceHistory(doc, sel, options) {
    var done = doc.history.done, last = lst(done);
    if (last && last.ranges) {
      done[done.length - 1] = sel;
      setSelectionNoUndo(doc, sel, options);
    } else {
      setSelection(doc, sel, options);
    }
  }

  // Set a new selection.
  function setSelection(doc, sel, options) {
    setSelectionNoUndo(doc, sel, options);
    addSelectionToHistory(doc, doc.sel, doc.cm ? doc.cm.curOp.id : NaN, options);
  }

  function setSelectionNoUndo(doc, sel, options) {
    if (hasHandler(doc, "beforeSelectionChange") || doc.cm && hasHandler(doc.cm, "beforeSelectionChange"))
      { sel = filterSelectionChange(doc, sel, options); }

    var bias = options && options.bias ||
      (cmp(sel.primary().head, doc.sel.primary().head) < 0 ? -1 : 1);
    setSelectionInner(doc, skipAtomicInSelection(doc, sel, bias, true));

    if (!(options && options.scroll === false) && doc.cm)
      { ensureCursorVisible(doc.cm); }
  }

  function setSelectionInner(doc, sel) {
    if (sel.equals(doc.sel)) { return }

    doc.sel = sel;

    if (doc.cm) {
      doc.cm.curOp.updateInput = 1;
      doc.cm.curOp.selectionChanged = true;
      signalCursorActivity(doc.cm);
    }
    signalLater(doc, "cursorActivity", doc);
  }

  // Verify that the selection does not partially select any atomic
  // marked ranges.
  function reCheckSelection(doc) {
    setSelectionInner(doc, skipAtomicInSelection(doc, doc.sel, null, false));
  }

  // Return a selection that does not partially select any atomic
  // ranges.
  function skipAtomicInSelection(doc, sel, bias, mayClear) {
    var out;
    for (var i = 0; i < sel.ranges.length; i++) {
      var range = sel.ranges[i];
      var old = sel.ranges.length == doc.sel.ranges.length && doc.sel.ranges[i];
      var newAnchor = skipAtomic(doc, range.anchor, old && old.anchor, bias, mayClear);
      var newHead = skipAtomic(doc, range.head, old && old.head, bias, mayClear);
      if (out || newAnchor != range.anchor || newHead != range.head) {
        if (!out) { out = sel.ranges.slice(0, i); }
        out[i] = new Range(newAnchor, newHead);
      }
    }
    return out ? normalizeSelection(doc.cm, out, sel.primIndex) : sel
  }

  function skipAtomicInner(doc, pos, oldPos, dir, mayClear) {
    var line = getLine(doc, pos.line);
    if (line.markedSpans) { for (var i = 0; i < line.markedSpans.length; ++i) {
      var sp = line.markedSpans[i], m = sp.marker;

      // Determine if we should prevent the cursor being placed to the left/right of an atomic marker
      // Historically this was determined using the inclusiveLeft/Right option, but the new way to control it
      // is with selectLeft/Right
      var preventCursorLeft = ("selectLeft" in m) ? !m.selectLeft : m.inclusiveLeft;
      var preventCursorRight = ("selectRight" in m) ? !m.selectRight : m.inclusiveRight;

      if ((sp.from == null || (preventCursorLeft ? sp.from <= pos.ch : sp.from < pos.ch)) &&
          (sp.to == null || (preventCursorRight ? sp.to >= pos.ch : sp.to > pos.ch))) {
        if (mayClear) {
          signal(m, "beforeCursorEnter");
          if (m.explicitlyCleared) {
            if (!line.markedSpans) { break }
            else {--i; continue}
          }
        }
        if (!m.atomic) { continue }

        if (oldPos) {
          var near = m.find(dir < 0 ? 1 : -1), diff = (void 0);
          if (dir < 0 ? preventCursorRight : preventCursorLeft)
            { near = movePos(doc, near, -dir, near && near.line == pos.line ? line : null); }
          if (near && near.line == pos.line && (diff = cmp(near, oldPos)) && (dir < 0 ? diff < 0 : diff > 0))
            { return skipAtomicInner(doc, near, pos, dir, mayClear) }
        }

        var far = m.find(dir < 0 ? -1 : 1);
        if (dir < 0 ? preventCursorLeft : preventCursorRight)
          { far = movePos(doc, far, dir, far.line == pos.line ? line : null); }
        return far ? skipAtomicInner(doc, far, pos, dir, mayClear) : null
      }
    } }
    return pos
  }

  // Ensure a given position is not inside an atomic range.
  function skipAtomic(doc, pos, oldPos, bias, mayClear) {
    var dir = bias || 1;
    var found = skipAtomicInner(doc, pos, oldPos, dir, mayClear) ||
        (!mayClear && skipAtomicInner(doc, pos, oldPos, dir, true)) ||
        skipAtomicInner(doc, pos, oldPos, -dir, mayClear) ||
        (!mayClear && skipAtomicInner(doc, pos, oldPos, -dir, true));
    if (!found) {
      doc.cantEdit = true;
      return Pos(doc.first, 0)
    }
    return found
  }

  function movePos(doc, pos, dir, line) {
    if (dir < 0 && pos.ch == 0) {
      if (pos.line > doc.first) { return clipPos(doc, Pos(pos.line - 1)) }
      else { return null }
    } else if (dir > 0 && pos.ch == (line || getLine(doc, pos.line)).text.length) {
      if (pos.line < doc.first + doc.size - 1) { return Pos(pos.line + 1, 0) }
      else { return null }
    } else {
      return new Pos(pos.line, pos.ch + dir)
    }
  }

  function selectAll(cm) {
    cm.setSelection(Pos(cm.firstLine(), 0), Pos(cm.lastLine()), sel_dontScroll);
  }

  // UPDATING

  // Allow "beforeChange" event handlers to influence a change
  function filterChange(doc, change, update) {
    var obj = {
      canceled: false,
      from: change.from,
      to: change.to,
      text: change.text,
      origin: change.origin,
      cancel: function () { return obj.canceled = true; }
    };
    if (update) { obj.update = function (from, to, text, origin) {
      if (from) { obj.from = clipPos(doc, from); }
      if (to) { obj.to = clipPos(doc, to); }
      if (text) { obj.text = text; }
      if (origin !== undefined) { obj.origin = origin; }
    }; }
    signal(doc, "beforeChange", doc, obj);
    if (doc.cm) { signal(doc.cm, "beforeChange", doc.cm, obj); }

    if (obj.canceled) {
      if (doc.cm) { doc.cm.curOp.updateInput = 2; }
      return null
    }
    return {from: obj.from, to: obj.to, text: obj.text, origin: obj.origin}
  }

  // Apply a change to a document, and add it to the document's
  // history, and propagating it to all linked documents.
  function makeChange(doc, change, ignoreReadOnly) {
    if (doc.cm) {
      if (!doc.cm.curOp) { return operation(doc.cm, makeChange)(doc, change, ignoreReadOnly) }
      if (doc.cm.state.suppressEdits) { return }
    }

    if (hasHandler(doc, "beforeChange") || doc.cm && hasHandler(doc.cm, "beforeChange")) {
      change = filterChange(doc, change, true);
      if (!change) { return }
    }

    // Possibly split or suppress the update based on the presence
    // of read-only spans in its range.
    var split = sawReadOnlySpans && !ignoreReadOnly && removeReadOnlyRanges(doc, change.from, change.to);
    if (split) {
      for (var i = split.length - 1; i >= 0; --i)
        { makeChangeInner(doc, {from: split[i].from, to: split[i].to, text: i ? [""] : change.text, origin: change.origin}); }
    } else {
      makeChangeInner(doc, change);
    }
  }

  function makeChangeInner(doc, change) {
    if (change.text.length == 1 && change.text[0] == "" && cmp(change.from, change.to) == 0) { return }
    var selAfter = computeSelAfterChange(doc, change);
    addChangeToHistory(doc, change, selAfter, doc.cm ? doc.cm.curOp.id : NaN);

    makeChangeSingleDoc(doc, change, selAfter, stretchSpansOverChange(doc, change));
    var rebased = [];

    linkedDocs(doc, function (doc, sharedHist) {
      if (!sharedHist && indexOf(rebased, doc.history) == -1) {
        rebaseHist(doc.history, change);
        rebased.push(doc.history);
      }
      makeChangeSingleDoc(doc, change, null, stretchSpansOverChange(doc, change));
    });
  }

  // Revert a change stored in a document's history.
  function makeChangeFromHistory(doc, type, allowSelectionOnly) {
    var suppress = doc.cm && doc.cm.state.suppressEdits;
    if (suppress && !allowSelectionOnly) { return }

    var hist = doc.history, event, selAfter = doc.sel;
    var source = type == "undo" ? hist.done : hist.undone, dest = type == "undo" ? hist.undone : hist.done;

    // Verify that there is a useable event (so that ctrl-z won't
    // needlessly clear selection events)
    var i = 0;
    for (; i < source.length; i++) {
      event = source[i];
      if (allowSelectionOnly ? event.ranges && !event.equals(doc.sel) : !event.ranges)
        { break }
    }
    if (i == source.length) { return }
    hist.lastOrigin = hist.lastSelOrigin = null;

    for (;;) {
      event = source.pop();
      if (event.ranges) {
        pushSelectionToHistory(event, dest);
        if (allowSelectionOnly && !event.equals(doc.sel)) {
          setSelection(doc, event, {clearRedo: false});
          return
        }
        selAfter = event;
      } else if (suppress) {
        source.push(event);
        return
      } else { break }
    }

    // Build up a reverse change object to add to the opposite history
    // stack (redo when undoing, and vice versa).
    var antiChanges = [];
    pushSelectionToHistory(selAfter, dest);
    dest.push({changes: antiChanges, generation: hist.generation});
    hist.generation = event.generation || ++hist.maxGeneration;

    var filter = hasHandler(doc, "beforeChange") || doc.cm && hasHandler(doc.cm, "beforeChange");

    var loop = function ( i ) {
      var change = event.changes[i];
      change.origin = type;
      if (filter && !filterChange(doc, change, false)) {
        source.length = 0;
        return {}
      }

      antiChanges.push(historyChangeFromChange(doc, change));

      var after = i ? computeSelAfterChange(doc, change) : lst(source);
      makeChangeSingleDoc(doc, change, after, mergeOldSpans(doc, change));
      if (!i && doc.cm) { doc.cm.scrollIntoView({from: change.from, to: changeEnd(change)}); }
      var rebased = [];

      // Propagate to the linked documents
      linkedDocs(doc, function (doc, sharedHist) {
        if (!sharedHist && indexOf(rebased, doc.history) == -1) {
          rebaseHist(doc.history, change);
          rebased.push(doc.history);
        }
        makeChangeSingleDoc(doc, change, null, mergeOldSpans(doc, change));
      });
    };

    for (var i$1 = event.changes.length - 1; i$1 >= 0; --i$1) {
      var returned = loop( i$1 );

      if ( returned ) return returned.v;
    }
  }

  // Sub-views need their line numbers shifted when text is added
  // above or below them in the parent document.
  function shiftDoc(doc, distance) {
    if (distance == 0) { return }
    doc.first += distance;
    doc.sel = new Selection(map(doc.sel.ranges, function (range) { return new Range(
      Pos(range.anchor.line + distance, range.anchor.ch),
      Pos(range.head.line + distance, range.head.ch)
    ); }), doc.sel.primIndex);
    if (doc.cm) {
      regChange(doc.cm, doc.first, doc.first - distance, distance);
      for (var d = doc.cm.display, l = d.viewFrom; l < d.viewTo; l++)
        { regLineChange(doc.cm, l, "gutter"); }
    }
  }

  // More lower-level change function, handling only a single document
  // (not linked ones).
  function makeChangeSingleDoc(doc, change, selAfter, spans) {
    if (doc.cm && !doc.cm.curOp)
      { return operation(doc.cm, makeChangeSingleDoc)(doc, change, selAfter, spans) }

    if (change.to.line < doc.first) {
      shiftDoc(doc, change.text.length - 1 - (change.to.line - change.from.line));
      return
    }
    if (change.from.line > doc.lastLine()) { return }

    // Clip the change to the size of this doc
    if (change.from.line < doc.first) {
      var shift = change.text.length - 1 - (doc.first - change.from.line);
      shiftDoc(doc, shift);
      change = {from: Pos(doc.first, 0), to: Pos(change.to.line + shift, change.to.ch),
                text: [lst(change.text)], origin: change.origin};
    }
    var last = doc.lastLine();
    if (change.to.line > last) {
      change = {from: change.from, to: Pos(last, getLine(doc, last).text.length),
                text: [change.text[0]], origin: change.origin};
    }

    change.removed = getBetween(doc, change.from, change.to);

    if (!selAfter) { selAfter = computeSelAfterChange(doc, change); }
    if (doc.cm) { makeChangeSingleDocInEditor(doc.cm, change, spans); }
    else { updateDoc(doc, change, spans); }
    setSelectionNoUndo(doc, selAfter, sel_dontScroll);

    if (doc.cantEdit && skipAtomic(doc, Pos(doc.firstLine(), 0)))
      { doc.cantEdit = false; }
  }

  // Handle the interaction of a change to a document with the editor
  // that this document is part of.
  function makeChangeSingleDocInEditor(cm, change, spans) {
    var doc = cm.doc, display = cm.display, from = change.from, to = change.to;

    var recomputeMaxLength = false, checkWidthStart = from.line;
    if (!cm.options.lineWrapping) {
      checkWidthStart = lineNo(visualLine(getLine(doc, from.line)));
      doc.iter(checkWidthStart, to.line + 1, function (line) {
        if (line == display.maxLine) {
          recomputeMaxLength = true;
          return true
        }
      });
    }

    if (doc.sel.contains(change.from, change.to) > -1)
      { signalCursorActivity(cm); }

    updateDoc(doc, change, spans, estimateHeight(cm));

    if (!cm.options.lineWrapping) {
      doc.iter(checkWidthStart, from.line + change.text.length, function (line) {
        var len = lineLength(line);
        if (len > display.maxLineLength) {
          display.maxLine = line;
          display.maxLineLength = len;
          display.maxLineChanged = true;
          recomputeMaxLength = false;
        }
      });
      if (recomputeMaxLength) { cm.curOp.updateMaxLine = true; }
    }

    retreatFrontier(doc, from.line);
    startWorker(cm, 400);

    var lendiff = change.text.length - (to.line - from.line) - 1;
    // Remember that these lines changed, for updating the display
    if (change.full)
      { regChange(cm); }
    else if (from.line == to.line && change.text.length == 1 && !isWholeLineUpdate(cm.doc, change))
      { regLineChange(cm, from.line, "text"); }
    else
      { regChange(cm, from.line, to.line + 1, lendiff); }

    var changesHandler = hasHandler(cm, "changes"), changeHandler = hasHandler(cm, "change");
    if (changeHandler || changesHandler) {
      var obj = {
        from: from, to: to,
        text: change.text,
        removed: change.removed,
        origin: change.origin
      };
      if (changeHandler) { signalLater(cm, "change", cm, obj); }
      if (changesHandler) { (cm.curOp.changeObjs || (cm.curOp.changeObjs = [])).push(obj); }
    }
    cm.display.selForContextMenu = null;
  }

  function replaceRange(doc, code, from, to, origin) {
    var assign;

    if (!to) { to = from; }
    if (cmp(to, from) < 0) { (assign = [to, from], from = assign[0], to = assign[1]); }
    if (typeof code == "string") { code = doc.splitLines(code); }
    makeChange(doc, {from: from, to: to, text: code, origin: origin});
  }

  // Rebasing/resetting history to deal with externally-sourced changes

  function rebaseHistSelSingle(pos, from, to, diff) {
    if (to < pos.line) {
      pos.line += diff;
    } else if (from < pos.line) {
      pos.line = from;
      pos.ch = 0;
    }
  }

  // Tries to rebase an array of history events given a change in the
  // document. If the change touches the same lines as the event, the
  // event, and everything 'behind' it, is discarded. If the change is
  // before the event, the event's positions are updated. Uses a
  // copy-on-write scheme for the positions, to avoid having to
  // reallocate them all on every rebase, but also avoid problems with
  // shared position objects being unsafely updated.
  function rebaseHistArray(array, from, to, diff) {
    for (var i = 0; i < array.length; ++i) {
      var sub = array[i], ok = true;
      if (sub.ranges) {
        if (!sub.copied) { sub = array[i] = sub.deepCopy(); sub.copied = true; }
        for (var j = 0; j < sub.ranges.length; j++) {
          rebaseHistSelSingle(sub.ranges[j].anchor, from, to, diff);
          rebaseHistSelSingle(sub.ranges[j].head, from, to, diff);
        }
        continue
      }
      for (var j$1 = 0; j$1 < sub.changes.length; ++j$1) {
        var cur = sub.changes[j$1];
        if (to < cur.from.line) {
          cur.from = Pos(cur.from.line + diff, cur.from.ch);
          cur.to = Pos(cur.to.line + diff, cur.to.ch);
        } else if (from <= cur.to.line) {
          ok = false;
          break
        }
      }
      if (!ok) {
        array.splice(0, i + 1);
        i = 0;
      }
    }
  }

  function rebaseHist(hist, change) {
    var from = change.from.line, to = change.to.line, diff = change.text.length - (to - from) - 1;
    rebaseHistArray(hist.done, from, to, diff);
    rebaseHistArray(hist.undone, from, to, diff);
  }

  // Utility for applying a change to a line by handle or number,
  // returning the number and optionally registering the line as
  // changed.
  function changeLine(doc, handle, changeType, op) {
    var no = handle, line = handle;
    if (typeof handle == "number") { line = getLine(doc, clipLine(doc, handle)); }
    else { no = lineNo(handle); }
    if (no == null) { return null }
    if (op(line, no) && doc.cm) { regLineChange(doc.cm, no, changeType); }
    return line
  }

  // The document is represented as a BTree consisting of leaves, with
  // chunk of lines in them, and branches, with up to ten leaves or
  // other branch nodes below them. The top node is always a branch
  // node, and is the document object itself (meaning it has
  // additional methods and properties).
  //
  // All nodes have parent links. The tree is used both to go from
  // line numbers to line objects, and to go from objects to numbers.
  // It also indexes by height, and is used to convert between height
  // and line object, and to find the total height of the document.
  //
  // See also http://marijnhaverbeke.nl/blog/codemirror-line-tree.html

  function LeafChunk(lines) {
    var this$1 = this;

    this.lines = lines;
    this.parent = null;
    var height = 0;
    for (var i = 0; i < lines.length; ++i) {
      lines[i].parent = this$1;
      height += lines[i].height;
    }
    this.height = height;
  }

  LeafChunk.prototype = {
    chunkSize: function() { return this.lines.length },

    // Remove the n lines at offset 'at'.
    removeInner: function(at, n) {
      var this$1 = this;

      for (var i = at, e = at + n; i < e; ++i) {
        var line = this$1.lines[i];
        this$1.height -= line.height;
        cleanUpLine(line);
        signalLater(line, "delete");
      }
      this.lines.splice(at, n);
    },

    // Helper used to collapse a small branch into a single leaf.
    collapse: function(lines) {
      lines.push.apply(lines, this.lines);
    },

    // Insert the given array of lines at offset 'at', count them as
    // having the given height.
    insertInner: function(at, lines, height) {
      var this$1 = this;

      this.height += height;
      this.lines = this.lines.slice(0, at).concat(lines).concat(this.lines.slice(at));
      for (var i = 0; i < lines.length; ++i) { lines[i].parent = this$1; }
    },

    // Used to iterate over a part of the tree.
    iterN: function(at, n, op) {
      var this$1 = this;

      for (var e = at + n; at < e; ++at)
        { if (op(this$1.lines[at])) { return true } }
    }
  };

  function BranchChunk(children) {
    var this$1 = this;

    this.children = children;
    var size = 0, height = 0;
    for (var i = 0; i < children.length; ++i) {
      var ch = children[i];
      size += ch.chunkSize(); height += ch.height;
      ch.parent = this$1;
    }
    this.size = size;
    this.height = height;
    this.parent = null;
  }

  BranchChunk.prototype = {
    chunkSize: function() { return this.size },

    removeInner: function(at, n) {
      var this$1 = this;

      this.size -= n;
      for (var i = 0; i < this.children.length; ++i) {
        var child = this$1.children[i], sz = child.chunkSize();
        if (at < sz) {
          var rm = Math.min(n, sz - at), oldHeight = child.height;
          child.removeInner(at, rm);
          this$1.height -= oldHeight - child.height;
          if (sz == rm) { this$1.children.splice(i--, 1); child.parent = null; }
          if ((n -= rm) == 0) { break }
          at = 0;
        } else { at -= sz; }
      }
      // If the result is smaller than 25 lines, ensure that it is a
      // single leaf node.
      if (this.size - n < 25 &&
          (this.children.length > 1 || !(this.children[0] instanceof LeafChunk))) {
        var lines = [];
        this.collapse(lines);
        this.children = [new LeafChunk(lines)];
        this.children[0].parent = this;
      }
    },

    collapse: function(lines) {
      var this$1 = this;

      for (var i = 0; i < this.children.length; ++i) { this$1.children[i].collapse(lines); }
    },

    insertInner: function(at, lines, height) {
      var this$1 = this;

      this.size += lines.length;
      this.height += height;
      for (var i = 0; i < this.children.length; ++i) {
        var child = this$1.children[i], sz = child.chunkSize();
        if (at <= sz) {
          child.insertInner(at, lines, height);
          if (child.lines && child.lines.length > 50) {
            // To avoid memory thrashing when child.lines is huge (e.g. first view of a large file), it's never spliced.
            // Instead, small slices are taken. They're taken in order because sequential memory accesses are fastest.
            var remaining = child.lines.length % 25 + 25;
            for (var pos = remaining; pos < child.lines.length;) {
              var leaf = new LeafChunk(child.lines.slice(pos, pos += 25));
              child.height -= leaf.height;
              this$1.children.splice(++i, 0, leaf);
              leaf.parent = this$1;
            }
            child.lines = child.lines.slice(0, remaining);
            this$1.maybeSpill();
          }
          break
        }
        at -= sz;
      }
    },

    // When a node has grown, check whether it should be split.
    maybeSpill: function() {
      if (this.children.length <= 10) { return }
      var me = this;
      do {
        var spilled = me.children.splice(me.children.length - 5, 5);
        var sibling = new BranchChunk(spilled);
        if (!me.parent) { // Become the parent node
          var copy = new BranchChunk(me.children);
          copy.parent = me;
          me.children = [copy, sibling];
          me = copy;
       } else {
          me.size -= sibling.size;
          me.height -= sibling.height;
          var myIndex = indexOf(me.parent.children, me);
          me.parent.children.splice(myIndex + 1, 0, sibling);
        }
        sibling.parent = me.parent;
      } while (me.children.length > 10)
      me.parent.maybeSpill();
    },

    iterN: function(at, n, op) {
      var this$1 = this;

      for (var i = 0; i < this.children.length; ++i) {
        var child = this$1.children[i], sz = child.chunkSize();
        if (at < sz) {
          var used = Math.min(n, sz - at);
          if (child.iterN(at, used, op)) { return true }
          if ((n -= used) == 0) { break }
          at = 0;
        } else { at -= sz; }
      }
    }
  };

  // Line widgets are block elements displayed above or below a line.

  var LineWidget = function(doc, node, options) {
    var this$1 = this;

    if (options) { for (var opt in options) { if (options.hasOwnProperty(opt))
      { this$1[opt] = options[opt]; } } }
    this.doc = doc;
    this.node = node;
  };

  LineWidget.prototype.clear = function () {
      var this$1 = this;

    var cm = this.doc.cm, ws = this.line.widgets, line = this.line, no = lineNo(line);
    if (no == null || !ws) { return }
    for (var i = 0; i < ws.length; ++i) { if (ws[i] == this$1) { ws.splice(i--, 1); } }
    if (!ws.length) { line.widgets = null; }
    var height = widgetHeight(this);
    updateLineHeight(line, Math.max(0, line.height - height));
    if (cm) {
      runInOp(cm, function () {
        adjustScrollWhenAboveVisible(cm, line, -height);
        regLineChange(cm, no, "widget");
      });
      signalLater(cm, "lineWidgetCleared", cm, this, no);
    }
  };

  LineWidget.prototype.changed = function () {
      var this$1 = this;

    var oldH = this.height, cm = this.doc.cm, line = this.line;
    this.height = null;
    var diff = widgetHeight(this) - oldH;
    if (!diff) { return }
    if (!lineIsHidden(this.doc, line)) { updateLineHeight(line, line.height + diff); }
    if (cm) {
      runInOp(cm, function () {
        cm.curOp.forceUpdate = true;
        adjustScrollWhenAboveVisible(cm, line, diff);
        signalLater(cm, "lineWidgetChanged", cm, this$1, lineNo(line));
      });
    }
  };
  eventMixin(LineWidget);

  function adjustScrollWhenAboveVisible(cm, line, diff) {
    if (heightAtLine(line) < ((cm.curOp && cm.curOp.scrollTop) || cm.doc.scrollTop))
      { addToScrollTop(cm, diff); }
  }

  function addLineWidget(doc, handle, node, options) {
    var widget = new LineWidget(doc, node, options);
    var cm = doc.cm;
    if (cm && widget.noHScroll) { cm.display.alignWidgets = true; }
    changeLine(doc, handle, "widget", function (line) {
      var widgets = line.widgets || (line.widgets = []);
      if (widget.insertAt == null) { widgets.push(widget); }
      else { widgets.splice(Math.min(widgets.length - 1, Math.max(0, widget.insertAt)), 0, widget); }
      widget.line = line;
      if (cm && !lineIsHidden(doc, line)) {
        var aboveVisible = heightAtLine(line) < doc.scrollTop;
        updateLineHeight(line, line.height + widgetHeight(widget));
        if (aboveVisible) { addToScrollTop(cm, widget.height); }
        cm.curOp.forceUpdate = true;
      }
      return true
    });
    if (cm) { signalLater(cm, "lineWidgetAdded", cm, widget, typeof handle == "number" ? handle : lineNo(handle)); }
    return widget
  }

  // TEXTMARKERS

  // Created with markText and setBookmark methods. A TextMarker is a
  // handle that can be used to clear or find a marked position in the
  // document. Line objects hold arrays (markedSpans) containing
  // {from, to, marker} object pointing to such marker objects, and
  // indicating that such a marker is present on that line. Multiple
  // lines may point to the same marker when it spans across lines.
  // The spans will have null for their from/to properties when the
  // marker continues beyond the start/end of the line. Markers have
  // links back to the lines they currently touch.

  // Collapsed markers have unique ids, in order to be able to order
  // them, which is needed for uniquely determining an outer marker
  // when they overlap (they may nest, but not partially overlap).
  var nextMarkerId = 0;

  var TextMarker = function(doc, type) {
    this.lines = [];
    this.type = type;
    this.doc = doc;
    this.id = ++nextMarkerId;
  };

  // Clear the marker.
  TextMarker.prototype.clear = function () {
      var this$1 = this;

    if (this.explicitlyCleared) { return }
    var cm = this.doc.cm, withOp = cm && !cm.curOp;
    if (withOp) { startOperation(cm); }
    if (hasHandler(this, "clear")) {
      var found = this.find();
      if (found) { signalLater(this, "clear", found.from, found.to); }
    }
    var min = null, max = null;
    for (var i = 0; i < this.lines.length; ++i) {
      var line = this$1.lines[i];
      var span = getMarkedSpanFor(line.markedSpans, this$1);
      if (cm && !this$1.collapsed) { regLineChange(cm, lineNo(line), "text"); }
      else if (cm) {
        if (span.to != null) { max = lineNo(line); }
        if (span.from != null) { min = lineNo(line); }
      }
      line.markedSpans = removeMarkedSpan(line.markedSpans, span);
      if (span.from == null && this$1.collapsed && !lineIsHidden(this$1.doc, line) && cm)
        { updateLineHeight(line, textHeight(cm.display)); }
    }
    if (cm && this.collapsed && !cm.options.lineWrapping) { for (var i$1 = 0; i$1 < this.lines.length; ++i$1) {
      var visual = visualLine(this$1.lines[i$1]), len = lineLength(visual);
      if (len > cm.display.maxLineLength) {
        cm.display.maxLine = visual;
        cm.display.maxLineLength = len;
        cm.display.maxLineChanged = true;
      }
    } }

    if (min != null && cm && this.collapsed) { regChange(cm, min, max + 1); }
    this.lines.length = 0;
    this.explicitlyCleared = true;
    if (this.atomic && this.doc.cantEdit) {
      this.doc.cantEdit = false;
      if (cm) { reCheckSelection(cm.doc); }
    }
    if (cm) { signalLater(cm, "markerCleared", cm, this, min, max); }
    if (withOp) { endOperation(cm); }
    if (this.parent) { this.parent.clear(); }
  };

  // Find the position of the marker in the document. Returns a {from,
  // to} object by default. Side can be passed to get a specific side
  // -- 0 (both), -1 (left), or 1 (right). When lineObj is true, the
  // Pos objects returned contain a line object, rather than a line
  // number (used to prevent looking up the same line twice).
  TextMarker.prototype.find = function (side, lineObj) {
      var this$1 = this;

    if (side == null && this.type == "bookmark") { side = 1; }
    var from, to;
    for (var i = 0; i < this.lines.length; ++i) {
      var line = this$1.lines[i];
      var span = getMarkedSpanFor(line.markedSpans, this$1);
      if (span.from != null) {
        from = Pos(lineObj ? line : lineNo(line), span.from);
        if (side == -1) { return from }
      }
      if (span.to != null) {
        to = Pos(lineObj ? line : lineNo(line), span.to);
        if (side == 1) { return to }
      }
    }
    return from && {from: from, to: to}
  };

  // Signals that the marker's widget changed, and surrounding layout
  // should be recomputed.
  TextMarker.prototype.changed = function () {
      var this$1 = this;

    var pos = this.find(-1, true), widget = this, cm = this.doc.cm;
    if (!pos || !cm) { return }
    runInOp(cm, function () {
      var line = pos.line, lineN = lineNo(pos.line);
      var view = findViewForLine(cm, lineN);
      if (view) {
        clearLineMeasurementCacheFor(view);
        cm.curOp.selectionChanged = cm.curOp.forceUpdate = true;
      }
      cm.curOp.updateMaxLine = true;
      if (!lineIsHidden(widget.doc, line) && widget.height != null) {
        var oldHeight = widget.height;
        widget.height = null;
        var dHeight = widgetHeight(widget) - oldHeight;
        if (dHeight)
          { updateLineHeight(line, line.height + dHeight); }
      }
      signalLater(cm, "markerChanged", cm, this$1);
    });
  };

  TextMarker.prototype.attachLine = function (line) {
    if (!this.lines.length && this.doc.cm) {
      var op = this.doc.cm.curOp;
      if (!op.maybeHiddenMarkers || indexOf(op.maybeHiddenMarkers, this) == -1)
        { (op.maybeUnhiddenMarkers || (op.maybeUnhiddenMarkers = [])).push(this); }
    }
    this.lines.push(line);
  };

  TextMarker.prototype.detachLine = function (line) {
    this.lines.splice(indexOf(this.lines, line), 1);
    if (!this.lines.length && this.doc.cm) {
      var op = this.doc.cm.curOp
      ;(op.maybeHiddenMarkers || (op.maybeHiddenMarkers = [])).push(this);
    }
  };
  eventMixin(TextMarker);

  // Create a marker, wire it up to the right lines, and
  function markText(doc, from, to, options, type) {
    // Shared markers (across linked documents) are handled separately
    // (markTextShared will call out to this again, once per
    // document).
    if (options && options.shared) { return markTextShared(doc, from, to, options, type) }
    // Ensure we are in an operation.
    if (doc.cm && !doc.cm.curOp) { return operation(doc.cm, markText)(doc, from, to, options, type) }

    var marker = new TextMarker(doc, type), diff = cmp(from, to);
    if (options) { copyObj(options, marker, false); }
    // Don't connect empty markers unless clearWhenEmpty is false
    if (diff > 0 || diff == 0 && marker.clearWhenEmpty !== false)
      { return marker }
    if (marker.replacedWith) {
      // Showing up as a widget implies collapsed (widget replaces text)
      marker.collapsed = true;
      marker.widgetNode = eltP("span", [marker.replacedWith], "CodeMirror-widget");
      if (!options.handleMouseEvents) { marker.widgetNode.setAttribute("cm-ignore-events", "true"); }
      if (options.insertLeft) { marker.widgetNode.insertLeft = true; }
    }
    if (marker.collapsed) {
      if (conflictingCollapsedRange(doc, from.line, from, to, marker) ||
          from.line != to.line && conflictingCollapsedRange(doc, to.line, from, to, marker))
        { throw new Error("Inserting collapsed marker partially overlapping an existing one") }
      seeCollapsedSpans();
    }

    if (marker.addToHistory)
      { addChangeToHistory(doc, {from: from, to: to, origin: "markText"}, doc.sel, NaN); }

    var curLine = from.line, cm = doc.cm, updateMaxLine;
    doc.iter(curLine, to.line + 1, function (line) {
      if (cm && marker.collapsed && !cm.options.lineWrapping && visualLine(line) == cm.display.maxLine)
        { updateMaxLine = true; }
      if (marker.collapsed && curLine != from.line) { updateLineHeight(line, 0); }
      addMarkedSpan(line, new MarkedSpan(marker,
                                         curLine == from.line ? from.ch : null,
                                         curLine == to.line ? to.ch : null));
      ++curLine;
    });
    // lineIsHidden depends on the presence of the spans, so needs a second pass
    if (marker.collapsed) { doc.iter(from.line, to.line + 1, function (line) {
      if (lineIsHidden(doc, line)) { updateLineHeight(line, 0); }
    }); }

    if (marker.clearOnEnter) { on(marker, "beforeCursorEnter", function () { return marker.clear(); }); }

    if (marker.readOnly) {
      seeReadOnlySpans();
      if (doc.history.done.length || doc.history.undone.length)
        { doc.clearHistory(); }
    }
    if (marker.collapsed) {
      marker.id = ++nextMarkerId;
      marker.atomic = true;
    }
    if (cm) {
      // Sync editor state
      if (updateMaxLine) { cm.curOp.updateMaxLine = true; }
      if (marker.collapsed)
        { regChange(cm, from.line, to.line + 1); }
      else if (marker.className || marker.startStyle || marker.endStyle || marker.css ||
               marker.attributes || marker.title)
        { for (var i = from.line; i <= to.line; i++) { regLineChange(cm, i, "text"); } }
      if (marker.atomic) { reCheckSelection(cm.doc); }
      signalLater(cm, "markerAdded", cm, marker);
    }
    return marker
  }

  // SHARED TEXTMARKERS

  // A shared marker spans multiple linked documents. It is
  // implemented as a meta-marker-object controlling multiple normal
  // markers.
  var SharedTextMarker = function(markers, primary) {
    var this$1 = this;

    this.markers = markers;
    this.primary = primary;
    for (var i = 0; i < markers.length; ++i)
      { markers[i].parent = this$1; }
  };

  SharedTextMarker.prototype.clear = function () {
      var this$1 = this;

    if (this.explicitlyCleared) { return }
    this.explicitlyCleared = true;
    for (var i = 0; i < this.markers.length; ++i)
      { this$1.markers[i].clear(); }
    signalLater(this, "clear");
  };

  SharedTextMarker.prototype.find = function (side, lineObj) {
    return this.primary.find(side, lineObj)
  };
  eventMixin(SharedTextMarker);

  function markTextShared(doc, from, to, options, type) {
    options = copyObj(options);
    options.shared = false;
    var markers = [markText(doc, from, to, options, type)], primary = markers[0];
    var widget = options.widgetNode;
    linkedDocs(doc, function (doc) {
      if (widget) { options.widgetNode = widget.cloneNode(true); }
      markers.push(markText(doc, clipPos(doc, from), clipPos(doc, to), options, type));
      for (var i = 0; i < doc.linked.length; ++i)
        { if (doc.linked[i].isParent) { return } }
      primary = lst(markers);
    });
    return new SharedTextMarker(markers, primary)
  }

  function findSharedMarkers(doc) {
    return doc.findMarks(Pos(doc.first, 0), doc.clipPos(Pos(doc.lastLine())), function (m) { return m.parent; })
  }

  function copySharedMarkers(doc, markers) {
    for (var i = 0; i < markers.length; i++) {
      var marker = markers[i], pos = marker.find();
      var mFrom = doc.clipPos(pos.from), mTo = doc.clipPos(pos.to);
      if (cmp(mFrom, mTo)) {
        var subMark = markText(doc, mFrom, mTo, marker.primary, marker.primary.type);
        marker.markers.push(subMark);
        subMark.parent = marker;
      }
    }
  }

  function detachSharedMarkers(markers) {
    var loop = function ( i ) {
      var marker = markers[i], linked = [marker.primary.doc];
      linkedDocs(marker.primary.doc, function (d) { return linked.push(d); });
      for (var j = 0; j < marker.markers.length; j++) {
        var subMarker = marker.markers[j];
        if (indexOf(linked, subMarker.doc) == -1) {
          subMarker.parent = null;
          marker.markers.splice(j--, 1);
        }
      }
    };

    for (var i = 0; i < markers.length; i++) loop( i );
  }

  var nextDocId = 0;
  var Doc = function(text, mode, firstLine, lineSep, direction) {
    if (!(this instanceof Doc)) { return new Doc(text, mode, firstLine, lineSep, direction) }
    if (firstLine == null) { firstLine = 0; }

    BranchChunk.call(this, [new LeafChunk([new Line("", null)])]);
    this.first = firstLine;
    this.scrollTop = this.scrollLeft = 0;
    this.cantEdit = false;
    this.cleanGeneration = 1;
    this.modeFrontier = this.highlightFrontier = firstLine;
    var start = Pos(firstLine, 0);
    this.sel = simpleSelection(start);
    this.history = new History(null);
    this.id = ++nextDocId;
    this.modeOption = mode;
    this.lineSep = lineSep;
    this.direction = (direction == "rtl") ? "rtl" : "ltr";
    this.extend = false;

    if (typeof text == "string") { text = this.splitLines(text); }
    updateDoc(this, {from: start, to: start, text: text});
    setSelection(this, simpleSelection(start), sel_dontScroll);
  };

  Doc.prototype = createObj(BranchChunk.prototype, {
    constructor: Doc,
    // Iterate over the document. Supports two forms -- with only one
    // argument, it calls that for each line in the document. With
    // three, it iterates over the range given by the first two (with
    // the second being non-inclusive).
    iter: function(from, to, op) {
      if (op) { this.iterN(from - this.first, to - from, op); }
      else { this.iterN(this.first, this.first + this.size, from); }
    },

    // Non-public interface for adding and removing lines.
    insert: function(at, lines) {
      var height = 0;
      for (var i = 0; i < lines.length; ++i) { height += lines[i].height; }
      this.insertInner(at - this.first, lines, height);
    },
    remove: function(at, n) { this.removeInner(at - this.first, n); },

    // From here, the methods are part of the public interface. Most
    // are also available from CodeMirror (editor) instances.

    getValue: function(lineSep) {
      var lines = getLines(this, this.first, this.first + this.size);
      if (lineSep === false) { return lines }
      return lines.join(lineSep || this.lineSeparator())
    },
    setValue: docMethodOp(function(code) {
      var top = Pos(this.first, 0), last = this.first + this.size - 1;
      makeChange(this, {from: top, to: Pos(last, getLine(this, last).text.length),
                        text: this.splitLines(code), origin: "setValue", full: true}, true);
      if (this.cm) { scrollToCoords(this.cm, 0, 0); }
      setSelection(this, simpleSelection(top), sel_dontScroll);
    }),
    replaceRange: function(code, from, to, origin) {
      from = clipPos(this, from);
      to = to ? clipPos(this, to) : from;
      replaceRange(this, code, from, to, origin);
    },
    getRange: function(from, to, lineSep) {
      var lines = getBetween(this, clipPos(this, from), clipPos(this, to));
      if (lineSep === false) { return lines }
      return lines.join(lineSep || this.lineSeparator())
    },

    getLine: function(line) {var l = this.getLineHandle(line); return l && l.text},

    getLineHandle: function(line) {if (isLine(this, line)) { return getLine(this, line) }},
    getLineNumber: function(line) {return lineNo(line)},

    getLineHandleVisualStart: function(line) {
      if (typeof line == "number") { line = getLine(this, line); }
      return visualLine(line)
    },

    lineCount: function() {return this.size},
    firstLine: function() {return this.first},
    lastLine: function() {return this.first + this.size - 1},

    clipPos: function(pos) {return clipPos(this, pos)},

    getCursor: function(start) {
      var range$$1 = this.sel.primary(), pos;
      if (start == null || start == "head") { pos = range$$1.head; }
      else if (start == "anchor") { pos = range$$1.anchor; }
      else if (start == "end" || start == "to" || start === false) { pos = range$$1.to(); }
      else { pos = range$$1.from(); }
      return pos
    },
    listSelections: function() { return this.sel.ranges },
    somethingSelected: function() {return this.sel.somethingSelected()},

    setCursor: docMethodOp(function(line, ch, options) {
      setSimpleSelection(this, clipPos(this, typeof line == "number" ? Pos(line, ch || 0) : line), null, options);
    }),
    setSelection: docMethodOp(function(anchor, head, options) {
      setSimpleSelection(this, clipPos(this, anchor), clipPos(this, head || anchor), options);
    }),
    extendSelection: docMethodOp(function(head, other, options) {
      extendSelection(this, clipPos(this, head), other && clipPos(this, other), options);
    }),
    extendSelections: docMethodOp(function(heads, options) {
      extendSelections(this, clipPosArray(this, heads), options);
    }),
    extendSelectionsBy: docMethodOp(function(f, options) {
      var heads = map(this.sel.ranges, f);
      extendSelections(this, clipPosArray(this, heads), options);
    }),
    setSelections: docMethodOp(function(ranges, primary, options) {
      var this$1 = this;

      if (!ranges.length) { return }
      var out = [];
      for (var i = 0; i < ranges.length; i++)
        { out[i] = new Range(clipPos(this$1, ranges[i].anchor),
                           clipPos(this$1, ranges[i].head)); }
      if (primary == null) { primary = Math.min(ranges.length - 1, this.sel.primIndex); }
      setSelection(this, normalizeSelection(this.cm, out, primary), options);
    }),
    addSelection: docMethodOp(function(anchor, head, options) {
      var ranges = this.sel.ranges.slice(0);
      ranges.push(new Range(clipPos(this, anchor), clipPos(this, head || anchor)));
      setSelection(this, normalizeSelection(this.cm, ranges, ranges.length - 1), options);
    }),

    getSelection: function(lineSep) {
      var this$1 = this;

      var ranges = this.sel.ranges, lines;
      for (var i = 0; i < ranges.length; i++) {
        var sel = getBetween(this$1, ranges[i].from(), ranges[i].to());
        lines = lines ? lines.concat(sel) : sel;
      }
      if (lineSep === false) { return lines }
      else { return lines.join(lineSep || this.lineSeparator()) }
    },
    getSelections: function(lineSep) {
      var this$1 = this;

      var parts = [], ranges = this.sel.ranges;
      for (var i = 0; i < ranges.length; i++) {
        var sel = getBetween(this$1, ranges[i].from(), ranges[i].to());
        if (lineSep !== false) { sel = sel.join(lineSep || this$1.lineSeparator()); }
        parts[i] = sel;
      }
      return parts
    },
    replaceSelection: function(code, collapse, origin) {
      var dup = [];
      for (var i = 0; i < this.sel.ranges.length; i++)
        { dup[i] = code; }
      this.replaceSelections(dup, collapse, origin || "+input");
    },
    replaceSelections: docMethodOp(function(code, collapse, origin) {
      var this$1 = this;

      var changes = [], sel = this.sel;
      for (var i = 0; i < sel.ranges.length; i++) {
        var range$$1 = sel.ranges[i];
        changes[i] = {from: range$$1.from(), to: range$$1.to(), text: this$1.splitLines(code[i]), origin: origin};
      }
      var newSel = collapse && collapse != "end" && computeReplacedSel(this, changes, collapse);
      for (var i$1 = changes.length - 1; i$1 >= 0; i$1--)
        { makeChange(this$1, changes[i$1]); }
      if (newSel) { setSelectionReplaceHistory(this, newSel); }
      else if (this.cm) { ensureCursorVisible(this.cm); }
    }),
    undo: docMethodOp(function() {makeChangeFromHistory(this, "undo");}),
    redo: docMethodOp(function() {makeChangeFromHistory(this, "redo");}),
    undoSelection: docMethodOp(function() {makeChangeFromHistory(this, "undo", true);}),
    redoSelection: docMethodOp(function() {makeChangeFromHistory(this, "redo", true);}),

    setExtending: function(val) {this.extend = val;},
    getExtending: function() {return this.extend},

    historySize: function() {
      var hist = this.history, done = 0, undone = 0;
      for (var i = 0; i < hist.done.length; i++) { if (!hist.done[i].ranges) { ++done; } }
      for (var i$1 = 0; i$1 < hist.undone.length; i$1++) { if (!hist.undone[i$1].ranges) { ++undone; } }
      return {undo: done, redo: undone}
    },
    clearHistory: function() {this.history = new History(this.history.maxGeneration);},

    markClean: function() {
      this.cleanGeneration = this.changeGeneration(true);
    },
    changeGeneration: function(forceSplit) {
      if (forceSplit)
        { this.history.lastOp = this.history.lastSelOp = this.history.lastOrigin = null; }
      return this.history.generation
    },
    isClean: function (gen) {
      return this.history.generation == (gen || this.cleanGeneration)
    },

    getHistory: function() {
      return {done: copyHistoryArray(this.history.done),
              undone: copyHistoryArray(this.history.undone)}
    },
    setHistory: function(histData) {
      var hist = this.history = new History(this.history.maxGeneration);
      hist.done = copyHistoryArray(histData.done.slice(0), null, true);
      hist.undone = copyHistoryArray(histData.undone.slice(0), null, true);
    },

    setGutterMarker: docMethodOp(function(line, gutterID, value) {
      return changeLine(this, line, "gutter", function (line) {
        var markers = line.gutterMarkers || (line.gutterMarkers = {});
        markers[gutterID] = value;
        if (!value && isEmpty(markers)) { line.gutterMarkers = null; }
        return true
      })
    }),

    clearGutter: docMethodOp(function(gutterID) {
      var this$1 = this;

      this.iter(function (line) {
        if (line.gutterMarkers && line.gutterMarkers[gutterID]) {
          changeLine(this$1, line, "gutter", function () {
            line.gutterMarkers[gutterID] = null;
            if (isEmpty(line.gutterMarkers)) { line.gutterMarkers = null; }
            return true
          });
        }
      });
    }),

    lineInfo: function(line) {
      var n;
      if (typeof line == "number") {
        if (!isLine(this, line)) { return null }
        n = line;
        line = getLine(this, line);
        if (!line) { return null }
      } else {
        n = lineNo(line);
        if (n == null) { return null }
      }
      return {line: n, handle: line, text: line.text, gutterMarkers: line.gutterMarkers,
              textClass: line.textClass, bgClass: line.bgClass, wrapClass: line.wrapClass,
              widgets: line.widgets}
    },

    addLineClass: docMethodOp(function(handle, where, cls) {
      return changeLine(this, handle, where == "gutter" ? "gutter" : "class", function (line) {
        var prop = where == "text" ? "textClass"
                 : where == "background" ? "bgClass"
                 : where == "gutter" ? "gutterClass" : "wrapClass";
        if (!line[prop]) { line[prop] = cls; }
        else if (classTest(cls).test(line[prop])) { return false }
        else { line[prop] += " " + cls; }
        return true
      })
    }),
    removeLineClass: docMethodOp(function(handle, where, cls) {
      return changeLine(this, handle, where == "gutter" ? "gutter" : "class", function (line) {
        var prop = where == "text" ? "textClass"
                 : where == "background" ? "bgClass"
                 : where == "gutter" ? "gutterClass" : "wrapClass";
        var cur = line[prop];
        if (!cur) { return false }
        else if (cls == null) { line[prop] = null; }
        else {
          var found = cur.match(classTest(cls));
          if (!found) { return false }
          var end = found.index + found[0].length;
          line[prop] = cur.slice(0, found.index) + (!found.index || end == cur.length ? "" : " ") + cur.slice(end) || null;
        }
        return true
      })
    }),

    addLineWidget: docMethodOp(function(handle, node, options) {
      return addLineWidget(this, handle, node, options)
    }),
    removeLineWidget: function(widget) { widget.clear(); },

    markText: function(from, to, options) {
      return markText(this, clipPos(this, from), clipPos(this, to), options, options && options.type || "range")
    },
    setBookmark: function(pos, options) {
      var realOpts = {replacedWith: options && (options.nodeType == null ? options.widget : options),
                      insertLeft: options && options.insertLeft,
                      clearWhenEmpty: false, shared: options && options.shared,
                      handleMouseEvents: options && options.handleMouseEvents};
      pos = clipPos(this, pos);
      return markText(this, pos, pos, realOpts, "bookmark")
    },
    findMarksAt: function(pos) {
      pos = clipPos(this, pos);
      var markers = [], spans = getLine(this, pos.line).markedSpans;
      if (spans) { for (var i = 0; i < spans.length; ++i) {
        var span = spans[i];
        if ((span.from == null || span.from <= pos.ch) &&
            (span.to == null || span.to >= pos.ch))
          { markers.push(span.marker.parent || span.marker); }
      } }
      return markers
    },
    findMarks: function(from, to, filter) {
      from = clipPos(this, from); to = clipPos(this, to);
      var found = [], lineNo$$1 = from.line;
      this.iter(from.line, to.line + 1, function (line) {
        var spans = line.markedSpans;
        if (spans) { for (var i = 0; i < spans.length; i++) {
          var span = spans[i];
          if (!(span.to != null && lineNo$$1 == from.line && from.ch >= span.to ||
                span.from == null && lineNo$$1 != from.line ||
                span.from != null && lineNo$$1 == to.line && span.from >= to.ch) &&
              (!filter || filter(span.marker)))
            { found.push(span.marker.parent || span.marker); }
        } }
        ++lineNo$$1;
      });
      return found
    },
    getAllMarks: function() {
      var markers = [];
      this.iter(function (line) {
        var sps = line.markedSpans;
        if (sps) { for (var i = 0; i < sps.length; ++i)
          { if (sps[i].from != null) { markers.push(sps[i].marker); } } }
      });
      return markers
    },

    posFromIndex: function(off) {
      var ch, lineNo$$1 = this.first, sepSize = this.lineSeparator().length;
      this.iter(function (line) {
        var sz = line.text.length + sepSize;
        if (sz > off) { ch = off; return true }
        off -= sz;
        ++lineNo$$1;
      });
      return clipPos(this, Pos(lineNo$$1, ch))
    },
    indexFromPos: function (coords) {
      coords = clipPos(this, coords);
      var index = coords.ch;
      if (coords.line < this.first || coords.ch < 0) { return 0 }
      var sepSize = this.lineSeparator().length;
      this.iter(this.first, coords.line, function (line) { // iter aborts when callback returns a truthy value
        index += line.text.length + sepSize;
      });
      return index
    },

    copy: function(copyHistory) {
      var doc = new Doc(getLines(this, this.first, this.first + this.size),
                        this.modeOption, this.first, this.lineSep, this.direction);
      doc.scrollTop = this.scrollTop; doc.scrollLeft = this.scrollLeft;
      doc.sel = this.sel;
      doc.extend = false;
      if (copyHistory) {
        doc.history.undoDepth = this.history.undoDepth;
        doc.setHistory(this.getHistory());
      }
      return doc
    },

    linkedDoc: function(options) {
      if (!options) { options = {}; }
      var from = this.first, to = this.first + this.size;
      if (options.from != null && options.from > from) { from = options.from; }
      if (options.to != null && options.to < to) { to = options.to; }
      var copy = new Doc(getLines(this, from, to), options.mode || this.modeOption, from, this.lineSep, this.direction);
      if (options.sharedHist) { copy.history = this.history
      ; }(this.linked || (this.linked = [])).push({doc: copy, sharedHist: options.sharedHist});
      copy.linked = [{doc: this, isParent: true, sharedHist: options.sharedHist}];
      copySharedMarkers(copy, findSharedMarkers(this));
      return copy
    },
    unlinkDoc: function(other) {
      var this$1 = this;

      if (other instanceof CodeMirror) { other = other.doc; }
      if (this.linked) { for (var i = 0; i < this.linked.length; ++i) {
        var link = this$1.linked[i];
        if (link.doc != other) { continue }
        this$1.linked.splice(i, 1);
        other.unlinkDoc(this$1);
        detachSharedMarkers(findSharedMarkers(this$1));
        break
      } }
      // If the histories were shared, split them again
      if (other.history == this.history) {
        var splitIds = [other.id];
        linkedDocs(other, function (doc) { return splitIds.push(doc.id); }, true);
        other.history = new History(null);
        other.history.done = copyHistoryArray(this.history.done, splitIds);
        other.history.undone = copyHistoryArray(this.history.undone, splitIds);
      }
    },
    iterLinkedDocs: function(f) {linkedDocs(this, f);},

    getMode: function() {return this.mode},
    getEditor: function() {return this.cm},

    splitLines: function(str) {
      if (this.lineSep) { return str.split(this.lineSep) }
      return splitLinesAuto(str)
    },
    lineSeparator: function() { return this.lineSep || "\n" },

    setDirection: docMethodOp(function (dir) {
      if (dir != "rtl") { dir = "ltr"; }
      if (dir == this.direction) { return }
      this.direction = dir;
      this.iter(function (line) { return line.order = null; });
      if (this.cm) { directionChanged(this.cm); }
    })
  });

  // Public alias.
  Doc.prototype.eachLine = Doc.prototype.iter;

  // Kludge to work around strange IE behavior where it'll sometimes
  // re-fire a series of drag-related events right after the drop (#1551)
  var lastDrop = 0;

  function onDrop(e) {
    var cm = this;
    clearDragCursor(cm);
    if (signalDOMEvent(cm, e) || eventInWidget(cm.display, e))
      { return }
    e_preventDefault(e);
    if (ie) { lastDrop = +new Date; }
    var pos = posFromMouse(cm, e, true), files = e.dataTransfer.files;
    if (!pos || cm.isReadOnly()) { return }
    // Might be a file drop, in which case we simply extract the text
    // and insert it.
    if (files && files.length && window.FileReader && window.File) {
      var n = files.length, text = Array(n), read = 0;
      var loadFile = function (file, i) {
        if (cm.options.allowDropFileTypes &&
            indexOf(cm.options.allowDropFileTypes, file.type) == -1)
          { return }

        var reader = new FileReader;
        reader.onload = operation(cm, function () {
          var content = reader.result;
          if (/[\x00-\x08\x0e-\x1f]{2}/.test(content)) { content = ""; }
          text[i] = content;
          if (++read == n) {
            pos = clipPos(cm.doc, pos);
            var change = {from: pos, to: pos,
                          text: cm.doc.splitLines(text.join(cm.doc.lineSeparator())),
                          origin: "paste"};
            makeChange(cm.doc, change);
            setSelectionReplaceHistory(cm.doc, simpleSelection(pos, changeEnd(change)));
          }
        });
        reader.readAsText(file);
      };
      for (var i = 0; i < n; ++i) { loadFile(files[i], i); }
    } else { // Normal drop
      // Don't do a replace if the drop happened inside of the selected text.
      if (cm.state.draggingText && cm.doc.sel.contains(pos) > -1) {
        cm.state.draggingText(e);
        // Ensure the editor is re-focused
        setTimeout(function () { return cm.display.input.focus(); }, 20);
        return
      }
      try {
        var text$1 = e.dataTransfer.getData("Text");
        if (text$1) {
          var selected;
          if (cm.state.draggingText && !cm.state.draggingText.copy)
            { selected = cm.listSelections(); }
          setSelectionNoUndo(cm.doc, simpleSelection(pos, pos));
          if (selected) { for (var i$1 = 0; i$1 < selected.length; ++i$1)
            { replaceRange(cm.doc, "", selected[i$1].anchor, selected[i$1].head, "drag"); } }
          cm.replaceSelection(text$1, "around", "paste");
          cm.display.input.focus();
        }
      }
      catch(e){}
    }
  }

  function onDragStart(cm, e) {
    if (ie && (!cm.state.draggingText || +new Date - lastDrop < 100)) { e_stop(e); return }
    if (signalDOMEvent(cm, e) || eventInWidget(cm.display, e)) { return }

    e.dataTransfer.setData("Text", cm.getSelection());
    e.dataTransfer.effectAllowed = "copyMove";

    // Use dummy image instead of default browsers image.
    // Recent Safari (~6.0.2) have a tendency to segfault when this happens, so we don't do it there.
    if (e.dataTransfer.setDragImage && !safari) {
      var img = elt("img", null, null, "position: fixed; left: 0; top: 0;");
      img.src = "data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==";
      if (presto) {
        img.width = img.height = 1;
        cm.display.wrapper.appendChild(img);
        // Force a relayout, or Opera won't use our image for some obscure reason
        img._top = img.offsetTop;
      }
      e.dataTransfer.setDragImage(img, 0, 0);
      if (presto) { img.parentNode.removeChild(img); }
    }
  }

  function onDragOver(cm, e) {
    var pos = posFromMouse(cm, e);
    if (!pos) { return }
    var frag = document.createDocumentFragment();
    drawSelectionCursor(cm, pos, frag);
    if (!cm.display.dragCursor) {
      cm.display.dragCursor = elt("div", null, "CodeMirror-cursors CodeMirror-dragcursors");
      cm.display.lineSpace.insertBefore(cm.display.dragCursor, cm.display.cursorDiv);
    }
    removeChildrenAndAdd(cm.display.dragCursor, frag);
  }

  function clearDragCursor(cm) {
    if (cm.display.dragCursor) {
      cm.display.lineSpace.removeChild(cm.display.dragCursor);
      cm.display.dragCursor = null;
    }
  }

  // These must be handled carefully, because naively registering a
  // handler for each editor will cause the editors to never be
  // garbage collected.

  function forEachCodeMirror(f) {
    if (!document.getElementsByClassName) { return }
    var byClass = document.getElementsByClassName("CodeMirror"), editors = [];
    for (var i = 0; i < byClass.length; i++) {
      var cm = byClass[i].CodeMirror;
      if (cm) { editors.push(cm); }
    }
    if (editors.length) { editors[0].operation(function () {
      for (var i = 0; i < editors.length; i++) { f(editors[i]); }
    }); }
  }

  var globalsRegistered = false;
  function ensureGlobalHandlers() {
    if (globalsRegistered) { return }
    registerGlobalHandlers();
    globalsRegistered = true;
  }
  function registerGlobalHandlers() {
    // When the window resizes, we need to refresh active editors.
    var resizeTimer;
    on(window, "resize", function () {
      if (resizeTimer == null) { resizeTimer = setTimeout(function () {
        resizeTimer = null;
        forEachCodeMirror(onResize);
      }, 100); }
    });
    // When the window loses focus, we want to show the editor as blurred
    on(window, "blur", function () { return forEachCodeMirror(onBlur); });
  }
  // Called when the window resizes
  function onResize(cm) {
    var d = cm.display;
    // Might be a text scaling operation, clear size caches.
    d.cachedCharWidth = d.cachedTextHeight = d.cachedPaddingH = null;
    d.scrollbarsClipped = false;
    cm.setSize();
  }

  var keyNames = {
    3: "Pause", 8: "Backspace", 9: "Tab", 13: "Enter", 16: "Shift", 17: "Ctrl", 18: "Alt",
    19: "Pause", 20: "CapsLock", 27: "Esc", 32: "Space", 33: "PageUp", 34: "PageDown", 35: "End",
    36: "Home", 37: "Left", 38: "Up", 39: "Right", 40: "Down", 44: "PrintScrn", 45: "Insert",
    46: "Delete", 59: ";", 61: "=", 91: "Mod", 92: "Mod", 93: "Mod",
    106: "*", 107: "=", 109: "-", 110: ".", 111: "/", 145: "ScrollLock",
    173: "-", 186: ";", 187: "=", 188: ",", 189: "-", 190: ".", 191: "/", 192: "`", 219: "[", 220: "\\",
    221: "]", 222: "'", 63232: "Up", 63233: "Down", 63234: "Left", 63235: "Right", 63272: "Delete",
    63273: "Home", 63275: "End", 63276: "PageUp", 63277: "PageDown", 63302: "Insert"
  };

  // Number keys
  for (var i = 0; i < 10; i++) { keyNames[i + 48] = keyNames[i + 96] = String(i); }
  // Alphabetic keys
  for (var i$1 = 65; i$1 <= 90; i$1++) { keyNames[i$1] = String.fromCharCode(i$1); }
  // Function keys
  for (var i$2 = 1; i$2 <= 12; i$2++) { keyNames[i$2 + 111] = keyNames[i$2 + 63235] = "F" + i$2; }

  var keyMap = {};

  keyMap.basic = {
    "Left": "goCharLeft", "Right": "goCharRight", "Up": "goLineUp", "Down": "goLineDown",
    "End": "goLineEnd", "Home": "goLineStartSmart", "PageUp": "goPageUp", "PageDown": "goPageDown",
    "Delete": "delCharAfter", "Backspace": "delCharBefore", "Shift-Backspace": "delCharBefore",
    "Tab": "defaultTab", "Shift-Tab": "indentAuto",
    "Enter": "newlineAndIndent", "Insert": "toggleOverwrite",
    "Esc": "singleSelection"
  };
  // Note that the save and find-related commands aren't defined by
  // default. User code or addons can define them. Unknown commands
  // are simply ignored.
  keyMap.pcDefault = {
    "Ctrl-A": "selectAll", "Ctrl-D": "deleteLine", "Ctrl-Z": "undo", "Shift-Ctrl-Z": "redo", "Ctrl-Y": "redo",
    "Ctrl-Home": "goDocStart", "Ctrl-End": "goDocEnd", "Ctrl-Up": "goLineUp", "Ctrl-Down": "goLineDown",
    "Ctrl-Left": "goGroupLeft", "Ctrl-Right": "goGroupRight", "Alt-Left": "goLineStart", "Alt-Right": "goLineEnd",
    "Ctrl-Backspace": "delGroupBefore", "Ctrl-Delete": "delGroupAfter", "Ctrl-S": "save", "Ctrl-F": "find",
    "Ctrl-G": "findNext", "Shift-Ctrl-G": "findPrev", "Shift-Ctrl-F": "replace", "Shift-Ctrl-R": "replaceAll",
    "Ctrl-[": "indentLess", "Ctrl-]": "indentMore",
    "Ctrl-U": "undoSelection", "Shift-Ctrl-U": "redoSelection", "Alt-U": "redoSelection",
    "fallthrough": "basic"
  };
  // Very basic readline/emacs-style bindings, which are standard on Mac.
  keyMap.emacsy = {
    "Ctrl-F": "goCharRight", "Ctrl-B": "goCharLeft", "Ctrl-P": "goLineUp", "Ctrl-N": "goLineDown",
    "Alt-F": "goWordRight", "Alt-B": "goWordLeft", "Ctrl-A": "goLineStart", "Ctrl-E": "goLineEnd",
    "Ctrl-V": "goPageDown", "Shift-Ctrl-V": "goPageUp", "Ctrl-D": "delCharAfter", "Ctrl-H": "delCharBefore",
    "Alt-D": "delWordAfter", "Alt-Backspace": "delWordBefore", "Ctrl-K": "killLine", "Ctrl-T": "transposeChars",
    "Ctrl-O": "openLine"
  };
  keyMap.macDefault = {
    "Cmd-A": "selectAll", "Cmd-D": "deleteLine", "Cmd-Z": "undo", "Shift-Cmd-Z": "redo", "Cmd-Y": "redo",
    "Cmd-Home": "goDocStart", "Cmd-Up": "goDocStart", "Cmd-End": "goDocEnd", "Cmd-Down": "goDocEnd", "Alt-Left": "goGroupLeft",
    "Alt-Right": "goGroupRight", "Cmd-Left": "goLineLeft", "Cmd-Right": "goLineRight", "Alt-Backspace": "delGroupBefore",
    "Ctrl-Alt-Backspace": "delGroupAfter", "Alt-Delete": "delGroupAfter", "Cmd-S": "save", "Cmd-F": "find",
    "Cmd-G": "findNext", "Shift-Cmd-G": "findPrev", "Cmd-Alt-F": "replace", "Shift-Cmd-Alt-F": "replaceAll",
    "Cmd-[": "indentLess", "Cmd-]": "indentMore", "Cmd-Backspace": "delWrappedLineLeft", "Cmd-Delete": "delWrappedLineRight",
    "Cmd-U": "undoSelection", "Shift-Cmd-U": "redoSelection", "Ctrl-Up": "goDocStart", "Ctrl-Down": "goDocEnd",
    "fallthrough": ["basic", "emacsy"]
  };
  keyMap["default"] = mac ? keyMap.macDefault : keyMap.pcDefault;

  // KEYMAP DISPATCH

  function normalizeKeyName(name) {
    var parts = name.split(/-(?!$)/);
    name = parts[parts.length - 1];
    var alt, ctrl, shift, cmd;
    for (var i = 0; i < parts.length - 1; i++) {
      var mod = parts[i];
      if (/^(cmd|meta|m)$/i.test(mod)) { cmd = true; }
      else if (/^a(lt)?$/i.test(mod)) { alt = true; }
      else if (/^(c|ctrl|control)$/i.test(mod)) { ctrl = true; }
      else if (/^s(hift)?$/i.test(mod)) { shift = true; }
      else { throw new Error("Unrecognized modifier name: " + mod) }
    }
    if (alt) { name = "Alt-" + name; }
    if (ctrl) { name = "Ctrl-" + name; }
    if (cmd) { name = "Cmd-" + name; }
    if (shift) { name = "Shift-" + name; }
    return name
  }

  // This is a kludge to keep keymaps mostly working as raw objects
  // (backwards compatibility) while at the same time support features
  // like normalization and multi-stroke key bindings. It compiles a
  // new normalized keymap, and then updates the old object to reflect
  // this.
  function normalizeKeyMap(keymap) {
    var copy = {};
    for (var keyname in keymap) { if (keymap.hasOwnProperty(keyname)) {
      var value = keymap[keyname];
      if (/^(name|fallthrough|(de|at)tach)$/.test(keyname)) { continue }
      if (value == "...") { delete keymap[keyname]; continue }

      var keys = map(keyname.split(" "), normalizeKeyName);
      for (var i = 0; i < keys.length; i++) {
        var val = (void 0), name = (void 0);
        if (i == keys.length - 1) {
          name = keys.join(" ");
          val = value;
        } else {
          name = keys.slice(0, i + 1).join(" ");
          val = "...";
        }
        var prev = copy[name];
        if (!prev) { copy[name] = val; }
        else if (prev != val) { throw new Error("Inconsistent bindings for " + name) }
      }
      delete keymap[keyname];
    } }
    for (var prop in copy) { keymap[prop] = copy[prop]; }
    return keymap
  }

  function lookupKey(key, map$$1, handle, context) {
    map$$1 = getKeyMap(map$$1);
    var found = map$$1.call ? map$$1.call(key, context) : map$$1[key];
    if (found === false) { return "nothing" }
    if (found === "...") { return "multi" }
    if (found != null && handle(found)) { return "handled" }

    if (map$$1.fallthrough) {
      if (Object.prototype.toString.call(map$$1.fallthrough) != "[object Array]")
        { return lookupKey(key, map$$1.fallthrough, handle, context) }
      for (var i = 0; i < map$$1.fallthrough.length; i++) {
        var result = lookupKey(key, map$$1.fallthrough[i], handle, context);
        if (result) { return result }
      }
    }
  }

  // Modifier key presses don't count as 'real' key presses for the
  // purpose of keymap fallthrough.
  function isModifierKey(value) {
    var name = typeof value == "string" ? value : keyNames[value.keyCode];
    return name == "Ctrl" || name == "Alt" || name == "Shift" || name == "Mod"
  }

  function addModifierNames(name, event, noShift) {
    var base = name;
    if (event.altKey && base != "Alt") { name = "Alt-" + name; }
    if ((flipCtrlCmd ? event.metaKey : event.ctrlKey) && base != "Ctrl") { name = "Ctrl-" + name; }
    if ((flipCtrlCmd ? event.ctrlKey : event.metaKey) && base != "Cmd") { name = "Cmd-" + name; }
    if (!noShift && event.shiftKey && base != "Shift") { name = "Shift-" + name; }
    return name
  }

  // Look up the name of a key as indicated by an event object.
  function keyName(event, noShift) {
    if (presto && event.keyCode == 34 && event["char"]) { return false }
    var name = keyNames[event.keyCode];
    if (name == null || event.altGraphKey) { return false }
    // Ctrl-ScrollLock has keyCode 3, same as Ctrl-Pause,
    // so we'll use event.code when available (Chrome 48+, FF 38+, Safari 10.1+)
    if (event.keyCode == 3 && event.code) { name = event.code; }
    return addModifierNames(name, event, noShift)
  }

  function getKeyMap(val) {
    return typeof val == "string" ? keyMap[val] : val
  }

  // Helper for deleting text near the selection(s), used to implement
  // backspace, delete, and similar functionality.
  function deleteNearSelection(cm, compute) {
    var ranges = cm.doc.sel.ranges, kill = [];
    // Build up a set of ranges to kill first, merging overlapping
    // ranges.
    for (var i = 0; i < ranges.length; i++) {
      var toKill = compute(ranges[i]);
      while (kill.length && cmp(toKill.from, lst(kill).to) <= 0) {
        var replaced = kill.pop();
        if (cmp(replaced.from, toKill.from) < 0) {
          toKill.from = replaced.from;
          break
        }
      }
      kill.push(toKill);
    }
    // Next, remove those actual ranges.
    runInOp(cm, function () {
      for (var i = kill.length - 1; i >= 0; i--)
        { replaceRange(cm.doc, "", kill[i].from, kill[i].to, "+delete"); }
      ensureCursorVisible(cm);
    });
  }

  function moveCharLogically(line, ch, dir) {
    var target = skipExtendingChars(line.text, ch + dir, dir);
    return target < 0 || target > line.text.length ? null : target
  }

  function moveLogically(line, start, dir) {
    var ch = moveCharLogically(line, start.ch, dir);
    return ch == null ? null : new Pos(start.line, ch, dir < 0 ? "after" : "before")
  }

  function endOfLine(visually, cm, lineObj, lineNo, dir) {
    if (visually) {
      var order = getOrder(lineObj, cm.doc.direction);
      if (order) {
        var part = dir < 0 ? lst(order) : order[0];
        var moveInStorageOrder = (dir < 0) == (part.level == 1);
        var sticky = moveInStorageOrder ? "after" : "before";
        var ch;
        // With a wrapped rtl chunk (possibly spanning multiple bidi parts),
        // it could be that the last bidi part is not on the last visual line,
        // since visual lines contain content order-consecutive chunks.
        // Thus, in rtl, we are looking for the first (content-order) character
        // in the rtl chunk that is on the last line (that is, the same line
        // as the last (content-order) character).
        if (part.level > 0 || cm.doc.direction == "rtl") {
          var prep = prepareMeasureForLine(cm, lineObj);
          ch = dir < 0 ? lineObj.text.length - 1 : 0;
          var targetTop = measureCharPrepared(cm, prep, ch).top;
          ch = findFirst(function (ch) { return measureCharPrepared(cm, prep, ch).top == targetTop; }, (dir < 0) == (part.level == 1) ? part.from : part.to - 1, ch);
          if (sticky == "before") { ch = moveCharLogically(lineObj, ch, 1); }
        } else { ch = dir < 0 ? part.to : part.from; }
        return new Pos(lineNo, ch, sticky)
      }
    }
    return new Pos(lineNo, dir < 0 ? lineObj.text.length : 0, dir < 0 ? "before" : "after")
  }

  function moveVisually(cm, line, start, dir) {
    var bidi = getOrder(line, cm.doc.direction);
    if (!bidi) { return moveLogically(line, start, dir) }
    if (start.ch >= line.text.length) {
      start.ch = line.text.length;
      start.sticky = "before";
    } else if (start.ch <= 0) {
      start.ch = 0;
      start.sticky = "after";
    }
    var partPos = getBidiPartAt(bidi, start.ch, start.sticky), part = bidi[partPos];
    if (cm.doc.direction == "ltr" && part.level % 2 == 0 && (dir > 0 ? part.to > start.ch : part.from < start.ch)) {
      // Case 1: We move within an ltr part in an ltr editor. Even with wrapped lines,
      // nothing interesting happens.
      return moveLogically(line, start, dir)
    }

    var mv = function (pos, dir) { return moveCharLogically(line, pos instanceof Pos ? pos.ch : pos, dir); };
    var prep;
    var getWrappedLineExtent = function (ch) {
      if (!cm.options.lineWrapping) { return {begin: 0, end: line.text.length} }
      prep = prep || prepareMeasureForLine(cm, line);
      return wrappedLineExtentChar(cm, line, prep, ch)
    };
    var wrappedLineExtent = getWrappedLineExtent(start.sticky == "before" ? mv(start, -1) : start.ch);

    if (cm.doc.direction == "rtl" || part.level == 1) {
      var moveInStorageOrder = (part.level == 1) == (dir < 0);
      var ch = mv(start, moveInStorageOrder ? 1 : -1);
      if (ch != null && (!moveInStorageOrder ? ch >= part.from && ch >= wrappedLineExtent.begin : ch <= part.to && ch <= wrappedLineExtent.end)) {
        // Case 2: We move within an rtl part or in an rtl editor on the same visual line
        var sticky = moveInStorageOrder ? "before" : "after";
        return new Pos(start.line, ch, sticky)
      }
    }

    // Case 3: Could not move within this bidi part in this visual line, so leave
    // the current bidi part

    var searchInVisualLine = function (partPos, dir, wrappedLineExtent) {
      var getRes = function (ch, moveInStorageOrder) { return moveInStorageOrder
        ? new Pos(start.line, mv(ch, 1), "before")
        : new Pos(start.line, ch, "after"); };

      for (; partPos >= 0 && partPos < bidi.length; partPos += dir) {
        var part = bidi[partPos];
        var moveInStorageOrder = (dir > 0) == (part.level != 1);
        var ch = moveInStorageOrder ? wrappedLineExtent.begin : mv(wrappedLineExtent.end, -1);
        if (part.from <= ch && ch < part.to) { return getRes(ch, moveInStorageOrder) }
        ch = moveInStorageOrder ? part.from : mv(part.to, -1);
        if (wrappedLineExtent.begin <= ch && ch < wrappedLineExtent.end) { return getRes(ch, moveInStorageOrder) }
      }
    };

    // Case 3a: Look for other bidi parts on the same visual line
    var res = searchInVisualLine(partPos + dir, dir, wrappedLineExtent);
    if (res) { return res }

    // Case 3b: Look for other bidi parts on the next visual line
    var nextCh = dir > 0 ? wrappedLineExtent.end : mv(wrappedLineExtent.begin, -1);
    if (nextCh != null && !(dir > 0 && nextCh == line.text.length)) {
      res = searchInVisualLine(dir > 0 ? 0 : bidi.length - 1, dir, getWrappedLineExtent(nextCh));
      if (res) { return res }
    }

    // Case 4: Nowhere to move
    return null
  }

  // Commands are parameter-less actions that can be performed on an
  // editor, mostly used for keybindings.
  var commands = {
    selectAll: selectAll,
    singleSelection: function (cm) { return cm.setSelection(cm.getCursor("anchor"), cm.getCursor("head"), sel_dontScroll); },
    killLine: function (cm) { return deleteNearSelection(cm, function (range) {
      if (range.empty()) {
        var len = getLine(cm.doc, range.head.line).text.length;
        if (range.head.ch == len && range.head.line < cm.lastLine())
          { return {from: range.head, to: Pos(range.head.line + 1, 0)} }
        else
          { return {from: range.head, to: Pos(range.head.line, len)} }
      } else {
        return {from: range.from(), to: range.to()}
      }
    }); },
    deleteLine: function (cm) { return deleteNearSelection(cm, function (range) { return ({
      from: Pos(range.from().line, 0),
      to: clipPos(cm.doc, Pos(range.to().line + 1, 0))
    }); }); },
    delLineLeft: function (cm) { return deleteNearSelection(cm, function (range) { return ({
      from: Pos(range.from().line, 0), to: range.from()
    }); }); },
    delWrappedLineLeft: function (cm) { return deleteNearSelection(cm, function (range) {
      var top = cm.charCoords(range.head, "div").top + 5;
      var leftPos = cm.coordsChar({left: 0, top: top}, "div");
      return {from: leftPos, to: range.from()}
    }); },
    delWrappedLineRight: function (cm) { return deleteNearSelection(cm, function (range) {
      var top = cm.charCoords(range.head, "div").top + 5;
      var rightPos = cm.coordsChar({left: cm.display.lineDiv.offsetWidth + 100, top: top}, "div");
      return {from: range.from(), to: rightPos }
    }); },
    undo: function (cm) { return cm.undo(); },
    redo: function (cm) { return cm.redo(); },
    undoSelection: function (cm) { return cm.undoSelection(); },
    redoSelection: function (cm) { return cm.redoSelection(); },
    goDocStart: function (cm) { return cm.extendSelection(Pos(cm.firstLine(), 0)); },
    goDocEnd: function (cm) { return cm.extendSelection(Pos(cm.lastLine())); },
    goLineStart: function (cm) { return cm.extendSelectionsBy(function (range) { return lineStart(cm, range.head.line); },
      {origin: "+move", bias: 1}
    ); },
    goLineStartSmart: function (cm) { return cm.extendSelectionsBy(function (range) { return lineStartSmart(cm, range.head); },
      {origin: "+move", bias: 1}
    ); },
    goLineEnd: function (cm) { return cm.extendSelectionsBy(function (range) { return lineEnd(cm, range.head.line); },
      {origin: "+move", bias: -1}
    ); },
    goLineRight: function (cm) { return cm.extendSelectionsBy(function (range) {
      var top = cm.cursorCoords(range.head, "div").top + 5;
      return cm.coordsChar({left: cm.display.lineDiv.offsetWidth + 100, top: top}, "div")
    }, sel_move); },
    goLineLeft: function (cm) { return cm.extendSelectionsBy(function (range) {
      var top = cm.cursorCoords(range.head, "div").top + 5;
      return cm.coordsChar({left: 0, top: top}, "div")
    }, sel_move); },
    goLineLeftSmart: function (cm) { return cm.extendSelectionsBy(function (range) {
      var top = cm.cursorCoords(range.head, "div").top + 5;
      var pos = cm.coordsChar({left: 0, top: top}, "div");
      if (pos.ch < cm.getLine(pos.line).search(/\S/)) { return lineStartSmart(cm, range.head) }
      return pos
    }, sel_move); },
    goLineUp: function (cm) { return cm.moveV(-1, "line"); },
    goLineDown: function (cm) { return cm.moveV(1, "line"); },
    goPageUp: function (cm) { return cm.moveV(-1, "page"); },
    goPageDown: function (cm) { return cm.moveV(1, "page"); },
    goCharLeft: function (cm) { return cm.moveH(-1, "char"); },
    goCharRight: function (cm) { return cm.moveH(1, "char"); },
    goColumnLeft: function (cm) { return cm.moveH(-1, "column"); },
    goColumnRight: function (cm) { return cm.moveH(1, "column"); },
    goWordLeft: function (cm) { return cm.moveH(-1, "word"); },
    goGroupRight: function (cm) { return cm.moveH(1, "group"); },
    goGroupLeft: function (cm) { return cm.moveH(-1, "group"); },
    goWordRight: function (cm) { return cm.moveH(1, "word"); },
    delCharBefore: function (cm) { return cm.deleteH(-1, "char"); },
    delCharAfter: function (cm) { return cm.deleteH(1, "char"); },
    delWordBefore: function (cm) { return cm.deleteH(-1, "word"); },
    delWordAfter: function (cm) { return cm.deleteH(1, "word"); },
    delGroupBefore: function (cm) { return cm.deleteH(-1, "group"); },
    delGroupAfter: function (cm) { return cm.deleteH(1, "group"); },
    indentAuto: function (cm) { return cm.indentSelection("smart"); },
    indentMore: function (cm) { return cm.indentSelection("add"); },
    indentLess: function (cm) { return cm.indentSelection("subtract"); },
    insertTab: function (cm) { return cm.replaceSelection("\t"); },
    insertSoftTab: function (cm) {
      var spaces = [], ranges = cm.listSelections(), tabSize = cm.options.tabSize;
      for (var i = 0; i < ranges.length; i++) {
        var pos = ranges[i].from();
        var col = countColumn(cm.getLine(pos.line), pos.ch, tabSize);
        spaces.push(spaceStr(tabSize - col % tabSize));
      }
      cm.replaceSelections(spaces);
    },
    defaultTab: function (cm) {
      if (cm.somethingSelected()) { cm.indentSelection("add"); }
      else { cm.execCommand("insertTab"); }
    },
    // Swap the two chars left and right of each selection's head.
    // Move cursor behind the two swapped characters afterwards.
    //
    // Doesn't consider line feeds a character.
    // Doesn't scan more than one line above to find a character.
    // Doesn't do anything on an empty line.
    // Doesn't do anything with non-empty selections.
    transposeChars: function (cm) { return runInOp(cm, function () {
      var ranges = cm.listSelections(), newSel = [];
      for (var i = 0; i < ranges.length; i++) {
        if (!ranges[i].empty()) { continue }
        var cur = ranges[i].head, line = getLine(cm.doc, cur.line).text;
        if (line) {
          if (cur.ch == line.length) { cur = new Pos(cur.line, cur.ch - 1); }
          if (cur.ch > 0) {
            cur = new Pos(cur.line, cur.ch + 1);
            cm.replaceRange(line.charAt(cur.ch - 1) + line.charAt(cur.ch - 2),
                            Pos(cur.line, cur.ch - 2), cur, "+transpose");
          } else if (cur.line > cm.doc.first) {
            var prev = getLine(cm.doc, cur.line - 1).text;
            if (prev) {
              cur = new Pos(cur.line, 1);
              cm.replaceRange(line.charAt(0) + cm.doc.lineSeparator() +
                              prev.charAt(prev.length - 1),
                              Pos(cur.line - 1, prev.length - 1), cur, "+transpose");
            }
          }
        }
        newSel.push(new Range(cur, cur));
      }
      cm.setSelections(newSel);
    }); },
    newlineAndIndent: function (cm) { return runInOp(cm, function () {
      var sels = cm.listSelections();
      for (var i = sels.length - 1; i >= 0; i--)
        { cm.replaceRange(cm.doc.lineSeparator(), sels[i].anchor, sels[i].head, "+input"); }
      sels = cm.listSelections();
      for (var i$1 = 0; i$1 < sels.length; i$1++)
        { cm.indentLine(sels[i$1].from().line, null, true); }
      ensureCursorVisible(cm);
    }); },
    openLine: function (cm) { return cm.replaceSelection("\n", "start"); },
    toggleOverwrite: function (cm) { return cm.toggleOverwrite(); }
  };


  function lineStart(cm, lineN) {
    var line = getLine(cm.doc, lineN);
    var visual = visualLine(line);
    if (visual != line) { lineN = lineNo(visual); }
    return endOfLine(true, cm, visual, lineN, 1)
  }
  function lineEnd(cm, lineN) {
    var line = getLine(cm.doc, lineN);
    var visual = visualLineEnd(line);
    if (visual != line) { lineN = lineNo(visual); }
    return endOfLine(true, cm, line, lineN, -1)
  }
  function lineStartSmart(cm, pos) {
    var start = lineStart(cm, pos.line);
    var line = getLine(cm.doc, start.line);
    var order = getOrder(line, cm.doc.direction);
    if (!order || order[0].level == 0) {
      var firstNonWS = Math.max(0, line.text.search(/\S/));
      var inWS = pos.line == start.line && pos.ch <= firstNonWS && pos.ch;
      return Pos(start.line, inWS ? 0 : firstNonWS, start.sticky)
    }
    return start
  }

  // Run a handler that was bound to a key.
  function doHandleBinding(cm, bound, dropShift) {
    if (typeof bound == "string") {
      bound = commands[bound];
      if (!bound) { return false }
    }
    // Ensure previous input has been read, so that the handler sees a
    // consistent view of the document
    cm.display.input.ensurePolled();
    var prevShift = cm.display.shift, done = false;
    try {
      if (cm.isReadOnly()) { cm.state.suppressEdits = true; }
      if (dropShift) { cm.display.shift = false; }
      done = bound(cm) != Pass;
    } finally {
      cm.display.shift = prevShift;
      cm.state.suppressEdits = false;
    }
    return done
  }

  function lookupKeyForEditor(cm, name, handle) {
    for (var i = 0; i < cm.state.keyMaps.length; i++) {
      var result = lookupKey(name, cm.state.keyMaps[i], handle, cm);
      if (result) { return result }
    }
    return (cm.options.extraKeys && lookupKey(name, cm.options.extraKeys, handle, cm))
      || lookupKey(name, cm.options.keyMap, handle, cm)
  }

  // Note that, despite the name, this function is also used to check
  // for bound mouse clicks.

  var stopSeq = new Delayed;

  function dispatchKey(cm, name, e, handle) {
    var seq = cm.state.keySeq;
    if (seq) {
      if (isModifierKey(name)) { return "handled" }
      if (/\'$/.test(name))
        { cm.state.keySeq = null; }
      else
        { stopSeq.set(50, function () {
          if (cm.state.keySeq == seq) {
            cm.state.keySeq = null;
            cm.display.input.reset();
          }
        }); }
      if (dispatchKeyInner(cm, seq + " " + name, e, handle)) { return true }
    }
    return dispatchKeyInner(cm, name, e, handle)
  }

  function dispatchKeyInner(cm, name, e, handle) {
    var result = lookupKeyForEditor(cm, name, handle);

    if (result == "multi")
      { cm.state.keySeq = name; }
    if (result == "handled")
      { signalLater(cm, "keyHandled", cm, name, e); }

    if (result == "handled" || result == "multi") {
      e_preventDefault(e);
      restartBlink(cm);
    }

    return !!result
  }

  // Handle a key from the keydown event.
  function handleKeyBinding(cm, e) {
    var name = keyName(e, true);
    if (!name) { return false }

    if (e.shiftKey && !cm.state.keySeq) {
      // First try to resolve full name (including 'Shift-'). Failing
      // that, see if there is a cursor-motion command (starting with
      // 'go') bound to the keyname without 'Shift-'.
      return dispatchKey(cm, "Shift-" + name, e, function (b) { return doHandleBinding(cm, b, true); })
          || dispatchKey(cm, name, e, function (b) {
               if (typeof b == "string" ? /^go[A-Z]/.test(b) : b.motion)
                 { return doHandleBinding(cm, b) }
             })
    } else {
      return dispatchKey(cm, name, e, function (b) { return doHandleBinding(cm, b); })
    }
  }

  // Handle a key from the keypress event
  function handleCharBinding(cm, e, ch) {
    return dispatchKey(cm, "'" + ch + "'", e, function (b) { return doHandleBinding(cm, b, true); })
  }

  var lastStoppedKey = null;
  function onKeyDown(e) {
    var cm = this;
    cm.curOp.focus = activeElt();
    if (signalDOMEvent(cm, e)) { return }
    // IE does strange things with escape.
    if (ie && ie_version < 11 && e.keyCode == 27) { e.returnValue = false; }
    var code = e.keyCode;
    cm.display.shift = code == 16 || e.shiftKey;
    var handled = handleKeyBinding(cm, e);
    if (presto) {
      lastStoppedKey = handled ? code : null;
      // Opera has no cut event... we try to at least catch the key combo
      if (!handled && code == 88 && !hasCopyEvent && (mac ? e.metaKey : e.ctrlKey))
        { cm.replaceSelection("", null, "cut"); }
    }

    // Turn mouse into crosshair when Alt is held on Mac.
    if (code == 18 && !/\bCodeMirror-crosshair\b/.test(cm.display.lineDiv.className))
      { showCrossHair(cm); }
  }

  function showCrossHair(cm) {
    var lineDiv = cm.display.lineDiv;
    addClass(lineDiv, "CodeMirror-crosshair");

    function up(e) {
      if (e.keyCode == 18 || !e.altKey) {
        rmClass(lineDiv, "CodeMirror-crosshair");
        off(document, "keyup", up);
        off(document, "mouseover", up);
      }
    }
    on(document, "keyup", up);
    on(document, "mouseover", up);
  }

  function onKeyUp(e) {
    if (e.keyCode == 16) { this.doc.sel.shift = false; }
    signalDOMEvent(this, e);
  }

  function onKeyPress(e) {
    var cm = this;
    if (eventInWidget(cm.display, e) || signalDOMEvent(cm, e) || e.ctrlKey && !e.altKey || mac && e.metaKey) { return }
    var keyCode = e.keyCode, charCode = e.charCode;
    if (presto && keyCode == lastStoppedKey) {lastStoppedKey = null; e_preventDefault(e); return}
    if ((presto && (!e.which || e.which < 10)) && handleKeyBinding(cm, e)) { return }
    var ch = String.fromCharCode(charCode == null ? keyCode : charCode);
    // Some browsers fire keypress events for backspace
    if (ch == "\x08") { return }
    if (handleCharBinding(cm, e, ch)) { return }
    cm.display.input.onKeyPress(e);
  }

  var DOUBLECLICK_DELAY = 400;

  var PastClick = function(time, pos, button) {
    this.time = time;
    this.pos = pos;
    this.button = button;
  };

  PastClick.prototype.compare = function (time, pos, button) {
    return this.time + DOUBLECLICK_DELAY > time &&
      cmp(pos, this.pos) == 0 && button == this.button
  };

  var lastClick, lastDoubleClick;
  function clickRepeat(pos, button) {
    var now = +new Date;
    if (lastDoubleClick && lastDoubleClick.compare(now, pos, button)) {
      lastClick = lastDoubleClick = null;
      return "triple"
    } else if (lastClick && lastClick.compare(now, pos, button)) {
      lastDoubleClick = new PastClick(now, pos, button);
      lastClick = null;
      return "double"
    } else {
      lastClick = new PastClick(now, pos, button);
      lastDoubleClick = null;
      return "single"
    }
  }

  // A mouse down can be a single click, double click, triple click,
  // start of selection drag, start of text drag, new cursor
  // (ctrl-click), rectangle drag (alt-drag), or xwin
  // middle-click-paste. Or it might be a click on something we should
  // not interfere with, such as a scrollbar or widget.
  function onMouseDown(e) {
    var cm = this, display = cm.display;
    if (signalDOMEvent(cm, e) || display.activeTouch && display.input.supportsTouch()) { return }
    display.input.ensurePolled();
    display.shift = e.shiftKey;

    if (eventInWidget(display, e)) {
      if (!webkit) {
        // Briefly turn off draggability, to allow widgets to do
        // normal dragging things.
        display.scroller.draggable = false;
        setTimeout(function () { return display.scroller.draggable = true; }, 100);
      }
      return
    }
    if (clickInGutter(cm, e)) { return }
    var pos = posFromMouse(cm, e), button = e_button(e), repeat = pos ? clickRepeat(pos, button) : "single";
    window.focus();

    // #3261: make sure, that we're not starting a second selection
    if (button == 1 && cm.state.selectingText)
      { cm.state.selectingText(e); }

    if (pos && handleMappedButton(cm, button, pos, repeat, e)) { return }

    if (button == 1) {
      if (pos) { leftButtonDown(cm, pos, repeat, e); }
      else if (e_target(e) == display.scroller) { e_preventDefault(e); }
    } else if (button == 2) {
      if (pos) { extendSelection(cm.doc, pos); }
      setTimeout(function () { return display.input.focus(); }, 20);
    } else if (button == 3) {
      if (captureRightClick) { cm.display.input.onContextMenu(e); }
      else { delayBlurEvent(cm); }
    }
  }

  function handleMappedButton(cm, button, pos, repeat, event) {
    var name = "Click";
    if (repeat == "double") { name = "Double" + name; }
    else if (repeat == "triple") { name = "Triple" + name; }
    name = (button == 1 ? "Left" : button == 2 ? "Middle" : "Right") + name;

    return dispatchKey(cm,  addModifierNames(name, event), event, function (bound) {
      if (typeof bound == "string") { bound = commands[bound]; }
      if (!bound) { return false }
      var done = false;
      try {
        if (cm.isReadOnly()) { cm.state.suppressEdits = true; }
        done = bound(cm, pos) != Pass;
      } finally {
        cm.state.suppressEdits = false;
      }
      return done
    })
  }

  function configureMouse(cm, repeat, event) {
    var option = cm.getOption("configureMouse");
    var value = option ? option(cm, repeat, event) : {};
    if (value.unit == null) {
      var rect = chromeOS ? event.shiftKey && event.metaKey : event.altKey;
      value.unit = rect ? "rectangle" : repeat == "single" ? "char" : repeat == "double" ? "word" : "line";
    }
    if (value.extend == null || cm.doc.extend) { value.extend = cm.doc.extend || event.shiftKey; }
    if (value.addNew == null) { value.addNew = mac ? event.metaKey : event.ctrlKey; }
    if (value.moveOnDrag == null) { value.moveOnDrag = !(mac ? event.altKey : event.ctrlKey); }
    return value
  }

  function leftButtonDown(cm, pos, repeat, event) {
    if (ie) { setTimeout(bind(ensureFocus, cm), 0); }
    else { cm.curOp.focus = activeElt(); }

    var behavior = configureMouse(cm, repeat, event);

    var sel = cm.doc.sel, contained;
    if (cm.options.dragDrop && dragAndDrop && !cm.isReadOnly() &&
        repeat == "single" && (contained = sel.contains(pos)) > -1 &&
        (cmp((contained = sel.ranges[contained]).from(), pos) < 0 || pos.xRel > 0) &&
        (cmp(contained.to(), pos) > 0 || pos.xRel < 0))
      { leftButtonStartDrag(cm, event, pos, behavior); }
    else
      { leftButtonSelect(cm, event, pos, behavior); }
  }

  // Start a text drag. When it ends, see if any dragging actually
  // happen, and treat as a click if it didn't.
  function leftButtonStartDrag(cm, event, pos, behavior) {
    var display = cm.display, moved = false;
    var dragEnd = operation(cm, function (e) {
      if (webkit) { display.scroller.draggable = false; }
      cm.state.draggingText = false;
      off(display.wrapper.ownerDocument, "mouseup", dragEnd);
      off(display.wrapper.ownerDocument, "mousemove", mouseMove);
      off(display.scroller, "dragstart", dragStart);
      off(display.scroller, "drop", dragEnd);
      if (!moved) {
        e_preventDefault(e);
        if (!behavior.addNew)
          { extendSelection(cm.doc, pos, null, null, behavior.extend); }
        // Work around unexplainable focus problem in IE9 (#2127) and Chrome (#3081)
        if (webkit || ie && ie_version == 9)
          { setTimeout(function () {display.wrapper.ownerDocument.body.focus(); display.input.focus();}, 20); }
        else
          { display.input.focus(); }
      }
    });
    var mouseMove = function(e2) {
      moved = moved || Math.abs(event.clientX - e2.clientX) + Math.abs(event.clientY - e2.clientY) >= 10;
    };
    var dragStart = function () { return moved = true; };
    // Let the drag handler handle this.
    if (webkit) { display.scroller.draggable = true; }
    cm.state.draggingText = dragEnd;
    dragEnd.copy = !behavior.moveOnDrag;
    // IE's approach to draggable
    if (display.scroller.dragDrop) { display.scroller.dragDrop(); }
    on(display.wrapper.ownerDocument, "mouseup", dragEnd);
    on(display.wrapper.ownerDocument, "mousemove", mouseMove);
    on(display.scroller, "dragstart", dragStart);
    on(display.scroller, "drop", dragEnd);

    delayBlurEvent(cm);
    setTimeout(function () { return display.input.focus(); }, 20);
  }

  function rangeForUnit(cm, pos, unit) {
    if (unit == "char") { return new Range(pos, pos) }
    if (unit == "word") { return cm.findWordAt(pos) }
    if (unit == "line") { return new Range(Pos(pos.line, 0), clipPos(cm.doc, Pos(pos.line + 1, 0))) }
    var result = unit(cm, pos);
    return new Range(result.from, result.to)
  }

  // Normal selection, as opposed to text dragging.
  function leftButtonSelect(cm, event, start, behavior) {
    var display = cm.display, doc = cm.doc;
    e_preventDefault(event);

    var ourRange, ourIndex, startSel = doc.sel, ranges = startSel.ranges;
    if (behavior.addNew && !behavior.extend) {
      ourIndex = doc.sel.contains(start);
      if (ourIndex > -1)
        { ourRange = ranges[ourIndex]; }
      else
        { ourRange = new Range(start, start); }
    } else {
      ourRange = doc.sel.primary();
      ourIndex = doc.sel.primIndex;
    }

    if (behavior.unit == "rectangle") {
      if (!behavior.addNew) { ourRange = new Range(start, start); }
      start = posFromMouse(cm, event, true, true);
      ourIndex = -1;
    } else {
      var range$$1 = rangeForUnit(cm, start, behavior.unit);
      if (behavior.extend)
        { ourRange = extendRange(ourRange, range$$1.anchor, range$$1.head, behavior.extend); }
      else
        { ourRange = range$$1; }
    }

    if (!behavior.addNew) {
      ourIndex = 0;
      setSelection(doc, new Selection([ourRange], 0), sel_mouse);
      startSel = doc.sel;
    } else if (ourIndex == -1) {
      ourIndex = ranges.length;
      setSelection(doc, normalizeSelection(cm, ranges.concat([ourRange]), ourIndex),
                   {scroll: false, origin: "*mouse"});
    } else if (ranges.length > 1 && ranges[ourIndex].empty() && behavior.unit == "char" && !behavior.extend) {
      setSelection(doc, normalizeSelection(cm, ranges.slice(0, ourIndex).concat(ranges.slice(ourIndex + 1)), 0),
                   {scroll: false, origin: "*mouse"});
      startSel = doc.sel;
    } else {
      replaceOneSelection(doc, ourIndex, ourRange, sel_mouse);
    }

    var lastPos = start;
    function extendTo(pos) {
      if (cmp(lastPos, pos) == 0) { return }
      lastPos = pos;

      if (behavior.unit == "rectangle") {
        var ranges = [], tabSize = cm.options.tabSize;
        var startCol = countColumn(getLine(doc, start.line).text, start.ch, tabSize);
        var posCol = countColumn(getLine(doc, pos.line).text, pos.ch, tabSize);
        var left = Math.min(startCol, posCol), right = Math.max(startCol, posCol);
        for (var line = Math.min(start.line, pos.line), end = Math.min(cm.lastLine(), Math.max(start.line, pos.line));
             line <= end; line++) {
          var text = getLine(doc, line).text, leftPos = findColumn(text, left, tabSize);
          if (left == right)
            { ranges.push(new Range(Pos(line, leftPos), Pos(line, leftPos))); }
          else if (text.length > leftPos)
            { ranges.push(new Range(Pos(line, leftPos), Pos(line, findColumn(text, right, tabSize)))); }
        }
        if (!ranges.length) { ranges.push(new Range(start, start)); }
        setSelection(doc, normalizeSelection(cm, startSel.ranges.slice(0, ourIndex).concat(ranges), ourIndex),
                     {origin: "*mouse", scroll: false});
        cm.scrollIntoView(pos);
      } else {
        var oldRange = ourRange;
        var range$$1 = rangeForUnit(cm, pos, behavior.unit);
        var anchor = oldRange.anchor, head;
        if (cmp(range$$1.anchor, anchor) > 0) {
          head = range$$1.head;
          anchor = minPos(oldRange.from(), range$$1.anchor);
        } else {
          head = range$$1.anchor;
          anchor = maxPos(oldRange.to(), range$$1.head);
        }
        var ranges$1 = startSel.ranges.slice(0);
        ranges$1[ourIndex] = bidiSimplify(cm, new Range(clipPos(doc, anchor), head));
        setSelection(doc, normalizeSelection(cm, ranges$1, ourIndex), sel_mouse);
      }
    }

    var editorSize = display.wrapper.getBoundingClientRect();
    // Used to ensure timeout re-tries don't fire when another extend
    // happened in the meantime (clearTimeout isn't reliable -- at
    // least on Chrome, the timeouts still happen even when cleared,
    // if the clear happens after their scheduled firing time).
    var counter = 0;

    function extend(e) {
      var curCount = ++counter;
      var cur = posFromMouse(cm, e, true, behavior.unit == "rectangle");
      if (!cur) { return }
      if (cmp(cur, lastPos) != 0) {
        cm.curOp.focus = activeElt();
        extendTo(cur);
        var visible = visibleLines(display, doc);
        if (cur.line >= visible.to || cur.line < visible.from)
          { setTimeout(operation(cm, function () {if (counter == curCount) { extend(e); }}), 150); }
      } else {
        var outside = e.clientY < editorSize.top ? -20 : e.clientY > editorSize.bottom ? 20 : 0;
        if (outside) { setTimeout(operation(cm, function () {
          if (counter != curCount) { return }
          display.scroller.scrollTop += outside;
          extend(e);
        }), 50); }
      }
    }

    function done(e) {
      cm.state.selectingText = false;
      counter = Infinity;
      // If e is null or undefined we interpret this as someone trying
      // to explicitly cancel the selection rather than the user
      // letting go of the mouse button.
      if (e) {
        e_preventDefault(e);
        display.input.focus();
      }
      off(display.wrapper.ownerDocument, "mousemove", move);
      off(display.wrapper.ownerDocument, "mouseup", up);
      doc.history.lastSelOrigin = null;
    }

    var move = operation(cm, function (e) {
      if (e.buttons === 0 || !e_button(e)) { done(e); }
      else { extend(e); }
    });
    var up = operation(cm, done);
    cm.state.selectingText = up;
    on(display.wrapper.ownerDocument, "mousemove", move);
    on(display.wrapper.ownerDocument, "mouseup", up);
  }

  // Used when mouse-selecting to adjust the anchor to the proper side
  // of a bidi jump depending on the visual position of the head.
  function bidiSimplify(cm, range$$1) {
    var anchor = range$$1.anchor;
    var head = range$$1.head;
    var anchorLine = getLine(cm.doc, anchor.line);
    if (cmp(anchor, head) == 0 && anchor.sticky == head.sticky) { return range$$1 }
    var order = getOrder(anchorLine);
    if (!order) { return range$$1 }
    var index = getBidiPartAt(order, anchor.ch, anchor.sticky), part = order[index];
    if (part.from != anchor.ch && part.to != anchor.ch) { return range$$1 }
    var boundary = index + ((part.from == anchor.ch) == (part.level != 1) ? 0 : 1);
    if (boundary == 0 || boundary == order.length) { return range$$1 }

    // Compute the relative visual position of the head compared to the
    // anchor (<0 is to the left, >0 to the right)
    var leftSide;
    if (head.line != anchor.line) {
      leftSide = (head.line - anchor.line) * (cm.doc.direction == "ltr" ? 1 : -1) > 0;
    } else {
      var headIndex = getBidiPartAt(order, head.ch, head.sticky);
      var dir = headIndex - index || (head.ch - anchor.ch) * (part.level == 1 ? -1 : 1);
      if (headIndex == boundary - 1 || headIndex == boundary)
        { leftSide = dir < 0; }
      else
        { leftSide = dir > 0; }
    }

    var usePart = order[boundary + (leftSide ? -1 : 0)];
    var from = leftSide == (usePart.level == 1);
    var ch = from ? usePart.from : usePart.to, sticky = from ? "after" : "before";
    return anchor.ch == ch && anchor.sticky == sticky ? range$$1 : new Range(new Pos(anchor.line, ch, sticky), head)
  }


  // Determines whether an event happened in the gutter, and fires the
  // handlers for the corresponding event.
  function gutterEvent(cm, e, type, prevent) {
    var mX, mY;
    if (e.touches) {
      mX = e.touches[0].clientX;
      mY = e.touches[0].clientY;
    } else {
      try { mX = e.clientX; mY = e.clientY; }
      catch(e) { return false }
    }
    if (mX >= Math.floor(cm.display.gutters.getBoundingClientRect().right)) { return false }
    if (prevent) { e_preventDefault(e); }

    var display = cm.display;
    var lineBox = display.lineDiv.getBoundingClientRect();

    if (mY > lineBox.bottom || !hasHandler(cm, type)) { return e_defaultPrevented(e) }
    mY -= lineBox.top - display.viewOffset;

    for (var i = 0; i < cm.display.gutterSpecs.length; ++i) {
      var g = display.gutters.childNodes[i];
      if (g && g.getBoundingClientRect().right >= mX) {
        var line = lineAtHeight(cm.doc, mY);
        var gutter = cm.display.gutterSpecs[i];
        signal(cm, type, cm, line, gutter.className, e);
        return e_defaultPrevented(e)
      }
    }
  }

  function clickInGutter(cm, e) {
    return gutterEvent(cm, e, "gutterClick", true)
  }

  // CONTEXT MENU HANDLING

  // To make the context menu work, we need to briefly unhide the
  // textarea (making it as unobtrusive as possible) to let the
  // right-click take effect on it.
  function onContextMenu(cm, e) {
    if (eventInWidget(cm.display, e) || contextMenuInGutter(cm, e)) { return }
    if (signalDOMEvent(cm, e, "contextmenu")) { return }
    if (!captureRightClick) { cm.display.input.onContextMenu(e); }
  }

  function contextMenuInGutter(cm, e) {
    if (!hasHandler(cm, "gutterContextMenu")) { return false }
    return gutterEvent(cm, e, "gutterContextMenu", false)
  }

  function themeChanged(cm) {
    cm.display.wrapper.className = cm.display.wrapper.className.replace(/\s*cm-s-\S+/g, "") +
      cm.options.theme.replace(/(^|\s)\s*/g, " cm-s-");
    clearCaches(cm);
  }

  var Init = {toString: function(){return "CodeMirror.Init"}};

  var defaults = {};
  var optionHandlers = {};

  function defineOptions(CodeMirror) {
    var optionHandlers = CodeMirror.optionHandlers;

    function option(name, deflt, handle, notOnInit) {
      CodeMirror.defaults[name] = deflt;
      if (handle) { optionHandlers[name] =
        notOnInit ? function (cm, val, old) {if (old != Init) { handle(cm, val, old); }} : handle; }
    }

    CodeMirror.defineOption = option;

    // Passed to option handlers when there is no old value.
    CodeMirror.Init = Init;

    // These two are, on init, called from the constructor because they
    // have to be initialized before the editor can start at all.
    option("value", "", function (cm, val) { return cm.setValue(val); }, true);
    option("mode", null, function (cm, val) {
      cm.doc.modeOption = val;
      loadMode(cm);
    }, true);

    option("indentUnit", 2, loadMode, true);
    option("indentWithTabs", false);
    option("smartIndent", true);
    option("tabSize", 4, function (cm) {
      resetModeState(cm);
      clearCaches(cm);
      regChange(cm);
    }, true);

    option("lineSeparator", null, function (cm, val) {
      cm.doc.lineSep = val;
      if (!val) { return }
      var newBreaks = [], lineNo = cm.doc.first;
      cm.doc.iter(function (line) {
        for (var pos = 0;;) {
          var found = line.text.indexOf(val, pos);
          if (found == -1) { break }
          pos = found + val.length;
          newBreaks.push(Pos(lineNo, found));
        }
        lineNo++;
      });
      for (var i = newBreaks.length - 1; i >= 0; i--)
        { replaceRange(cm.doc, val, newBreaks[i], Pos(newBreaks[i].line, newBreaks[i].ch + val.length)); }
    });
    option("specialChars", /[\u0000-\u001f\u007f-\u009f\u00ad\u061c\u200b-\u200f\u2028\u2029\ufeff\ufff9-\ufffc]/g, function (cm, val, old) {
      cm.state.specialChars = new RegExp(val.source + (val.test("\t") ? "" : "|\t"), "g");
      if (old != Init) { cm.refresh(); }
    });
    option("specialCharPlaceholder", defaultSpecialCharPlaceholder, function (cm) { return cm.refresh(); }, true);
    option("electricChars", true);
    option("inputStyle", mobile ? "contenteditable" : "textarea", function () {
      throw new Error("inputStyle can not (yet) be changed in a running editor") // FIXME
    }, true);
    option("spellcheck", false, function (cm, val) { return cm.getInputField().spellcheck = val; }, true);
    option("autocorrect", false, function (cm, val) { return cm.getInputField().autocorrect = val; }, true);
    option("autocapitalize", false, function (cm, val) { return cm.getInputField().autocapitalize = val; }, true);
    option("rtlMoveVisually", !windows);
    option("wholeLineUpdateBefore", true);

    option("theme", "default", function (cm) {
      themeChanged(cm);
      updateGutters(cm);
    }, true);
    option("keyMap", "default", function (cm, val, old) {
      var next = getKeyMap(val);
      var prev = old != Init && getKeyMap(old);
      if (prev && prev.detach) { prev.detach(cm, next); }
      if (next.attach) { next.attach(cm, prev || null); }
    });
    option("extraKeys", null);
    option("configureMouse", null);

    option("lineWrapping", false, wrappingChanged, true);
    option("gutters", [], function (cm, val) {
      cm.display.gutterSpecs = getGutters(val, cm.options.lineNumbers);
      updateGutters(cm);
    }, true);
    option("fixedGutter", true, function (cm, val) {
      cm.display.gutters.style.left = val ? compensateForHScroll(cm.display) + "px" : "0";
      cm.refresh();
    }, true);
    option("coverGutterNextToScrollbar", false, function (cm) { return updateScrollbars(cm); }, true);
    option("scrollbarStyle", "native", function (cm) {
      initScrollbars(cm);
      updateScrollbars(cm);
      cm.display.scrollbars.setScrollTop(cm.doc.scrollTop);
      cm.display.scrollbars.setScrollLeft(cm.doc.scrollLeft);
    }, true);
    option("lineNumbers", false, function (cm, val) {
      cm.display.gutterSpecs = getGutters(cm.options.gutters, val);
      updateGutters(cm);
    }, true);
    option("firstLineNumber", 1, updateGutters, true);
    option("lineNumberFormatter", function (integer) { return integer; }, updateGutters, true);
    option("showCursorWhenSelecting", false, updateSelection, true);

    option("resetSelectionOnContextMenu", true);
    option("lineWiseCopyCut", true);
    option("pasteLinesPerSelection", true);
    option("selectionsMayTouch", false);

    option("readOnly", false, function (cm, val) {
      if (val == "nocursor") {
        onBlur(cm);
        cm.display.input.blur();
      }
      cm.display.input.readOnlyChanged(val);
    });
    option("disableInput", false, function (cm, val) {if (!val) { cm.display.input.reset(); }}, true);
    option("dragDrop", true, dragDropChanged);
    option("allowDropFileTypes", null);

    option("cursorBlinkRate", 530);
    option("cursorScrollMargin", 0);
    option("cursorHeight", 1, updateSelection, true);
    option("singleCursorHeightPerLine", true, updateSelection, true);
    option("workTime", 100);
    option("workDelay", 100);
    option("flattenSpans", true, resetModeState, true);
    option("addModeClass", false, resetModeState, true);
    option("pollInterval", 100);
    option("undoDepth", 200, function (cm, val) { return cm.doc.history.undoDepth = val; });
    option("historyEventDelay", 1250);
    option("viewportMargin", 10, function (cm) { return cm.refresh(); }, true);
    option("maxHighlightLength", 10000, resetModeState, true);
    option("moveInputWithCursor", true, function (cm, val) {
      if (!val) { cm.display.input.resetPosition(); }
    });

    option("tabindex", null, function (cm, val) { return cm.display.input.getField().tabIndex = val || ""; });
    option("autofocus", null);
    option("direction", "ltr", function (cm, val) { return cm.doc.setDirection(val); }, true);
    option("phrases", null);
  }

  function dragDropChanged(cm, value, old) {
    var wasOn = old && old != Init;
    if (!value != !wasOn) {
      var funcs = cm.display.dragFunctions;
      var toggle = value ? on : off;
      toggle(cm.display.scroller, "dragstart", funcs.start);
      toggle(cm.display.scroller, "dragenter", funcs.enter);
      toggle(cm.display.scroller, "dragover", funcs.over);
      toggle(cm.display.scroller, "dragleave", funcs.leave);
      toggle(cm.display.scroller, "drop", funcs.drop);
    }
  }

  function wrappingChanged(cm) {
    if (cm.options.lineWrapping) {
      addClass(cm.display.wrapper, "CodeMirror-wrap");
      cm.display.sizer.style.minWidth = "";
      cm.display.sizerWidth = null;
    } else {
      rmClass(cm.display.wrapper, "CodeMirror-wrap");
      findMaxLine(cm);
    }
    estimateLineHeights(cm);
    regChange(cm);
    clearCaches(cm);
    setTimeout(function () { return updateScrollbars(cm); }, 100);
  }

  // A CodeMirror instance represents an editor. This is the object
  // that user code is usually dealing with.

  function CodeMirror(place, options) {
    var this$1 = this;

    if (!(this instanceof CodeMirror)) { return new CodeMirror(place, options) }

    this.options = options = options ? copyObj(options) : {};
    // Determine effective options based on given values and defaults.
    copyObj(defaults, options, false);

    var doc = options.value;
    if (typeof doc == "string") { doc = new Doc(doc, options.mode, null, options.lineSeparator, options.direction); }
    else if (options.mode) { doc.modeOption = options.mode; }
    this.doc = doc;

    var input = new CodeMirror.inputStyles[options.inputStyle](this);
    var display = this.display = new Display(place, doc, input, options);
    display.wrapper.CodeMirror = this;
    themeChanged(this);
    if (options.lineWrapping)
      { this.display.wrapper.className += " CodeMirror-wrap"; }
    initScrollbars(this);

    this.state = {
      keyMaps: [],  // stores maps added by addKeyMap
      overlays: [], // highlighting overlays, as added by addOverlay
      modeGen: 0,   // bumped when mode/overlay changes, used to invalidate highlighting info
      overwrite: false,
      delayingBlurEvent: false,
      focused: false,
      suppressEdits: false, // used to disable editing during key handlers when in readOnly mode
      pasteIncoming: -1, cutIncoming: -1, // help recognize paste/cut edits in input.poll
      selectingText: false,
      draggingText: false,
      highlight: new Delayed(), // stores highlight worker timeout
      keySeq: null,  // Unfinished key sequence
      specialChars: null
    };

    if (options.autofocus && !mobile) { display.input.focus(); }

    // Override magic textarea content restore that IE sometimes does
    // on our hidden textarea on reload
    if (ie && ie_version < 11) { setTimeout(function () { return this$1.display.input.reset(true); }, 20); }

    registerEventHandlers(this);
    ensureGlobalHandlers();

    startOperation(this);
    this.curOp.forceUpdate = true;
    attachDoc(this, doc);

    if ((options.autofocus && !mobile) || this.hasFocus())
      { setTimeout(bind(onFocus, this), 20); }
    else
      { onBlur(this); }

    for (var opt in optionHandlers) { if (optionHandlers.hasOwnProperty(opt))
      { optionHandlers[opt](this$1, options[opt], Init); } }
    maybeUpdateLineNumberWidth(this);
    if (options.finishInit) { options.finishInit(this); }
    for (var i = 0; i < initHooks.length; ++i) { initHooks[i](this$1); }
    endOperation(this);
    // Suppress optimizelegibility in Webkit, since it breaks text
    // measuring on line wrapping boundaries.
    if (webkit && options.lineWrapping &&
        getComputedStyle(display.lineDiv).textRendering == "optimizelegibility")
      { display.lineDiv.style.textRendering = "auto"; }
  }

  // The default configuration options.
  CodeMirror.defaults = defaults;
  // Functions to run when options are changed.
  CodeMirror.optionHandlers = optionHandlers;

  // Attach the necessary event handlers when initializing the editor
  function registerEventHandlers(cm) {
    var d = cm.display;
    on(d.scroller, "mousedown", operation(cm, onMouseDown));
    // Older IE's will not fire a second mousedown for a double click
    if (ie && ie_version < 11)
      { on(d.scroller, "dblclick", operation(cm, function (e) {
        if (signalDOMEvent(cm, e)) { return }
        var pos = posFromMouse(cm, e);
        if (!pos || clickInGutter(cm, e) || eventInWidget(cm.display, e)) { return }
        e_preventDefault(e);
        var word = cm.findWordAt(pos);
        extendSelection(cm.doc, word.anchor, word.head);
      })); }
    else
      { on(d.scroller, "dblclick", function (e) { return signalDOMEvent(cm, e) || e_preventDefault(e); }); }
    // Some browsers fire contextmenu *after* opening the menu, at
    // which point we can't mess with it anymore. Context menu is
    // handled in onMouseDown for these browsers.
    on(d.scroller, "contextmenu", function (e) { return onContextMenu(cm, e); });

    // Used to suppress mouse event handling when a touch happens
    var touchFinished, prevTouch = {end: 0};
    function finishTouch() {
      if (d.activeTouch) {
        touchFinished = setTimeout(function () { return d.activeTouch = null; }, 1000);
        prevTouch = d.activeTouch;
        prevTouch.end = +new Date;
      }
    }
    function isMouseLikeTouchEvent(e) {
      if (e.touches.length != 1) { return false }
      var touch = e.touches[0];
      return touch.radiusX <= 1 && touch.radiusY <= 1
    }
    function farAway(touch, other) {
      if (other.left == null) { return true }
      var dx = other.left - touch.left, dy = other.top - touch.top;
      return dx * dx + dy * dy > 20 * 20
    }
    on(d.scroller, "touchstart", function (e) {
      if (!signalDOMEvent(cm, e) && !isMouseLikeTouchEvent(e) && !clickInGutter(cm, e)) {
        d.input.ensurePolled();
        clearTimeout(touchFinished);
        var now = +new Date;
        d.activeTouch = {start: now, moved: false,
                         prev: now - prevTouch.end <= 300 ? prevTouch : null};
        if (e.touches.length == 1) {
          d.activeTouch.left = e.touches[0].pageX;
          d.activeTouch.top = e.touches[0].pageY;
        }
      }
    });
    on(d.scroller, "touchmove", function () {
      if (d.activeTouch) { d.activeTouch.moved = true; }
    });
    on(d.scroller, "touchend", function (e) {
      var touch = d.activeTouch;
      if (touch && !eventInWidget(d, e) && touch.left != null &&
          !touch.moved && new Date - touch.start < 300) {
        var pos = cm.coordsChar(d.activeTouch, "page"), range;
        if (!touch.prev || farAway(touch, touch.prev)) // Single tap
          { range = new Range(pos, pos); }
        else if (!touch.prev.prev || farAway(touch, touch.prev.prev)) // Double tap
          { range = cm.findWordAt(pos); }
        else // Triple tap
          { range = new Range(Pos(pos.line, 0), clipPos(cm.doc, Pos(pos.line + 1, 0))); }
        cm.setSelection(range.anchor, range.head);
        cm.focus();
        e_preventDefault(e);
      }
      finishTouch();
    });
    on(d.scroller, "touchcancel", finishTouch);

    // Sync scrolling between fake scrollbars and real scrollable
    // area, ensure viewport is updated when scrolling.
    on(d.scroller, "scroll", function () {
      if (d.scroller.clientHeight) {
        updateScrollTop(cm, d.scroller.scrollTop);
        setScrollLeft(cm, d.scroller.scrollLeft, true);
        signal(cm, "scroll", cm);
      }
    });

    // Listen to wheel events in order to try and update the viewport on time.
    on(d.scroller, "mousewheel", function (e) { return onScrollWheel(cm, e); });
    on(d.scroller, "DOMMouseScroll", function (e) { return onScrollWheel(cm, e); });

    // Prevent wrapper from ever scrolling
    on(d.wrapper, "scroll", function () { return d.wrapper.scrollTop = d.wrapper.scrollLeft = 0; });

    d.dragFunctions = {
      enter: function (e) {if (!signalDOMEvent(cm, e)) { e_stop(e); }},
      over: function (e) {if (!signalDOMEvent(cm, e)) { onDragOver(cm, e); e_stop(e); }},
      start: function (e) { return onDragStart(cm, e); },
      drop: operation(cm, onDrop),
      leave: function (e) {if (!signalDOMEvent(cm, e)) { clearDragCursor(cm); }}
    };

    var inp = d.input.getField();
    on(inp, "keyup", function (e) { return onKeyUp.call(cm, e); });
    on(inp, "keydown", operation(cm, onKeyDown));
    on(inp, "keypress", operation(cm, onKeyPress));
    on(inp, "focus", function (e) { return onFocus(cm, e); });
    on(inp, "blur", function (e) { return onBlur(cm, e); });
  }

  var initHooks = [];
  CodeMirror.defineInitHook = function (f) { return initHooks.push(f); };

  // Indent the given line. The how parameter can be "smart",
  // "add"/null, "subtract", or "prev". When aggressive is false
  // (typically set to true for forced single-line indents), empty
  // lines are not indented, and places where the mode returns Pass
  // are left alone.
  function indentLine(cm, n, how, aggressive) {
    var doc = cm.doc, state;
    if (how == null) { how = "add"; }
    if (how == "smart") {
      // Fall back to "prev" when the mode doesn't have an indentation
      // method.
      if (!doc.mode.indent) { how = "prev"; }
      else { state = getContextBefore(cm, n).state; }
    }

    var tabSize = cm.options.tabSize;
    var line = getLine(doc, n), curSpace = countColumn(line.text, null, tabSize);
    if (line.stateAfter) { line.stateAfter = null; }
    var curSpaceString = line.text.match(/^\s*/)[0], indentation;
    if (!aggressive && !/\S/.test(line.text)) {
      indentation = 0;
      how = "not";
    } else if (how == "smart") {
      indentation = doc.mode.indent(state, line.text.slice(curSpaceString.length), line.text);
      if (indentation == Pass || indentation > 150) {
        if (!aggressive) { return }
        how = "prev";
      }
    }
    if (how == "prev") {
      if (n > doc.first) { indentation = countColumn(getLine(doc, n-1).text, null, tabSize); }
      else { indentation = 0; }
    } else if (how == "add") {
      indentation = curSpace + cm.options.indentUnit;
    } else if (how == "subtract") {
      indentation = curSpace - cm.options.indentUnit;
    } else if (typeof how == "number") {
      indentation = curSpace + how;
    }
    indentation = Math.max(0, indentation);

    var indentString = "", pos = 0;
    if (cm.options.indentWithTabs)
      { for (var i = Math.floor(indentation / tabSize); i; --i) {pos += tabSize; indentString += "\t";} }
    if (pos < indentation) { indentString += spaceStr(indentation - pos); }

    if (indentString != curSpaceString) {
      replaceRange(doc, indentString, Pos(n, 0), Pos(n, curSpaceString.length), "+input");
      line.stateAfter = null;
      return true
    } else {
      // Ensure that, if the cursor was in the whitespace at the start
      // of the line, it is moved to the end of that space.
      for (var i$1 = 0; i$1 < doc.sel.ranges.length; i$1++) {
        var range = doc.sel.ranges[i$1];
        if (range.head.line == n && range.head.ch < curSpaceString.length) {
          var pos$1 = Pos(n, curSpaceString.length);
          replaceOneSelection(doc, i$1, new Range(pos$1, pos$1));
          break
        }
      }
    }
  }

  // This will be set to a {lineWise: bool, text: [string]} object, so
  // that, when pasting, we know what kind of selections the copied
  // text was made out of.
  var lastCopied = null;

  function setLastCopied(newLastCopied) {
    lastCopied = newLastCopied;
  }

  function applyTextInput(cm, inserted, deleted, sel, origin) {
    var doc = cm.doc;
    cm.display.shift = false;
    if (!sel) { sel = doc.sel; }

    var recent = +new Date - 200;
    var paste = origin == "paste" || cm.state.pasteIncoming > recent;
    var textLines = splitLinesAuto(inserted), multiPaste = null;
    // When pasting N lines into N selections, insert one line per selection
    if (paste && sel.ranges.length > 1) {
      if (lastCopied && lastCopied.text.join("\n") == inserted) {
        if (sel.ranges.length % lastCopied.text.length == 0) {
          multiPaste = [];
          for (var i = 0; i < lastCopied.text.length; i++)
            { multiPaste.push(doc.splitLines(lastCopied.text[i])); }
        }
      } else if (textLines.length == sel.ranges.length && cm.options.pasteLinesPerSelection) {
        multiPaste = map(textLines, function (l) { return [l]; });
      }
    }

    var updateInput = cm.curOp.updateInput;
    // Normal behavior is to insert the new text into every selection
    for (var i$1 = sel.ranges.length - 1; i$1 >= 0; i$1--) {
      var range$$1 = sel.ranges[i$1];
      var from = range$$1.from(), to = range$$1.to();
      if (range$$1.empty()) {
        if (deleted && deleted > 0) // Handle deletion
          { from = Pos(from.line, from.ch - deleted); }
        else if (cm.state.overwrite && !paste) // Handle overwrite
          { to = Pos(to.line, Math.min(getLine(doc, to.line).text.length, to.ch + lst(textLines).length)); }
        else if (paste && lastCopied && lastCopied.lineWise && lastCopied.text.join("\n") == inserted)
          { from = to = Pos(from.line, 0); }
      }
      var changeEvent = {from: from, to: to, text: multiPaste ? multiPaste[i$1 % multiPaste.length] : textLines,
                         origin: origin || (paste ? "paste" : cm.state.cutIncoming > recent ? "cut" : "+input")};
      makeChange(cm.doc, changeEvent);
      signalLater(cm, "inputRead", cm, changeEvent);
    }
    if (inserted && !paste)
      { triggerElectric(cm, inserted); }

    ensureCursorVisible(cm);
    if (cm.curOp.updateInput < 2) { cm.curOp.updateInput = updateInput; }
    cm.curOp.typing = true;
    cm.state.pasteIncoming = cm.state.cutIncoming = -1;
  }

  function handlePaste(e, cm) {
    var pasted = e.clipboardData && e.clipboardData.getData("Text");
    if (pasted) {
      e.preventDefault();
      if (!cm.isReadOnly() && !cm.options.disableInput)
        { runInOp(cm, function () { return applyTextInput(cm, pasted, 0, null, "paste"); }); }
      return true
    }
  }

  function triggerElectric(cm, inserted) {
    // When an 'electric' character is inserted, immediately trigger a reindent
    if (!cm.options.electricChars || !cm.options.smartIndent) { return }
    var sel = cm.doc.sel;

    for (var i = sel.ranges.length - 1; i >= 0; i--) {
      var range$$1 = sel.ranges[i];
      if (range$$1.head.ch > 100 || (i && sel.ranges[i - 1].head.line == range$$1.head.line)) { continue }
      var mode = cm.getModeAt(range$$1.head);
      var indented = false;
      if (mode.electricChars) {
        for (var j = 0; j < mode.electricChars.length; j++)
          { if (inserted.indexOf(mode.electricChars.charAt(j)) > -1) {
            indented = indentLine(cm, range$$1.head.line, "smart");
            break
          } }
      } else if (mode.electricInput) {
        if (mode.electricInput.test(getLine(cm.doc, range$$1.head.line).text.slice(0, range$$1.head.ch)))
          { indented = indentLine(cm, range$$1.head.line, "smart"); }
      }
      if (indented) { signalLater(cm, "electricInput", cm, range$$1.head.line); }
    }
  }

  function copyableRanges(cm) {
    var text = [], ranges = [];
    for (var i = 0; i < cm.doc.sel.ranges.length; i++) {
      var line = cm.doc.sel.ranges[i].head.line;
      var lineRange = {anchor: Pos(line, 0), head: Pos(line + 1, 0)};
      ranges.push(lineRange);
      text.push(cm.getRange(lineRange.anchor, lineRange.head));
    }
    return {text: text, ranges: ranges}
  }

  function disableBrowserMagic(field, spellcheck, autocorrect, autocapitalize) {
    field.setAttribute("autocorrect", autocorrect ? "" : "off");
    field.setAttribute("autocapitalize", autocapitalize ? "" : "off");
    field.setAttribute("spellcheck", !!spellcheck);
  }

  function hiddenTextarea() {
    var te = elt("textarea", null, null, "position: absolute; bottom: -1em; padding: 0; width: 1px; height: 1em; outline: none");
    var div = elt("div", [te], null, "overflow: hidden; position: relative; width: 3px; height: 0px;");
    // The textarea is kept positioned near the cursor to prevent the
    // fact that it'll be scrolled into view on input from scrolling
    // our fake cursor out of view. On webkit, when wrap=off, paste is
    // very slow. So make the area wide instead.
    if (webkit) { te.style.width = "1000px"; }
    else { te.setAttribute("wrap", "off"); }
    // If border: 0; -- iOS fails to open keyboard (issue #1287)
    if (ios) { te.style.border = "1px solid black"; }
    disableBrowserMagic(te);
    return div
  }

  // The publicly visible API. Note that methodOp(f) means
  // 'wrap f in an operation, performed on its `this` parameter'.

  // This is not the complete set of editor methods. Most of the
  // methods defined on the Doc type are also injected into
  // CodeMirror.prototype, for backwards compatibility and
  // convenience.

  function addEditorMethods(CodeMirror) {
    var optionHandlers = CodeMirror.optionHandlers;

    var helpers = CodeMirror.helpers = {};

    CodeMirror.prototype = {
      constructor: CodeMirror,
      focus: function(){window.focus(); this.display.input.focus();},

      setOption: function(option, value) {
        var options = this.options, old = options[option];
        if (options[option] == value && option != "mode") { return }
        options[option] = value;
        if (optionHandlers.hasOwnProperty(option))
          { operation(this, optionHandlers[option])(this, value, old); }
        signal(this, "optionChange", this, option);
      },

      getOption: function(option) {return this.options[option]},
      getDoc: function() {return this.doc},

      addKeyMap: function(map$$1, bottom) {
        this.state.keyMaps[bottom ? "push" : "unshift"](getKeyMap(map$$1));
      },
      removeKeyMap: function(map$$1) {
        var maps = this.state.keyMaps;
        for (var i = 0; i < maps.length; ++i)
          { if (maps[i] == map$$1 || maps[i].name == map$$1) {
            maps.splice(i, 1);
            return true
          } }
      },

      addOverlay: methodOp(function(spec, options) {
        var mode = spec.token ? spec : CodeMirror.getMode(this.options, spec);
        if (mode.startState) { throw new Error("Overlays may not be stateful.") }
        insertSorted(this.state.overlays,
                     {mode: mode, modeSpec: spec, opaque: options && options.opaque,
                      priority: (options && options.priority) || 0},
                     function (overlay) { return overlay.priority; });
        this.state.modeGen++;
        regChange(this);
      }),
      removeOverlay: methodOp(function(spec) {
        var this$1 = this;

        var overlays = this.state.overlays;
        for (var i = 0; i < overlays.length; ++i) {
          var cur = overlays[i].modeSpec;
          if (cur == spec || typeof spec == "string" && cur.name == spec) {
            overlays.splice(i, 1);
            this$1.state.modeGen++;
            regChange(this$1);
            return
          }
        }
      }),

      indentLine: methodOp(function(n, dir, aggressive) {
        if (typeof dir != "string" && typeof dir != "number") {
          if (dir == null) { dir = this.options.smartIndent ? "smart" : "prev"; }
          else { dir = dir ? "add" : "subtract"; }
        }
        if (isLine(this.doc, n)) { indentLine(this, n, dir, aggressive); }
      }),
      indentSelection: methodOp(function(how) {
        var this$1 = this;

        var ranges = this.doc.sel.ranges, end = -1;
        for (var i = 0; i < ranges.length; i++) {
          var range$$1 = ranges[i];
          if (!range$$1.empty()) {
            var from = range$$1.from(), to = range$$1.to();
            var start = Math.max(end, from.line);
            end = Math.min(this$1.lastLine(), to.line - (to.ch ? 0 : 1)) + 1;
            for (var j = start; j < end; ++j)
              { indentLine(this$1, j, how); }
            var newRanges = this$1.doc.sel.ranges;
            if (from.ch == 0 && ranges.length == newRanges.length && newRanges[i].from().ch > 0)
              { replaceOneSelection(this$1.doc, i, new Range(from, newRanges[i].to()), sel_dontScroll); }
          } else if (range$$1.head.line > end) {
            indentLine(this$1, range$$1.head.line, how, true);
            end = range$$1.head.line;
            if (i == this$1.doc.sel.primIndex) { ensureCursorVisible(this$1); }
          }
        }
      }),

      // Fetch the parser token for a given character. Useful for hacks
      // that want to inspect the mode state (say, for completion).
      getTokenAt: function(pos, precise) {
        return takeToken(this, pos, precise)
      },

      getLineTokens: function(line, precise) {
        return takeToken(this, Pos(line), precise, true)
      },

      getTokenTypeAt: function(pos) {
        pos = clipPos(this.doc, pos);
        var styles = getLineStyles(this, getLine(this.doc, pos.line));
        var before = 0, after = (styles.length - 1) / 2, ch = pos.ch;
        var type;
        if (ch == 0) { type = styles[2]; }
        else { for (;;) {
          var mid = (before + after) >> 1;
          if ((mid ? styles[mid * 2 - 1] : 0) >= ch) { after = mid; }
          else if (styles[mid * 2 + 1] < ch) { before = mid + 1; }
          else { type = styles[mid * 2 + 2]; break }
        } }
        var cut = type ? type.indexOf("overlay ") : -1;
        return cut < 0 ? type : cut == 0 ? null : type.slice(0, cut - 1)
      },

      getModeAt: function(pos) {
        var mode = this.doc.mode;
        if (!mode.innerMode) { return mode }
        return CodeMirror.innerMode(mode, this.getTokenAt(pos).state).mode
      },

      getHelper: function(pos, type) {
        return this.getHelpers(pos, type)[0]
      },

      getHelpers: function(pos, type) {
        var this$1 = this;

        var found = [];
        if (!helpers.hasOwnProperty(type)) { return found }
        var help = helpers[type], mode = this.getModeAt(pos);
        if (typeof mode[type] == "string") {
          if (help[mode[type]]) { found.push(help[mode[type]]); }
        } else if (mode[type]) {
          for (var i = 0; i < mode[type].length; i++) {
            var val = help[mode[type][i]];
            if (val) { found.push(val); }
          }
        } else if (mode.helperType && help[mode.helperType]) {
          found.push(help[mode.helperType]);
        } else if (help[mode.name]) {
          found.push(help[mode.name]);
        }
        for (var i$1 = 0; i$1 < help._global.length; i$1++) {
          var cur = help._global[i$1];
          if (cur.pred(mode, this$1) && indexOf(found, cur.val) == -1)
            { found.push(cur.val); }
        }
        return found
      },

      getStateAfter: function(line, precise) {
        var doc = this.doc;
        line = clipLine(doc, line == null ? doc.first + doc.size - 1: line);
        return getContextBefore(this, line + 1, precise).state
      },

      cursorCoords: function(start, mode) {
        var pos, range$$1 = this.doc.sel.primary();
        if (start == null) { pos = range$$1.head; }
        else if (typeof start == "object") { pos = clipPos(this.doc, start); }
        else { pos = start ? range$$1.from() : range$$1.to(); }
        return cursorCoords(this, pos, mode || "page")
      },

      charCoords: function(pos, mode) {
        return charCoords(this, clipPos(this.doc, pos), mode || "page")
      },

      coordsChar: function(coords, mode) {
        coords = fromCoordSystem(this, coords, mode || "page");
        return coordsChar(this, coords.left, coords.top)
      },

      lineAtHeight: function(height, mode) {
        height = fromCoordSystem(this, {top: height, left: 0}, mode || "page").top;
        return lineAtHeight(this.doc, height + this.display.viewOffset)
      },
      heightAtLine: function(line, mode, includeWidgets) {
        var end = false, lineObj;
        if (typeof line == "number") {
          var last = this.doc.first + this.doc.size - 1;
          if (line < this.doc.first) { line = this.doc.first; }
          else if (line > last) { line = last; end = true; }
          lineObj = getLine(this.doc, line);
        } else {
          lineObj = line;
        }
        return intoCoordSystem(this, lineObj, {top: 0, left: 0}, mode || "page", includeWidgets || end).top +
          (end ? this.doc.height - heightAtLine(lineObj) : 0)
      },

      defaultTextHeight: function() { return textHeight(this.display) },
      defaultCharWidth: function() { return charWidth(this.display) },

      getViewport: function() { return {from: this.display.viewFrom, to: this.display.viewTo}},

      addWidget: function(pos, node, scroll, vert, horiz) {
        var display = this.display;
        pos = cursorCoords(this, clipPos(this.doc, pos));
        var top = pos.bottom, left = pos.left;
        node.style.position = "absolute";
        node.setAttribute("cm-ignore-events", "true");
        this.display.input.setUneditable(node);
        display.sizer.appendChild(node);
        if (vert == "over") {
          top = pos.top;
        } else if (vert == "above" || vert == "near") {
          var vspace = Math.max(display.wrapper.clientHeight, this.doc.height),
          hspace = Math.max(display.sizer.clientWidth, display.lineSpace.clientWidth);
          // Default to positioning above (if specified and possible); otherwise default to positioning below
          if ((vert == 'above' || pos.bottom + node.offsetHeight > vspace) && pos.top > node.offsetHeight)
            { top = pos.top - node.offsetHeight; }
          else if (pos.bottom + node.offsetHeight <= vspace)
            { top = pos.bottom; }
          if (left + node.offsetWidth > hspace)
            { left = hspace - node.offsetWidth; }
        }
        node.style.top = top + "px";
        node.style.left = node.style.right = "";
        if (horiz == "right") {
          left = display.sizer.clientWidth - node.offsetWidth;
          node.style.right = "0px";
        } else {
          if (horiz == "left") { left = 0; }
          else if (horiz == "middle") { left = (display.sizer.clientWidth - node.offsetWidth) / 2; }
          node.style.left = left + "px";
        }
        if (scroll)
          { scrollIntoView(this, {left: left, top: top, right: left + node.offsetWidth, bottom: top + node.offsetHeight}); }
      },

      triggerOnKeyDown: methodOp(onKeyDown),
      triggerOnKeyPress: methodOp(onKeyPress),
      triggerOnKeyUp: onKeyUp,
      triggerOnMouseDown: methodOp(onMouseDown),

      execCommand: function(cmd) {
        if (commands.hasOwnProperty(cmd))
          { return commands[cmd].call(null, this) }
      },

      triggerElectric: methodOp(function(text) { triggerElectric(this, text); }),

      findPosH: function(from, amount, unit, visually) {
        var this$1 = this;

        var dir = 1;
        if (amount < 0) { dir = -1; amount = -amount; }
        var cur = clipPos(this.doc, from);
        for (var i = 0; i < amount; ++i) {
          cur = findPosH(this$1.doc, cur, dir, unit, visually);
          if (cur.hitSide) { break }
        }
        return cur
      },

      moveH: methodOp(function(dir, unit) {
        var this$1 = this;

        this.extendSelectionsBy(function (range$$1) {
          if (this$1.display.shift || this$1.doc.extend || range$$1.empty())
            { return findPosH(this$1.doc, range$$1.head, dir, unit, this$1.options.rtlMoveVisually) }
          else
            { return dir < 0 ? range$$1.from() : range$$1.to() }
        }, sel_move);
      }),

      deleteH: methodOp(function(dir, unit) {
        var sel = this.doc.sel, doc = this.doc;
        if (sel.somethingSelected())
          { doc.replaceSelection("", null, "+delete"); }
        else
          { deleteNearSelection(this, function (range$$1) {
            var other = findPosH(doc, range$$1.head, dir, unit, false);
            return dir < 0 ? {from: other, to: range$$1.head} : {from: range$$1.head, to: other}
          }); }
      }),

      findPosV: function(from, amount, unit, goalColumn) {
        var this$1 = this;

        var dir = 1, x = goalColumn;
        if (amount < 0) { dir = -1; amount = -amount; }
        var cur = clipPos(this.doc, from);
        for (var i = 0; i < amount; ++i) {
          var coords = cursorCoords(this$1, cur, "div");
          if (x == null) { x = coords.left; }
          else { coords.left = x; }
          cur = findPosV(this$1, coords, dir, unit);
          if (cur.hitSide) { break }
        }
        return cur
      },

      moveV: methodOp(function(dir, unit) {
        var this$1 = this;

        var doc = this.doc, goals = [];
        var collapse = !this.display.shift && !doc.extend && doc.sel.somethingSelected();
        doc.extendSelectionsBy(function (range$$1) {
          if (collapse)
            { return dir < 0 ? range$$1.from() : range$$1.to() }
          var headPos = cursorCoords(this$1, range$$1.head, "div");
          if (range$$1.goalColumn != null) { headPos.left = range$$1.goalColumn; }
          goals.push(headPos.left);
          var pos = findPosV(this$1, headPos, dir, unit);
          if (unit == "page" && range$$1 == doc.sel.primary())
            { addToScrollTop(this$1, charCoords(this$1, pos, "div").top - headPos.top); }
          return pos
        }, sel_move);
        if (goals.length) { for (var i = 0; i < doc.sel.ranges.length; i++)
          { doc.sel.ranges[i].goalColumn = goals[i]; } }
      }),

      // Find the word at the given position (as returned by coordsChar).
      findWordAt: function(pos) {
        var doc = this.doc, line = getLine(doc, pos.line).text;
        var start = pos.ch, end = pos.ch;
        if (line) {
          var helper = this.getHelper(pos, "wordChars");
          if ((pos.sticky == "before" || end == line.length) && start) { --start; } else { ++end; }
          var startChar = line.charAt(start);
          var check = isWordChar(startChar, helper)
            ? function (ch) { return isWordChar(ch, helper); }
            : /\s/.test(startChar) ? function (ch) { return /\s/.test(ch); }
            : function (ch) { return (!/\s/.test(ch) && !isWordChar(ch)); };
          while (start > 0 && check(line.charAt(start - 1))) { --start; }
          while (end < line.length && check(line.charAt(end))) { ++end; }
        }
        return new Range(Pos(pos.line, start), Pos(pos.line, end))
      },

      toggleOverwrite: function(value) {
        if (value != null && value == this.state.overwrite) { return }
        if (this.state.overwrite = !this.state.overwrite)
          { addClass(this.display.cursorDiv, "CodeMirror-overwrite"); }
        else
          { rmClass(this.display.cursorDiv, "CodeMirror-overwrite"); }

        signal(this, "overwriteToggle", this, this.state.overwrite);
      },
      hasFocus: function() { return this.display.input.getField() == activeElt() },
      isReadOnly: function() { return !!(this.options.readOnly || this.doc.cantEdit) },

      scrollTo: methodOp(function (x, y) { scrollToCoords(this, x, y); }),
      getScrollInfo: function() {
        var scroller = this.display.scroller;
        return {left: scroller.scrollLeft, top: scroller.scrollTop,
                height: scroller.scrollHeight - scrollGap(this) - this.display.barHeight,
                width: scroller.scrollWidth - scrollGap(this) - this.display.barWidth,
                clientHeight: displayHeight(this), clientWidth: displayWidth(this)}
      },

      scrollIntoView: methodOp(function(range$$1, margin) {
        if (range$$1 == null) {
          range$$1 = {from: this.doc.sel.primary().head, to: null};
          if (margin == null) { margin = this.options.cursorScrollMargin; }
        } else if (typeof range$$1 == "number") {
          range$$1 = {from: Pos(range$$1, 0), to: null};
        } else if (range$$1.from == null) {
          range$$1 = {from: range$$1, to: null};
        }
        if (!range$$1.to) { range$$1.to = range$$1.from; }
        range$$1.margin = margin || 0;

        if (range$$1.from.line != null) {
          scrollToRange(this, range$$1);
        } else {
          scrollToCoordsRange(this, range$$1.from, range$$1.to, range$$1.margin);
        }
      }),

      setSize: methodOp(function(width, height) {
        var this$1 = this;

        var interpret = function (val) { return typeof val == "number" || /^\d+$/.test(String(val)) ? val + "px" : val; };
        if (width != null) { this.display.wrapper.style.width = interpret(width); }
        if (height != null) { this.display.wrapper.style.height = interpret(height); }
        if (this.options.lineWrapping) { clearLineMeasurementCache(this); }
        var lineNo$$1 = this.display.viewFrom;
        this.doc.iter(lineNo$$1, this.display.viewTo, function (line) {
          if (line.widgets) { for (var i = 0; i < line.widgets.length; i++)
            { if (line.widgets[i].noHScroll) { regLineChange(this$1, lineNo$$1, "widget"); break } } }
          ++lineNo$$1;
        });
        this.curOp.forceUpdate = true;
        signal(this, "refresh", this);
      }),

      operation: function(f){return runInOp(this, f)},
      startOperation: function(){return startOperation(this)},
      endOperation: function(){return endOperation(this)},

      refresh: methodOp(function() {
        var oldHeight = this.display.cachedTextHeight;
        regChange(this);
        this.curOp.forceUpdate = true;
        clearCaches(this);
        scrollToCoords(this, this.doc.scrollLeft, this.doc.scrollTop);
        updateGutterSpace(this.display);
        if (oldHeight == null || Math.abs(oldHeight - textHeight(this.display)) > .5)
          { estimateLineHeights(this); }
        signal(this, "refresh", this);
      }),

      swapDoc: methodOp(function(doc) {
        var old = this.doc;
        old.cm = null;
        // Cancel the current text selection if any (#5821)
        if (this.state.selectingText) { this.state.selectingText(); }
        attachDoc(this, doc);
        clearCaches(this);
        this.display.input.reset();
        scrollToCoords(this, doc.scrollLeft, doc.scrollTop);
        this.curOp.forceScroll = true;
        signalLater(this, "swapDoc", this, old);
        return old
      }),

      phrase: function(phraseText) {
        var phrases = this.options.phrases;
        return phrases && Object.prototype.hasOwnProperty.call(phrases, phraseText) ? phrases[phraseText] : phraseText
      },

      getInputField: function(){return this.display.input.getField()},
      getWrapperElement: function(){return this.display.wrapper},
      getScrollerElement: function(){return this.display.scroller},
      getGutterElement: function(){return this.display.gutters}
    };
    eventMixin(CodeMirror);

    CodeMirror.registerHelper = function(type, name, value) {
      if (!helpers.hasOwnProperty(type)) { helpers[type] = CodeMirror[type] = {_global: []}; }
      helpers[type][name] = value;
    };
    CodeMirror.registerGlobalHelper = function(type, name, predicate, value) {
      CodeMirror.registerHelper(type, name, value);
      helpers[type]._global.push({pred: predicate, val: value});
    };
  }

  // Used for horizontal relative motion. Dir is -1 or 1 (left or
  // right), unit can be "char", "column" (like char, but doesn't
  // cross line boundaries), "word" (across next word), or "group" (to
  // the start of next group of word or non-word-non-whitespace
  // chars). The visually param controls whether, in right-to-left
  // text, direction 1 means to move towards the next index in the
  // string, or towards the character to the right of the current
  // position. The resulting position will have a hitSide=true
  // property if it reached the end of the document.
  function findPosH(doc, pos, dir, unit, visually) {
    var oldPos = pos;
    var origDir = dir;
    var lineObj = getLine(doc, pos.line);
    function findNextLine() {
      var l = pos.line + dir;
      if (l < doc.first || l >= doc.first + doc.size) { return false }
      pos = new Pos(l, pos.ch, pos.sticky);
      return lineObj = getLine(doc, l)
    }
    function moveOnce(boundToLine) {
      var next;
      if (visually) {
        next = moveVisually(doc.cm, lineObj, pos, dir);
      } else {
        next = moveLogically(lineObj, pos, dir);
      }
      if (next == null) {
        if (!boundToLine && findNextLine())
          { pos = endOfLine(visually, doc.cm, lineObj, pos.line, dir); }
        else
          { return false }
      } else {
        pos = next;
      }
      return true
    }

    if (unit == "char") {
      moveOnce();
    } else if (unit == "column") {
      moveOnce(true);
    } else if (unit == "word" || unit == "group") {
      var sawType = null, group = unit == "group";
      var helper = doc.cm && doc.cm.getHelper(pos, "wordChars");
      for (var first = true;; first = false) {
        if (dir < 0 && !moveOnce(!first)) { break }
        var cur = lineObj.text.charAt(pos.ch) || "\n";
        var type = isWordChar(cur, helper) ? "w"
          : group && cur == "\n" ? "n"
          : !group || /\s/.test(cur) ? null
          : "p";
        if (group && !first && !type) { type = "s"; }
        if (sawType && sawType != type) {
          if (dir < 0) {dir = 1; moveOnce(); pos.sticky = "after";}
          break
        }

        if (type) { sawType = type; }
        if (dir > 0 && !moveOnce(!first)) { break }
      }
    }
    var result = skipAtomic(doc, pos, oldPos, origDir, true);
    if (equalCursorPos(oldPos, result)) { result.hitSide = true; }
    return result
  }

  // For relative vertical movement. Dir may be -1 or 1. Unit can be
  // "page" or "line". The resulting position will have a hitSide=true
  // property if it reached the end of the document.
  function findPosV(cm, pos, dir, unit) {
    var doc = cm.doc, x = pos.left, y;
    if (unit == "page") {
      var pageSize = Math.min(cm.display.wrapper.clientHeight, window.innerHeight || document.documentElement.clientHeight);
      var moveAmount = Math.max(pageSize - .5 * textHeight(cm.display), 3);
      y = (dir > 0 ? pos.bottom : pos.top) + dir * moveAmount;

    } else if (unit == "line") {
      y = dir > 0 ? pos.bottom + 3 : pos.top - 3;
    }
    var target;
    for (;;) {
      target = coordsChar(cm, x, y);
      if (!target.outside) { break }
      if (dir < 0 ? y <= 0 : y >= doc.height) { target.hitSide = true; break }
      y += dir * 5;
    }
    return target
  }

  // CONTENTEDITABLE INPUT STYLE

  var ContentEditableInput = function(cm) {
    this.cm = cm;
    this.lastAnchorNode = this.lastAnchorOffset = this.lastFocusNode = this.lastFocusOffset = null;
    this.polling = new Delayed();
    this.composing = null;
    this.gracePeriod = false;
    this.readDOMTimeout = null;
  };

  ContentEditableInput.prototype.init = function (display) {
      var this$1 = this;

    var input = this, cm = input.cm;
    var div = input.div = display.lineDiv;
    disableBrowserMagic(div, cm.options.spellcheck, cm.options.autocorrect, cm.options.autocapitalize);

    on(div, "paste", function (e) {
      if (signalDOMEvent(cm, e) || handlePaste(e, cm)) { return }
      // IE doesn't fire input events, so we schedule a read for the pasted content in this way
      if (ie_version <= 11) { setTimeout(operation(cm, function () { return this$1.updateFromDOM(); }), 20); }
    });

    on(div, "compositionstart", function (e) {
      this$1.composing = {data: e.data, done: false};
    });
    on(div, "compositionupdate", function (e) {
      if (!this$1.composing) { this$1.composing = {data: e.data, done: false}; }
    });
    on(div, "compositionend", function (e) {
      if (this$1.composing) {
        if (e.data != this$1.composing.data) { this$1.readFromDOMSoon(); }
        this$1.composing.done = true;
      }
    });

    on(div, "touchstart", function () { return input.forceCompositionEnd(); });

    on(div, "input", function () {
      if (!this$1.composing) { this$1.readFromDOMSoon(); }
    });

    function onCopyCut(e) {
      if (signalDOMEvent(cm, e)) { return }
      if (cm.somethingSelected()) {
        setLastCopied({lineWise: false, text: cm.getSelections()});
        if (e.type == "cut") { cm.replaceSelection("", null, "cut"); }
      } else if (!cm.options.lineWiseCopyCut) {
        return
      } else {
        var ranges = copyableRanges(cm);
        setLastCopied({lineWise: true, text: ranges.text});
        if (e.type == "cut") {
          cm.operation(function () {
            cm.setSelections(ranges.ranges, 0, sel_dontScroll);
            cm.replaceSelection("", null, "cut");
          });
        }
      }
      if (e.clipboardData) {
        e.clipboardData.clearData();
        var content = lastCopied.text.join("\n");
        // iOS exposes the clipboard API, but seems to discard content inserted into it
        e.clipboardData.setData("Text", content);
        if (e.clipboardData.getData("Text") == content) {
          e.preventDefault();
          return
        }
      }
      // Old-fashioned briefly-focus-a-textarea hack
      var kludge = hiddenTextarea(), te = kludge.firstChild;
      cm.display.lineSpace.insertBefore(kludge, cm.display.lineSpace.firstChild);
      te.value = lastCopied.text.join("\n");
      var hadFocus = document.activeElement;
      selectInput(te);
      setTimeout(function () {
        cm.display.lineSpace.removeChild(kludge);
        hadFocus.focus();
        if (hadFocus == div) { input.showPrimarySelection(); }
      }, 50);
    }
    on(div, "copy", onCopyCut);
    on(div, "cut", onCopyCut);
  };

  ContentEditableInput.prototype.prepareSelection = function () {
    var result = prepareSelection(this.cm, false);
    result.focus = this.cm.state.focused;
    return result
  };

  ContentEditableInput.prototype.showSelection = function (info, takeFocus) {
    if (!info || !this.cm.display.view.length) { return }
    if (info.focus || takeFocus) { this.showPrimarySelection(); }
    this.showMultipleSelections(info);
  };

  ContentEditableInput.prototype.getSelection = function () {
    return this.cm.display.wrapper.ownerDocument.getSelection()
  };

  ContentEditableInput.prototype.showPrimarySelection = function () {
    var sel = this.getSelection(), cm = this.cm, prim = cm.doc.sel.primary();
    var from = prim.from(), to = prim.to();

    if (cm.display.viewTo == cm.display.viewFrom || from.line >= cm.display.viewTo || to.line < cm.display.viewFrom) {
      sel.removeAllRanges();
      return
    }

    var curAnchor = domToPos(cm, sel.anchorNode, sel.anchorOffset);
    var curFocus = domToPos(cm, sel.focusNode, sel.focusOffset);
    if (curAnchor && !curAnchor.bad && curFocus && !curFocus.bad &&
        cmp(minPos(curAnchor, curFocus), from) == 0 &&
        cmp(maxPos(curAnchor, curFocus), to) == 0)
      { return }

    var view = cm.display.view;
    var start = (from.line >= cm.display.viewFrom && posToDOM(cm, from)) ||
        {node: view[0].measure.map[2], offset: 0};
    var end = to.line < cm.display.viewTo && posToDOM(cm, to);
    if (!end) {
      var measure = view[view.length - 1].measure;
      var map$$1 = measure.maps ? measure.maps[measure.maps.length - 1] : measure.map;
      end = {node: map$$1[map$$1.length - 1], offset: map$$1[map$$1.length - 2] - map$$1[map$$1.length - 3]};
    }

    if (!start || !end) {
      sel.removeAllRanges();
      return
    }

    var old = sel.rangeCount && sel.getRangeAt(0), rng;
    try { rng = range(start.node, start.offset, end.offset, end.node); }
    catch(e) {} // Our model of the DOM might be outdated, in which case the range we try to set can be impossible
    if (rng) {
      if (!gecko && cm.state.focused) {
        sel.collapse(start.node, start.offset);
        if (!rng.collapsed) {
          sel.removeAllRanges();
          sel.addRange(rng);
        }
      } else {
        sel.removeAllRanges();
        sel.addRange(rng);
      }
      if (old && sel.anchorNode == null) { sel.addRange(old); }
      else if (gecko) { this.startGracePeriod(); }
    }
    this.rememberSelection();
  };

  ContentEditableInput.prototype.startGracePeriod = function () {
      var this$1 = this;

    clearTimeout(this.gracePeriod);
    this.gracePeriod = setTimeout(function () {
      this$1.gracePeriod = false;
      if (this$1.selectionChanged())
        { this$1.cm.operation(function () { return this$1.cm.curOp.selectionChanged = true; }); }
    }, 20);
  };

  ContentEditableInput.prototype.showMultipleSelections = function (info) {
    removeChildrenAndAdd(this.cm.display.cursorDiv, info.cursors);
    removeChildrenAndAdd(this.cm.display.selectionDiv, info.selection);
  };

  ContentEditableInput.prototype.rememberSelection = function () {
    var sel = this.getSelection();
    this.lastAnchorNode = sel.anchorNode; this.lastAnchorOffset = sel.anchorOffset;
    this.lastFocusNode = sel.focusNode; this.lastFocusOffset = sel.focusOffset;
  };

  ContentEditableInput.prototype.selectionInEditor = function () {
    var sel = this.getSelection();
    if (!sel.rangeCount) { return false }
    var node = sel.getRangeAt(0).commonAncestorContainer;
    return contains(this.div, node)
  };

  ContentEditableInput.prototype.focus = function () {
    if (this.cm.options.readOnly != "nocursor") {
      if (!this.selectionInEditor())
        { this.showSelection(this.prepareSelection(), true); }
      this.div.focus();
    }
  };
  ContentEditableInput.prototype.blur = function () { this.div.blur(); };
  ContentEditableInput.prototype.getField = function () { return this.div };

  ContentEditableInput.prototype.supportsTouch = function () { return true };

  ContentEditableInput.prototype.receivedFocus = function () {
    var input = this;
    if (this.selectionInEditor())
      { this.pollSelection(); }
    else
      { runInOp(this.cm, function () { return input.cm.curOp.selectionChanged = true; }); }

    function poll() {
      if (input.cm.state.focused) {
        input.pollSelection();
        input.polling.set(input.cm.options.pollInterval, poll);
      }
    }
    this.polling.set(this.cm.options.pollInterval, poll);
  };

  ContentEditableInput.prototype.selectionChanged = function () {
    var sel = this.getSelection();
    return sel.anchorNode != this.lastAnchorNode || sel.anchorOffset != this.lastAnchorOffset ||
      sel.focusNode != this.lastFocusNode || sel.focusOffset != this.lastFocusOffset
  };

  ContentEditableInput.prototype.pollSelection = function () {
    if (this.readDOMTimeout != null || this.gracePeriod || !this.selectionChanged()) { return }
    var sel = this.getSelection(), cm = this.cm;
    // On Android Chrome (version 56, at least), backspacing into an
    // uneditable block element will put the cursor in that element,
    // and then, because it's not editable, hide the virtual keyboard.
    // Because Android doesn't allow us to actually detect backspace
    // presses in a sane way, this code checks for when that happens
    // and simulates a backspace press in this case.
    if (android && chrome && this.cm.display.gutterSpecs.length && isInGutter(sel.anchorNode)) {
      this.cm.triggerOnKeyDown({type: "keydown", keyCode: 8, preventDefault: Math.abs});
      this.blur();
      this.focus();
      return
    }
    if (this.composing) { return }
    this.rememberSelection();
    var anchor = domToPos(cm, sel.anchorNode, sel.anchorOffset);
    var head = domToPos(cm, sel.focusNode, sel.focusOffset);
    if (anchor && head) { runInOp(cm, function () {
      setSelection(cm.doc, simpleSelection(anchor, head), sel_dontScroll);
      if (anchor.bad || head.bad) { cm.curOp.selectionChanged = true; }
    }); }
  };

  ContentEditableInput.prototype.pollContent = function () {
    if (this.readDOMTimeout != null) {
      clearTimeout(this.readDOMTimeout);
      this.readDOMTimeout = null;
    }

    var cm = this.cm, display = cm.display, sel = cm.doc.sel.primary();
    var from = sel.from(), to = sel.to();
    if (from.ch == 0 && from.line > cm.firstLine())
      { from = Pos(from.line - 1, getLine(cm.doc, from.line - 1).length); }
    if (to.ch == getLine(cm.doc, to.line).text.length && to.line < cm.lastLine())
      { to = Pos(to.line + 1, 0); }
    if (from.line < display.viewFrom || to.line > display.viewTo - 1) { return false }

    var fromIndex, fromLine, fromNode;
    if (from.line == display.viewFrom || (fromIndex = findViewIndex(cm, from.line)) == 0) {
      fromLine = lineNo(display.view[0].line);
      fromNode = display.view[0].node;
    } else {
      fromLine = lineNo(display.view[fromIndex].line);
      fromNode = display.view[fromIndex - 1].node.nextSibling;
    }
    var toIndex = findViewIndex(cm, to.line);
    var toLine, toNode;
    if (toIndex == display.view.length - 1) {
      toLine = display.viewTo - 1;
      toNode = display.lineDiv.lastChild;
    } else {
      toLine = lineNo(display.view[toIndex + 1].line) - 1;
      toNode = display.view[toIndex + 1].node.previousSibling;
    }

    if (!fromNode) { return false }
    var newText = cm.doc.splitLines(domTextBetween(cm, fromNode, toNode, fromLine, toLine));
    var oldText = getBetween(cm.doc, Pos(fromLine, 0), Pos(toLine, getLine(cm.doc, toLine).text.length));
    while (newText.length > 1 && oldText.length > 1) {
      if (lst(newText) == lst(oldText)) { newText.pop(); oldText.pop(); toLine--; }
      else if (newText[0] == oldText[0]) { newText.shift(); oldText.shift(); fromLine++; }
      else { break }
    }

    var cutFront = 0, cutEnd = 0;
    var newTop = newText[0], oldTop = oldText[0], maxCutFront = Math.min(newTop.length, oldTop.length);
    while (cutFront < maxCutFront && newTop.charCodeAt(cutFront) == oldTop.charCodeAt(cutFront))
      { ++cutFront; }
    var newBot = lst(newText), oldBot = lst(oldText);
    var maxCutEnd = Math.min(newBot.length - (newText.length == 1 ? cutFront : 0),
                             oldBot.length - (oldText.length == 1 ? cutFront : 0));
    while (cutEnd < maxCutEnd &&
           newBot.charCodeAt(newBot.length - cutEnd - 1) == oldBot.charCodeAt(oldBot.length - cutEnd - 1))
      { ++cutEnd; }
    // Try to move start of change to start of selection if ambiguous
    if (newText.length == 1 && oldText.length == 1 && fromLine == from.line) {
      while (cutFront && cutFront > from.ch &&
             newBot.charCodeAt(newBot.length - cutEnd - 1) == oldBot.charCodeAt(oldBot.length - cutEnd - 1)) {
        cutFront--;
        cutEnd++;
      }
    }

    newText[newText.length - 1] = newBot.slice(0, newBot.length - cutEnd).replace(/^\u200b+/, "");
    newText[0] = newText[0].slice(cutFront).replace(/\u200b+$/, "");

    var chFrom = Pos(fromLine, cutFront);
    var chTo = Pos(toLine, oldText.length ? lst(oldText).length - cutEnd : 0);
    if (newText.length > 1 || newText[0] || cmp(chFrom, chTo)) {
      replaceRange(cm.doc, newText, chFrom, chTo, "+input");
      return true
    }
  };

  ContentEditableInput.prototype.ensurePolled = function () {
    this.forceCompositionEnd();
  };
  ContentEditableInput.prototype.reset = function () {
    this.forceCompositionEnd();
  };
  ContentEditableInput.prototype.forceCompositionEnd = function () {
    if (!this.composing) { return }
    clearTimeout(this.readDOMTimeout);
    this.composing = null;
    this.updateFromDOM();
    this.div.blur();
    this.div.focus();
  };
  ContentEditableInput.prototype.readFromDOMSoon = function () {
      var this$1 = this;

    if (this.readDOMTimeout != null) { return }
    this.readDOMTimeout = setTimeout(function () {
      this$1.readDOMTimeout = null;
      if (this$1.composing) {
        if (this$1.composing.done) { this$1.composing = null; }
        else { return }
      }
      this$1.updateFromDOM();
    }, 80);
  };

  ContentEditableInput.prototype.updateFromDOM = function () {
      var this$1 = this;

    if (this.cm.isReadOnly() || !this.pollContent())
      { runInOp(this.cm, function () { return regChange(this$1.cm); }); }
  };

  ContentEditableInput.prototype.setUneditable = function (node) {
    node.contentEditable = "false";
  };

  ContentEditableInput.prototype.onKeyPress = function (e) {
    if (e.charCode == 0 || this.composing) { return }
    e.preventDefault();
    if (!this.cm.isReadOnly())
      { operation(this.cm, applyTextInput)(this.cm, String.fromCharCode(e.charCode == null ? e.keyCode : e.charCode), 0); }
  };

  ContentEditableInput.prototype.readOnlyChanged = function (val) {
    this.div.contentEditable = String(val != "nocursor");
  };

  ContentEditableInput.prototype.onContextMenu = function () {};
  ContentEditableInput.prototype.resetPosition = function () {};

  ContentEditableInput.prototype.needsContentAttribute = true;

  function posToDOM(cm, pos) {
    var view = findViewForLine(cm, pos.line);
    if (!view || view.hidden) { return null }
    var line = getLine(cm.doc, pos.line);
    var info = mapFromLineView(view, line, pos.line);

    var order = getOrder(line, cm.doc.direction), side = "left";
    if (order) {
      var partPos = getBidiPartAt(order, pos.ch);
      side = partPos % 2 ? "right" : "left";
    }
    var result = nodeAndOffsetInLineMap(info.map, pos.ch, side);
    result.offset = result.collapse == "right" ? result.end : result.start;
    return result
  }

  function isInGutter(node) {
    for (var scan = node; scan; scan = scan.parentNode)
      { if (/CodeMirror-gutter-wrapper/.test(scan.className)) { return true } }
    return false
  }

  function badPos(pos, bad) { if (bad) { pos.bad = true; } return pos }

  function domTextBetween(cm, from, to, fromLine, toLine) {
    var text = "", closing = false, lineSep = cm.doc.lineSeparator(), extraLinebreak = false;
    function recognizeMarker(id) { return function (marker) { return marker.id == id; } }
    function close() {
      if (closing) {
        text += lineSep;
        if (extraLinebreak) { text += lineSep; }
        closing = extraLinebreak = false;
      }
    }
    function addText(str) {
      if (str) {
        close();
        text += str;
      }
    }
    function walk(node) {
      if (node.nodeType == 1) {
        var cmText = node.getAttribute("cm-text");
        if (cmText) {
          addText(cmText);
          return
        }
        var markerID = node.getAttribute("cm-marker"), range$$1;
        if (markerID) {
          var found = cm.findMarks(Pos(fromLine, 0), Pos(toLine + 1, 0), recognizeMarker(+markerID));
          if (found.length && (range$$1 = found[0].find(0)))
            { addText(getBetween(cm.doc, range$$1.from, range$$1.to).join(lineSep)); }
          return
        }
        if (node.getAttribute("contenteditable") == "false") { return }
        var isBlock = /^(pre|div|p|li|table|br)$/i.test(node.nodeName);
        if (!/^br$/i.test(node.nodeName) && node.textContent.length == 0) { return }

        if (isBlock) { close(); }
        for (var i = 0; i < node.childNodes.length; i++)
          { walk(node.childNodes[i]); }

        if (/^(pre|p)$/i.test(node.nodeName)) { extraLinebreak = true; }
        if (isBlock) { closing = true; }
      } else if (node.nodeType == 3) {
        addText(node.nodeValue.replace(/\u200b/g, "").replace(/\u00a0/g, " "));
      }
    }
    for (;;) {
      walk(from);
      if (from == to) { break }
      from = from.nextSibling;
      extraLinebreak = false;
    }
    return text
  }

  function domToPos(cm, node, offset) {
    var lineNode;
    if (node == cm.display.lineDiv) {
      lineNode = cm.display.lineDiv.childNodes[offset];
      if (!lineNode) { return badPos(cm.clipPos(Pos(cm.display.viewTo - 1)), true) }
      node = null; offset = 0;
    } else {
      for (lineNode = node;; lineNode = lineNode.parentNode) {
        if (!lineNode || lineNode == cm.display.lineDiv) { return null }
        if (lineNode.parentNode && lineNode.parentNode == cm.display.lineDiv) { break }
      }
    }
    for (var i = 0; i < cm.display.view.length; i++) {
      var lineView = cm.display.view[i];
      if (lineView.node == lineNode)
        { return locateNodeInLineView(lineView, node, offset) }
    }
  }

  function locateNodeInLineView(lineView, node, offset) {
    var wrapper = lineView.text.firstChild, bad = false;
    if (!node || !contains(wrapper, node)) { return badPos(Pos(lineNo(lineView.line), 0), true) }
    if (node == wrapper) {
      bad = true;
      node = wrapper.childNodes[offset];
      offset = 0;
      if (!node) {
        var line = lineView.rest ? lst(lineView.rest) : lineView.line;
        return badPos(Pos(lineNo(line), line.text.length), bad)
      }
    }

    var textNode = node.nodeType == 3 ? node : null, topNode = node;
    if (!textNode && node.childNodes.length == 1 && node.firstChild.nodeType == 3) {
      textNode = node.firstChild;
      if (offset) { offset = textNode.nodeValue.length; }
    }
    while (topNode.parentNode != wrapper) { topNode = topNode.parentNode; }
    var measure = lineView.measure, maps = measure.maps;

    function find(textNode, topNode, offset) {
      for (var i = -1; i < (maps ? maps.length : 0); i++) {
        var map$$1 = i < 0 ? measure.map : maps[i];
        for (var j = 0; j < map$$1.length; j += 3) {
          var curNode = map$$1[j + 2];
          if (curNode == textNode || curNode == topNode) {
            var line = lineNo(i < 0 ? lineView.line : lineView.rest[i]);
            var ch = map$$1[j] + offset;
            if (offset < 0 || curNode != textNode) { ch = map$$1[j + (offset ? 1 : 0)]; }
            return Pos(line, ch)
          }
        }
      }
    }
    var found = find(textNode, topNode, offset);
    if (found) { return badPos(found, bad) }

    // FIXME this is all really shaky. might handle the few cases it needs to handle, but likely to cause problems
    for (var after = topNode.nextSibling, dist = textNode ? textNode.nodeValue.length - offset : 0; after; after = after.nextSibling) {
      found = find(after, after.firstChild, 0);
      if (found)
        { return badPos(Pos(found.line, found.ch - dist), bad) }
      else
        { dist += after.textContent.length; }
    }
    for (var before = topNode.previousSibling, dist$1 = offset; before; before = before.previousSibling) {
      found = find(before, before.firstChild, -1);
      if (found)
        { return badPos(Pos(found.line, found.ch + dist$1), bad) }
      else
        { dist$1 += before.textContent.length; }
    }
  }

  // TEXTAREA INPUT STYLE

  var TextareaInput = function(cm) {
    this.cm = cm;
    // See input.poll and input.reset
    this.prevInput = "";

    // Flag that indicates whether we expect input to appear real soon
    // now (after some event like 'keypress' or 'input') and are
    // polling intensively.
    this.pollingFast = false;
    // Self-resetting timeout for the poller
    this.polling = new Delayed();
    // Used to work around IE issue with selection being forgotten when focus moves away from textarea
    this.hasSelection = false;
    this.composing = null;
  };

  TextareaInput.prototype.init = function (display) {
      var this$1 = this;

    var input = this, cm = this.cm;
    this.createField(display);
    var te = this.textarea;

    display.wrapper.insertBefore(this.wrapper, display.wrapper.firstChild);

    // Needed to hide big blue blinking cursor on Mobile Safari (doesn't seem to work in iOS 8 anymore)
    if (ios) { te.style.width = "0px"; }

    on(te, "input", function () {
      if (ie && ie_version >= 9 && this$1.hasSelection) { this$1.hasSelection = null; }
      input.poll();
    });

    on(te, "paste", function (e) {
      if (signalDOMEvent(cm, e) || handlePaste(e, cm)) { return }

      cm.state.pasteIncoming = +new Date;
      input.fastPoll();
    });

    function prepareCopyCut(e) {
      if (signalDOMEvent(cm, e)) { return }
      if (cm.somethingSelected()) {
        setLastCopied({lineWise: false, text: cm.getSelections()});
      } else if (!cm.options.lineWiseCopyCut) {
        return
      } else {
        var ranges = copyableRanges(cm);
        setLastCopied({lineWise: true, text: ranges.text});
        if (e.type == "cut") {
          cm.setSelections(ranges.ranges, null, sel_dontScroll);
        } else {
          input.prevInput = "";
          te.value = ranges.text.join("\n");
          selectInput(te);
        }
      }
      if (e.type == "cut") { cm.state.cutIncoming = +new Date; }
    }
    on(te, "cut", prepareCopyCut);
    on(te, "copy", prepareCopyCut);

    on(display.scroller, "paste", function (e) {
      if (eventInWidget(display, e) || signalDOMEvent(cm, e)) { return }
      if (!te.dispatchEvent) {
        cm.state.pasteIncoming = +new Date;
        input.focus();
        return
      }

      // Pass the `paste` event to the textarea so it's handled by its event listener.
      var event = new Event("paste");
      event.clipboardData = e.clipboardData;
      te.dispatchEvent(event);
    });

    // Prevent normal selection in the editor (we handle our own)
    on(display.lineSpace, "selectstart", function (e) {
      if (!eventInWidget(display, e)) { e_preventDefault(e); }
    });

    on(te, "compositionstart", function () {
      var start = cm.getCursor("from");
      if (input.composing) { input.composing.range.clear(); }
      input.composing = {
        start: start,
        range: cm.markText(start, cm.getCursor("to"), {className: "CodeMirror-composing"})
      };
    });
    on(te, "compositionend", function () {
      if (input.composing) {
        input.poll();
        input.composing.range.clear();
        input.composing = null;
      }
    });
  };

  TextareaInput.prototype.createField = function (_display) {
    // Wraps and hides input textarea
    this.wrapper = hiddenTextarea();
    // The semihidden textarea that is focused when the editor is
    // focused, and receives input.
    this.textarea = this.wrapper.firstChild;
  };

  TextareaInput.prototype.prepareSelection = function () {
    // Redraw the selection and/or cursor
    var cm = this.cm, display = cm.display, doc = cm.doc;
    var result = prepareSelection(cm);

    // Move the hidden textarea near the cursor to prevent scrolling artifacts
    if (cm.options.moveInputWithCursor) {
      var headPos = cursorCoords(cm, doc.sel.primary().head, "div");
      var wrapOff = display.wrapper.getBoundingClientRect(), lineOff = display.lineDiv.getBoundingClientRect();
      result.teTop = Math.max(0, Math.min(display.wrapper.clientHeight - 10,
                                          headPos.top + lineOff.top - wrapOff.top));
      result.teLeft = Math.max(0, Math.min(display.wrapper.clientWidth - 10,
                                           headPos.left + lineOff.left - wrapOff.left));
    }

    return result
  };

  TextareaInput.prototype.showSelection = function (drawn) {
    var cm = this.cm, display = cm.display;
    removeChildrenAndAdd(display.cursorDiv, drawn.cursors);
    removeChildrenAndAdd(display.selectionDiv, drawn.selection);
    if (drawn.teTop != null) {
      this.wrapper.style.top = drawn.teTop + "px";
      this.wrapper.style.left = drawn.teLeft + "px";
    }
  };

  // Reset the input to correspond to the selection (or to be empty,
  // when not typing and nothing is selected)
  TextareaInput.prototype.reset = function (typing) {
    if (this.contextMenuPending || this.composing) { return }
    var cm = this.cm;
    if (cm.somethingSelected()) {
      this.prevInput = "";
      var content = cm.getSelection();
      this.textarea.value = content;
      if (cm.state.focused) { selectInput(this.textarea); }
      if (ie && ie_version >= 9) { this.hasSelection = content; }
    } else if (!typing) {
      this.prevInput = this.textarea.value = "";
      if (ie && ie_version >= 9) { this.hasSelection = null; }
    }
  };

  TextareaInput.prototype.getField = function () { return this.textarea };

  TextareaInput.prototype.supportsTouch = function () { return false };

  TextareaInput.prototype.focus = function () {
    if (this.cm.options.readOnly != "nocursor" && (!mobile || activeElt() != this.textarea)) {
      try { this.textarea.focus(); }
      catch (e) {} // IE8 will throw if the textarea is display: none or not in DOM
    }
  };

  TextareaInput.prototype.blur = function () { this.textarea.blur(); };

  TextareaInput.prototype.resetPosition = function () {
    this.wrapper.style.top = this.wrapper.style.left = 0;
  };

  TextareaInput.prototype.receivedFocus = function () { this.slowPoll(); };

  // Poll for input changes, using the normal rate of polling. This
  // runs as long as the editor is focused.
  TextareaInput.prototype.slowPoll = function () {
      var this$1 = this;

    if (this.pollingFast) { return }
    this.polling.set(this.cm.options.pollInterval, function () {
      this$1.poll();
      if (this$1.cm.state.focused) { this$1.slowPoll(); }
    });
  };

  // When an event has just come in that is likely to add or change
  // something in the input textarea, we poll faster, to ensure that
  // the change appears on the screen quickly.
  TextareaInput.prototype.fastPoll = function () {
    var missed = false, input = this;
    input.pollingFast = true;
    function p() {
      var changed = input.poll();
      if (!changed && !missed) {missed = true; input.polling.set(60, p);}
      else {input.pollingFast = false; input.slowPoll();}
    }
    input.polling.set(20, p);
  };

  // Read input from the textarea, and update the document to match.
  // When something is selected, it is present in the textarea, and
  // selected (unless it is huge, in which case a placeholder is
  // used). When nothing is selected, the cursor sits after previously
  // seen text (can be empty), which is stored in prevInput (we must
  // not reset the textarea when typing, because that breaks IME).
  TextareaInput.prototype.poll = function () {
      var this$1 = this;

    var cm = this.cm, input = this.textarea, prevInput = this.prevInput;
    // Since this is called a *lot*, try to bail out as cheaply as
    // possible when it is clear that nothing happened. hasSelection
    // will be the case when there is a lot of text in the textarea,
    // in which case reading its value would be expensive.
    if (this.contextMenuPending || !cm.state.focused ||
        (hasSelection(input) && !prevInput && !this.composing) ||
        cm.isReadOnly() || cm.options.disableInput || cm.state.keySeq)
      { return false }

    var text = input.value;
    // If nothing changed, bail.
    if (text == prevInput && !cm.somethingSelected()) { return false }
    // Work around nonsensical selection resetting in IE9/10, and
    // inexplicable appearance of private area unicode characters on
    // some key combos in Mac (#2689).
    if (ie && ie_version >= 9 && this.hasSelection === text ||
        mac && /[\uf700-\uf7ff]/.test(text)) {
      cm.display.input.reset();
      return false
    }

    if (cm.doc.sel == cm.display.selForContextMenu) {
      var first = text.charCodeAt(0);
      if (first == 0x200b && !prevInput) { prevInput = "\u200b"; }
      if (first == 0x21da) { this.reset(); return this.cm.execCommand("undo") }
    }
    // Find the part of the input that is actually new
    var same = 0, l = Math.min(prevInput.length, text.length);
    while (same < l && prevInput.charCodeAt(same) == text.charCodeAt(same)) { ++same; }

    runInOp(cm, function () {
      applyTextInput(cm, text.slice(same), prevInput.length - same,
                     null, this$1.composing ? "*compose" : null);

      // Don't leave long text in the textarea, since it makes further polling slow
      if (text.length > 1000 || text.indexOf("\n") > -1) { input.value = this$1.prevInput = ""; }
      else { this$1.prevInput = text; }

      if (this$1.composing) {
        this$1.composing.range.clear();
        this$1.composing.range = cm.markText(this$1.composing.start, cm.getCursor("to"),
                                           {className: "CodeMirror-composing"});
      }
    });
    return true
  };

  TextareaInput.prototype.ensurePolled = function () {
    if (this.pollingFast && this.poll()) { this.pollingFast = false; }
  };

  TextareaInput.prototype.onKeyPress = function () {
    if (ie && ie_version >= 9) { this.hasSelection = null; }
    this.fastPoll();
  };

  TextareaInput.prototype.onContextMenu = function (e) {
    var input = this, cm = input.cm, display = cm.display, te = input.textarea;
    if (input.contextMenuPending) { input.contextMenuPending(); }
    var pos = posFromMouse(cm, e), scrollPos = display.scroller.scrollTop;
    if (!pos || presto) { return } // Opera is difficult.

    // Reset the current text selection only if the click is done outside of the selection
    // and 'resetSelectionOnContextMenu' option is true.
    var reset = cm.options.resetSelectionOnContextMenu;
    if (reset && cm.doc.sel.contains(pos) == -1)
      { operation(cm, setSelection)(cm.doc, simpleSelection(pos), sel_dontScroll); }

    var oldCSS = te.style.cssText, oldWrapperCSS = input.wrapper.style.cssText;
    var wrapperBox = input.wrapper.offsetParent.getBoundingClientRect();
    input.wrapper.style.cssText = "position: static";
    te.style.cssText = "position: absolute; width: 30px; height: 30px;\n      top: " + (e.clientY - wrapperBox.top - 5) + "px; left: " + (e.clientX - wrapperBox.left - 5) + "px;\n      z-index: 1000; background: " + (ie ? "rgba(255, 255, 255, .05)" : "transparent") + ";\n      outline: none; border-width: 0; outline: none; overflow: hidden; opacity: .05; filter: alpha(opacity=5);";
    var oldScrollY;
    if (webkit) { oldScrollY = window.scrollY; } // Work around Chrome issue (#2712)
    display.input.focus();
    if (webkit) { window.scrollTo(null, oldScrollY); }
    display.input.reset();
    // Adds "Select all" to context menu in FF
    if (!cm.somethingSelected()) { te.value = input.prevInput = " "; }
    input.contextMenuPending = rehide;
    display.selForContextMenu = cm.doc.sel;
    clearTimeout(display.detectingSelectAll);

    // Select-all will be greyed out if there's nothing to select, so
    // this adds a zero-width space so that we can later check whether
    // it got selected.
    function prepareSelectAllHack() {
      if (te.selectionStart != null) {
        var selected = cm.somethingSelected();
        var extval = "\u200b" + (selected ? te.value : "");
        te.value = "\u21da"; // Used to catch context-menu undo
        te.value = extval;
        input.prevInput = selected ? "" : "\u200b";
        te.selectionStart = 1; te.selectionEnd = extval.length;
        // Re-set this, in case some other handler touched the
        // selection in the meantime.
        display.selForContextMenu = cm.doc.sel;
      }
    }
    function rehide() {
      if (input.contextMenuPending != rehide) { return }
      input.contextMenuPending = false;
      input.wrapper.style.cssText = oldWrapperCSS;
      te.style.cssText = oldCSS;
      if (ie && ie_version < 9) { display.scrollbars.setScrollTop(display.scroller.scrollTop = scrollPos); }

      // Try to detect the user choosing select-all
      if (te.selectionStart != null) {
        if (!ie || (ie && ie_version < 9)) { prepareSelectAllHack(); }
        var i = 0, poll = function () {
          if (display.selForContextMenu == cm.doc.sel && te.selectionStart == 0 &&
              te.selectionEnd > 0 && input.prevInput == "\u200b") {
            operation(cm, selectAll)(cm);
          } else if (i++ < 10) {
            display.detectingSelectAll = setTimeout(poll, 500);
          } else {
            display.selForContextMenu = null;
            display.input.reset();
          }
        };
        display.detectingSelectAll = setTimeout(poll, 200);
      }
    }

    if (ie && ie_version >= 9) { prepareSelectAllHack(); }
    if (captureRightClick) {
      e_stop(e);
      var mouseup = function () {
        off(window, "mouseup", mouseup);
        setTimeout(rehide, 20);
      };
      on(window, "mouseup", mouseup);
    } else {
      setTimeout(rehide, 50);
    }
  };

  TextareaInput.prototype.readOnlyChanged = function (val) {
    if (!val) { this.reset(); }
    this.textarea.disabled = val == "nocursor";
  };

  TextareaInput.prototype.setUneditable = function () {};

  TextareaInput.prototype.needsContentAttribute = false;

  function fromTextArea(textarea, options) {
    options = options ? copyObj(options) : {};
    options.value = textarea.value;
    if (!options.tabindex && textarea.tabIndex)
      { options.tabindex = textarea.tabIndex; }
    if (!options.placeholder && textarea.placeholder)
      { options.placeholder = textarea.placeholder; }
    // Set autofocus to true if this textarea is focused, or if it has
    // autofocus and no other element is focused.
    if (options.autofocus == null) {
      var hasFocus = activeElt();
      options.autofocus = hasFocus == textarea ||
        textarea.getAttribute("autofocus") != null && hasFocus == document.body;
    }

    function save() {textarea.value = cm.getValue();}

    var realSubmit;
    if (textarea.form) {
      on(textarea.form, "submit", save);
      // Deplorable hack to make the submit method do the right thing.
      if (!options.leaveSubmitMethodAlone) {
        var form = textarea.form;
        realSubmit = form.submit;
        try {
          var wrappedSubmit = form.submit = function () {
            save();
            form.submit = realSubmit;
            form.submit();
            form.submit = wrappedSubmit;
          };
        } catch(e) {}
      }
    }

    options.finishInit = function (cm) {
      cm.save = save;
      cm.getTextArea = function () { return textarea; };
      cm.toTextArea = function () {
        cm.toTextArea = isNaN; // Prevent this from being ran twice
        save();
        textarea.parentNode.removeChild(cm.getWrapperElement());
        textarea.style.display = "";
        if (textarea.form) {
          off(textarea.form, "submit", save);
          if (!options.leaveSubmitMethodAlone && typeof textarea.form.submit == "function")
            { textarea.form.submit = realSubmit; }
        }
      };
    };

    textarea.style.display = "none";
    var cm = CodeMirror(function (node) { return textarea.parentNode.insertBefore(node, textarea.nextSibling); },
      options);
    return cm
  }

  function addLegacyProps(CodeMirror) {
    CodeMirror.off = off;
    CodeMirror.on = on;
    CodeMirror.wheelEventPixels = wheelEventPixels;
    CodeMirror.Doc = Doc;
    CodeMirror.splitLines = splitLinesAuto;
    CodeMirror.countColumn = countColumn;
    CodeMirror.findColumn = findColumn;
    CodeMirror.isWordChar = isWordCharBasic;
    CodeMirror.Pass = Pass;
    CodeMirror.signal = signal;
    CodeMirror.Line = Line;
    CodeMirror.changeEnd = changeEnd;
    CodeMirror.scrollbarModel = scrollbarModel;
    CodeMirror.Pos = Pos;
    CodeMirror.cmpPos = cmp;
    CodeMirror.modes = modes;
    CodeMirror.mimeModes = mimeModes;
    CodeMirror.resolveMode = resolveMode;
    CodeMirror.getMode = getMode;
    CodeMirror.modeExtensions = modeExtensions;
    CodeMirror.extendMode = extendMode;
    CodeMirror.copyState = copyState;
    CodeMirror.startState = startState;
    CodeMirror.innerMode = innerMode;
    CodeMirror.commands = commands;
    CodeMirror.keyMap = keyMap;
    CodeMirror.keyName = keyName;
    CodeMirror.isModifierKey = isModifierKey;
    CodeMirror.lookupKey = lookupKey;
    CodeMirror.normalizeKeyMap = normalizeKeyMap;
    CodeMirror.StringStream = StringStream;
    CodeMirror.SharedTextMarker = SharedTextMarker;
    CodeMirror.TextMarker = TextMarker;
    CodeMirror.LineWidget = LineWidget;
    CodeMirror.e_preventDefault = e_preventDefault;
    CodeMirror.e_stopPropagation = e_stopPropagation;
    CodeMirror.e_stop = e_stop;
    CodeMirror.addClass = addClass;
    CodeMirror.contains = contains;
    CodeMirror.rmClass = rmClass;
    CodeMirror.keyNames = keyNames;
  }

  // EDITOR CONSTRUCTOR

  defineOptions(CodeMirror);

  addEditorMethods(CodeMirror);

  // Set up methods on CodeMirror's prototype to redirect to the editor's document.
  var dontDelegate = "iter insert remove copy getEditor constructor".split(" ");
  for (var prop in Doc.prototype) { if (Doc.prototype.hasOwnProperty(prop) && indexOf(dontDelegate, prop) < 0)
    { CodeMirror.prototype[prop] = (function(method) {
      return function() {return method.apply(this.doc, arguments)}
    })(Doc.prototype[prop]); } }

  eventMixin(Doc);
  CodeMirror.inputStyles = {"textarea": TextareaInput, "contenteditable": ContentEditableInput};

  // Extra arguments are stored as the mode's dependencies, which is
  // used by (legacy) mechanisms like loadmode.js to automatically
  // load a mode. (Preferred mechanism is the require/define calls.)
  CodeMirror.defineMode = function(name/*, mode, …*/) {
    if (!CodeMirror.defaults.mode && name != "null") { CodeMirror.defaults.mode = name; }
    defineMode.apply(this, arguments);
  };

  CodeMirror.defineMIME = defineMIME;

  // Minimal default mode.
  CodeMirror.defineMode("null", function () { return ({token: function (stream) { return stream.skipToEnd(); }}); });
  CodeMirror.defineMIME("text/plain", "null");

  // EXTENSIONS

  CodeMirror.defineExtension = function (name, func) {
    CodeMirror.prototype[name] = func;
  };
  CodeMirror.defineDocExtension = function (name, func) {
    Doc.prototype[name] = func;
  };

  CodeMirror.fromTextArea = fromTextArea;

  addLegacyProps(CodeMirror);

  CodeMirror.version = "5.49.0";

  return CodeMirror;

})));
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

// Open simple dialogs on top of an editor. Relies on dialog.css.

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
  function dialogDiv(cm, template, bottom) {
    var wrap = cm.getWrapperElement();
    var dialog;
    dialog = wrap.appendChild(document.createElement("div"));
    if (bottom)
      dialog.className = "CodeMirror-dialog CodeMirror-dialog-bottom";
    else
      dialog.className = "CodeMirror-dialog CodeMirror-dialog-top";

    if (typeof template == "string") {
      dialog.innerHTML = template;
    } else { // Assuming it's a detached DOM element.
      dialog.appendChild(template);
    }
    CodeMirror.addClass(wrap, 'dialog-opened');
    return dialog;
  }

  function closeNotification(cm, newVal) {
    if (cm.state.currentNotificationClose)
      cm.state.currentNotificationClose();
    cm.state.currentNotificationClose = newVal;
  }

  CodeMirror.defineExtension("openDialog", function(template, callback, options) {
    if (!options) options = {};

    closeNotification(this, null);

    var dialog = dialogDiv(this, template, options.bottom);
    var closed = false, me = this;
    function close(newVal) {
      if (typeof newVal == 'string') {
        inp.value = newVal;
      } else {
        if (closed) return;
        closed = true;
        CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
        dialog.parentNode.removeChild(dialog);
        me.focus();

        if (options.onClose) options.onClose(dialog);
      }
    }

    var inp = dialog.getElementsByTagName("input")[0], button;
    if (inp) {
      inp.focus();

      if (options.value) {
        inp.value = options.value;
        if (options.selectValueOnOpen !== false) {
          inp.select();
        }
      }

      if (options.onInput)
        CodeMirror.on(inp, "input", function(e) { options.onInput(e, inp.value, close);});
      if (options.onKeyUp)
        CodeMirror.on(inp, "keyup", function(e) {options.onKeyUp(e, inp.value, close);});

      CodeMirror.on(inp, "keydown", function(e) {
        if (options && options.onKeyDown && options.onKeyDown(e, inp.value, close)) { return; }
        if (e.keyCode == 27 || (options.closeOnEnter !== false && e.keyCode == 13)) {
          inp.blur();
          CodeMirror.e_stop(e);
          close();
        }
        if (e.keyCode == 13) callback(inp.value, e);
      });

      if (options.closeOnBlur !== false) CodeMirror.on(inp, "blur", close);
    } else if (button = dialog.getElementsByTagName("button")[0]) {
      CodeMirror.on(button, "click", function() {
        close();
        me.focus();
      });

      if (options.closeOnBlur !== false) CodeMirror.on(button, "blur", close);

      button.focus();
    }
    return close;
  });

  CodeMirror.defineExtension("openConfirm", function(template, callbacks, options) {
    closeNotification(this, null);
    var dialog = dialogDiv(this, template, options && options.bottom);
    var buttons = dialog.getElementsByTagName("button");
    var closed = false, me = this, blurring = 1;
    function close() {
      if (closed) return;
      closed = true;
      CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
      dialog.parentNode.removeChild(dialog);
      me.focus();
    }
    buttons[0].focus();
    for (var i = 0; i < buttons.length; ++i) {
      var b = buttons[i];
      (function(callback) {
        CodeMirror.on(b, "click", function(e) {
          CodeMirror.e_preventDefault(e);
          close();
          if (callback) callback(me);
        });
      })(callbacks[i]);
      CodeMirror.on(b, "blur", function() {
        --blurring;
        setTimeout(function() { if (blurring <= 0) close(); }, 200);
      });
      CodeMirror.on(b, "focus", function() { ++blurring; });
    }
  });

  /*
   * openNotification
   * Opens a notification, that can be closed with an optional timer
   * (default 5000ms timer) and always closes on click.
   *
   * If a notification is opened while another is opened, it will close the
   * currently opened one and open the new one immediately.
   */
  CodeMirror.defineExtension("openNotification", function(template, options) {
    closeNotification(this, close);
    var dialog = dialogDiv(this, template, options && options.bottom);
    var closed = false, doneTimer;
    var duration = options && typeof options.duration !== "undefined" ? options.duration : 5000;

    function close() {
      if (closed) return;
      closed = true;
      clearTimeout(doneTimer);
      CodeMirror.rmClass(dialog.parentNode, 'dialog-opened');
      dialog.parentNode.removeChild(dialog);
    }

    CodeMirror.on(dialog, 'click', function(e) {
      CodeMirror.e_preventDefault(e);
      close();
    });

    if (duration)
      doneTimer = setTimeout(close, duration);

    return close;
  });
});
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"))
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod)
  else // Plain browser env
    mod(CodeMirror)
})(function(CodeMirror) {
  "use strict"
  var Pos = CodeMirror.Pos

  function regexpFlags(regexp) {
    var flags = regexp.flags
    return flags != null ? flags : (regexp.ignoreCase ? "i" : "")
      + (regexp.global ? "g" : "")
      + (regexp.multiline ? "m" : "")
  }

  function ensureFlags(regexp, flags) {
    var current = regexpFlags(regexp), target = current
    for (var i = 0; i < flags.length; i++) if (target.indexOf(flags.charAt(i)) == -1)
      target += flags.charAt(i)
    return current == target ? regexp : new RegExp(regexp.source, target)
  }

  function maybeMultiline(regexp) {
    return /\\s|\\n|\n|\\W|\\D|\[\^/.test(regexp.source)
  }

  function searchRegexpForward(doc, regexp, start) {
    regexp = ensureFlags(regexp, "g")
    for (var line = start.line, ch = start.ch, last = doc.lastLine(); line <= last; line++, ch = 0) {
      regexp.lastIndex = ch
      var string = doc.getLine(line), match = regexp.exec(string)
      if (match)
        return {from: Pos(line, match.index),
                to: Pos(line, match.index + match[0].length),
                match: match}
    }
  }

  function searchRegexpForwardMultiline(doc, regexp, start) {
    if (!maybeMultiline(regexp)) return searchRegexpForward(doc, regexp, start)

    regexp = ensureFlags(regexp, "gm")
    var string, chunk = 1
    for (var line = start.line, last = doc.lastLine(); line <= last;) {
      // This grows the search buffer in exponentially-sized chunks
      // between matches, so that nearby matches are fast and don't
      // require concatenating the whole document (in case we're
      // searching for something that has tons of matches), but at the
      // same time, the amount of retries is limited.
      for (var i = 0; i < chunk; i++) {
        if (line > last) break
        var curLine = doc.getLine(line++)
        string = string == null ? curLine : string + "\n" + curLine
      }
      chunk = chunk * 2
      regexp.lastIndex = start.ch
      var match = regexp.exec(string)
      if (match) {
        var before = string.slice(0, match.index).split("\n"), inside = match[0].split("\n")
        var startLine = start.line + before.length - 1, startCh = before[before.length - 1].length
        return {from: Pos(startLine, startCh),
                to: Pos(startLine + inside.length - 1,
                        inside.length == 1 ? startCh + inside[0].length : inside[inside.length - 1].length),
                match: match}
      }
    }
  }

  function lastMatchIn(string, regexp) {
    var cutOff = 0, match
    for (;;) {
      regexp.lastIndex = cutOff
      var newMatch = regexp.exec(string)
      if (!newMatch) return match
      match = newMatch
      cutOff = match.index + (match[0].length || 1)
      if (cutOff == string.length) return match
    }
  }

  function searchRegexpBackward(doc, regexp, start) {
    regexp = ensureFlags(regexp, "g")
    for (var line = start.line, ch = start.ch, first = doc.firstLine(); line >= first; line--, ch = -1) {
      var string = doc.getLine(line)
      if (ch > -1) string = string.slice(0, ch)
      var match = lastMatchIn(string, regexp)
      if (match)
        return {from: Pos(line, match.index),
                to: Pos(line, match.index + match[0].length),
                match: match}
    }
  }

  function searchRegexpBackwardMultiline(doc, regexp, start) {
    regexp = ensureFlags(regexp, "gm")
    var string, chunk = 1
    for (var line = start.line, first = doc.firstLine(); line >= first;) {
      for (var i = 0; i < chunk; i++) {
        var curLine = doc.getLine(line--)
        string = string == null ? curLine.slice(0, start.ch) : curLine + "\n" + string
      }
      chunk *= 2

      var match = lastMatchIn(string, regexp)
      if (match) {
        var before = string.slice(0, match.index).split("\n"), inside = match[0].split("\n")
        var startLine = line + before.length, startCh = before[before.length - 1].length
        return {from: Pos(startLine, startCh),
                to: Pos(startLine + inside.length - 1,
                        inside.length == 1 ? startCh + inside[0].length : inside[inside.length - 1].length),
                match: match}
      }
    }
  }

  var doFold, noFold
  if (String.prototype.normalize) {
    doFold = function(str) { return str.normalize("NFD").toLowerCase() }
    noFold = function(str) { return str.normalize("NFD") }
  } else {
    doFold = function(str) { return str.toLowerCase() }
    noFold = function(str) { return str }
  }

  // Maps a position in a case-folded line back to a position in the original line
  // (compensating for codepoints increasing in number during folding)
  function adjustPos(orig, folded, pos, foldFunc) {
    if (orig.length == folded.length) return pos
    for (var min = 0, max = pos + Math.max(0, orig.length - folded.length);;) {
      if (min == max) return min
      var mid = (min + max) >> 1
      var len = foldFunc(orig.slice(0, mid)).length
      if (len == pos) return mid
      else if (len > pos) max = mid
      else min = mid + 1
    }
  }

  function searchStringForward(doc, query, start, caseFold) {
    // Empty string would match anything and never progress, so we
    // define it to match nothing instead.
    if (!query.length) return null
    var fold = caseFold ? doFold : noFold
    var lines = fold(query).split(/\r|\n\r?/)

    search: for (var line = start.line, ch = start.ch, last = doc.lastLine() + 1 - lines.length; line <= last; line++, ch = 0) {
      var orig = doc.getLine(line).slice(ch), string = fold(orig)
      if (lines.length == 1) {
        var found = string.indexOf(lines[0])
        if (found == -1) continue search
        var start = adjustPos(orig, string, found, fold) + ch
        return {from: Pos(line, adjustPos(orig, string, found, fold) + ch),
                to: Pos(line, adjustPos(orig, string, found + lines[0].length, fold) + ch)}
      } else {
        var cutFrom = string.length - lines[0].length
        if (string.slice(cutFrom) != lines[0]) continue search
        for (var i = 1; i < lines.length - 1; i++)
          if (fold(doc.getLine(line + i)) != lines[i]) continue search
        var end = doc.getLine(line + lines.length - 1), endString = fold(end), lastLine = lines[lines.length - 1]
        if (endString.slice(0, lastLine.length) != lastLine) continue search
        return {from: Pos(line, adjustPos(orig, string, cutFrom, fold) + ch),
                to: Pos(line + lines.length - 1, adjustPos(end, endString, lastLine.length, fold))}
      }
    }
  }

  function searchStringBackward(doc, query, start, caseFold) {
    if (!query.length) return null
    var fold = caseFold ? doFold : noFold
    var lines = fold(query).split(/\r|\n\r?/)

    search: for (var line = start.line, ch = start.ch, first = doc.firstLine() - 1 + lines.length; line >= first; line--, ch = -1) {
      var orig = doc.getLine(line)
      if (ch > -1) orig = orig.slice(0, ch)
      var string = fold(orig)
      if (lines.length == 1) {
        var found = string.lastIndexOf(lines[0])
        if (found == -1) continue search
        return {from: Pos(line, adjustPos(orig, string, found, fold)),
                to: Pos(line, adjustPos(orig, string, found + lines[0].length, fold))}
      } else {
        var lastLine = lines[lines.length - 1]
        if (string.slice(0, lastLine.length) != lastLine) continue search
        for (var i = 1, start = line - lines.length + 1; i < lines.length - 1; i++)
          if (fold(doc.getLine(start + i)) != lines[i]) continue search
        var top = doc.getLine(line + 1 - lines.length), topString = fold(top)
        if (topString.slice(topString.length - lines[0].length) != lines[0]) continue search
        return {from: Pos(line + 1 - lines.length, adjustPos(top, topString, top.length - lines[0].length, fold)),
                to: Pos(line, adjustPos(orig, string, lastLine.length, fold))}
      }
    }
  }

  function SearchCursor(doc, query, pos, options) {
    this.atOccurrence = false
    this.doc = doc
    pos = pos ? doc.clipPos(pos) : Pos(0, 0)
    this.pos = {from: pos, to: pos}

    var caseFold
    if (typeof options == "object") {
      caseFold = options.caseFold
    } else { // Backwards compat for when caseFold was the 4th argument
      caseFold = options
      options = null
    }

    if (typeof query == "string") {
      if (caseFold == null) caseFold = false
      this.matches = function(reverse, pos) {
        return (reverse ? searchStringBackward : searchStringForward)(doc, query, pos, caseFold)
      }
    } else {
      query = ensureFlags(query, "gm")
      if (!options || options.multiline !== false)
        this.matches = function(reverse, pos) {
          return (reverse ? searchRegexpBackwardMultiline : searchRegexpForwardMultiline)(doc, query, pos)
        }
      else
        this.matches = function(reverse, pos) {
          return (reverse ? searchRegexpBackward : searchRegexpForward)(doc, query, pos)
        }
    }
  }

  SearchCursor.prototype = {
    findNext: function() {return this.find(false)},
    findPrevious: function() {return this.find(true)},

    find: function(reverse) {
      var result = this.matches(reverse, this.doc.clipPos(reverse ? this.pos.from : this.pos.to))

      // Implements weird auto-growing behavior on null-matches for
      // backwards-compatiblity with the vim code (unfortunately)
      while (result && CodeMirror.cmpPos(result.from, result.to) == 0) {
        if (reverse) {
          if (result.from.ch) result.from = Pos(result.from.line, result.from.ch - 1)
          else if (result.from.line == this.doc.firstLine()) result = null
          else result = this.matches(reverse, this.doc.clipPos(Pos(result.from.line - 1)))
        } else {
          if (result.to.ch < this.doc.getLine(result.to.line).length) result.to = Pos(result.to.line, result.to.ch + 1)
          else if (result.to.line == this.doc.lastLine()) result = null
          else result = this.matches(reverse, Pos(result.to.line + 1, 0))
        }
      }

      if (result) {
        this.pos = result
        this.atOccurrence = true
        return this.pos.match || true
      } else {
        var end = Pos(reverse ? this.doc.firstLine() : this.doc.lastLine() + 1, 0)
        this.pos = {from: end, to: end}
        return this.atOccurrence = false
      }
    },

    from: function() {if (this.atOccurrence) return this.pos.from},
    to: function() {if (this.atOccurrence) return this.pos.to},

    replace: function(newText, origin) {
      if (!this.atOccurrence) return
      var lines = CodeMirror.splitLines(newText)
      this.doc.replaceRange(lines, this.pos.from, this.pos.to, origin)
      this.pos.to = Pos(this.pos.from.line + lines.length - 1,
                        lines[lines.length - 1].length + (lines.length == 1 ? this.pos.from.ch : 0))
    }
  }

  CodeMirror.defineExtension("getSearchCursor", function(query, pos, caseFold) {
    return new SearchCursor(this.doc, query, pos, caseFold)
  })
  CodeMirror.defineDocExtension("getSearchCursor", function(query, pos, caseFold) {
    return new SearchCursor(this, query, pos, caseFold)
  })

  CodeMirror.defineExtension("selectMatches", function(query, caseFold) {
    var ranges = []
    var cur = this.getSearchCursor(query, this.getCursor("from"), caseFold)
    while (cur.findNext()) {
      if (CodeMirror.cmpPos(cur.to(), this.getCursor("to")) > 0) break
      ranges.push({anchor: cur.from(), head: cur.to()})
    }
    if (ranges.length)
      this.setSelections(ranges, 0)
  })
});
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

// Define search commands. Depends on dialog.js or another
// implementation of the openDialog method.

// Replace works a little oddly -- it will do the replace on the next
// Ctrl-G (or whatever is bound to findNext) press. You prevent a
// replace by making sure the match is no longer selected when hitting
// Ctrl-G.

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"), require("./searchcursor"), require("../dialog/dialog"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror", "./searchcursor", "../dialog/dialog"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
  "use strict";

  function searchOverlay(query, caseInsensitive) {
    if (typeof query == "string")
      query = new RegExp(query.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&"), caseInsensitive ? "gi" : "g");
    else if (!query.global)
      query = new RegExp(query.source, query.ignoreCase ? "gi" : "g");

    return {token: function(stream) {
      query.lastIndex = stream.pos;
      var match = query.exec(stream.string);
      if (match && match.index == stream.pos) {
        stream.pos += match[0].length || 1;
        return "searching";
      } else if (match) {
        stream.pos = match.index;
      } else {
        stream.skipToEnd();
      }
    }};
  }

  function SearchState() {
    this.posFrom = this.posTo = this.lastQuery = this.query = null;
    this.overlay = null;
  }

  function getSearchState(cm) {
    return cm.state.search || (cm.state.search = new SearchState());
  }

  function queryCaseInsensitive(query) {
    return typeof query == "string" && query == query.toLowerCase();
  }

  function getSearchCursor(cm, query, pos) {
    // Heuristic: if the query string is all lowercase, do a case insensitive search.
    return cm.getSearchCursor(query, pos, {caseFold: queryCaseInsensitive(query), multiline: true});
  }

  function persistentDialog(cm, text, deflt, onEnter, onKeyDown) {
    cm.openDialog(text, onEnter, {
      value: deflt,
      selectValueOnOpen: true,
      closeOnEnter: false,
      onClose: function() { clearSearch(cm); },
      onKeyDown: onKeyDown
    });
  }

  function dialog(cm, text, shortText, deflt, f) {
    if (cm.openDialog) cm.openDialog(text, f, {value: deflt, selectValueOnOpen: true});
    else f(prompt(shortText, deflt));
  }

  function confirmDialog(cm, text, shortText, fs) {
    if (cm.openConfirm) cm.openConfirm(text, fs);
    else if (confirm(shortText)) fs[0]();
  }

  function parseString(string) {
    return string.replace(/\\([nrt\\])/g, function(match, ch) {
      if (ch == "n") return "\n"
      if (ch == "r") return "\r"
      if (ch == "t") return "\t"
      if (ch == "\\") return "\\"
      return match
    })
  }

  function parseQuery(query) {
    var isRE = query.match(/^\/(.*)\/([a-z]*)$/);
    if (isRE) {
      try { query = new RegExp(isRE[1], isRE[2].indexOf("i") == -1 ? "" : "i"); }
      catch(e) {} // Not a regular expression after all, do a string search
    } else {
      query = parseString(query)
    }
    if (typeof query == "string" ? query == "" : query.test(""))
      query = /x^/;
    return query;
  }

  function startSearch(cm, state, query) {
    state.queryText = query;
    state.query = parseQuery(query);
    cm.removeOverlay(state.overlay, queryCaseInsensitive(state.query));
    state.overlay = searchOverlay(state.query, queryCaseInsensitive(state.query));
    cm.addOverlay(state.overlay);
    if (cm.showMatchesOnScrollbar) {
      if (state.annotate) { state.annotate.clear(); state.annotate = null; }
      state.annotate = cm.showMatchesOnScrollbar(state.query, queryCaseInsensitive(state.query));
    }
  }

  function doSearch(cm, rev, persistent, immediate) {
    var state = getSearchState(cm);
    if (state.query) return findNext(cm, rev);
    var q = cm.getSelection() || state.lastQuery;
    if (q instanceof RegExp && q.source == "x^") q = null
    if (persistent && cm.openDialog) {
      var hiding = null
      var searchNext = function(query, event) {
        CodeMirror.e_stop(event);
        if (!query) return;
        if (query != state.queryText) {
          startSearch(cm, state, query);
          state.posFrom = state.posTo = cm.getCursor();
        }
        if (hiding) hiding.style.opacity = 1
        findNext(cm, event.shiftKey, function(_, to) {
          var dialog
          if (to.line < 3 && document.querySelector &&
              (dialog = cm.display.wrapper.querySelector(".CodeMirror-dialog")) &&
              dialog.getBoundingClientRect().bottom - 4 > cm.cursorCoords(to, "window").top)
            (hiding = dialog).style.opacity = .4
        })
      };
      persistentDialog(cm, getQueryDialog(cm), q, searchNext, function(event, query) {
        var keyName = CodeMirror.keyName(event)
        var extra = cm.getOption('extraKeys'), cmd = (extra && extra[keyName]) || CodeMirror.keyMap[cm.getOption("keyMap")][keyName]
        if (cmd == "findNext" || cmd == "findPrev" ||
          cmd == "findPersistentNext" || cmd == "findPersistentPrev") {
          CodeMirror.e_stop(event);
          startSearch(cm, getSearchState(cm), query);
          cm.execCommand(cmd);
        } else if (cmd == "find" || cmd == "findPersistent") {
          CodeMirror.e_stop(event);
          searchNext(query, event);
        }
      });
      if (immediate && q) {
        startSearch(cm, state, q);
        findNext(cm, rev);
      }
    } else {
      dialog(cm, getQueryDialog(cm), "Search for:", q, function(query) {
        if (query && !state.query) cm.operation(function() {
          startSearch(cm, state, query);
          state.posFrom = state.posTo = cm.getCursor();
          findNext(cm, rev);
        });
      });
    }
  }

  function findNext(cm, rev, callback) {cm.operation(function() {
    var state = getSearchState(cm);
    var cursor = getSearchCursor(cm, state.query, rev ? state.posFrom : state.posTo);
    if (!cursor.find(rev)) {
      cursor = getSearchCursor(cm, state.query, rev ? CodeMirror.Pos(cm.lastLine()) : CodeMirror.Pos(cm.firstLine(), 0));
      if (!cursor.find(rev)) return;
    }
    cm.setSelection(cursor.from(), cursor.to());
    cm.scrollIntoView({from: cursor.from(), to: cursor.to()}, 20);
    state.posFrom = cursor.from(); state.posTo = cursor.to();
    if (callback) callback(cursor.from(), cursor.to())
  });}

  function clearSearch(cm) {cm.operation(function() {
    var state = getSearchState(cm);
    state.lastQuery = state.query;
    if (!state.query) return;
    state.query = state.queryText = null;
    cm.removeOverlay(state.overlay);
    if (state.annotate) { state.annotate.clear(); state.annotate = null; }
  });}


  function getQueryDialog(cm)  {
    return '<span class="CodeMirror-search-label">' + cm.phrase("Search:") + '</span> <input type="text" style="width: 10em" class="CodeMirror-search-field"/> <span style="color: #888" class="CodeMirror-search-hint">' + cm.phrase("(Use /re/ syntax for regexp search)") + '</span>';
  }
  function getReplaceQueryDialog(cm) {
    return ' <input type="text" style="width: 10em" class="CodeMirror-search-field"/> <span style="color: #888" class="CodeMirror-search-hint">' + cm.phrase("(Use /re/ syntax for regexp search)") + '</span>';
  }
  function getReplacementQueryDialog(cm) {
    return '<span class="CodeMirror-search-label">' + cm.phrase("With:") + '</span> <input type="text" style="width: 10em" class="CodeMirror-search-field"/>';
  }
  function getDoReplaceConfirm(cm) {
    return '<span class="CodeMirror-search-label">' + cm.phrase("Replace?") + '</span> <button>' + cm.phrase("Yes") + '</button> <button>' + cm.phrase("No") + '</button> <button>' + cm.phrase("All") + '</button> <button>' + cm.phrase("Stop") + '</button> ';
  }

  function replaceAll(cm, query, text) {
    cm.operation(function() {
      for (var cursor = getSearchCursor(cm, query); cursor.findNext();) {
        if (typeof query != "string") {
          var match = cm.getRange(cursor.from(), cursor.to()).match(query);
          cursor.replace(text.replace(/\$(\d)/g, function(_, i) {return match[i];}));
        } else cursor.replace(text);
      }
    });
  }

  function replace(cm, all) {
    if (cm.getOption("readOnly")) return;
    var query = cm.getSelection() || getSearchState(cm).lastQuery;
    var dialogText = '<span class="CodeMirror-search-label">' + (all ? cm.phrase("Replace all:") : cm.phrase("Replace:")) + '</span>';
    dialog(cm, dialogText + getReplaceQueryDialog(cm), dialogText, query, function(query) {
      if (!query) return;
      query = parseQuery(query);
      dialog(cm, getReplacementQueryDialog(cm), cm.phrase("Replace with:"), "", function(text) {
        text = parseString(text)
        if (all) {
          replaceAll(cm, query, text)
        } else {
          clearSearch(cm);
          var cursor = getSearchCursor(cm, query, cm.getCursor("from"));
          var advance = function() {
            var start = cursor.from(), match;
            if (!(match = cursor.findNext())) {
              cursor = getSearchCursor(cm, query);
              if (!(match = cursor.findNext()) ||
                  (start && cursor.from().line == start.line && cursor.from().ch == start.ch)) return;
            }
            cm.setSelection(cursor.from(), cursor.to());
            cm.scrollIntoView({from: cursor.from(), to: cursor.to()});
            confirmDialog(cm, getDoReplaceConfirm(cm), cm.phrase("Replace?"),
                          [function() {doReplace(match);}, advance,
                           function() {replaceAll(cm, query, text)}]);
          };
          var doReplace = function(match) {
            cursor.replace(typeof query == "string" ? text :
                           text.replace(/\$(\d)/g, function(_, i) {return match[i];}));
            advance();
          };
          advance();
        }
      });
    });
  }

  CodeMirror.commands.find = function(cm) {clearSearch(cm); doSearch(cm);};
  CodeMirror.commands.findPersistent = function(cm) {clearSearch(cm); doSearch(cm, false, true);};
  CodeMirror.commands.findPersistentNext = function(cm) {doSearch(cm, false, true, true);};
  CodeMirror.commands.findPersistentPrev = function(cm) {doSearch(cm, true, true, true);};
  CodeMirror.commands.findNext = doSearch;
  CodeMirror.commands.findPrev = function(cm) {doSearch(cm, true);};
  CodeMirror.commands.clearSearch = clearSearch;
  CodeMirror.commands.replace = replace;
  CodeMirror.commands.replaceAll = function(cm) {replace(cm, true);};
});
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
  "use strict";

  function doFold(cm, pos, options, force) {
    if (options && options.call) {
      var finder = options;
      options = null;
    } else {
      var finder = getOption(cm, options, "rangeFinder");
    }
    if (typeof pos == "number") pos = CodeMirror.Pos(pos, 0);
    var minSize = getOption(cm, options, "minFoldSize");

    function getRange(allowFolded) {
      var range = finder(cm, pos);
      if (!range || range.to.line - range.from.line < minSize) return null;
      var marks = cm.findMarksAt(range.from);
      for (var i = 0; i < marks.length; ++i) {
        if (marks[i].__isFold && force !== "fold") {
          if (!allowFolded) return null;
          range.cleared = true;
          marks[i].clear();
        }
      }
      return range;
    }

    var range = getRange(true);
    if (getOption(cm, options, "scanUp")) while (!range && pos.line > cm.firstLine()) {
      pos = CodeMirror.Pos(pos.line - 1, 0);
      range = getRange(false);
    }
    if (!range || range.cleared || force === "unfold") return;

    var myWidget = makeWidget(cm, options);
    CodeMirror.on(myWidget, "mousedown", function(e) {
      myRange.clear();
      CodeMirror.e_preventDefault(e);
    });
    var myRange = cm.markText(range.from, range.to, {
      replacedWith: myWidget,
      clearOnEnter: getOption(cm, options, "clearOnEnter"),
      __isFold: true
    });
    myRange.on("clear", function(from, to) {
      CodeMirror.signal(cm, "unfold", cm, from, to);
    });
    CodeMirror.signal(cm, "fold", cm, range.from, range.to);
  }

  function makeWidget(cm, options) {
    var widget = getOption(cm, options, "widget");
    if (typeof widget == "string") {
      var text = document.createTextNode(widget);
      widget = document.createElement("span");
      widget.appendChild(text);
      widget.className = "CodeMirror-foldmarker";
    } else if (widget) {
      widget = widget.cloneNode(true)
    }
    return widget;
  }

  // Clumsy backwards-compatible interface
  CodeMirror.newFoldFunction = function(rangeFinder, widget) {
    return function(cm, pos) { doFold(cm, pos, {rangeFinder: rangeFinder, widget: widget}); };
  };

  // New-style interface
  CodeMirror.defineExtension("foldCode", function(pos, options, force) {
    doFold(this, pos, options, force);
  });

  CodeMirror.defineExtension("isFolded", function(pos) {
    var marks = this.findMarksAt(pos);
    for (var i = 0; i < marks.length; ++i)
      if (marks[i].__isFold) return true;
  });

  CodeMirror.commands.toggleFold = function(cm) {
    cm.foldCode(cm.getCursor());
  };
  CodeMirror.commands.fold = function(cm) {
    cm.foldCode(cm.getCursor(), null, "fold");
  };
  CodeMirror.commands.unfold = function(cm) {
    cm.foldCode(cm.getCursor(), null, "unfold");
  };
  CodeMirror.commands.foldAll = function(cm) {
    cm.operation(function() {
      for (var i = cm.firstLine(), e = cm.lastLine(); i <= e; i++)
        cm.foldCode(CodeMirror.Pos(i, 0), null, "fold");
    });
  };
  CodeMirror.commands.unfoldAll = function(cm) {
    cm.operation(function() {
      for (var i = cm.firstLine(), e = cm.lastLine(); i <= e; i++)
        cm.foldCode(CodeMirror.Pos(i, 0), null, "unfold");
    });
  };

  CodeMirror.registerHelper("fold", "combine", function() {
    var funcs = Array.prototype.slice.call(arguments, 0);
    return function(cm, start) {
      for (var i = 0; i < funcs.length; ++i) {
        var found = funcs[i](cm, start);
        if (found) return found;
      }
    };
  });

  CodeMirror.registerHelper("fold", "auto", function(cm, start) {
    var helpers = cm.getHelpers(start, "fold");
    for (var i = 0; i < helpers.length; i++) {
      var cur = helpers[i](cm, start);
      if (cur) return cur;
    }
  });

  var defaultOptions = {
    rangeFinder: CodeMirror.fold.auto,
    widget: "\u2194",
    minFoldSize: 0,
    scanUp: false,
    clearOnEnter: true
  };

  CodeMirror.defineOption("foldOptions", null);

  function getOption(cm, options, name) {
    if (options && options[name] !== undefined)
      return options[name];
    var editorOptions = cm.options.foldOptions;
    if (editorOptions && editorOptions[name] !== undefined)
      return editorOptions[name];
    return defaultOptions[name];
  }

  CodeMirror.defineExtension("foldOption", function(options, name) {
    return getOption(this, options, name);
  });
});
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"), require("./foldcode"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror", "./foldcode"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
  "use strict";

  CodeMirror.defineOption("foldGutter", false, function(cm, val, old) {
    if (old && old != CodeMirror.Init) {
      cm.clearGutter(cm.state.foldGutter.options.gutter);
      cm.state.foldGutter = null;
      cm.off("gutterClick", onGutterClick);
      cm.off("changes", onChange);
      cm.off("viewportChange", onViewportChange);
      cm.off("fold", onFold);
      cm.off("unfold", onFold);
      cm.off("swapDoc", onChange);
    }
    if (val) {
      cm.state.foldGutter = new State(parseOptions(val));
      updateInViewport(cm);
      cm.on("gutterClick", onGutterClick);
      cm.on("changes", onChange);
      cm.on("viewportChange", onViewportChange);
      cm.on("fold", onFold);
      cm.on("unfold", onFold);
      cm.on("swapDoc", onChange);
    }
  });

  var Pos = CodeMirror.Pos;

  function State(options) {
    this.options = options;
    this.from = this.to = 0;
  }

  function parseOptions(opts) {
    if (opts === true) opts = {};
    if (opts.gutter == null) opts.gutter = "CodeMirror-foldgutter";
    if (opts.indicatorOpen == null) opts.indicatorOpen = "CodeMirror-foldgutter-open";
    if (opts.indicatorFolded == null) opts.indicatorFolded = "CodeMirror-foldgutter-folded";
    return opts;
  }

  function isFolded(cm, line) {
    var marks = cm.findMarks(Pos(line, 0), Pos(line + 1, 0));
    for (var i = 0; i < marks.length; ++i) {
      if (marks[i].__isFold) {
        var fromPos = marks[i].find(-1);
        if (fromPos && fromPos.line === line)
          return marks[i];
      }
    }
  }

  function marker(spec) {
    if (typeof spec == "string") {
      var elt = document.createElement("div");
      elt.className = spec + " CodeMirror-guttermarker-subtle";
      return elt;
    } else {
      return spec.cloneNode(true);
    }
  }

  function updateFoldInfo(cm, from, to) {
    var opts = cm.state.foldGutter.options, cur = from;
    var minSize = cm.foldOption(opts, "minFoldSize");
    var func = cm.foldOption(opts, "rangeFinder");
    cm.eachLine(from, to, function(line) {
      var mark = null;
      if (isFolded(cm, cur)) {
        mark = marker(opts.indicatorFolded);
      } else {
        var pos = Pos(cur, 0);
        var range = func && func(cm, pos);
        if (range && range.to.line - range.from.line >= minSize)
          mark = marker(opts.indicatorOpen);
      }
      cm.setGutterMarker(line, opts.gutter, mark);
      ++cur;
    });
  }

  function updateInViewport(cm) {
    var vp = cm.getViewport(), state = cm.state.foldGutter;
    if (!state) return;
    cm.operation(function() {
      updateFoldInfo(cm, vp.from, vp.to);
    });
    state.from = vp.from; state.to = vp.to;
  }

  function onGutterClick(cm, line, gutter) {
    var state = cm.state.foldGutter;
    if (!state) return;
    var opts = state.options;
    if (gutter != opts.gutter) return;
    var folded = isFolded(cm, line);
    if (folded) folded.clear();
    else cm.foldCode(Pos(line, 0), opts);
  }

  function onChange(cm) {
    var state = cm.state.foldGutter;
    if (!state) return;
    var opts = state.options;
    state.from = state.to = 0;
    clearTimeout(state.changeUpdate);
    state.changeUpdate = setTimeout(function() { updateInViewport(cm); }, opts.foldOnChangeTimeSpan || 600);
  }

  function onViewportChange(cm) {
    var state = cm.state.foldGutter;
    if (!state) return;
    var opts = state.options;
    clearTimeout(state.changeUpdate);
    state.changeUpdate = setTimeout(function() {
      var vp = cm.getViewport();
      if (state.from == state.to || vp.from - state.to > 20 || state.from - vp.to > 20) {
        updateInViewport(cm);
      } else {
        cm.operation(function() {
          if (vp.from < state.from) {
            updateFoldInfo(cm, vp.from, state.from);
            state.from = vp.from;
          }
          if (vp.to > state.to) {
            updateFoldInfo(cm, state.to, vp.to);
            state.to = vp.to;
          }
        });
      }
    }, opts.updateViewportTimeSpan || 400);
  }

  function onFold(cm, from) {
    var state = cm.state.foldGutter;
    if (!state) return;
    var line = from.line;
    if (line >= state.from && line < state.to)
      updateFoldInfo(cm, line, line + 1);
  }
});
//</script>
//   <script>// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: https://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
"use strict";

CodeMirror.registerHelper("fold", "brace", function(cm, start) {
  var line = start.line, lineText = cm.getLine(line);
  var tokenType;

  function findOpening(openCh) {
    for (var at = start.ch, pass = 0;;) {
      var found = at <= 0 ? -1 : lineText.lastIndexOf(openCh, at - 1);
      if (found == -1) {
        if (pass == 1) break;
        pass = 1;
        at = lineText.length;
        continue;
      }
      if (pass == 1 && found < start.ch) break;
      tokenType = cm.getTokenTypeAt(CodeMirror.Pos(line, found + 1));
      if (!/^(comment|string)/.test(tokenType)) return found + 1;
      at = found - 1;
    }
  }

  var startToken = "{", endToken = "}", startCh = findOpening("{");
  if (startCh == null) {
    startToken = "[", endToken = "]";
    startCh = findOpening("[");
  }

  if (startCh == null) return;
  var count = 1, lastLine = cm.lastLine(), end, endCh;
  outer: for (var i = line; i <= lastLine; ++i) {
    var text = cm.getLine(i), pos = i == line ? startCh : 0;
    for (;;) {
      var nextOpen = text.indexOf(startToken, pos), nextClose = text.indexOf(endToken, pos);
      if (nextOpen < 0) nextOpen = text.length;
      if (nextClose < 0) nextClose = text.length;
      pos = Math.min(nextOpen, nextClose);
      if (pos == text.length) break;
      if (cm.getTokenTypeAt(CodeMirror.Pos(i, pos + 1)) == tokenType) {
        if (pos == nextOpen) ++count;
        else if (!--count) { end = i; endCh = pos; break outer; }
      }
      ++pos;
    }
  }
  if (end == null || line == end) return;
  return {from: CodeMirror.Pos(line, startCh),
          to: CodeMirror.Pos(end, endCh)};
});

CodeMirror.registerHelper("fold", "import", function(cm, start) {
  function hasImport(line) {
    if (line < cm.firstLine() || line > cm.lastLine()) return null;
    var start = cm.getTokenAt(CodeMirror.Pos(line, 1));
    if (!/\S/.test(start.string)) start = cm.getTokenAt(CodeMirror.Pos(line, start.end + 1));
    if (start.type != "keyword" || start.string != "import") return null;
    // Now find closing semicolon, return its position
    for (var i = line, e = Math.min(cm.lastLine(), line + 10); i <= e; ++i) {
      var text = cm.getLine(i), semi = text.indexOf(";");
      if (semi != -1) return {startCh: start.end, end: CodeMirror.Pos(i, semi)};
    }
  }

  var startLine = start.line, has = hasImport(startLine), prev;
  if (!has || hasImport(startLine - 1) || ((prev = hasImport(startLine - 2)) && prev.end.line == startLine - 1))
    return null;
  for (var end = has.end;;) {
    var next = hasImport(end.line + 1);
    if (next == null) break;
    end = next.end;
  }
  return {from: cm.clipPos(CodeMirror.Pos(startLine, has.startCh + 1)), to: end};
});

CodeMirror.registerHelper("fold", "include", function(cm, start) {
  function hasInclude(line) {
    if (line < cm.firstLine() || line > cm.lastLine()) return null;
    var start = cm.getTokenAt(CodeMirror.Pos(line, 1));
    if (!/\S/.test(start.string)) start = cm.getTokenAt(CodeMirror.Pos(line, start.end + 1));
    if (start.type == "meta" && start.string.slice(0, 8) == "#include") return start.start + 8;
  }

  var startLine = start.line, has = hasInclude(startLine);
  if (has == null || hasInclude(startLine - 1) != null) return null;
  for (var end = startLine;;) {
    var next = hasInclude(end + 1);
    if (next == null) break;
    ++end;
  }
  return {from: CodeMirror.Pos(startLine, has + 1),
          to: cm.clipPos(CodeMirror.Pos(end))};
});

});
//</script>
//   <script>
CodeMirror.defineMode("glsl", function(config, parserConfig) {
  var indentUnit = config.indentUnit,
      keywords = parserConfig.keywords || {},
      builtins = parserConfig.builtins || {},
      blockKeywords = parserConfig.blockKeywords || {},
      atoms = parserConfig.atoms || {},
      hooks = parserConfig.hooks || {},
      multiLineStrings = parserConfig.multiLineStrings;
  var isOperatorChar = /[+\-*&%=<>!?|\/]/;

  var curPunc;

  function tokenBase(stream, state) {
    var ch = stream.next();
    if (hooks[ch]) {
      var result = hooks[ch](stream, state);
      if (result !== false) return result;
    }
    if (ch == '"' || ch == "'") {
      state.tokenize = tokenString(ch);
      return state.tokenize(stream, state);
    }
    if (/[\[\]{}\(\),;\:\.]/.test(ch)) {
      curPunc = ch;
      return "bracket";
    }
    if (/\d/.test(ch)) {
      stream.eatWhile(/[\w\.]/);
      return "number";
    }
    if (ch == "/") {
      if (stream.eat("*")) {
        state.tokenize = tokenComment;
        return tokenComment(stream, state);
      }
      if (stream.eat("/")) {
        stream.skipToEnd();
        return "comment";
      }
    }
    if (isOperatorChar.test(ch)) {
      stream.eatWhile(isOperatorChar);
      return "operator";
    }
    stream.eatWhile(/[\w\$_]/);
    var cur = stream.current();
    if (keywords.propertyIsEnumerable(cur)) {
      if (blockKeywords.propertyIsEnumerable(cur)) curPunc = "newstatement";
      return "keyword";
    }
    if (builtins.propertyIsEnumerable(cur)) {
      return "builtin";
    }
    if (atoms.propertyIsEnumerable(cur)) return "atom";
    return "word";
  }

  function tokenString(quote) {
    return function(stream, state) {
      var escaped = false, next, end = false;
      while ((next = stream.next()) != null) {
        if (next == quote && !escaped) {end = true; break;}
        escaped = !escaped && next == "\\";
      }
      if (end || !(escaped || multiLineStrings))
        state.tokenize = tokenBase;
      return "string";
    };
  }

  function tokenComment(stream, state) {
    var maybeEnd = false, ch;
    while (ch = stream.next()) {
      if (ch == "/" && maybeEnd) {
        state.tokenize = tokenBase;
        break;
      }
      maybeEnd = (ch == "*");
    }
    return "comment";
  }

  function Context(indented, column, type, align, prev) {
    this.indented = indented;
    this.column = column;
    this.type = type;
    this.align = align;
    this.prev = prev;
  }
  function pushContext(state, col, type) {
    return state.context = new Context(state.indented, col, type, null, state.context);
  }
  function popContext(state) {
    var t = state.context.type;
    if (t == ")" || t == "]" || t == "}")
      state.indented = state.context.indented;
    return state.context = state.context.prev;
  }

  // Interface

  return {
    startState: function(basecolumn) {
      return {
        tokenize: null,
        context: new Context((basecolumn || 0) - indentUnit, 0, "top", false),
        indented: 0,
        startOfLine: true
      };
    },

    token: function(stream, state) {
      var ctx = state.context;
      if (stream.sol()) {
        if (ctx.align == null) ctx.align = false;
        state.indented = stream.indentation();
        state.startOfLine = true;
      }
      if (stream.eatSpace()) return null;
      curPunc = null;
      var style = (state.tokenize || tokenBase)(stream, state);
      if (style == "comment" || style == "meta") return style;
      if (ctx.align == null) ctx.align = true;

      if ((curPunc == ";" || curPunc == ":") && ctx.type == "statement") popContext(state);
      else if (curPunc == "{") pushContext(state, stream.column(), "}");
      else if (curPunc == "[") pushContext(state, stream.column(), "]");
      else if (curPunc == "(") pushContext(state, stream.column(), ")");
      else if (curPunc == "}") {
        while (ctx.type == "statement") ctx = popContext(state);
        if (ctx.type == "}") ctx = popContext(state);
        while (ctx.type == "statement") ctx = popContext(state);
      }
      else if (curPunc == ctx.type) popContext(state);
      else if (ctx.type == "}" || ctx.type == "top" || (ctx.type == "statement" && curPunc == "newstatement"))
        pushContext(state, stream.column(), "statement");
      state.startOfLine = false;
      return style;
    },

    indent: function(state, textAfter) {
      if (state.tokenize != tokenBase && state.tokenize != null) return 0;
      var firstChar = textAfter && textAfter.charAt(0), ctx = state.context, closing = firstChar == ctx.type;
      if (ctx.type == "statement") return ctx.indented + (firstChar == "{" ? 0 : indentUnit);
      else if (ctx.align) return ctx.column + (closing ? 0 : 1);
      else return ctx.indented + (closing ? 0 : indentUnit);
    },

    electricChars: "{}",
    fold: "brace"
  };
});

(function() {
  function words(str) {
    var obj = {}, words = str.split(" ");
    for (var i = 0; i < words.length; ++i) obj[words[i]] = true;
    return obj;
  }
  var glslKeywords = "const uniform break continue " +
    "do for while if else switch case in out inout float int uint void bool true false " +
    "invariant discard return mat2 mat3 mat2x2 mat2x3 mat2x4 mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 " +
    "mat4 vec2 vec3 vec4 ivec2 ivec3 ivec4 uvec2 uvec3 uvec4 bvec2 bvec3 bvec4 sampler2D " +
    "samplerCube sampler3D struct";
  var glslBuiltins = "radians degrees sin cos tan asin acos atan pow sinh cosh tanh asinh acosh atanh " +
    "exp log exp2 log2 sqrt inversesqrt abs sign floor ceil round roundEven trunc fract mod modf " +
    "min max clamp mix step smoothstep length distance dot cross " +
    "determinant inverse normalize faceforward reflect refract matrixCompMult outerProduct transpose lessThan " +
    "lessThanEqual greaterThan greaterThanEqual equal notEqual any all not packUnorm2x16 unpackUnorm2x16 packSnorm2x16 unpackSnorm2x16 packHalf2x16 unpackHalf2x16 " +
    "dFdx dFdy fwidth textureSize texture textureProj textureLod textureGrad texelFetch texelFetchOffset " +
    "textureProjLod textureLodOffset textureGradOffset textureProjLodOffset textureProjGrad intBitsToFloat uintBitsToFloat floatBitsToInt floatBitsToUint isnan isinf";

  function cppHook(stream, state) {
    if (!state.startOfLine) return false;
    stream.skipToEnd();
    return "meta";
  }

  // C#-style strings where "" escapes a quote.
  function tokenAtString(stream, state) {
    var next;
    while ((next = stream.next()) != null) {
      if (next == '"' && !stream.eat('"')) {
        state.tokenize = null;
        break;
      }
    }
    return "string";
  }

  CodeMirror.defineMIME("text/x-glsl", {
    name: "glsl",
    keywords: words(glslKeywords),
    builtins: words(glslBuiltins),
    blockKeywords: words("case do else for if switch while struct"),
    atoms: words("null"),
    hooks: {"#": cppHook}
  });
}());
//</script>
//   <script>"use strict";

class SoundCloud 
{
    resolve(url, callback, errcallback) 
    {
        var url2 = "surl=" + url;
        var mHttpReq = new XMLHttpRequest();
        mHttpReq.open( "POST", "/shadertoy", true );
        mHttpReq.responseType = "json";
        mHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        mHttpReq.onload = function()
        {
            var jsn = mHttpReq.response;
            if( jsn===null ) errcallback(0);
            if( jsn.result!==0 )
            {
                callback(JSON.parse(jsn.soundcloud));
            } else {
                errcallback(0);
            }
        };
        mHttpReq.onerror = function()
        {
            errcallback(0);
        };
        mHttpReq.send( url2 );
    }
}

const SC = new SoundCloud();
//</script>
//   <script>"use strict"

function bbc2html( content, allowMultimedia )
{
    //content = content.replace(new RegExp('\r?\n','g'), '<br />');

    content = content.replace( /(\[url=)javascript:?(.*?)(\])(.*?)(\[\/url\])/gi, '');
    content = content.replace( /(\[url\])javascript:?(.*?)(\[\/url\])/gi,         '');
    content = content.replace( /(\[img\])javascript:?(.*?)(\[\/img\])/gi,         '');

    content = content.replace( /(\[url=)(.*?)(\])(.*?)(\[\/url\])/gi, '<a href="$2" class="regular" target="_blank">$4</a>');
    content = content.replace( /(\[url\])(.*?)(\[\/url\])/gi,         '<a href="$2" class="regular" target="_blank">$2</a>');

    content = content.replace( /(:\))/g,                  '<img src="/img/emoticonHappy.png">'   );
    content = content.replace( /(:\()/g,                  '<img src="/img/emoticonSad.png">'     );
    content = content.replace( /(:D)/g,                   '<img src="/img/emoticonLaugh.png">'   );
    content = content.replace( /(:love:)/gi,               '<img src="/img/emoticonLove.png">'    );
    content = content.replace( /(:octopus:)/gi,            '<img src="/img/emoticonOctopus.png">' );
    content = content.replace( /(:octopusballoon:)/gi,     '<img src="/img/emoticonOctopusBalloon.png">' );
    content = content.replace( /(:alpha:)/gi,              '&#945;'  );
    content = content.replace( /(:beta:)/gi,               '&#946;'  );
    content = content.replace( /(:delta:)/gi,              '&#9169;' );
    content = content.replace( /(:epsilon:)/gi,            '&#949;'  );
    content = content.replace( /(:nabla:)/gi,              '&#8711;' );
    content = content.replace( /(:square:)/gi,             '&#178;'  );
    content = content.replace( /(:sube:)/gi,               '&#179;'  );
    content = content.replace( /(:limit:)/gi,              '&#8784;' );

    content = content.replace( /(\[b\])([.\s\S]*?)(\[\/b\])/gi,       '<strong>$2</strong>' );
    content = content.replace( /(\[i\])([.\s\S]*?)(\[\/i\])/gi,       '<em>$2</em>'         );
    content = content.replace( /(\[u\])([.\s\S]*?)(\[\/u\])/gi,       '<u>$2</u>'           );
    content = content.replace( /(\[ul\])([.\s\S]*?)(\[\/ul\])/gi,     '<ul>$2</ul>'         );
    content = content.replace( /(\[li\])([.\s\S]*?)(\[\/li\])/gi,     '<li>$2</li>'         );
    content = content.replace( /(\[code\])([.\s\S]*?)(\[\/code\])/gi, '<pre>$2</pre>'       );

    if( allowMultimedia )
    {
        content = content.replace( /(\[img\])(.*?)(\[\/img\])/gi, '<a href="$2"><img src="$2" style="max-width:100%;"/></a>' );
        content = content.replace( /(\[video\])(?:http|https|)(?::\/\/|)(?:www.|)(?:youtu\.be\/|youtube\.com(?:\/embed\/|\/v\/|\/watch\?v=|\/ytscreeningroom\?v=|\/feeds\/api\/videos\/|\/user\S*[^\w\-\s]|\S*[^\w\-\s]))([\w\-]{11})(.*?)(\[\/video\])/gi, '<iframe width="100%" height="360" src="https://www.youtube.com/embed/$2?hd=1" frameborder="0" allowfullscreen></iframe>' );
    }

    return content;
}//</script>
//   <script>"use strict"

const kMaxCompileTime = 10.0;

function iReportCrash(shaderID)
{
    let req = new XMLHttpRequest();
    req.onload = function ()
    {
        let jsn = req.response;
        if (jsn === null) return;
        if (jsn.result === 0)
        {
            // yep
        }
    };
    req.open("POST", "/shadertoy", true);
    req.responseType = "json";
    req.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    req.send( "s=" + shaderID + "&r=2" );
}
//</script>
//   <script>"use strict"

function bufferID_to_assetID( id )
{
    if( id===0 ) return '4dXGR8';
    if( id===1 ) return 'XsXGR8';
    if( id===2 ) return '4sXGR8';
    if( id===3 ) return 'XdfGR8';
    return 'none';
}
function assetID_to_bufferID( id )
{
    if( id==='4dXGR8' ) return 0;
    if( id==='XsXGR8' ) return 1;
    if( id==='4sXGR8' ) return 2;
    if( id==='XdfGR8' ) return 3;
    return -1;
}

function assetID_to_cubemapBuferID( id )
{
    if( id==='4dX3Rr' ) return 0;
    return -1;
}
function cubamepBufferID_to_assetID( id )
{
    if( id===0 ) return '4dX3Rr';
    return 'none';
}

function EffectPass( renderer, is20, isLowEnd, hasShaderTextureLOD, callback, obj, forceMuted, forcePaused, outputGainNode, copyProgram, id, effect  )
{
    this.mID = id;
    this.mInputs  = [null, null, null, null ];
    this.mOutputs = [null, null, null, null ];
    this.mSource = null;

    this.mGainNode = outputGainNode;
    this.mSoundShaderCompiled = false;

    this.mEffect = effect;
    this.mRenderer = renderer;
    this.mProgramCopy = copyProgram; 
    this.mCompilationTime = 0;

    this.mType = "none";
    this.mName = "none";
    this.mFrame = 0;

    this.mShaderTextureLOD = hasShaderTextureLOD;
    this.mIs20 = is20;
    this.mIsLowEnd = isLowEnd;
    this.mTextureCallbackFun = callback;
    this.mTextureCallbackObj = obj;
    this.mForceMuted = forceMuted;
    this.mForcePaused = forcePaused;
}

EffectPass.prototype.MakeHeader_Image = function()
{
    let header = "";

    header += "#define HW_PERFORMANCE " + ((this.mIsLowEnd===true)?"0":"1") + "\n";

    header += "uniform vec3      iResolution;\n" +
              "uniform float     iTime;\n" +
              "uniform float     iChannelTime[4];\n" +
              "uniform vec4      iMouse;\n" +
              "uniform vec4      iDate;\n" +
              "uniform float     iSampleRate;\n" +
              "uniform vec3      iChannelResolution[4];\n" +
              "uniform int       iFrame;\n" +
              "uniform float     iTimeDelta;\n" +
              "uniform float     iFrameRate;\n";

    for( let i=0; i<this.mInputs.length; i++ )
    {
        let inp = this.mInputs[i];

        // old API
             if( inp===null )                  header += "uniform sampler2D iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="cubemap" ) header += "uniform samplerCube iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="volume"  ) header += "uniform sampler3D iChannel" + i + ";\n";
        else                                   header += "uniform sampler2D iChannel" + i + ";\n";

        // new API (see shadertoy.com/view/wtdGW8)
        header += "uniform struct {\n";
             if( inp===null )                  header += "  sampler2D";
        else if( inp.mInfo.mType==="cubemap" ) header += "  samplerCube";
        else if( inp.mInfo.mType==="volume"  ) header += "  sampler3D";
        else                                  header += "  sampler2D";
        header +=        " sampler;\n";
        header += "  vec3  size;\n";
        header += "  float time;\n";
        header += "  int   loaded;\n";
        header += "}iCh" + i + ";\n";
    }
	header += "void mainImage( out vec4 c, in vec2 f );\n";
    header += "void st_assert( bool cond );\n";
    header += "void st_assert( bool cond, int v );\n";

    if( this.mIs20 ) 
    {
        header += "\nout vec4 shadertoy_out_color;\n" +
        "void st_assert( bool cond, int v ) {if(!cond){if(v==0)shadertoy_out_color.x=-1.0;else if(v==1)shadertoy_out_color.y=-1.0;else if(v==2)shadertoy_out_color.z=-1.0;else shadertoy_out_color.w=-1.0;}}\n" +
        "void st_assert( bool cond        ) {if(!cond)shadertoy_out_color.x=-1.0;}\n" +
        "void main( void )" +
        "{" +
            "shadertoy_out_color = vec4(1.0,1.0,1.0,1.0);" + 
            "vec4 color = vec4(1e20);" +
            "mainImage( color, gl_FragCoord.xy );" +
            "if(shadertoy_out_color.x<0.0) color=vec4(1.0,0.0,0.0,1.0);" +
            "if(shadertoy_out_color.y<0.0) color=vec4(0.0,1.0,0.0,1.0);" +
            "if(shadertoy_out_color.z<0.0) color=vec4(0.0,0.0,1.0,1.0);" +
            "if(shadertoy_out_color.w<0.0) color=vec4(1.0,1.0,0.0,1.0);" +
            "shadertoy_out_color = vec4(color.xyz,1.0);" +
        "}";
    }
    else
    {
        header += "" +
        "void st_assert( bool cond, int v ) {if(!cond){if(v==0)gl_FragColor.x=-1.0;else if(v==1)gl_FragColor.y=-1.0;else if(v==2)gl_FragColor.z=-1.0;else gl_FragColor.w=-1.0;}}\n" +
        "void st_assert( bool cond        ) {if(!cond)gl_FragColor.x=-1.0;}\n" +
        "void main( void )" +
        "{" +
            "gl_FragColor = vec4(0.0,0.0,0.0,1.0);" + 
            "vec4 color = vec4(1e20);" +
            "mainImage( color, gl_FragCoord.xy );" +
            "color.w = 1.0;" +
            "if(gl_FragColor.w<0.0) color=vec4(1.0,0.0,0.0,1.0);" +
            "if(gl_FragColor.x<0.0) color=vec4(1.0,0.0,0.0,1.0);" +
            "if(gl_FragColor.y<0.0) color=vec4(0.0,1.0,0.0,1.0);" +
            "if(gl_FragColor.z<0.0) color=vec4(0.0,0.0,1.0,1.0);" +
            "if(gl_FragColor.w<0.0) color=vec4(1.0,1.0,0.0,1.0);" +
            "gl_FragColor = vec4(color.xyz,1.0);"+
        "}";
    }
    header += "\n";

    /*
    this.mImagePassFooterVR = "\n" +
    "uniform vec4 unViewport;\n" +
    "uniform vec3 unCorners[5];\n";
    if( this.mIs20 ) 
        this.mImagePassFooterVR += "\nout vec4 outColor;\n";
    this.mImagePassFooterVR += "void main( void )" +
    "{" +
        "vec4 color = vec4(1e20);" +

        "vec3 ro = unCorners[4];" +
        "vec2 uv = (gl_FragCoord.xy - unViewport.xy)/unViewport.zw;" + 
        "vec3 rd = normalize( mix( mix( unCorners[0], unCorners[1], uv.x )," +
                                  "mix( unCorners[3], unCorners[2], uv.x ), uv.y ) - ro);" + 

        "mainVR( color, gl_FragCoord.xy-unViewport.xy, ro, rd );" +
        "color.w = 1.0;"
    if( this.mIs20 ) 
        this.mImagePassFooterVR +=  "outColor = color;}";
    else
        this.mImagePassFooterVR +=  "gl_FragColor = color;}";
    */
    this.mHeader = header;
    this.mHeaderLength = 0;
}

EffectPass.prototype.MakeHeader_Buffer = function()
{
    let header = "";
    
    header += "#define HW_PERFORMANCE " + ((this.mIsLowEnd===true)?"0":"1") + "\n";

    header += "uniform vec3      iResolution;\n" +
              "uniform float     iTime;\n" +
              "uniform float     iChannelTime[4];\n" +
              "uniform vec4      iMouse;\n" +
              "uniform vec4      iDate;\n" +
              "uniform float     iSampleRate;\n" +
              "uniform vec3      iChannelResolution[4];\n" +
              "uniform int       iFrame;\n" +
              "uniform float     iTimeDelta;\n" +
              "uniform float     iFrameRate;\n";

    for (let i = 0; i < this.mInputs.length; i++)
    {
        let inp = this.mInputs[i];
             if( inp===null )                  header += "uniform sampler2D iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="cubemap" ) header += "uniform samplerCube iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="volume"  ) header += "uniform sampler3D iChannel" + i + ";\n";
        else                                  header += "uniform sampler2D iChannel" + i + ";\n";
    }

	header += "void mainImage( out vec4 c,  in vec2 f );\n"

    if( this.mIs20 )
        header += "\nout vec4 outColor;\n";
    header += "\nvoid main( void )\n" +
    "{" +
        "vec4 color = vec4(1e20);" +
        "mainImage( color, gl_FragCoord.xy );";
    if( this.mIs20 )
        header +="outColor = color; }";
    else
        header +="gl_FragColor = color; }";
    header += "\n";

    /*
    this.mImagePassFooterVR = "\n" +
    "uniform vec4 unViewport;\n" +
    "uniform vec3 unCorners[5];\n";
    if( this.mIs20 )
    this.mImagePassFooterVR += "\nout vec4 outColor;\n";
    this.mImagePassFooterVR += "\nvoid main( void )\n" +
    "{" +
        "vec4 color = vec4(1e20);" +

        "vec3 ro = unCorners[4];" +
        "vec2 uv = (gl_FragCoord.xy - unViewport.xy)/unViewport.zw;" + 
        "vec3 rd = normalize( mix( mix( unCorners[0], unCorners[1], uv.x )," +
                                  "mix( unCorners[3], unCorners[2], uv.x ), uv.y ) - ro);" + 

        "mainVR( color, gl_FragCoord.xy-unViewport.xy, ro, rd );";
    if( this.mIs20 )
        this.mImagePassFooterVR +="outColor = color; }";
    else
        this.mImagePassFooterVR +="gl_FragColor = color; }";
    */
    this.mHeader = header;
    this.mHeaderLength = 0;
}

EffectPass.prototype.MakeHeader_Cubemap = function()
{
    let header = "";
    
    header += "#define HW_PERFORMANCE " + ((this.mIsLowEnd===true)?"0":"1") + "\n";

    header += "uniform vec3      iResolution;\n" +
              "uniform float     iTime;\n" +
              "uniform float     iChannelTime[4];\n" +
              "uniform vec4      iMouse;\n" +
              "uniform vec4      iDate;\n" +
              "uniform float     iSampleRate;\n" +
              "uniform vec3      iChannelResolution[4];\n" +
              "uniform int       iFrame;\n" +
              "uniform float     iTimeDelta;\n" +
              "uniform float     iFrameRate;\n";

    for (let i = 0; i < this.mInputs.length; i++)
    {
        let inp = this.mInputs[i];
             if( inp===null )                  header += "uniform sampler2D iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="cubemap" ) header += "uniform samplerCube iChannel" + i + ";\n";
        else if( inp.mInfo.mType==="volume"  ) header += "uniform sampler3D iChannel" + i + ";\n";
        else                                   header += "uniform sampler2D iChannel" + i + ";\n";
    }

	header += "void mainCubemap( out vec4 c, in vec2 f, in vec3 ro, in vec3 rd );\n"

    header += "\n" +
    "uniform vec4 unViewport;\n" +
    "uniform vec3 unCorners[5];\n";
    if( this.mIs20 )
        header += "\nout vec4 outColor;\n";
    header += "\nvoid main( void )\n" +
    "{" +
        "vec4 color = vec4(1e20);" +

        "vec3 ro = unCorners[4];" +
        "vec2 uv = (gl_FragCoord.xy - unViewport.xy)/unViewport.zw;" + 
        "vec3 rd = normalize( mix( mix( unCorners[0], unCorners[1], uv.x )," +
                                  "mix( unCorners[3], unCorners[2], uv.x ), uv.y ) - ro);" + 

        "mainCubemap( color, gl_FragCoord.xy-unViewport.xy, ro, rd );";
    if( this.mIs20 )
        header +="outColor = color; }";
    else
        header +="gl_FragColor = color; }";
    header += "\n";

    this.mHeader = header;
    this.mHeaderLength = 0;
}

EffectPass.prototype.MakeHeader_Sound = function()
{
    let header = "";

    header += "#define HW_PERFORMANCE " + ((this.mIsLowEnd===true)?"0":"1") + "\n";

    header += "uniform float     iChannelTime[4];\n" +
              "uniform float     iTimeOffset;\n" +
              "uniform int       iSampleOffset;\n" +
              "uniform vec4      iDate;\n" +
              "uniform float     iSampleRate;\n" +
              "uniform vec3      iChannelResolution[4];\n";

    for (let i=0; i<this.mInputs.length; i++ )
    {
        let inp = this.mInputs[i];

        if( inp!==null && inp.mInfo.mType==="cubemap" )
            header += "uniform samplerCube iChannel" + i + ";\n";
        else
            header += "uniform sampler2D iChannel" + i + ";\n";
    }
    header += "\n";
    header += "vec2 mainSound( in int samp, float time );\n";

    if( this.mIs20 )
    {
        header += "out vec4 outColor; void main()" +
            "{" +
            "float t = iTimeOffset + ((gl_FragCoord.x-0.5) + (gl_FragCoord.y-0.5)*512.0)/iSampleRate;" +
            "int   s = iSampleOffset + int(gl_FragCoord.y-0.2)*512 + int(gl_FragCoord.x-0.2);" +
            "vec2 y = mainSound( s, t );" +
            "vec2 v  = floor((0.5+0.5*y)*65536.0);" +
            "vec2 vl =   mod(v,256.0)/255.0;" +
            "vec2 vh = floor(v/256.0)/255.0;" +
            "outColor = vec4(vl.x,vh.x,vl.y,vh.y);" +
            "}";
    }
    else
    {
        header += "void main()" +
            "{" +
            "float t = iTimeOffset + ((gl_FragCoord.x-0.5) + (gl_FragCoord.y-0.5)*512.0)/iSampleRate;" +
            "vec2 y = mainSound( 0, t );" +
            "vec2 v  = floor((0.5+0.5*y)*65536.0);" +
            "vec2 vl =   mod(v,256.0)/255.0;" +
            "vec2 vh = floor(v/256.0)/255.0;" +
            "gl_FragColor = vec4(vl.x,vh.x,vl.y,vh.y);" +
            "}";
    }
    header += "\n";
    this.mHeader = header;
    this.mHeaderLength = 0;
}


EffectPass.prototype.MakeHeader_Common = function ()
{
    let header = "";
    let headerlength = 0;

    header += "uniform vec4      iDate;\n" +
              "uniform float     iSampleRate;\n";
    headerlength += 2;

    if (this.mIs20)
    {
        header += "out vec4 outColor;\n";
        headerlength += 1;
    }
    header += "void main( void )\n";
    headerlength += 1;

    if (this.mIs20)
        header += "{ outColor = vec4(0.0); }";
    else
        header += "{ gl_FragColor = vec4(0.0); }";
    headerlength += 1;
    header += "\n";
    headerlength += 1;

    this.mHeader = header;
    this.mHeaderLength = headerlength;
}

EffectPass.prototype.MakeHeader = function()
{
         if( this.mType==="image" ) this.MakeHeader_Image();
    else if( this.mType==="sound" ) this.MakeHeader_Sound();
    else if( this.mType==="buffer") this.MakeHeader_Buffer();
    else if( this.mType==="common") this.MakeHeader_Common();
    else if( this.mType==="cubemap") this.MakeHeader_Cubemap();
    else console.log("ERROR 4");
}

EffectPass.prototype.Create_Image = function( wa )
{
    this.MakeHeader();
    this.mSampleRate = 44100;
    this.mSupportsVR = false;
    this.mProgram = null;
    this.mError = false;
    this.mErrorStr = "";
    this.mTranslatedSource = null;
    //this.mProgramVR = null;
}
EffectPass.prototype.Destroy_Image = function( wa )
{
}

EffectPass.prototype.Create_Buffer = function( wa )
{
    this.MakeHeader();
    this.mSampleRate = 44100;
    this.mSupportsVR = false;
    this.mProgram = null;
    this.mError = false;
    this.mErrorStr = "";
    this.mTranslatedSource = null;
    //this.mProgramVR = null;
}

EffectPass.prototype.Destroy_Buffer = function( wa )
{
}

EffectPass.prototype.Create_Cubemap = function( wa )
{
    this.MakeHeader();
    this.mSampleRate = 44100;
    this.mProgram = null;
    this.mError = false;
    this.mErrorStr = "";
    this.mTranslatedSource = null;
}

EffectPass.prototype.Destroy_Cubemap = function( wa )
{
}

EffectPass.prototype.Create_Common = function( wa )
{
    this.mProgram = null;
    this.mError = false;
    this.mErrorStr = "";
    this.MakeHeader();
}
EffectPass.prototype.Destroy_Common = function( wa )
{
}

EffectPass.prototype.Create_Sound = function (wa)
{
    this.MakeHeader();

    this.mProgram = null;
    this.mError = false;
    this.mErrorStr = "";
    this.mTranslatedSource = null;
    this.mSampleRate = 44100;
    this.mPlayTime = 60*3;
    this.mPlaySamples = this.mPlayTime*this.mSampleRate;
    this.mBuffer = wa.createBuffer( 2, this.mPlaySamples, this.mSampleRate );

    //-------------------
    this.mTextureDimensions = 512;
    this.mRenderTexture = this.mRenderer.CreateTexture(this.mRenderer.TEXTYPE.T2D, 
                                                       this.mTextureDimensions, this.mTextureDimensions,
                                                       this.mRenderer.TEXFMT.C4I8,
                                                       this.mRenderer.FILTER.NONE,
                                                       this.mRenderer.TEXWRP.CLAMP, null);
    this.mRenderFBO = this.mRenderer.CreateRenderTarget(this.mRenderTexture, null, null, null, null, false);

    //-----------------------------

    // ArrayBufferView pixels;
    this.mTmpBufferSamples = this.mTextureDimensions*this.mTextureDimensions;
    this.mData = new Uint8Array( this.mTmpBufferSamples*4 );

    this.mPlaying = false;
}

EffectPass.prototype.Destroy_Sound = function( wa )
{
    if( this.mPlayNode!==null ) this.mPlayNode.stop();
    this.mPlayNode = null;
    this.mBuffer = null;
    this.mData = null;

    this.mRenderer.DestroyRenderTarget(this.mRenderFBO);
    this.mRenderer.DestroyTexture(this.mRenderTexture);
}

EffectPass.prototype.Create = function( passType, wa )
{
    this.mType = passType;
    this.mSource = null;

         if( passType==="image" ) this.Create_Image( wa );
    else if( passType==="sound" ) this.Create_Sound( wa );
    else if( passType==="buffer") this.Create_Buffer( wa );
    else if( passType==="common") this.Create_Common( wa );
    else if( passType==="cubemap") this.Create_Cubemap( wa );
    else alert("ERROR 1");
}

EffectPass.prototype.SetName = function (passName)
{
    this.mName = passName;
}

EffectPass.prototype.SetCode = function (src)
{
    this.mSource = src;
}

EffectPass.prototype.Destroy = function( wa )
{
    this.mSource = null;
         if( this.mType==="image" ) this.Destroy_Image( wa );
    else if( this.mType==="sound" ) this.Destroy_Sound( wa );
    else if( this.mType==="buffer") this.Destroy_Buffer( wa );
    else if( this.mType==="common") this.Destroy_Common( wa );
    else if( this.mType==="cubemap") this.Destroy_Cubemap( wa );
    else alert("ERROR 2");
}

EffectPass.prototype.NewShader_Sound = function( shaderCode, commonShaderCodes)
{
    let vsSource = null;

    if( this.mIs20 )
        vsSource = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
    else
        vsSource = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";

    let fsSource = this.mHeader;
    for( let i=0; i<commonShaderCodes.length; i++ )
    {
        fsSource += commonShaderCodes[i]+'\n';
    }
    this.mHeaderLength = fsSource.split(/\r\n|\r|\n/).length;
    fsSource += shaderCode;

    this.mSoundShaderCompiled = false;

    return [vsSource, fsSource];
}

EffectPass.prototype.NewShader_Image = function ( shaderCode, commonShaderCodes )
{
    this.mSupportsVR = false;

    let vsSource = null;
    if( this.mIs20 )
        vsSource = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
    else
        vsSource = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";

    let fsSource = this.mHeader;
    for (let i = 0; i < commonShaderCodes.length; i++)
    {
        fsSource += commonShaderCodes[i]+'\n';
    }
    this.mHeaderLength = fsSource.split(/\r\n|\r|\n/).length;
    fsSource += shaderCode;

    return [vsSource, fsSource];


    /*
    let n1 = shaderCode.indexOf("mainVR(");
    let n2 = shaderCode.indexOf("mainVR (");
    let n3 = shaderCode.indexOf("mainVR  (");
    if( n1>0 || n2>0 || n3>0 )
    {
        let vsSourceVR;
        if( this.mIs20 )
            vsSourceVR = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
        else
            vsSourceVR = "attribute in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";

        let fsSourceVR = this.mHeader;
        for (let i = 0; i < commonShaderCodes.length; i++) {
            fsSourceVR += commonShaderCodes[i];
        }
        fsSourceVR += shaderCode;
        fsSourceVR += this.mImagePassFooterVR;

        let res = this.mRenderer.CreateShader(vsSource, fsSourceVR, preventCache);
        if( res.mResult == false )
        {
            return res.mInfo;
        }
        if( this.mProgramVR != null )
            this.mRenderer.DestroyShader( this.mProgramVR );

        this.mSupportsVR = true;
        this.mProgramVR = res;
    }
    */
}

EffectPass.prototype.NewShader_Cubemap = function( shaderCode, commonShaderCodes )
{
    let vsSource = null;
    if( this.mIs20 )
        vsSource = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
    else
        vsSource = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";

    let fsSource = this.mHeader;
    for (let i = 0; i < commonShaderCodes.length; i++)
    {
        fsSource += commonShaderCodes[i]+'\n';
    }

    this.mHeaderLength = fsSource.split(/\r\n|\r|\n/).length;

    fsSource += shaderCode;

    return [vsSource, fsSource];
}

EffectPass.prototype.NewShader_Common = function (shaderCode )
{
    let vsSource = null;
    if (this.mIs20)
        vsSource = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
    else
        vsSource = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";

    let fsSource = this.mHeader + shaderCode;

    return [vsSource, fsSource];
}

EffectPass.prototype.NewShader = function ( commonSourceCodes, preventCache, onResolve)
{
    if( this.mRenderer===null ) return;

    let vs_fs = null;

         if( this.mType==="sound"  ) vs_fs = this.NewShader_Sound(   this.mSource, commonSourceCodes );
    else if( this.mType==="image"  ) vs_fs = this.NewShader_Image(   this.mSource, commonSourceCodes );
    else if( this.mType==="buffer" ) vs_fs = this.NewShader_Image(   this.mSource, commonSourceCodes );
    else if( this.mType==="common" ) vs_fs = this.NewShader_Common(  this.mSource,                   );
    else if( this.mType==="cubemap") vs_fs = this.NewShader_Cubemap( this.mSource, commonSourceCodes );
    else { console.log("ERROR 3: \"" + this.mType + "\""); return; }

    let me = this;
    this.mRenderer.CreateShader(vs_fs[0], vs_fs[1], preventCache, false,
        function (worked, info)
        {
            if (worked === true)
            {
                if (me.mType === "sound")
                {
                    me.mSoundShaderCompiled = true;
                }

                me.mCompilationTime = info.mTime;
                me.mError = false;
                me.mErrorStr = "No Errors";
                if (me.mProgram !== null)
                    me.mRenderer.DestroyShader(me.mProgram);
                me.mTranslatedSource = me.mRenderer.GetTranslatedShaderSource(info);
                me.mProgram = info;
            }
            else
            {
                me.mError = true;
                me.mErrorStr = info.mErrorStr;
            }
            onResolve();
        });
}

EffectPass.prototype.DestroyInput = function( id )
{
    if( this.mInputs[id]===null ) return;

    if( this.mInputs[id].mInfo.mType==="texture" )
    {
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    if( this.mInputs[id].mInfo.mType==="volume" )
    {
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="webcam" )
    {
        this.mInputs[id].video.pause();
        this.mInputs[id].video.src = "";

        if( this.mInputs[id].video.srcObject!==null )
        {
        let tracks = this.mInputs[id].video.srcObject.getVideoTracks();
        if( tracks ) tracks[0].stop();
        }
        this.mInputs[id].video = null;
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="video" )
    {
        this.mInputs[id].video.pause();
        this.mInputs[id].video = null;
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="music" || this.mInputs[id].mInfo.mType==="musicstream")
    {
        this.mInputs[id].audio.pause();
        this.mInputs[id].audio.mSound.mFreqData = null;
        this.mInputs[id].audio.mSound.mWaveData = null;
        this.mInputs[id].audio = null;
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="cubemap" )
    {
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="keyboard" )
    {
        //if( this.mInputs[id].globject != null )
          //  this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }
    else if( this.mInputs[id].mInfo.mType==="mic" )
    {
        this.mInputs[id].mic = null;
        if( this.mInputs[id].globject !== null )
            this.mRenderer.DestroyTexture(this.mInputs[id].globject);
    }

    this.mInputs[id] = null;
}

EffectPass.prototype.TooglePauseInput = function( wa, id )
{
    var me = this;
    let inp = this.mInputs[id];

    if( inp===null )
    {
    }
    else if( inp.mInfo.mType==="texture" )
    {
    }
    else if( inp.mInfo.mType==="volume" )
    {
    }
    else if( inp.mInfo.mType==="video" )
    {
        if( inp.video.mPaused )
        {
            inp.video.play();
            inp.video.mPaused = false;
        }
        else
        {
            inp.video.pause();
            inp.video.mPaused = true;
        }
        return inp.video.mPaused;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream")
    {
        wa.resume()
        if( inp.audio.mPaused )
        {
            if( inp.loaded )
            {
                inp.audio.play();
            }
            inp.audio.mPaused = false;
        }
        else
        {
            inp.audio.pause();
            inp.audio.mPaused = true;
        }
        return inp.audio.mPaused;
    }

    return null;
}

EffectPass.prototype.StopInput = function( id )
{
    let inp = this.mInputs[id];

    if( inp===null )
    {
    }
    else if( inp.mInfo.mType==="texture" )
    {
    }
    else if( inp.mInfo.mType==="volume" )
    {
    }
    else if( inp.mInfo.mType==="video" )
    {
        if( inp.video.mPaused === false )
        {
            inp.video.pause();
            inp.video.mPaused = true;
        }
        return inp.video.mPaused;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
    {
        if( inp.audio.mPaused === false )
        {
            inp.audio.pause();
            inp.audio.mPaused = true;
        }
        return inp.audio.mPaused;
    }
    return null;
}

EffectPass.prototype.ResumeInput = function( id )
{
    let inp = this.mInputs[id];

    if( inp===null )
    {
    }
    else if( inp.mInfo.mType==="texture" )
    {
    }
    else if( inp.mInfo.mType==="volume" )
    {
    }
    else if( inp.mInfo.mType==="video" )
    {
        if( inp.video.mPaused )
        {
            inp.video.play();
            inp.video.mPaused = false;
        }
        return inp.video.mPaused;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
    {
        if( inp.audio.mPaused )
        {
            inp.audio.play();
            inp.audio.mPaused = false;
        }
        return inp.audio.mPaused;
    }
    return null;
}

EffectPass.prototype.RewindInput = function( wa, id )
{
    var me = this;
    let inp = this.mInputs[id];

    if( inp==null )
    {
    }
    else if( inp.mInfo.mType==="texture" )
    {
    }
    else if( inp.mInfo.mType==="volume" )
    {
    }
    else if( inp.mInfo.mType==="video" )
    {
        if( inp.loaded )
        {
            inp.video.currentTime = 0;
        }
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream")
    {
        wa.resume()
        if( inp.loaded )
        {
            inp.audio.currentTime = 0;
        }
    }
}

EffectPass.prototype.MuteInput = function( wa, id )
{
    let inp = this.mInputs[id];
    if( inp===null ) return;

    if( inp.mInfo.mType==="video" )
    {
        inp.video.muted = true;
        inp.video.mMuted = true;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream")
    {
        if (wa !== null) inp.audio.mSound.mGain.gain.value = 0.0;
        inp.audio.mMuted = true;
    }
}

EffectPass.prototype.UnMuteInput = function( wa, id )
{
    let inp = this.mInputs[id];
    if( inp===null ) return;

    if( inp.mInfo.mType==="video" )
    {
        inp.video.muted = false;
        inp.video.mMuted = false;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream")
    {
        if (wa !== null) inp.audio.mSound.mGain.gain.value = 1.0;
        inp.audio.mMuted = false;
    }
}

EffectPass.prototype.ToggleMuteInput = function( wa, id )
{
    var me = this;
    let inp = this.mInputs[id];
    if( inp===null ) return null;

    if( inp.mInfo.mType==="video" )
    {
        if( inp.video.mMuted ) this.UnMuteInput(wa,id);
        else                   this.MuteInput(wa,id);
        return inp.video.mMuted;
    }
    else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream")
    {
        if( inp.audio.mMuted ) this.UnMuteInput(wa,id);
        else                   this.MuteInput(wa,id);
        return inp.audio.mMuted;
    }

    return null;
}

EffectPass.prototype.UpdateInputs = function( wa, forceUpdate, keyboard )
{
   for (let i=0; i<this.mInputs.length; i++ )
   {
        let inp = this.mInputs[i];

        if( inp===null )
        {
            if( forceUpdate )
            {
              if( this.mTextureCallbackFun!==null )
                  this.mTextureCallbackFun( this.mTextureCallbackObj, i, null, false, 0, 0, -1.0, this.mID );
            }
        }
        else if( inp.mInfo.mType==="texture" )
        {
            if( inp.loaded && forceUpdate )
            {
              if( this.mTextureCallbackFun!==null )
                  this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.image, true, 1, 1, -1.0, this.mID );
            }
        }
        else if( inp.mInfo.mType==="volume" )
        {
            if( inp.loaded && forceUpdate )
            {
              if( this.mTextureCallbackFun!==null )
                  this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.mPreview, true, 1, 1, -1.0, this.mID );
            }
        }
        else if( inp.mInfo.mType==="cubemap" )
        {
            if( inp.loaded && forceUpdate )
            {
                if( this.mTextureCallbackFun!==null )
                {
                    let img = (assetID_to_cubemapBuferID(inp.mInfo.mID)===-1) ? inp.image[0] : inp.mImage;
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, img, true, 2, 1, -1.0, this.mID );
                }
            }
        }
        else if( inp.mInfo.mType==="keyboard" )
        {
            if( this.mTextureCallbackFun!==null )
                this.mTextureCallbackFun( this.mTextureCallbackObj, i, {mImage:keyboard.mIcon,mData:keyboard.mData}, false, 6, 0, -1.0, this.mID );
        }
        else if( inp.mInfo.mType==="video" )
        {
            if( inp.video.readyState === inp.video.HAVE_ENOUGH_DATA )
            {
                if( this.mTextureCallbackFun!==null )
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.video, false, 3, 1, -1, this.mID );
            }
        }
        else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
        {
              if( inp.loaded && inp.audio.mPaused === false && inp.audio.mForceMuted === false )
              {
                  if( wa !== null )
                  {
                      inp.audio.mSound.mAnalyser.getByteFrequencyData(  inp.audio.mSound.mFreqData );
                      inp.audio.mSound.mAnalyser.getByteTimeDomainData( inp.audio.mSound.mWaveData );
                  }

                  if (this.mTextureCallbackFun!==null)
                  {
                           if (inp.mInfo.mType === "music")       this.mTextureCallbackFun(this.mTextureCallbackObj, i, {wave:(wa==null)?null:inp.audio.mSound.mFreqData}, false, 4, 1, inp.audio.currentTime, this.mID);
                      else if (inp.mInfo.mType === "musicstream") this.mTextureCallbackFun(this.mTextureCallbackObj, i, {wave:(wa==null)?null:inp.audio.mSound.mFreqData, info: inp.audio.soundcloudInfo}, false, 8, 1, inp.audio.currentTime, this.mID);
                  }
              }
              else if( inp.loaded===false )
              {
                  if (this.mTextureCallbackFun!==null)
                      this.mTextureCallbackFun(this.mTextureCallbackObj, i, {wave:null}, false, 4, 0, -1.0, this.mID);
              }
        }
        else if( inp.mInfo.mType==="mic" )
        {
              if( inp.loaded && inp.mForceMuted === false )
              {
                  if( wa !== null )
                  {
                      inp.mAnalyser.getByteFrequencyData(  inp.mFreqData );
                      inp.mAnalyser.getByteTimeDomainData( inp.mWaveData );
                  }
                  if( this.mTextureCallbackFun!==null )
                      this.mTextureCallbackFun( this.mTextureCallbackObj, i, {wave: ((wa==null)?null:inp.mFreqData) }, false, 5, 1, 0, this.mID );
              }
        }
        else if( inp.mInfo.mType==="buffer" )
        {
            if( inp.loaded && forceUpdate )
            {
              if( this.mTextureCallbackFun!==null )
                  this.mTextureCallbackFun( this.mTextureCallbackObj, i, {texture:inp.image, data:null}, true, 9, 1, -1.0, this.mID );
            }
        }
    }
}

EffectPass.prototype.Sampler2Renderer = function (sampler)
{
    let filter = this.mRenderer.FILTER.NONE;
    if (sampler.filter === "linear") filter = this.mRenderer.FILTER.LINEAR;
    if (sampler.filter === "mipmap") filter = this.mRenderer.FILTER.MIPMAP;
    let wrap = this.mRenderer.TEXWRP.REPEAT;
    if (sampler.wrap === "clamp") wrap = this.mRenderer.TEXWRP.CLAMP;
    let vflip = false;
    if (sampler.vflip === "true") vflip = true;

    return { mFilter: filter, mWrap: wrap, mVFlip: vflip };
}

EffectPass.prototype.GetSamplerVFlip = function (id)
{
    let inp = this.mInputs[id];
    return inp.mInfo.mSampler.vflip;
}

EffectPass.prototype.GetTranslatedShaderSource = function ()
{
    return this.mTranslatedSource;
}

EffectPass.prototype.SetSamplerVFlip = function (id, str) 
{
    var me = this;
    var renderer = this.mRenderer;
    let inp = this.mInputs[id];

    let filter = false;
    if (str === "true") filter = true;

    if (inp === null)
    {
    }
    else if (inp.mInfo.mType === "texture")
    {
        if (inp.loaded)
        {
            renderer.SetSamplerVFlip(inp.globject, filter, inp.image);
            inp.mInfo.mSampler.vflip = str;
        }
    }
    else if (inp.mInfo.mType === "volume")
    {
    }
    else if (inp.mInfo.mType === "video")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerVFlip(inp.globject, filter, inp.image);
            inp.mInfo.mSampler.vflip = str;
        }
    }
    else if (inp.mInfo.mType === "cubemap")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerVFlip(inp.globject, filter, inp.image);
            inp.mInfo.mSampler.vflip = str;
        }
    }
    else if (inp.mInfo.mType === "webcam")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerVFlip(inp.globject, filter, null);
            inp.mInfo.mSampler.vflip = str;
        }
    }
}

EffectPass.prototype.GetAcceptsVFlip = function (id)
{
    let inp = this.mInputs[id];

    if (inp === null) return false;
    if (inp.mInfo.mType === "texture") return true;
    if (inp.mInfo.mType === "volume") return false;
    if (inp.mInfo.mType === "video")  return true;
    if (inp.mInfo.mType === "cubemap") return true;
    if (inp.mInfo.mType === "webcam")  return true;
    if (inp.mInfo.mType === "music")  return false;
    if (inp.mInfo.mType === "musicstream") return false;
    if (inp.mInfo.mType === "mic")  return false;
    if (inp.mInfo.mType === "keyboard")  return false;
    if (inp.mInfo.mType === "buffer") return false;
    return true;
}

EffectPass.prototype.GetSamplerFilter = function (id)
{
    let inp = this.mInputs[id];
    if( inp===null) return;
    return inp.mInfo.mSampler.filter;
}

EffectPass.prototype.SetSamplerFilter = function (id, str, buffers, cubeBuffers) 
{
    var me = this;
    var renderer = this.mRenderer;
    let inp = this.mInputs[id];

    let filter = renderer.FILTER.NONE;
    if (str === "linear") filter = renderer.FILTER.LINEAR;
    if (str === "mipmap") filter = renderer.FILTER.MIPMAP;

    if (inp === null)
    {
    }
    else if (inp.mInfo.mType === "texture")
    {
        if (inp.loaded)
        {
            renderer.SetSamplerFilter(inp.globject, filter, true);
            inp.mInfo.mSampler.filter = str;
        }
    }
    else if (inp.mInfo.mType === "volume")
    {
        if (inp.loaded)
        {
            renderer.SetSamplerFilter(inp.globject, filter, true);
            inp.mInfo.mSampler.filter = str;
        }
    }
    else if (inp.mInfo.mType === "video")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerFilter(inp.globject, filter, true);
            inp.mInfo.mSampler.filter = str;
        }
    }
    else if (inp.mInfo.mType === "cubemap")
    {
        if (inp.loaded) 
        {
            if( assetID_to_cubemapBuferID(inp.mInfo.mID)===0)
            {
                renderer.SetSamplerFilter(cubeBuffers[0].mTexture[0], filter, true);
                renderer.SetSamplerFilter(cubeBuffers[0].mTexture[1], filter, true);
                inp.mInfo.mSampler.filter = str;
            }
            else
            {
                renderer.SetSamplerFilter(inp.globject, filter, true);
                inp.mInfo.mSampler.filter = str;
            }
        }
    }
    else if (inp.mInfo.mType === "webcam")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerFilter(inp.globject, filter, true);
            inp.mInfo.mSampler.filter = str;
        }
    }
    else if (inp.mInfo.mType === "buffer")
    {
        renderer.SetSamplerFilter(buffers[inp.id].mTexture[0], filter, true);
        renderer.SetSamplerFilter(buffers[inp.id].mTexture[1], filter, true);
        inp.mInfo.mSampler.filter = str;
    }
    else if (inp.mInfo.mType === "keyboard")
    {
        inp.mInfo.mSampler.filter = str;
    }
}

EffectPass.prototype.GetAcceptsMipmapping = function (id)
{
    let inp = this.mInputs[id];

    if (inp === null) return false;
    if (inp.mInfo.mType === "texture") return true;
    if (inp.mInfo.mType === "volume") return true;
    if (inp.mInfo.mType === "video")  return this.mIs20;
    if (inp.mInfo.mType === "cubemap") return true;
    if (inp.mInfo.mType === "webcam")  return this.mIs20;
    if (inp.mInfo.mType === "music")  return false;
    if (inp.mInfo.mType === "musicstream") return false;
    if (inp.mInfo.mType === "mic")  return false;
    if (inp.mInfo.mType === "keyboard")  return false;
    if (inp.mInfo.mType === "buffer") return this.mIs20;
    return false;
}

EffectPass.prototype.GetAcceptsLinear = function (id)
{
    let inp = this.mInputs[id];

    if (inp === null) return false;
    if (inp.mInfo.mType === "texture") return true;
    if (inp.mInfo.mType === "volume") return true;
    if (inp.mInfo.mType === "video")  return true;
    if (inp.mInfo.mType === "cubemap") return true;
    if (inp.mInfo.mType === "webcam")  return true;
    if (inp.mInfo.mType === "music")  return true;
    if (inp.mInfo.mType === "musicstream") return true;
    if (inp.mInfo.mType === "mic")  return true;
    if (inp.mInfo.mType === "keyboard")  return false;
    if (inp.mInfo.mType === "buffer") return true;
    return false;
}

EffectPass.prototype.GetAcceptsWrapRepeat = function (id)
{
    let inp = this.mInputs[id];

    if (inp === null) return false;
    if (inp.mInfo.mType === "texture") return true;
    if (inp.mInfo.mType === "volume") return true;
    if (inp.mInfo.mType === "video")  return this.mIs20;
    if (inp.mInfo.mType === "cubemap") return false;
    if (inp.mInfo.mType === "webcam")  return this.mIs20;
    if (inp.mInfo.mType === "music")  return false;
    if (inp.mInfo.mType === "musicstream") return false;
    if (inp.mInfo.mType === "mic")  return false;
    if (inp.mInfo.mType === "keyboard")  return false;
    if (inp.mInfo.mType === "buffer") return this.mIs20;
    return false;
}

EffectPass.prototype.GetSamplerWrap = function (id)
{
    return this.mInputs[id].mInfo.mSampler.wrap;
}

EffectPass.prototype.SetSamplerWrap = function (id, str, buffers)
{
    var me = this;
    var renderer = this.mRenderer;
    let inp = this.mInputs[id];

    let restr = renderer.TEXWRP.REPEAT;
    if (str === "clamp") restr = renderer.TEXWRP.CLAMP;

    if (inp === null) 
    {
    }
    else if (inp.mInfo.mType === "texture")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerWrap(inp.globject, restr);
            inp.mInfo.mSampler.wrap = str;
        }
    }
    else if (inp.mInfo.mType === "volume")
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerWrap(inp.globject, restr);
            inp.mInfo.mSampler.wrap = str;
        }
    }
    else if (inp.mInfo.mType === "video") 
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerWrap(inp.globject, restr);
            inp.mInfo.mSampler.wrap = str;
        }
    }
    else if (inp.mInfo.mType === "cubemap") 
    {
        if (inp.loaded)
        {
            renderer.SetSamplerWrap(inp.globject, restr);
            inp.mInfo.mSampler.wrap = str;
        }
    }
    else if (inp.mInfo.mType === "webcam") 
    {
        if (inp.loaded) 
        {
            renderer.SetSamplerWrap(inp.globject, restr);
            inp.mInfo.mSampler.wrap = str;
        }
    }
    else if (inp.mInfo.mType === "buffer")
    {
        renderer.SetSamplerWrap(buffers[inp.id].mTexture[0], restr);
        renderer.SetSamplerWrap(buffers[inp.id].mTexture[1], restr);
        inp.mInfo.mSampler.wrap = str;
    }
}


EffectPass.prototype.GetTexture = function( slot )
{
    let inp = this.mInputs[slot];
    if( inp===null ) return null;
    return inp.mInfo;
}

EffectPass.prototype.SetOutputs = function( slot, id )
{
    this.mOutputs[slot] = id;
}

EffectPass.prototype.SetOutputsByBufferID = function( slot, id )
{
    if( this.mType==="buffer" )
    {
        this.mOutputs[slot] = bufferID_to_assetID( id );

        this.mEffect.ResizeBuffer( id, this.mEffect.mXres, this.mEffect.mYres, false );
    }
    else if( this.mType==="cubemap" )
    {
        this.mOutputs[slot] = cubamepBufferID_to_assetID( id );
        this.mEffect.ResizeCubemapBuffer(id, 1024, 1024 );
    }
}

EffectPass.prototype.NewTexture = function( wa, slot, url, buffers, cubeBuffers, keyboard )
{
    var me = this;
    var renderer = this.mRenderer;

    if( renderer===null ) return;

    let texture = null;

    if( url===null || url.mType===null )
    {
        if( me.mTextureCallbackFun!==null )
            me.mTextureCallbackFun( this.mTextureCallbackObj, slot, null, true, 0, 0, -1.0, me.mID );
        me.DestroyInput( slot );
        me.mInputs[slot] = null;
        me.MakeHeader();
        return { mFailed:false, mNeedsShaderCompile:false };
    }
    else if( url.mType==="texture" )
    {
        texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;
        texture.image = new Image();
        texture.image.crossOrigin = '';
        texture.image.onload = function()
        {
            let rti = me.Sampler2Renderer(url.mSampler);

            // O.M.G. FIX THIS
            let channels = renderer.TEXFMT.C4I8;
            if (url.mID === "Xdf3zn" || url.mID === "4sf3Rn" || url.mID === "4dXGzn" || url.mID === "4sf3Rr")
                channels = renderer.TEXFMT.C1I8;
            
            texture.globject = renderer.CreateTextureFromImage(renderer.TEXTYPE.T2D, texture.image, channels, rti.mFilter, rti.mWrap, rti.mVFlip);

            texture.loaded = true;
            if( me.mTextureCallbackFun!==null )
                me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.image, true, 1, 1, -1.0, me.mID );
        }
        texture.image.src = url.mSrc;


        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="volume" )
    {
        texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;
        texture.mImage = { mData:null, mXres:1, mYres:0, mZres:0 };
        texture.mPreview = new Image();
        texture.mPreview.crossOrigin = '';

	    var xmlHttp = new XMLHttpRequest();
        if( xmlHttp===null ) return { mFailed:true };

        xmlHttp.open('GET', url.mSrc, true);
        xmlHttp.responseType = "arraybuffer";
        xmlHttp.onerror = function()
        {
            console.log( "Error 1 loading Volume" );
        }
        xmlHttp.onload = function()
        {
            let data = xmlHttp.response;
            if (!data ) { console.log( "Error 2 loading Volume" ); return; }

            let file = piFile(data);

            let signature = file.ReadUInt32();
            texture.mImage.mXres = file.ReadUInt32();
            texture.mImage.mYres = file.ReadUInt32();
            texture.mImage.mZres = file.ReadUInt32();
            let binNumChannels = file.ReadUInt8();
            let binLayout = file.ReadUInt8();
            let binFormat = file.ReadUInt16();
            let format = renderer.TEXFMT.C1I8;
                 if( binNumChannels===1 && binFormat===0 )  format = renderer.TEXFMT.C1I8;
            else if( binNumChannels===2 && binFormat===0 )  format = renderer.TEXFMT.C2I8;
            else if( binNumChannels===3 && binFormat===0 )  format = renderer.TEXFMT.C3I8;
            else if( binNumChannels===4 && binFormat===0 )  format = renderer.TEXFMT.C4I8;
            else if( binNumChannels===1 && binFormat===10 ) format = renderer.TEXFMT.C1F32;
            else if( binNumChannels===2 && binFormat===10 ) format = renderer.TEXFMT.C2F32;
            else if( binNumChannels===3 && binFormat===10 ) format = renderer.TEXFMT.C3F32;
            else if( binNumChannels===4 && binFormat===10 ) format = renderer.TEXFMT.C4F32;
            else return;

            let buffer = new Uint8Array(data, 20); // skip 16 bytes (header of .bin)

            let rti = me.Sampler2Renderer(url.mSampler);

            texture.globject = renderer.CreateTexture(renderer.TEXTYPE.T3D, texture.mImage.mXres, texture.mImage.mYres, format, rti.mFilter, rti.mWrap, buffer);

            if( texture.globject===null )
            {
                console.log( "Error 4: loading Volume" ); 
                return { mFailed:true };
            }

            if (me.mTextureCallbackFun !== null)
            {
                me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.mPreview, true, 1, 1, -1.0, me.mID );
            }

            texture.loaded = true;

            // load icon for it
            texture.mPreview.onload = function()
            {
                if( me.mTextureCallbackFun!==null )
                    me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.mPreview, true, 1, 1, -1.0, me.mID );
            }
            texture.mPreview.src = url.mPreviewSrc;
        }
        xmlHttp.send("");


        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="volume")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="cubemap" )
    {
        texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;

        let rti = me.Sampler2Renderer(url.mSampler);

        if( assetID_to_cubemapBuferID(url.mID)!==-1 )
        {
            texture.mImage = new Image();
            texture.mImage.onload = function()
            {
                texture.loaded = true;
                if( me.mTextureCallbackFun!==null )
                    me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.mImage, true, 2, 1, -1.0, me.mID );
            }
            texture.mImage.src = "/media/previz/cubemap00.png";

            this.mEffect.ResizeCubemapBuffer(0, 1024, 1024 );

        }
        else
        {
            texture.image = [ new Image(), new Image(), new Image(), new Image(), new Image(), new Image() ];

            let numLoaded = 0;
            for (var i=0; i<6; i++ )
            {
                texture.image[i].mId = i;
                texture.image[i].crossOrigin = '';
                texture.image[i].onload = function()
                {
                    var id = this.mId;
                    numLoaded++;
                    if( numLoaded===6 )
                    {
                        texture.globject = renderer.CreateTextureFromImage(renderer.TEXTYPE.CUBEMAP, texture.image, renderer.TEXFMT.C4I8, rti.mFilter, rti.mWrap, rti.mVFlip);
                        texture.loaded = true;
                        if (me.mTextureCallbackFun !== null)
                            me.mTextureCallbackFun(me.mTextureCallbackObj, slot, texture.image[0], true, 2, 1, -1.0, me.mID);
                    }
                }

                if( i === 0) 
                {
                    texture.image[i].src = url.mSrc;
                } 
                else 
                {
                    let n = url.mSrc.lastIndexOf(".");
                    texture.image[i].src = url.mSrc.substring(0, n) + "_" + i + url.mSrc.substring(n, url.mSrc.length);
                }
            }
        }

        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="cubemap")) };

        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="webcam" )
    {
        texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;

        texture.video = document.createElement('video');
    	texture.video.width = 320;
    	texture.video.height = 240;
    	texture.video.autoplay = true;
    	texture.video.loop = true;
        texture.mForceMuted = this.mForceMuted;
        texture.mImage = null;

        let rti = me.Sampler2Renderer(url.mSampler);

        var loadImageInsteadOfWebCam = function()
        {
            texture.mImage = new Image();
            texture.mImage.onload = function()
            {
                texture.loaded = true;
                texture.globject = renderer.CreateTextureFromImage(renderer.TEXTYPE.T2D, texture.mImage, renderer.TEXFMT.C4I8, rti.mFilter, rti.mWrap, rti.mVFlip);
                if( me.mTextureCallbackFun!==null )
                    me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.mImage, true, 7, 1, -1.0, me.mID );
            }
            texture.mImage.src = "/media/previz/webcam.png";
        }
        
        loadImageInsteadOfWebCam();

        if( typeof navigator.getUserMedia !== "undefined"  && texture.mForceMuted===false )
        {
            texture.video.addEventListener("canplay", function (e)
            {
				try
				{
                    texture.mImage = null;
                    if( texture.globject != null )
                        renderer.DestroyTexture( texture.globject );
					texture.globject = renderer.CreateTextureFromImage(renderer.TEXTYPE.T2D, texture.video, renderer.TEXFMT.C4I8, rti.mFilter, rti.mWrap, rti.mVFlip);
					texture.loaded = true;
                }
                catch(e)
                {
                    loadImageInsteadOfWebCam();
	                alert( 'Your browser can not transfer webcam data to the GPU.');
                }
            } );

            navigator.mediaDevices.getUserMedia( 
                                { "video": { width: 1280, height: 720 }, "audio": false } )
                                .then( function(stream)
                                       {
                                            texture.video.srcObject = stream;
    	                               } )
                                .catch( function(error)
                                        {
                                            loadImageInsteadOfWebCam();
    		                                alert( 'Unable to capture WebCam. Please reload the page.' );
    	                                } );
        }
        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="mic" )
    {
        texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;
        texture.mForceMuted = this.mForceMuted;
        texture.mAnalyser = null;
        let num = 512;
        texture.mFreqData = new Uint8Array( num );
        texture.mWaveData = new Uint8Array( num );

        if( wa === null || typeof navigator.getUserMedia === "undefined" )
        {
            if( !texture.mForceMuted ) alert( "Shadertoy: Web Audio not implement in this browser" );
            texture.mForceMuted = true; 
        }

        if( texture.mForceMuted )
        {
            texture.globject = renderer.CreateTexture(renderer.TEXTYPE.T2D, num, 2, renderer.TEXFMT.C1I8, renderer.FILTER.LINEAR, renderer.TEXWRP.CLAMP, null)
            texture.loaded = true;
        }
        else
        {
        navigator.getUserMedia( { "audio": true },
                                function(stream)
                                {
                                  texture.globject = renderer.CreateTexture(renderer.TEXTYPE.T2D, 512, 2, renderer.TEXFMT.C1I8, renderer.FILTER.LINEAR, null)
                                  texture.mic = wa.createMediaStreamSource(stream);
                                  texture.mAnalyser = wa.createAnalyser();
                                  texture.mic.connect( texture.mAnalyser );
                                  texture.loaded = true;
    	                        },
                                function(error)
                                {
    		                        alert( 'Unable open Mic. Please reload the page.' );
    	                        } );
        }
        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="video" )
    {
    	texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;
        texture.video = document.createElement('video');
    	texture.video.loop = true;
        texture.video.preload = "auto";
        texture.video.mPaused = this.mForcePaused;
        texture.video.mMuted = true;//this.mForceMuted;
    	texture.video.muted  = true;//this.mForceMuted;
        if( this.mForceMuted===true )
            texture.video.volume = 0;
    	texture.video.autoplay = false;
        texture.video.hasFalled = false;
        
        let rti = me.Sampler2Renderer(url.mSampler);

        texture.video.addEventListener("canplay", function (e)
        {
            texture.video.play().then( function()
                                       {
                                           texture.video.mPaused = false;

                                           texture.globject = renderer.CreateTextureFromImage(renderer.TEXTYPE.T2D, texture.video, renderer.TEXFMT.C4I8, rti.mFilter, rti.mWrap, rti.mVFlip);
                                           texture.loaded = true;
            
                                           if( me.mTextureCallbackFun!=null )
                                               me.mTextureCallbackFun( me.mTextureCallbackObj, slot, texture.video, true, 3, 1, -1.0, me.mID );
                                        } )
                               .catch( function(error)
                                       {
                                           console.log( error );
                                       }
                                );
        } );

        texture.video.addEventListener( "error", function(e)
        {
               if( texture.video.hasFalled===true ) { alert("Error: cannot load video" ); return; }
               let str = texture.video.src;
               str = str.substr(0,str.lastIndexOf('.') ) + ".mp4";
               texture.video.src = str;
               texture.video.hasFalled = true;
        } );

        texture.video.src = url.mSrc;

        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="music" || url.mType==="musicstream" )
    {
    	texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = false;
        texture.audio = document.createElement('audio');
    	texture.audio.loop = true;
        texture.audio.mMuted = this.mForceMuted;
        texture.audio.mForceMuted = this.mForceMuted;
        texture.audio.muted = this.mForceMuted;
        if( this.mForceMuted===true )
            texture.audio.volume = 0;
        texture.audio.autoplay = false;
        texture.audio.hasFalled = false;
        texture.audio.mPaused = false;
        texture.audio.mSound = {};

        if( this.mForceMuted===false )
        {
            if(url.mType==="musicstream" && SC === null)
            {
                alert( "Shadertoy: Soundcloud could not be reached" );
                texture.audio.mForceMuted = true;
            }
            }

        if( wa === null && this.mForceMuted===false )
        {
            alert( "Shadertoy: Web Audio not implement in this browser" );
            texture.audio.mForceMuted = true;
        }

        if( texture.audio.mForceMuted )
        {
            texture.globject = renderer.CreateTexture(renderer.TEXTYPE.T2D, 512, 2, renderer.TEXFMT.C1I8, renderer.FILTER.LINEAR, renderer.TEXWRP.CLAMP, null);
            let num = 512;
            texture.audio.mSound.mFreqData = new Uint8Array( num );
            texture.audio.mSound.mWaveData = new Uint8Array( num );
            texture.loaded = true;
        }

        texture.audio.addEventListener( "canplay", function()
        {
            if( texture===null || texture.audio===null ) return;
            if( this.mForceMuted  ) return;
            if( texture.loaded === true ) return;

            texture.globject = renderer.CreateTexture(renderer.TEXTYPE.T2D, 512, 2, renderer.TEXFMT.C1I8, renderer.FILTER.LINEAR, renderer.TEXWRP.CLAMP, null)

            texture.audio.mSound.mSource   = wa.createMediaElementSource( texture.audio );
            texture.audio.mSound.mAnalyser = wa.createAnalyser();
            texture.audio.mSound.mGain     = wa.createGain();

            texture.audio.mSound.mSource.connect(   texture.audio.mSound.mAnalyser );
            texture.audio.mSound.mAnalyser.connect( texture.audio.mSound.mGain );
            texture.audio.mSound.mGain.connect(me.mGainNode);

            texture.audio.mSound.mFreqData = new Uint8Array( texture.audio.mSound.mAnalyser.frequencyBinCount );
            texture.audio.mSound.mWaveData = new Uint8Array( texture.audio.mSound.mAnalyser.frequencyBinCount );

            if( texture.audio.mPaused )
            {
                texture.audio.pause();
            }
            else
            {
                texture.audio.play().then( function() {} ).catch( function(e){console.log("error " + e);} );
            }
            texture.loaded = true;
        } );

        texture.audio.addEventListener( "error", function(e)
        {
               if( this.mForceMuted  ) return;

               if( texture.audio.hasFalled===true ) { return; }
               let str = texture.audio.src;
               str = str.substr(0,str.lastIndexOf('.') ) + ".ogg";
               texture.audio.src = str;
               texture.audio.hasFalled = true;
        } );

        if( !texture.audio.mForceMuted )
        {
            if(url.mType==="musicstream")
            {
                SC.resolve(url.mSrc, 
                    function(song) 
                    {
                        if( song.streamable===true )
                        {
                            texture.audio.crossOrigin = 'anonymous';
                            texture.audio.src = song.stream_url;
                            texture.audio.soundcloudInfo = song;
                        }
                        else
                        {
                            alert('Shadertoy: Soundcloud 3 - This track cannot be streamed' );
                        }
                    },
                    function(error)
                    {
                        if (me.mTextureCallbackFun!==null)
                        {
                            me.mTextureCallbackFun(me.mTextureCallbackObj, slot, {wave:null}, false, 4, 0, -1.0, me.mID);
                        }
                    } );
            } 
            else
            {
                texture.audio.src = url.mSrc;
            }
        }

        if (me.mTextureCallbackFun!==null)
        {
            if (url.mType === "music")            me.mTextureCallbackFun(me.mTextureCallbackObj, slot, {wave:null}, false, 4, 0, -1.0, me.mID);
            else if (url.mType === "musicstream") me.mTextureCallbackFun(me.mTextureCallbackObj, slot, {wave:null, info: texture.audio.soundcloudInfo}, false, 8, 0, -1.0, me.mID);
        }

        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="keyboard" )
    {
    	texture = {};
        texture.mInfo = url;
        texture.globject = null;
        texture.loaded = true;

        texture.keyboard = {};

        if( me.mTextureCallbackFun!==null )
            me.mTextureCallbackFun( me.mTextureCallbackObj, slot, {mImage: keyboard.mIcon, mData: keyboard.mData}, false, 6, 1, -1.0, me.mID );

        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!="music") && 
                                                                (this.mInputs[slot].mInfo.mType!="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!="video")) };
        this.DestroyInput( slot );
        this.mInputs[slot] = texture;
        this.MakeHeader();
        return returnValue;
    }
    else if( url.mType==="buffer" )
    {
        texture = {};
        texture.mInfo = url;

        texture.image = new Image();
        texture.image.onload = function()
        {
            if( me.mTextureCallbackFun!==null )
                me.mTextureCallbackFun( me.mTextureCallbackObj, slot, {texture: texture.image, data:null}, true, 9, 1, -1.0, me.mID );
        }
        texture.image.src = url.mSrc;
        texture.id = assetID_to_bufferID( url.mID );
        texture.loaded = true;

        let returnValue = { mFailed:false, mNeedsShaderCompile: (this.mInputs[slot]===null ) || (
                                                                (this.mInputs[slot].mInfo.mType!=="texture") && 
                                                                (this.mInputs[slot].mInfo.mType!=="webcam") && 
                                                                (this.mInputs[slot].mInfo.mType!=="mic") && 
                                                                (this.mInputs[slot].mInfo.mType!=="music") && 
                                                                (this.mInputs[slot].mInfo.mType!=="musicstream") && 
                                                                (this.mInputs[slot].mInfo.mType!=="keyboard") && 
                                                                (this.mInputs[slot].mInfo.mType!=="video")) };

        this.DestroyInput( slot );
        this.mInputs[slot] = texture;

        this.mEffect.ResizeBuffer(texture.id, this.mEffect.mXres, this.mEffect.mYres, false );

        this.SetSamplerFilter(slot, url.mSampler.filter, buffers, cubeBuffers, true);
        this.SetSamplerVFlip(slot, url.mSampler.vflip);
        this.SetSamplerWrap(slot, url.mSampler.wrap, buffers);

        this.MakeHeader();
        return returnValue;
    }
    else
    {
        alert( "input type error" );
        return { mFailed: true };
    }

    return { mFailed: true };
}

EffectPass.prototype.Paint_Image = function( vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard )
{
    let times = [ 0.0, 0.0, 0.0, 0.0 ];

    let dates = [ d.getFullYear(), // the year (four digits)
                  d.getMonth(),	   // the month (from 0-11)
                  d.getDate(),     // the day of the month (from 1-31)
                  d.getHours()*60.0*60 + d.getMinutes()*60 + d.getSeconds()  + d.getMilliseconds()/1000.0 ];

    let mouse = [  mousePosX, mousePosY, mouseOriX, mouseOriY ];

    //------------------------
    
    let resos = [ 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0 ];
    let texIsLoaded = [0, 0, 0, 0 ];
    let texID = [ null, null, null, null];

    for (let i=0; i<this.mInputs.length; i++ )
    {
        let inp = this.mInputs[i];

        if( inp===null )
        {
        }
        else if( inp.mInfo.mType==="texture" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                texIsLoaded[i] = 1;
                resos[3*i+0] = inp.image.width;
                resos[3*i+1] = inp.image.height;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="volume" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                texIsLoaded[i] = 1;
                resos[3*i+0] = inp.mImage.mXres;
                resos[3*i+1] = inp.mImage.mYres;
                resos[3*i+2] = inp.mImage.mZres;
            }
        }
        else if( inp.mInfo.mType==="keyboard" )
        {
            texID[i] = keyboard.mTexture;
            texIsLoaded[i] = 1;
            resos[3*i+0] = 256;
            resos[3*i+1] = 3;
            resos[3*i+2] = 1;
        }
        else if( inp.mInfo.mType==="cubemap" )
        {
            if (inp.loaded === true)
            {
                let id = assetID_to_cubemapBuferID(inp.mInfo.mID);
                if( id!==-1 )
                {
                    texID[i] = cubeBuffers[id].mTexture[ cubeBuffers[id].mLastRenderDone ];
                    resos[3*i+0] = cubeBuffers[id].mResolution[0];
                    resos[3*i+1] = cubeBuffers[id].mResolution[1];
                    resos[3*i+2] = 1;
                    texIsLoaded[i] = 1;

                    // hack. in webgl2.0 we have samplers, so we don't need this crap here
                    let filter = this.mRenderer.FILTER.NONE;
                         if (inp.mInfo.mSampler.filter === "linear") filter = this.mRenderer.FILTER.LINEAR;
                    else if (inp.mInfo.mSampler.filter === "mipmap") filter = this.mRenderer.FILTER.MIPMAP;
                    this.mRenderer.SetSamplerFilter( texID[i], filter, false);
                }
                else
                {
                    texID[i] = inp.globject;
                    texIsLoaded[i] = 1;
                }
            }
        }
        else if( inp.mInfo.mType==="webcam" )
        {
            if( inp.loaded===true )
            {
                if( inp.mImage !== null )
                {
                    if( this.mTextureCallbackFun!==null )
                        this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.mImage, false, 7, 1, -1, this.mID );

                    texID[i] = inp.globject;
                    texIsLoaded[i] = 1;
                    resos[3*i+0] = inp.mImage.width;
                    resos[3*i+1] = inp.mImage.height;
                    resos[3*i+2] = 1;
                }
                else  if( inp.video.readyState === inp.video.HAVE_ENOUGH_DATA )
                {

                    if( this.mTextureCallbackFun!==null )
                        this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.video, false, 7, 1, -1, this.mID );

                    texID[i] = inp.globject;
                    this.mRenderer.UpdateTextureFromImage(inp.globject, inp.video);
                    if( inp.mInfo.mSampler.filter === "mipmap" )
                        this.mRenderer.CreateMipmaps(inp.globject);
                    resos[3*i+0] = inp.video.videoWidth;
                    resos[3*i+1] = inp.video.videoHeight;
                    resos[3*i+2] = 1;
                    texIsLoaded[i] = 1;
                }
            }
            else 
            {
                texID[i] = null;
                texIsLoaded[i] = 0;
                resos[3*i+0] = inp.video.width;
                resos[3*i+1] = inp.video.height;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="video" )
        {
            if( inp.video.mPaused === false )
            {
                if( this.mTextureCallbackFun!==null )
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.video, false, 3, 1, inp.video.currentTime, this.mID );
            }

            if( inp.loaded===true )
            { 
                times[i] = inp.video.currentTime;
                texID[i] = inp.globject;
                texIsLoaded[i] = 1;

      	        if( inp.video.mPaused === false )
      	        {
      	            this.mRenderer.UpdateTextureFromImage(inp.globject, inp.video);
                    if( inp.mInfo.mSampler.filter === "mipmap" )
                        this.mRenderer.CreateMipmaps(inp.globject);
                }
                resos[3*i+0] = inp.video.videoWidth;
                resos[3*i+1] = inp.video.videoHeight;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
        {
            if( inp.audio.mPaused === false && inp.audio.mForceMuted === false && inp.loaded===true )
            {
                if( wa !== null )
                {
                    inp.audio.mSound.mAnalyser.getByteFrequencyData(  inp.audio.mSound.mFreqData );
                    inp.audio.mSound.mAnalyser.getByteTimeDomainData( inp.audio.mSound.mWaveData );
                }

                if( this.mTextureCallbackFun!==null )
                {
                         if( inp.mInfo.mType==="music")       this.mTextureCallbackFun(this.mTextureCallbackObj, i, (wa === null) ? null : { wave : inp.audio.mSound.mFreqData }, false, 4, 1, inp.audio.currentTime, this.mID);
                    else if( inp.mInfo.mType==="musicstream") this.mTextureCallbackFun(this.mTextureCallbackObj, i, (wa === null) ? null : { wave : inp.audio.mSound.mFreqData, info : inp.audio.soundcloudInfo}, false, 8, 1, inp.audio.currentTime, this.mID);
                }
            }

            if( inp.loaded===true )
            {
                times[i] = inp.audio.currentTime;
                texID[i] = inp.globject;
                texIsLoaded[i] = 1;

                if( inp.audio.mForceMuted === true )
                {
                    times[i] = 10.0 + time;
                    let num = inp.audio.mSound.mFreqData.length;
                    for (let j=0; j<num; j++ )
                    {
                        let x = j / num;
                        let f =  (0.75 + 0.25*Math.sin( 10.0*j + 13.0*time )) * Math.exp( -3.0*x );

                        if( j<3 )
                            f =  Math.pow( 0.50 + 0.5*Math.sin( 6.2831*time ), 4.0 ) * (1.0-j/3.0);

                        inp.audio.mSound.mFreqData[j] = Math.floor(255.0*f) | 0;
                    }

                  //let num = inp.audio.mSound.mFreqData.length;
                    for (let j=0; j<num; j++ )
                    {
                        let f = 0.5 + 0.15*Math.sin( 17.0*time + 10.0*6.2831*j/num ) * Math.sin( 23.0*time + 1.9*j/num );
                        inp.audio.mSound.mWaveData[j] = Math.floor(255.0*f) | 0;
                    }

                }

      	        if( inp.audio.mPaused === false )
                {
      	            let waveLen = Math.min(inp.audio.mSound.mWaveData.length, 512);
      	            this.mRenderer.UpdateTexture(inp.globject, 0, 0, 512, 1, inp.audio.mSound.mFreqData);
      	            this.mRenderer.UpdateTexture(inp.globject, 0, 1, 512, 1, inp.audio.mSound.mWaveData);
                }

                resos[3*i+0] = 512;
                resos[3*i+1] = 2;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="mic" )
        {
            if( inp.loaded===false || inp.mForceMuted || wa === null || inp.mAnalyser == null )
            {
                    times[i] = 10.0 + time;
                    let num = inp.mFreqData.length;
                    for( let j=0; j<num; j++ )
                    {
                        let x = j / num;
                        let f =  (0.75 + 0.25*Math.sin( 10.0*j + 13.0*time )) * Math.exp( -3.0*x );

                        if( j<3 )
                            f =  Math.pow( 0.50 + 0.5*Math.sin( 6.2831*time ), 4.0 ) * (1.0-j/3.0);

                        inp.mFreqData[j] = Math.floor(255.0*f) | 0;
                    }

                    for( let j=0; j<num; j++ )
                    {
                        let f = 0.5 + 0.15*Math.sin( 17.0*time + 10.0*6.2831*j/num ) * Math.sin( 23.0*time + 1.9*j/num );
                        inp.mWaveData[j] = Math.floor(255.0*f) | 0;
                    }
            }
            else
            {
                inp.mAnalyser.getByteFrequencyData(  inp.mFreqData );
                inp.mAnalyser.getByteTimeDomainData( inp.mWaveData );
            }

            if( this.mTextureCallbackFun!==null )
                this.mTextureCallbackFun( this.mTextureCallbackObj, i, {wave:inp.mFreqData}, false, 5, 1, -1, this.mID );

            if( inp.loaded===true )
            {
                texID[i] = inp.globject;
                texIsLoaded[i] = 1;
                let waveLen = Math.min( inp.mWaveData.length, 512 );
                this.mRenderer.UpdateTexture(inp.globject, 0, 0, 512,     1, inp.mFreqData);
                this.mRenderer.UpdateTexture(inp.globject, 0, 1, waveLen, 1, inp.mWaveData);
                resos[3*i+0] = 512;
                resos[3*i+1] = 2;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="buffer" )
        {
            let id = inp.id;
            if( inp.loaded===true  )
            {
                texID[i] = buffers[id].mTexture[ buffers[id].mLastRenderDone ];
                texIsLoaded[i] = 1;
                resos[3*i+0] = xres;
                resos[3*i+1] = yres;
                resos[3*i+2] = 1;
                // hack. in webgl2.0 we have samplers, so we don't need this crap here
                let filter = this.mRenderer.FILTER.NONE;
                     if (inp.mInfo.mSampler.filter === "linear") filter = this.mRenderer.FILTER.LINEAR;
                else if (inp.mInfo.mSampler.filter === "mipmap") filter = this.mRenderer.FILTER.MIPMAP;
                this.mRenderer.SetSamplerFilter( texID[i], filter, false);
            }

            if( this.mTextureCallbackFun!==null )
            {
                this.mTextureCallbackFun( this.mTextureCallbackObj, i, {texture:inp.image, data:buffers[id].mThumbnailBuffer}, false, 9, 1, -1, this.mID );
            }
        }
    }

    this.mRenderer.AttachTextures( 4, texID[0], texID[1], texID[2], texID[3] );

    //-----------------------------------

    let prog = this.mProgram;

    //if( vrData!=null && this.mSupportsVR ) prog = this.mProgramVR;

    this.mRenderer.AttachShader(prog);

    this.mRenderer.SetShaderConstant1F(  "iTime", time);
    this.mRenderer.SetShaderConstant3F(  "iResolution", xres, yres, 1.0);
    this.mRenderer.SetShaderConstant4FV( "iMouse", mouse);
    this.mRenderer.SetShaderConstant1FV( "iChannelTime", times );              // OBSOLETE
    this.mRenderer.SetShaderConstant4FV( "iDate", dates );
    this.mRenderer.SetShaderConstant3FV( "iChannelResolution", resos );        // OBSOLETE
    this.mRenderer.SetShaderConstant1F(  "iSampleRate", this.mSampleRate);
    this.mRenderer.SetShaderTextureUnit( "iChannel0", 0 );
    this.mRenderer.SetShaderTextureUnit( "iChannel1", 1 );
    this.mRenderer.SetShaderTextureUnit( "iChannel2", 2 );
    this.mRenderer.SetShaderTextureUnit( "iChannel3", 3 );
    this.mRenderer.SetShaderConstant1I(  "iFrame", this.mFrame );
    this.mRenderer.SetShaderConstant1F(  "iTimeDelta", dtime);
    this.mRenderer.SetShaderConstant1F("iFrameRate", fps);

    this.mRenderer.SetShaderConstant1F(  "iCh0.time", times[0] );
    this.mRenderer.SetShaderConstant1F(  "iCh1.time", times[1] );
    this.mRenderer.SetShaderConstant1F(  "iCh2.time", times[2] );
    this.mRenderer.SetShaderConstant1F(  "iCh3.time", times[3] );
    this.mRenderer.SetShaderConstant3F(  "iCh0.size", resos[0], resos[ 1], resos[ 2] );
    this.mRenderer.SetShaderConstant3F(  "iCh1.size", resos[3], resos[ 4], resos[ 5] );
    this.mRenderer.SetShaderConstant3F(  "iCh2.size", resos[6], resos[ 7], resos[ 8] );
    this.mRenderer.SetShaderConstant3F(  "iCh3.size", resos[9], resos[10], resos[11] );
    this.mRenderer.SetShaderConstant1I(  "iCh0.loaded", texIsLoaded[0] );
    this.mRenderer.SetShaderConstant1I(  "iCh1.loaded", texIsLoaded[1] );
    this.mRenderer.SetShaderConstant1I(  "iCh2.loaded", texIsLoaded[2] );
    this.mRenderer.SetShaderConstant1I(  "iCh3.loaded", texIsLoaded[3] );

    let l1 = this.mRenderer.GetAttribLocation(this.mProgram, "pos");

    if( (vrData !== null) && this.mSupportsVR )
    {
        for (let i=0; i<2; i++ )
        {
            let ei = (i===0) ? vrData.mLeftEye : vrData.mRightEye;

            let vp = [i * xres / 2, 0, xres / 2, yres];

            this.mRenderer.SetViewport(vp);

            let fov = ei.mProjection;
            let corA = [ -fov[2], -fov[1], -1.0 ];
            let corB = [  fov[3], -fov[1], -1.0 ];
            let corC = [  fov[3],  fov[0], -1.0 ];
            let corD = [ -fov[2],  fov[0], -1.0 ];
            let apex = [ 0.0, 0.0, 0.0 ];

            let ma = invertFast( ei.mCamera );
            corA = matMulpoint( ma, corA ); 
            corB = matMulpoint( ma, corB ); 
            corC = matMulpoint( ma, corC ); 
            corD = matMulpoint( ma, corD ); 
            apex = matMulpoint( ma, apex ); 

            let corners = [ corA[0], corA[1], corA[2], 
                            corB[0], corB[1], corB[2], 
                            corC[0], corC[1], corC[2], 
                            corD[0], corD[1], corD[2],
                            apex[0], apex[1], apex[2]];

            this.mRenderer.SetShaderConstant3FV("unCorners", corners);
            this.mRenderer.SetShaderConstant4FV("unViewport", vp);

            this.mRenderer.DrawUnitQuad_XY(l1);
        }
    }
    else 
    {
        this.mRenderer.SetViewport([0, 0, xres, yres]);
        this.mRenderer.DrawFullScreenTriangle_XY( l1 );
    }

    this.mRenderer.DettachTextures();
}

EffectPass.prototype.iRenderSound = function(d, callback )
{
    let dates = [ d.getFullYear(), // the year (four digits)
                  d.getMonth(),	   // the month (from 0-11)
                  d.getDate(),     // the day of the month (from 1-31)
                  d.getHours()*60.0*60 + d.getMinutes()*60 + d.getSeconds() ];

    let resos = [ 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0 ];

    this.mRenderer.SetRenderTarget(this.mRenderFBO);

    this.mRenderer.SetViewport([0, 0, this.mTextureDimensions, this.mTextureDimensions]);
    this.mRenderer.AttachShader(this.mProgram);
    this.mRenderer.SetBlend( false );

    let texID = [null, null, null, null];
    for (let i = 0; i < this.mInputs.length; i++)
    {
        let inp = this.mInputs[i];

        if( inp===null )
        {
        }
        else if( inp.mInfo.mType==="texture" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                resos[3*i+0] = inp.image.width;
                resos[3*i+1] = inp.image.height;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="volume" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                resos[3*i+0] = inp.mImage.mXres;
                resos[3*i+1] = inp.mImage.mYres;
                resos[3*i+2] = inp.mImage.mZres;
            }
        }
    }

    this.mRenderer.AttachTextures(4, texID[0], texID[1], texID[2], texID[3]);

    let l2 = this.mRenderer.SetShaderConstantLocation(this.mProgram, "iTimeOffset");
    let l3 = this.mRenderer.SetShaderConstantLocation(this.mProgram, "iSampleOffset");
    this.mRenderer.SetShaderConstant4FV("iDate", dates);
    this.mRenderer.SetShaderConstant3FV("iChannelResolution", resos);
    this.mRenderer.SetShaderConstant1F("iSampleRate", this.mSampleRate);
    this.mRenderer.SetShaderTextureUnit("iChannel0", 0);
    this.mRenderer.SetShaderTextureUnit("iChannel1", 1);
    this.mRenderer.SetShaderTextureUnit("iChannel2", 2);
    this.mRenderer.SetShaderTextureUnit("iChannel3", 3);

    let l1 = this.mRenderer.GetAttribLocation(this.mProgram, "pos");

    //--------------------------------
    let numSamples = this.mTmpBufferSamples;
    let numBlocks = this.mPlaySamples / numSamples;
    for (let j=0; j<numBlocks; j++ )
    {
        let off = j*numSamples;
        
        this.mRenderer.SetShaderConstant1F_Pos(l2, off / this.mSampleRate);
        this.mRenderer.SetShaderConstant1I_Pos(l3, off );
        this.mRenderer.DrawUnitQuad_XY(l1);

        this.mRenderer.GetPixelData(this.mData, 0, this.mTextureDimensions, this.mTextureDimensions);

        callback( off, this.mData, numSamples );
    }

    this.mRenderer.DetachShader();
    this.mRenderer.DettachTextures();
    this.mRenderer.SetRenderTarget(null);
}

EffectPass.prototype.Paint_Sound = function( wa, d )
{
    let bufL = this.mBuffer.getChannelData(0); // Float32Array
    let bufR = this.mBuffer.getChannelData(1); // Float32Array

    this.iRenderSound( d, function(off, data, numSamples)
                         {
                            for( let i=0; i<numSamples; i++ )
                            {
                                bufL[off+i] = -1.0 + 2.0*(data[4*i+0]+256.0*data[4*i+1])/65535.0;
                                bufR[off+i] = -1.0 + 2.0*(data[4*i+2]+256.0*data[4*i+3])/65535.0;
                            }
                         }
                     );
}

EffectPass.prototype.SetUniforms = function(vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard )
{
    let times = [ 0.0, 0.0, 0.0, 0.0 ];

    let dates = [ d.getFullYear(), // the year (four digits)
                  d.getMonth(),	   // the month (from 0-11)
                  d.getDate(),     // the day of the month (from 1-31)
                  d.getHours()*60.0*60 + d.getMinutes()*60 + d.getSeconds()  + d.getMilliseconds()/1000.0 ];

    let mouse = [  mousePosX, mousePosY, mouseOriX, mouseOriY ];

    let resos = [ 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0, 0.0,0.0,0.0 ];

    //------------------------
    
    let texID = [ null, null, null, null];

    for( let i=0; i<this.mInputs.length; i++ )
    {
        let inp = this.mInputs[i];

        if( inp===null )
        {
        }
        else if( inp.mInfo.mType==="texture" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                resos[3*i+0] = inp.image.width;
                resos[3*i+1] = inp.image.height;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="volume" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = inp.globject;
                resos[3*i+0] = inp.mImage.mXres;
                resos[3*i+1] = inp.mImage.mYres;
                resos[3*i+2] = inp.mImage.mZres;
            }
        }
        else if( inp.mInfo.mType==="keyboard" )
        {
            texID[i] = keyboard.mTexture;
        }
        else if( inp.mInfo.mType=="cubemap" )
        {
            if (inp.loaded === true)
            {
                let id = assetID_to_cubemapBuferID(inp.mInfo.mID);
                if( id!==-1 )
                {
                    texID[i] = cubeBuffers[id].mTexture[ cubeBuffers[id].mLastRenderDone ];
                    resos[3*i+0] = cubeBuffers[id].mResolution[0];
                    resos[3*i+1] = cubeBuffers[id].mResolution[1];
                    resos[3*i+2] = 1;
    
                    // hack. in webgl2.0 we have samplers, so we don't need this crap here
                    let filter = this.mRenderer.FILTER.NONE;
                         if (inp.mInfo.mSampler.filter === "linear") filter = this.mRenderer.FILTER.LINEAR;
                    else if (inp.mInfo.mSampler.filter === "mipmap") filter = this.mRenderer.FILTER.MIPMAP;
                    this.mRenderer.SetSamplerFilter( texID[i], filter, false);
                }
                else
                {
                    texID[i] = inp.globject;
                }
            }

        }
        else if( inp.mInfo.mType==="webcam" )
        {
            if( inp.loaded===true )
            {
                if( inp.mImage !== null )
                {
                    texID[i] = inp.globject;
                    resos[3*i+0] = inp.mImage.width;
                    resos[3*i+1] = inp.mImage.height;
                    resos[3*i+2] = 1;
                }
                else  if( inp.video.readyState === inp.video.HAVE_ENOUGH_DATA )
                {
                    texID[i] = inp.globject;
                    resos[3*i+0] = inp.video.videoWidth;
                    resos[3*i+1] = inp.video.videoHeight;
                    resos[3*i+2] = 1;
                }
            }
            else 
            {
                texID[i] = null;
                resos[3*i+0] = inp.video.width;
                resos[3*i+1] = inp.video.height;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="video" )
        {
           if( inp.loaded===true )
           { 
                times[i] = inp.video.currentTime;
                texID[i] = inp.globject;
                resos[3*i+0] = inp.video.videoWidth;
                resos[3*i+1] = inp.video.videoHeight;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
        {
            if( inp.loaded===true )
            {
                times[i] = inp.audio.currentTime;
                texID[i] = inp.globject;

                if( inp.audio.mForceMuted === true )
                {
                    times[i] = 10.0 + time;
                }

                resos[3*i+0] = 512;
                resos[3*i+1] = 2;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="mic" )
        {
            if( inp.loaded===false || inp.mForceMuted || wa === null || inp.mAnalyser == null )
            {
                times[i] = 10.0 + time;
            }

            if( inp.loaded===true )
            {
                texID[i] = inp.globject;
                resos[3*i+0] = 512;
                resos[3*i+1] = 2;
                resos[3*i+2] = 1;
            }
        }
        else if( inp.mInfo.mType==="buffer" )
        {
            if( inp.loaded===true  )
            {
                texID[i] = buffers[inp.id].mTexture[ buffers[inp.id].mLastRenderDone ];
                resos[3*i+0] = buffers[inp.id].mResolution[0];
                resos[3*i+1] = buffers[inp.id].mResolution[1];
                resos[3*i+2] = 1;
            }
        }
    }

    this.mRenderer.AttachTextures( 4, texID[0], texID[1], texID[2], texID[3] );

    //-----------------------------------

    this.mRenderer.AttachShader(this.mProgram);

    this.mRenderer.SetShaderConstant1F(  "iTime", time);
    this.mRenderer.SetShaderConstant3F(  "iResolution", xres, yres, 1.0);
    this.mRenderer.SetShaderConstant4FV( "iMouse", mouse);
    this.mRenderer.SetShaderConstant1FV( "iChannelTime", times );              // OBSOLETE
    this.mRenderer.SetShaderConstant4FV( "iDate", dates );
    this.mRenderer.SetShaderConstant3FV( "iChannelResolution", resos );        // OBSOLETE
    this.mRenderer.SetShaderConstant1F(  "iSampleRate", this.mSampleRate);
    this.mRenderer.SetShaderTextureUnit( "iChannel0", 0 );
    this.mRenderer.SetShaderTextureUnit( "iChannel1", 1 );
    this.mRenderer.SetShaderTextureUnit( "iChannel2", 2 );
    this.mRenderer.SetShaderTextureUnit( "iChannel3", 3 );
    this.mRenderer.SetShaderConstant1I(  "iFrame", this.mFrame );
    this.mRenderer.SetShaderConstant1F(  "iTimeDelta", dtime);
    this.mRenderer.SetShaderConstant1F(  "iFrameRate", fps );

    this.mRenderer.SetShaderConstant1F(  "iChannel[0].time",       times[0] );
    this.mRenderer.SetShaderConstant1F(  "iChannel[1].time",       times[1] );
    this.mRenderer.SetShaderConstant1F(  "iChannel[2].time",       times[2] );
    this.mRenderer.SetShaderConstant1F(  "iChannel[3].time",       times[3] );
    this.mRenderer.SetShaderConstant3F(  "iChannel[0].resolution", resos[0], resos[ 1], resos[ 2] );
    this.mRenderer.SetShaderConstant3F(  "iChannel[1].resolution", resos[3], resos[ 4], resos[ 5] );
    this.mRenderer.SetShaderConstant3F(  "iChannel[2].resolution", resos[6], resos[ 7], resos[ 8] );
    this.mRenderer.SetShaderConstant3F(  "iChannel[3].resolution", resos[9], resos[10], resos[11] );
}

EffectPass.prototype.ProcessInputs = function(vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard )
{
    for (let i=0; i<this.mInputs.length; i++ )
    {
        let inp = this.mInputs[i];

        if( inp===null )
        {
        }
        else if( inp.mInfo.mType==="texture" )
        {
        }
        else if( inp.mInfo.mType==="volume" )
        {
        }
        else if( inp.mInfo.mType==="keyboard" )
        {
        }
        else if( inp.mInfo.mType==="cubemap" )
        {
        }
        else if( inp.mInfo.mType==="webcam" )
        {
            if( inp.loaded===true )
            {
                if( inp.mImage !== null )
                {
                    if( this.mTextureCallbackFun!==null )
                        this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.mImage, false, 7, 1, -1, this.mID );
                }
                else if( inp.video.readyState === inp.video.HAVE_ENOUGH_DATA )
                {
                    if( this.mTextureCallbackFun!==null )
                        this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.video, false, 7, 1, -1, this.mID );

                    this.mRenderer.UpdateTextureFromImage(inp.globject, inp.video);
                    if( inp.mInfo.mSampler.filter === "mipmap" )
                        this.mRenderer.CreateMipmaps(inp.globject);
                }
            }
        }
        else if( inp.mInfo.mType==="video" )
        {
            if( inp.video.mPaused === false )
            {
                if( this.mTextureCallbackFun!==null )
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, inp.video, false, 3, 1, inp.video.currentTime, this.mID );
            }

            if( inp.loaded===true )
            { 
      	        if( inp.video.mPaused === false )
      	        {
      	            this.mRenderer.UpdateTextureFromImage(inp.globject, inp.video);
                    if( inp.mInfo.mSampler.filter === "mipmap" )
                        this.mRenderer.CreateMipmaps(inp.globject);
                }
            }
        }
        else if( inp.mInfo.mType==="music" || inp.mInfo.mType==="musicstream" )
        {
            if( inp.audio.mPaused === false && inp.audio.mForceMuted === false && inp.loaded===true )
            {
                if( wa !== null )
                {
                    inp.audio.mSound.mAnalyser.getByteFrequencyData(  inp.audio.mSound.mFreqData );
                    inp.audio.mSound.mAnalyser.getByteTimeDomainData( inp.audio.mSound.mWaveData );
                }

                if( this.mTextureCallbackFun!==null )
                {
                         if( inp.mInfo.mType==="music")       this.mTextureCallbackFun(this.mTextureCallbackObj, i, (wa == null) ? null : { wave : inp.audio.mSound.mFreqData }, false, 4, 1, inp.audio.currentTime, this.mID);
                    else if( inp.mInfo.mType==="musicstream") this.mTextureCallbackFun(this.mTextureCallbackObj, i, (wa == null) ? null : { wave : inp.audio.mSound.mFreqData, info : inp.audio.soundcloudInfo}, false, 8, 1, inp.audio.currentTime, this.mID);
                }
            }

            if( inp.loaded===true )
            {
                if( inp.audio.mForceMuted === true )
                {
                    let num = inp.audio.mSound.mFreqData.length;
                    for (let j=0; j<num; j++ )
                    {
                        let x = j / num;
                        let f =  (0.75 + 0.25*Math.sin( 10.0*j + 13.0*time )) * Math.exp( -3.0*x );

                        if( j<3 )
                            f =  Math.pow( 0.50 + 0.5*Math.sin( 6.2831*time ), 4.0 ) * (1.0-j/3.0);

                        inp.audio.mSound.mFreqData[j] = Math.floor(255.0*f) | 0;
                    }

                  //let num = inp.audio.mSound.mFreqData.length;
                    for (let j=0; j<num; j++ )
                    {
                        let f = 0.5 + 0.15*Math.sin( 17.0*time + 10.0*6.2831*j/num ) * Math.sin( 23.0*time + 1.9*j/num );
                        inp.audio.mSound.mWaveData[j] = Math.floor(255.0*f) | 0;
                    }

                }

      	        if( inp.audio.mPaused === false )
                {
      	            let waveLen = Math.min(inp.audio.mSound.mWaveData.length, 512);
      	            this.mRenderer.UpdateTexture(inp.globject, 0, 0, 512, 1, inp.audio.mSound.mFreqData);
      	            this.mRenderer.UpdateTexture(inp.globject, 0, 1, 512, 1, inp.audio.mSound.mWaveData);
                }
            }
        }
        else if( inp.mInfo.mType==="mic" )
        {
            if( inp.loaded===false || inp.mForceMuted || wa === null || inp.mAnalyser == null )
            {
                    let num = inp.mFreqData.length;
                    for( let j=0; j<num; j++ )
                    {
                        let x = j / num;
                        let f =  (0.75 + 0.25*Math.sin( 10.0*j + 13.0*time )) * Math.exp( -3.0*x );

                        if( j<3 )
                            f =  Math.pow( 0.50 + 0.5*Math.sin( 6.2831*time ), 4.0 ) * (1.0-j/3.0);

                        inp.mFreqData[j] = Math.floor(255.0*f) | 0;
                    }

                    for( let j=0; j<num; j++ )
                    {
                        let f = 0.5 + 0.15*Math.sin( 17.0*time + 10.0*6.2831*j/num ) * Math.sin( 23.0*time + 1.9*j/num );
                        inp.mWaveData[j] = Math.floor(255.0*f) | 0;
                    }
            }
            else
            {
                inp.mAnalyser.getByteFrequencyData(  inp.mFreqData );
                inp.mAnalyser.getByteTimeDomainData( inp.mWaveData );
            }

            if( this.mTextureCallbackFun!==null )
                this.mTextureCallbackFun( this.mTextureCallbackObj, i, {wave:inp.mFreqData}, false, 5, 1, -1, this.mID );

            if( inp.loaded===true )
            {
                let waveLen = Math.min( inp.mWaveData.length, 512 );
                this.mRenderer.UpdateTexture(inp.globject, 0, 0, 512,     1, inp.mFreqData);
                this.mRenderer.UpdateTexture(inp.globject, 0, 1, waveLen, 1, inp.mWaveData);
            }
        }
        else if( inp.mInfo.mType==="buffer" )
        {
            if( inp.loaded===true  )
            {
                let id = inp.id;
                let texID = buffers[id].mTexture[ buffers[id].mLastRenderDone ];

                // hack. in webgl2.0 we have samplers, so we don't need this crap here
                let filter = this.mRenderer.FILTER.NONE;
                     if (inp.mInfo.mSampler.filter === "linear") filter = this.mRenderer.FILTER.LINEAR;
                else if (inp.mInfo.mSampler.filter === "mipmap") filter = this.mRenderer.FILTER.MIPMAP;
                this.mRenderer.SetSamplerFilter( texID, filter, false);
            }

            if( this.mTextureCallbackFun!==null )
            {
				let id = inp.id;
                this.mTextureCallbackFun( this.mTextureCallbackObj, i, {texture:inp.image, data:buffers[id].mThumbnailBuffer}, false, 9, 1, -1, this.mID );
            }
        }
    }
}

EffectPass.prototype.Paint_Cubemap = function( vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard, face )
{
    this.ProcessInputs(vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard, face );
    this.SetUniforms(vrData, wa, d, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard );

    let l1 = this.mRenderer.GetAttribLocation(this.mProgram, "pos");

    let vp = [0, 0, xres, yres];

    this.mRenderer.SetViewport(vp);

    let corA = [-1.0, -1.0, -1.0];
    let corB = [ 1.0, -1.0, -1.0];
    let corC = [ 1.0,  1.0, -1.0];
    let corD = [-1.0,  1.0, -1.0];
    let apex = [ 0.0,  0.0,  0.0];

         if( face===0 ) { corA=[ 1.0,  1.0,  1.0]; corB=[ 1.0,  1.0, -1.0]; corC=[ 1.0, -1.0, -1.0]; corD=[ 1.0, -1.0,  1.0]; }
    else if( face===1 ) { corA=[-1.0,  1.0, -1.0]; corB=[-1.0,  1.0,  1.0]; corC=[-1.0, -1.0,  1.0]; corD=[-1.0, -1.0, -1.0]; }
    else if( face===2 ) { corA=[-1.0,  1.0, -1.0]; corB=[ 1.0,  1.0, -1.0]; corC=[ 1.0,  1.0,  1.0]; corD=[-1.0,  1.0,  1.0]; }
    else if( face===3 ) { corA=[-1.0, -1.0,  1.0]; corB=[ 1.0, -1.0,  1.0]; corC=[ 1.0, -1.0, -1.0]; corD=[-1.0, -1.0, -1.0]; }
    else if( face===4 ) { corA=[-1.0,  1.0,  1.0]; corB=[ 1.0,  1.0,  1.0]; corC=[ 1.0, -1.0,  1.0]; corD=[-1.0, -1.0,  1.0]; }
    else if( face===5 ) { corA=[ 1.0,  1.0, -1.0]; corB=[-1.0,  1.0, -1.0]; corC=[-1.0, -1.0, -1.0]; corD=[ 1.0, -1.0, -1.0]; }

    let corners = [ corA[0], corA[1], corA[2], 
                    corB[0], corB[1], corB[2], 
                    corC[0], corC[1], corC[2], 
                    corD[0], corD[1], corD[2],
                    apex[0], apex[1], apex[2]];

    this.mRenderer.SetShaderConstant3FV("unCorners", corners);
    this.mRenderer.SetShaderConstant4FV("unViewport", vp);

    this.mRenderer.DrawUnitQuad_XY(l1);

    this.mRenderer.DettachTextures();
}

EffectPass.prototype.Paint = function( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, isPaused, bufferID, bufferNeedsMimaps, buffers, cubeBuffers, keyboard, effect )
{
    if( this.mType==="sound" )
    {
        if (this.mSoundShaderCompiled === true)
        {
            // make sure all textures are loaded
            for (let i=0; i<this.mInputs.length; i++ )
            {
                let inp = this.mInputs[i];
                if (inp === null) continue;

                if (inp.mInfo.mType === "texture" && !inp.loaded) return;
                if (inp.mInfo.mType === "cubemap" && !inp.loaded) return;
            }

            this.Paint_Sound(wa, da);
            this.mSoundShaderCompiled = false;
        }
        if (this.mFrame === 0)
        {
            if (this.mPlaying===true)
            {
                this.mPlayNode.disconnect();
                this.mPlayNode.stop();
                this.mPlayNode = null;
            }
            this.mPlaying = true;

            this.mPlayNode = wa.createBufferSource();
            this.mPlayNode.buffer = this.mBuffer;
            this.mPlayNode.connect(this.mGainNode);
            this.mPlayNode.start(0);
        }
        this.mFrame++;
    }
    else if( this.mType==="image" )
    {
        this.mRenderer.SetRenderTarget( null );
        this.Paint_Image( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard );
        this.mFrame++;
    }
    else if( this.mType==="common" )
    {
    }
    else if( this.mType==="buffer" )
    {
        this.mEffect.ResizeBuffer(bufferID, this.mEffect.mXres, this.mEffect.mYres, false );

        let buffer = buffers[bufferID];

        let dstID = 1 - buffer.mLastRenderDone;

        this.mRenderer.SetRenderTarget( buffer.mTarget[dstID] );
        this.Paint_Image( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard );

        // compute mipmaps if needd
        if( bufferNeedsMimaps )
        {
            this.mRenderer.CreateMipmaps( buffer.mTexture[dstID]);
        }

        // make thumbnail
        //if( this.mTextureCallbackFun != null )
        /*
        {
            this.mRenderer.SetRenderTarget( buffer.mThumbnailRenderTarget );
            let v = [0, 0, buffer.mThumbnailRes[0], buffer.mThumbnailRes[1]];
            this.mRenderer.SetBlend(false);
            this.mRenderer.SetViewport(v);
            this.mRenderer.AttachShader(this.mProgramCopy);
            let l1 = this.mRenderer.GetAttribLocation(this.mProgramCopy, "pos");
            this.mRenderer.SetShaderConstant4FV("v", v);
            this.mRenderer.AttachTextures(1, buffer.mTexture[dstID], null, null, null);
            this.mRenderer.DrawUnitQuad_XY(l1);
            this.mRenderer.DettachTextures();
            this.mRenderer.DetachShader();
            this.mRenderer.GetPixelData( new Uint8Array(buffer.mThumbnailBuffer.data.buffer), buffer.mThumbnailRes[0], buffer.mThumbnailRes[1] );
            this.mRenderer.SetRenderTarget(null);
        }
        */
        buffers[bufferID].mLastRenderDone = 1 - buffers[bufferID].mLastRenderDone;
        this.mFrame++;
    }
    else if( this.mType==="cubemap" )
    {
        this.mEffect.ResizeCubemapBuffer(bufferID, 1024, 1024, false );

        let buffer = cubeBuffers[bufferID];

        xres = buffer.mResolution[0];
        yres = buffer.mResolution[1];
        let dstID = 1 - buffer.mLastRenderDone;
        for( let face=0; face<6; face++ )
        {
            this.mRenderer.SetRenderTargetCubeMap( buffer.mTarget[dstID], face );
            this.Paint_Cubemap( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, buffers, cubeBuffers, keyboard, face );
        }
        this.mRenderer.SetRenderTargetCubeMap( null, 0 );

        // compute mipmaps if needd
        if( bufferNeedsMimaps )
        {
            this.mRenderer.CreateMipmaps( buffer.mTexture[dstID]);
        }
        cubeBuffers[bufferID].mLastRenderDone = 1 - cubeBuffers[bufferID].mLastRenderDone;

        this.mFrame++;
    }
}

EffectPass.prototype.StopOutput_Sound = function( wa )
{
    if( this.mPlayNode===null ) return;
    this.mPlayNode.disconnect();
}

EffectPass.prototype.ResumeOutput_Sound = function( wa )
{
    if( this.mPlayNode===null ) return;

    wa.resume()
    this.mPlayNode.connect( this.mGainNode );
}

EffectPass.prototype.StopOutput_Image = function( wa )
{
}

EffectPass.prototype.ResumeOutput_Image = function( wa )
{
}

EffectPass.prototype.StopOutput = function( wa )
{
    for (let j=0; j<this.mInputs.length; j++ )
        this.StopInput(j);

    if( this.mType==="sound" )
         this.StopOutput_Sound( wa );
    else
         this.StopOutput_Image( wa );
}

EffectPass.prototype.ResumeOutput = function( wa )
{
    for (let j=0; j<this.mInputs.length; j++ )
        this.ResumeInput(j);

    if( this.mType==="sound" )
         this.ResumeOutput_Sound( wa );
    else
         this.ResumeOutput_Image( wa );
}

EffectPass.prototype.GetCompilationTime = function()
{
    return this.mCompilationTime;
}

//============================================================================================================
function Screenshots()
{
    // private
    let mTexture = null;
    let mTarget = null;
    let mXres = 0;
    let mYres = 0;
    let mCubemapToEquirectProgram;
    let mRenderer = null;

    // public
    var me = {};

    me.Initialize = function(renderer)
    {
        mRenderer = renderer;
        let caps = mRenderer.GetCaps();
        let is20 = caps.mIsGL20;

        let vsSourceC, fsSourceC;
        if( is20 )
        {
            vsSourceC = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
            fsSourceC = "uniform samplerCube t; out vec4 outColor; void main() { vec2 px = gl_FragCoord.xy/vec2(4096.0,2048.0); vec2 an = 3.1415926535898 * (px*vec2(2.0, 1.0) - vec2(0.0,0.5)); vec3 rd = vec3(-cos(an.y) * sin(an.x), sin(an.y), cos(an.y) * cos(an.x)); outColor = texture(t, rd); }";
        }
        else
        {
            vsSourceC = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
            fsSourceC = "uniform samplerCube t; void main() { vec2 px = gl_FragCoord.xy/vec2(4096.0,2048.0); vec2 an = 3.1415926535898 * (px*vec2(2.0, 1.0) - vec2(0.0,0.5)); vec3 rd = vec3(-cos(an.y) * sin(an.x), sin(an.y), cos(an.y) * cos(an.x)); gl_FragColor = texture(t, rd); }";
        }

        let compileShader = function (worked, info)
        {
            if (worked === false)
            {
                console.log("Failed to compile cubemap resample shader (" + errorType + "): " + log);
            }
            else
            {
                mCubemapToEquirectProgram = info;
            }
        }
        mRenderer.CreateShader(vsSourceC, fsSourceC, false, true, compileShader);

        return true;
    };

    me.Allocate = function( xres, yres )
    {
        if( xres>mXres || yres>mYres )
        {
            let texture = mRenderer.CreateTexture(mRenderer.TEXTYPE.T2D, xres, yres, mRenderer.TEXFMT.C4F32, mRenderer.FILTER.NONE, mRenderer.TEXWRP.CLAMP, null);
            let target = mRenderer.CreateRenderTarget( texture, null, null, null, null, false);

            if( mXres!==0 )
            {
                mRenderer.DestroyTexture(mTexture);
                mRenderer.DestroyRenderTarget(mTarget);
            }

            mTexture = texture;
            mTarget = target;
            mXres = xres;
            mYres = yres;
        }
    };

    me.GetProgram = function()
    {
        return mCubemapToEquirectProgram;
    };
    me.GetTarget = function()
    {
        return mTarget;
    };

    return me;
};

//============================================================================================================

function Effect(vr, ac, canvas, callback, obj, forceMuted, forcePaused, resizeCallback, crashCallback )
{
    let xres = canvas.width;
    let yres = canvas.height;

    let me = this;
    this.mCanvas = canvas;
    this.mCreated = false;
    this.mRenderer = null;
    this.mAudioContext = ac;
    this.mGLContext = null;
    this.mWebVR = vr;
    this.mRenderingStereo = false;
    this.mXres = xres;
    this.mYres = yres;
    this.mForceMuted = forceMuted;
    if( ac===null ) this.mForceMuted = true;
    this.mForcePaused = forcePaused;
    this.mGainNode = null;
    this.mPasses = [];
    this.mFrame = 0;
    this.mTextureCallbackFun = callback;
    this.mTextureCallbackObj = obj;
    this.mMaxBuffers = 4;
    this.mMaxCubeBuffers = 1;
    this.mMaxPasses = this.mMaxBuffers + 1 + 1 + 1 + 1; // some day decouple passes from buffers (4 buffers + common + Imagen + sound + cubemap)
    this.mBuffers = [];
    this.mCubeBuffers = [];
    this.mScreenshotSytem = null;
    this.mCompilationTime = 0;
    this.mIsLowEnd = piIsMobile();

    this.mGLContext = piCreateGlContext(canvas, false, false, true, false); // need preserve-buffe to true in order to capture screenshots
    if (this.mGLContext === null)
    {
        return;
    }

    canvas.addEventListener("webglcontextlost", function (event)
        {
            event.preventDefault();
            crashCallback();
        }, false);

    this.mRenderer = piRenderer();
    if (!this.mRenderer.Initialize(this.mGLContext))
        return;

    this.mScreenshotSytem = Screenshots();
    if (!this.mScreenshotSytem.Initialize(this.mRenderer))
        return;

    var caps = this.mRenderer.GetCaps();
    this.mIs20 = caps.mIsGL20;
    this.mShaderTextureLOD = caps.mShaderTextureLOD;
    //-------------
    if( ac!==null )
    {   
        this.mGainNode = ac.createGain();
        if( !forceMuted )
        {
            this.mGainNode.connect( ac.destination);
        }
        if (this.mForceMuted )
            this.mGainNode.gain.value = 0.0;
        else
            this.mGainNode.gain.value = 1.0;
    }

    //-------------
    let vsSourceC, fsSourceC;
    if( this.mIs20 )
    {
        vsSourceC = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
        fsSourceC = "uniform vec4 v; uniform sampler2D t; out vec4 outColor; void main() { outColor = textureLod(t, gl_FragCoord.xy / v.zw, 0.0); }";
    }
    else
    {
        vsSourceC = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
        fsSourceC = "uniform vec4 v; uniform sampler2D t; void main() { gl_FragColor = texture2D(t, gl_FragCoord.xy / v.zw, -100.0); }";
    }

    this.mRenderer.CreateShader(vsSourceC, fsSourceC, false, true, function(worked, info)
        {
            if (worked === false) console.log("Failed to compile shader to copy buffers : " + info.mErrorStr);
            else me.mProgramCopy = info;
        });

    let vsSourceD, fsSourceD;
    if( this.mIs20 )
    {
        vsSourceD = "layout(location = 0) in vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
        fsSourceD = "uniform vec4 v; uniform sampler2D t; out vec4 outColor; void main() { vec2 uv = gl_FragCoord.xy / v.zw; outColor = texture(t, vec2(uv.x,1.0-uv.y)); }";
    }
    else
    {
        vsSourceD = "attribute vec2 pos; void main() { gl_Position = vec4(pos.xy,0.0,1.0); }";
        fsSourceD = "uniform vec4 v; uniform sampler2D t; void main() { vec2 uv = gl_FragCoord.xy / v.zw; gl_FragColor = texture2D(t, vec2(uv.x,1.0-uv.y)); }";
    }

    this.mRenderer.CreateShader(vsSourceD, fsSourceD, false, true, function (worked, info)
        {
            if (worked === false) console.log("Failed to compile shader to downscale buffers : " + info.mErrorStr);
            else me.mProgramDownscale = info;
        });


    // set all buffers and cubemaps to null
    for( let i=0; i<this.mMaxBuffers; i++ )
    {
        this.mBuffers[i] = { mTexture: [null, null], 
                             mTarget:  [null, null], 
                             mResolution: [0, 0],
                             mLastRenderDone: 0,
                             mThumbnailRenderTarget: null,
                             mThumbnailTexture: null,
                             mThumbnailBuffer:  null,
                             mThumbnailRes: [0, 0] };
    }

    for( let i=0; i<this.mMaxCubeBuffers; i++ )
    {
        this.mCubeBuffers[i] = { mTexture: [null, null], 
                                mTarget:  [null, null], 
                                mResolution: [0, 0],
                                mLastRenderDone: 0,
                                mThumbnailRenderTarget: null,
                                mThumbnailTexture: null,
                                mThumbnailBuffer:  null,
                                mThumbnailRes: [0, 0] };
    }

    //-------

    let keyboardData = new Uint8Array( 256*3 );
    for (let j=0; j<(256*3); j++ ) { keyboardData[j] = 0; }
    let kayboardTexture = this.mRenderer.CreateTexture( this.mRenderer.TEXTYPE.T2D, 256, 3, this.mRenderer.TEXFMT.C1I8, this.mRenderer.FILTER.NONE, this.mRenderer.TEXWRP.CLAMP, null);
    let keyboardImage = new Image();
    if( callback!==null )
        keyboardImage.src = "/img/keyboard.png"; // don't load PNG if no UI 
    this.mKeyboard = { mData: keyboardData, mTexture: kayboardTexture, mIcon: keyboardImage };

    let iResize = function( xres, yres )
    {
        me.mCanvas.width = xres;
        me.mCanvas.height = yres;
        me.mXres = xres;
        me.mYres = yres;
        me.ResizeBuffers(xres, yres);
        resizeCallback(xres, yres);
    };

    let bestAttemptFallback = function()
    {
        let devicePixelRatio = window.devicePixelRatio || 1;
        let xres = Math.round(me.mCanvas.offsetWidth  * devicePixelRatio) | 0;
        let yres = Math.round(me.mCanvas.offsetHeight * devicePixelRatio) | 0;
        iResize(xres, yres);
    };

    if(!window.ResizeObserver)
    {
        console.log("WARNING: This browser doesn't support ResizeObserver.");
        bestAttemptFallback();
        window.addEventListener("resize", bestAttemptFallback);
    }
    else
    {
        this.mRO = new ResizeObserver( function(entries, observer)
        {
            var entry = entries[0];
            if (!entry['devicePixelContentBoxSize'])
            {
                observer.unobserve(me.mCanvas);
                console.log("WARNING: This browser doesn't support ResizeObserver + device-pixel-content-box (2)");
                bestAttemptFallback();
                window.addEventListener("resize", bestAttemptFallback);
            }
            else
            {
                let box = entry.devicePixelContentBoxSize[0];
                let xres = box.inlineSize;
                let yres = box.blockSize;
                iResize(xres, yres);
            }
        });
        try
        {
            this.mRO.observe(this.mCanvas, { box: ["device-pixel-content-box"] });
            //this.mRO.observe(this.mCanvas);
        }
        catch (e)
        {
            console.log("WARNING: This browser doesn't support ResizeObserver + device-pixel-content-box (1)");
            bestAttemptFallback();
            window.addEventListener("resize", bestAttemptFallback);
        }
    }

    this.mCreated = true;
}

Effect.prototype.ResizeCubemapBuffer = function(i, xres, yres )
{
    let oldXres = this.mCubeBuffers[i].mResolution[0];
    let oldYres = this.mCubeBuffers[i].mResolution[1];

    if( this.mCubeBuffers[i].mTexture[0]===null || oldXres !== xres || oldYres !== yres )
    {
        let texture1 = this.mRenderer.CreateTexture(this.mRenderer.TEXTYPE.CUBEMAP,
            xres, yres,
            this.mRenderer.TEXFMT.C4F16,
            this.mRenderer.FILTER.LINEAR,
            this.mRenderer.TEXWRP.CLAMP, 
            null);
        let target1 = this.mRenderer.CreateRenderTargetCubeMap( texture1, null, false);

        let texture2 = this.mRenderer.CreateTexture(this.mRenderer.TEXTYPE.CUBEMAP,
            xres, yres,
            this.mRenderer.TEXFMT.C4F16,
            this.mRenderer.FILTER.LINEAR,
            this.mRenderer.TEXWRP.CLAMP, 
            null);

        let target2 = this.mRenderer.CreateRenderTargetCubeMap( texture2, null, false);

        // Store new buffers
        this.mCubeBuffers[i].mTexture = [texture1,texture2], 
        this.mCubeBuffers[i].mTarget =  [target1, target2 ], 
        this.mCubeBuffers[i].mLastRenderDone = 0;
        this.mCubeBuffers[i].mResolution[0] = xres;
        this.mCubeBuffers[i].mResolution[1] = yres;
    }
}

Effect.prototype.ResizeBuffer = function( i, xres, yres, skipIfNotExists )
{
    if( skipIfNotExists && this.mBuffers[i].mTexture[0]===null ) return;

    let oldXres = this.mBuffers[i].mResolution[0];
    let oldYres = this.mBuffers[i].mResolution[1];

    if( oldXres !== xres || oldYres !== yres )
    {
        let needCopy = (this.mBuffers[i].mTexture[0]!==null);

        let texture1 = this.mRenderer.CreateTexture(this.mRenderer.TEXTYPE.T2D,
            xres, yres,
            this.mRenderer.TEXFMT.C4F32,
            (needCopy) ? this.mBuffers[i].mTexture[0].mFilter : this.mRenderer.FILTER.NONE,
            (needCopy) ? this.mBuffers[i].mTexture[0].mWrap   : this.mRenderer.TEXWRP.CLAMP, 
            null);

        let texture2 = this.mRenderer.CreateTexture(this.mRenderer.TEXTYPE.T2D,
            xres, yres,
            this.mRenderer.TEXFMT.C4F32,
            (needCopy) ? this.mBuffers[i].mTexture[1].mFilter : this.mRenderer.FILTER.NONE,
            (needCopy) ? this.mBuffers[i].mTexture[1].mWrap   : this.mRenderer.TEXWRP.CLAMP, 
            null);

        let target1 = this.mRenderer.CreateRenderTarget( texture1, null, null, null, null, false);
        let target2 = this.mRenderer.CreateRenderTarget( texture2, null, null, null, null, false);

        if( needCopy )
        {
            let v = [0, 0, Math.min(xres, oldXres), Math.min(yres, oldYres)];
            this.mRenderer.SetBlend(false);
            this.mRenderer.SetViewport(v);
            this.mRenderer.AttachShader(this.mProgramCopy);
            let l1 = this.mRenderer.GetAttribLocation(this.mProgramCopy, "pos");
            let vOld = [0, 0, oldXres, oldYres];
            this.mRenderer.SetShaderConstant4FV("v", vOld);

            // Copy old buffers 1 to new buffer
            this.mRenderer.SetRenderTarget(target1);
            this.mRenderer.AttachTextures(1, this.mBuffers[i].mTexture[0], null, null, null);
            this.mRenderer.DrawUnitQuad_XY(l1);

            // Copy old buffers 2 to new buffer
            this.mRenderer.SetRenderTarget(target2);
            this.mRenderer.AttachTextures(1, this.mBuffers[i].mTexture[1], null, null, null);
            this.mRenderer.DrawUnitQuad_XY(l1);

            // Deallocate old memory
            this.mRenderer.DestroyTexture(this.mBuffers[i].mTexture[0]);
            this.mRenderer.DestroyRenderTarget(this.mBuffers[i].mTarget[0]);
            this.mRenderer.DestroyTexture(this.mBuffers[i].mTexture[1]);
            this.mRenderer.DestroyRenderTarget(this.mBuffers[i].mTarget[1]);
            //this.mRenderer.DestroyTexture(this.mBuffers[i].thumbnailTexture);
        }

        // Store new buffers
        this.mBuffers[i].mTexture = [texture1,texture2], 
        this.mBuffers[i].mTarget =  [target1, target2 ], 
        this.mBuffers[i].mLastRenderDone = 0;
        this.mBuffers[i].mResolution[0] = xres;
        this.mBuffers[i].mResolution[1] = yres;
    }
}

Effect.prototype.saveScreenshot = function(passid)
{
    let pass = this.mPasses[passid];

    if( pass.mType === "buffer" )
    {
        let bufferID = assetID_to_bufferID( this.mPasses[passid].mOutputs[0] );

        let texture = this.mBuffers[bufferID].mTarget[ this.mBuffers[bufferID].mLastRenderDone ];

        let numComponents = 3;
        let width = texture.mTex0.mXres;
        let height = texture.mTex0.mYres;
        let type = "Float"; // Other options Float, Half, Uint
        let bytes = new Float32Array(width * height * 4 );//numComponents);
        this.mRenderer.GetPixelDataRenderTarget( texture, bytes, width, height );
        let blob = piExportToEXR(width, height, numComponents, type, bytes);

        // Offer download automatically to the user
        piTriggerDownload("image.exr", blob);
    }
    else if( pass.mType === "cubemap" )
    {
        let xres = 4096;
        let yres = 2048;
        this.mScreenshotSytem.Allocate( xres, yres );

        let cubeBuffer = this.mCubeBuffers[0];

        let target = this.mScreenshotSytem.GetTarget();
        this.mRenderer.SetRenderTarget( target );

        let program = this.mScreenshotSytem.GetProgram();

        this.mRenderer.AttachShader(program);
        let l1 = this.mRenderer.GetAttribLocation(program, "pos");
        this.mRenderer.SetViewport( [0, 0, xres, yres] );
        this.mRenderer.AttachTextures(1, cubeBuffer.mTexture[ cubeBuffer.mLastRenderDone ], null, null, null);
        this.mRenderer.DrawUnitQuad_XY(l1);
        this.mRenderer.DettachTextures();
        this.mRenderer.SetRenderTarget( null );

        let data = new Float32Array(xres*yres*4);
        this.mRenderer.GetPixelDataRenderTarget( target, data, xres, yres );

        let blob = piExportToEXR(xres, yres, 3, "Float", data );
        piTriggerDownload("image.exr", blob);
    }
    else if( pass.mType === "sound" )
    {
        let offset = 0;
        const bits = 16;
        const numChannels = 2;
        let words = new Int16Array(60*pass.mSampleRate*numChannels );

        pass.iRenderSound( new Date(), function(off, data, numSamples)
                                         {
                                            for( let i=0; i<numSamples; i++ )
                                            {
                                                words[offset++] = (data[4*i+0]+256.0*data[4*i+1]) - 32767;
                                                words[offset++] = (data[4*i+2]+256.0*data[4*i+3]) - 32767;
                                            }
                                         }
                                     );

        let blob = piExportToWAV( 60*pass.mSampleRate, pass.mSampleRate, bits, numChannels, words);

        piTriggerDownload("sound.wav", blob);
    }    
}

Effect.prototype.ResizeBuffers = function(xres, yres)
{
    for (let i=0; i<this.mMaxBuffers; i++ )
    {
        this.ResizeBuffer(i, xres, yres, true);
    }
}

Effect.prototype.IsEnabledVR = function ()
{
    if (this.mRenderingStereo) return true;
    return false;
}

Effect.prototype.EnableVR = function()
{
    if( !this.mWebVR.IsSupported() ) return;
    if( this.mRenderingStereo ) return;

    this.mRenderingStereo = true;
    this.mWebVR.Enable();
}

Effect.prototype.DisableVR = function()
{
    if( !this.mWebVR.IsSupported() ) return;
    if( !this.mRenderingStereo ) return;

    this.mRenderingStereo = false;
    this.mWebVR.Disable();
}

Effect.prototype.GetTexture = function( passid, slot ) { return this.mPasses[passid].GetTexture( slot ); }
Effect.prototype.NewTexture = function (passid, slot, url) { return this.mPasses[passid].NewTexture( this.mAudioContext, slot, url, this.mBuffers, this.mCubeBuffers, this.mKeyboard ); }
Effect.prototype.SetOutputs = function( passid, slot, url ) { this.mPasses[passid].SetOutputs( slot, url ); }
Effect.prototype.SetOutputsByBufferID = function( passid, slot, id ) { this.mPasses[passid].SetOutputsByBufferID( slot, id ); }
Effect.prototype.GetAcceptsLinear = function (passid, slot) { return this.mPasses[passid].GetAcceptsLinear(slot); }
Effect.prototype.GetAcceptsMipmapping = function (passid, slot) { return this.mPasses[passid].GetAcceptsMipmapping(slot); }
Effect.prototype.GetAcceptsWrapRepeat = function (passid, slot) { return this.mPasses[passid].GetAcceptsWrapRepeat(slot); }
Effect.prototype.GetAcceptsVFlip = function (passid, slot) { return this.mPasses[passid].GetAcceptsVFlip(slot); }
Effect.prototype.SetSamplerFilter = function (passid, slot, str) { this.mPasses[passid].SetSamplerFilter(slot, str, this.mBuffers, this.mCubeBuffers); }
Effect.prototype.GetTranslatedShaderSource = function (passid) { return this.mPasses[passid].GetTranslatedShaderSource(); }
Effect.prototype.GetSamplerFilter = function (passid, slot) { return this.mPasses[passid].GetSamplerFilter(slot); }
Effect.prototype.SetSamplerWrap = function (passid, slot, str) { this.mPasses[passid].SetSamplerWrap(slot, str, this.mBuffers); }
Effect.prototype.GetSamplerWrap = function (passid, slot) { return this.mPasses[passid].GetSamplerWrap(slot); }
Effect.prototype.SetSamplerVFlip = function (passid, slot, str) { this.mPasses[passid].SetSamplerVFlip(slot, str); }
Effect.prototype.GetSamplerVFlip = function (passid, slot) { return this.mPasses[passid].GetSamplerVFlip(slot); }

Effect.prototype.GetHeaderSize = function (passid)
{
    return this.mPasses[passid].mHeaderLength + 
           this.mRenderer.GetShaderHeaderLines(1);
}

Effect.prototype.ToggleVolume = function()
{
    this.mForceMuted = !this.mForceMuted;

    // outp
    if (this.mForceMuted)
        this.mGainNode.gain.value = 0.0;
    else
        this.mGainNode.gain.value = 1.0;

    // inp
    let num = this.mPasses.length;
    for( let j=0; j<num; j++ )
    {
        for( let i=0; i<this.mPasses[j].mInputs.length; i++ )
        {
            if( this.mForceMuted )
                this.mPasses[j].MuteInput( this.mAudioContext, i );
            else
                this.mPasses[j].UnMuteInput( this.mAudioContext, i );
        }
    }

    return this.mForceMuted;
}

Effect.prototype.SetKeyDown = function( passid, k )
{
    if( this.mKeyboard.mData[ k + 0*256 ] == 255 ) return;

    this.mKeyboard.mData[ k + 0*256 ] = 255;
    this.mKeyboard.mData[ k + 1*256 ] = 255;
    this.mKeyboard.mData[ k + 2*256 ] = 255 - this.mKeyboard.mData[ k + 2*256 ];
    this.mRenderer.UpdateTexture( this.mKeyboard.mTexture, 0, 0, 256, 3, this.mKeyboard.mData );

    let num = this.mPasses.length;
    for (let j=0; j<num; j++ )
    {
        for (let i=0; i<this.mPasses[j].mInputs.length; i++ )
        {
            let inp = this.mPasses[j].mInputs[i];
            if( inp!==null && inp.mInfo.mType==="keyboard" )
            {
                if( this.mTextureCallbackFun!==null )
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, {mImage:this.mKeyboard.mIcon, mData: this.mKeyboard.mData}, false, 6, 1, -1.0, this.mPasses[j].mID );
            }
        }
    }
}

Effect.prototype.SetKeyUp = function( passid, k )
{
    this.mKeyboard.mData[ k + 0*256 ] = 0;
    this.mKeyboard.mData[ k + 1*256 ] = 0;
    this.mRenderer.UpdateTexture( this.mKeyboard.mTexture, 0, 0, 256, 3, this.mKeyboard.mData );

    let num = this.mPasses.length;
    for (let j=0; j<num; j++ )
    {
        for (let i=0; i<this.mPasses[j].mInputs.length; i++ )
        {
            let inp = this.mPasses[j].mInputs[i];
            if( inp!==null && inp.mInfo.mType==="keyboard" )
            {
                if( this.mTextureCallbackFun!==null )
                    this.mTextureCallbackFun( this.mTextureCallbackObj, i, {mImage:this.mKeyboard.mIcon, mData: this.mKeyboard.mData}, false, 6, 1, -1.0, this.mPasses[j].mID );
            }
        }
    }
}

Effect.prototype.StopOutputs = function()
{
    let wa = this.mAudioContext;

    let num = this.mPasses.length;
    for (let i=0; i<num; i++ )
    {
        this.mPasses[i].StopOutput( wa );
    }
}

Effect.prototype.ResumeOutputs = function()
{
    let wa = this.mAudioContext;

    let num = this.mPasses.length;
    for (let i=0; i<num; i++ )
    {
        this.mPasses[i].ResumeOutput( wa );
    }
}

Effect.prototype.PauseInput = function( passid, id )
{
    return this.mPasses[passid].TooglePauseInput( this.mAudioContext, id );
}

Effect.prototype.ToggleMuteInput = function( passid, id )
{
    return this.mPasses[passid].ToggleMuteInput( this.mAudioContext, id );
}

Effect.prototype.RewindInput = function( passid, id )
{
    this.mPasses[passid].RewindInput( this.mAudioContext, id );
}

Effect.prototype.UpdateInputs = function( passid, forceUpdate )
{
   this.mPasses[passid].UpdateInputs( this.mAudioContext, forceUpdate, this.mKeyboard );
}

Effect.prototype.ResetTime = function()
{
    this.mFrame = 0;
    this.mAudioContext.resume()

    let num = this.mPasses.length;
    for( let i=0; i<num; i++ )
    {
        this.mPasses[i].mFrame = 0;
        for( let j=0; j<this.mPasses[i].mInputs.length; j++ )
            this.mPasses[i].RewindInput(this.mAudioContext, j)
    }
}

Effect.prototype.RequestAnimationFrame = function (id)
{
    if (this.mRenderingStereo && this.mWebVR.IsPresenting())
    {
        this.mWebVR.RequestAnimationFrame(id);
    }
    else
    {
        requestAnimFrame(id);
    }
}

Effect.prototype.Paint = function(time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, isPaused)
{
    let wa = this.mAudioContext;
    let da = new Date();
    let vrData = null; if (this.mRenderingStereo) vrData = this.mWebVR.GetData();
    let xres = this.mXres / 1;
    let yres = this.mYres / 1;

    if( this.mFrame===0 )
    {
        for( let i=0; i<this.mMaxBuffers; i++ )
        {
            if( this.mBuffers[i].mTexture[0]!==null )
            {
                this.mRenderer.SetRenderTarget( this.mBuffers[i].mTarget[0] );
                this.mRenderer.Clear( this.mRenderer.CLEAR.Color, [0.0,0.0,0.0,0.0], 1.0, 0   );
                this.mRenderer.SetRenderTarget( this.mBuffers[i].mTarget[1] );
                this.mRenderer.Clear( this.mRenderer.CLEAR.Color, [0.0,0.0,0.0,0.0], 1.0, 0   );
				
				this.mRenderer.CreateMipmaps( this.mBuffers[i].mTexture[0] );
				this.mRenderer.CreateMipmaps( this.mBuffers[i].mTexture[1] );
            }
        }
        for( let i=0; i<this.mMaxCubeBuffers; i++ )
        {
            if( this.mCubeBuffers[i].mTexture[0]!==null )
            {
                for( let face=0; face<6; face++ )
                {
                    this.mRenderer.SetRenderTargetCubeMap( this.mCubeBuffers[i].mTarget[0], face );
                    this.mRenderer.Clear( this.mRenderer.CLEAR.Color, [0.0,0.0,0.0,0.0], 1.0, 0   );
                    this.mRenderer.SetRenderTargetCubeMap( this.mCubeBuffers[i].mTarget[1], face );
                    this.mRenderer.Clear( this.mRenderer.CLEAR.Color, [0.0,0.0,0.0,0.0], 1.0, 0   );
					this.mRenderer.CreateMipmaps( this.mCubeBuffers[i].mTexture[0] );
				    this.mRenderer.CreateMipmaps( this.mCubeBuffers[i].mTexture[1] );
                }
            }
        }
    }

    let num = this.mPasses.length;

    // render sound first
    for( let i=0; i<num; i++ )
    {
        if( this.mPasses[i].mType !== "sound" ) continue;
        if( this.mPasses[i].mProgram===null ) continue;
        this.mPasses[i].Paint( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, isPaused, null, false, this.mBuffers, this.mCubeBuffers, this.mKeyboard, this );
    }

    // render buffers second
    for( let i=0; i<num; i++ )
    {
        if( this.mPasses[i].mType !== "buffer" ) continue;
        if( this.mPasses[i].mProgram===null ) continue;
        let bufferID = assetID_to_bufferID( this.mPasses[i].mOutputs[0] );

        // check if any downstream pass needs mipmaps when reading from this buffer
        let needMipMaps = false;
        for (let j=0; j<num; j++ )
        {
            for (let k=0; k<this.mPasses[j].mInputs.length; k++ )
            {
                let inp = this.mPasses[j].mInputs[k];
                if( inp!==null && inp.mInfo.mType==="buffer" && inp.id === bufferID && inp.mInfo.mSampler.filter === "mipmap")
                {
                    needMipMaps = true;
                    break;
                }
            }
        }

        this.mPasses[i].Paint( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, isPaused, bufferID, needMipMaps, this.mBuffers, this.mCubeBuffers, this.mKeyboard, this );
    }

    // render cubemap buffers second
    for( let i=0; i<num; i++ )
    {
        if( this.mPasses[i].mType !== "cubemap" ) continue;
        if( this.mPasses[i].mProgram===null ) continue;
        let bufferID = 0;//assetID_to_bufferID( this.mPasses[i].mOutputs[0] );

        // check if any downstream pass needs mipmaps when reading from this buffer
        let needMipMaps = false;

        for (let j=0; j<num; j++ )
        {
            for (let k=0; k<this.mPasses[j].mInputs.length; k++ )
            {
                let inp = this.mPasses[j].mInputs[k];
                if( inp!==null && inp.mInfo.mType==="cubemap" )
                {
                    if( assetID_to_cubemapBuferID(inp.mInfo.mID)===0 && inp.mInfo.mSampler.filter === "mipmap" )
                    {
                        needMipMaps = true;
                        break;
                    }
                }
            }
        }

        this.mPasses[i].Paint( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, isPaused, bufferID, needMipMaps, this.mBuffers, this.mCubeBuffers, this.mKeyboard, this );
    }

    // render image last
    for( let i=0; i<num; i++ )
    {
        if( this.mPasses[i].mType !== "image" ) continue;
        if( this.mPasses[i].mProgram===null ) continue;
        this.mPasses[i].Paint( vrData, wa, da, time, dtime, fps, mouseOriX, mouseOriY, mousePosX, mousePosY, xres, yres, isPaused, null, false, this.mBuffers, this.mCubeBuffers, this.mKeyboard, this );
    }   

    // erase keypresses
    for (let k=0; k<256; k++ )
    {
       this.mKeyboard.mData[ k + 1*256 ] = 0;
    }
    this.mRenderer.UpdateTexture( this.mKeyboard.mTexture, 0, 0, 256, 3, this.mKeyboard.mData );

    if( this.mRenderingStereo ) this.mWebVR.Finish();

    this.mFrame++;
}

Effect.prototype.NewShader = function( passid, preventCache, onResolve )
{
    let commonSourceCodes = [];
    for (let i=0; i<this.mPasses.length; i++ )
    {
        if( this.mPasses[i].mType==="common")
        {
            commonSourceCodes.push(this.mPasses[i].mSource);
        }
    }

    this.mPasses[passid].NewShader(commonSourceCodes, preventCache, onResolve );
}

Effect.prototype.GetNumPasses = function()
{
    return this.mPasses.length;
}

Effect.prototype.GetNumOfType = function(passtype)
{
    let id = 0;
    for (let j=0; j<this.mPasses.length; j++ )
    {
        if( this.mPasses[j].mType===passtype )
        {
            id++;
        }
    }
    return id;
}

Effect.prototype.GetPassType = function( id ) { return this.mPasses[id].mType; }
Effect.prototype.GetPassName = function( id ) { return this.mPasses[id].mName; }
Effect.prototype.GetCode = function( id ) { return this.mPasses[id].mSource; }
Effect.prototype.SetCode = function( id, source ) { this.mPasses[id].SetCode(source); }
Effect.prototype.GetError = function (id) { return this.mPasses[id].mError; }
Effect.prototype.GetErrorStr = function (id) { return this.mPasses[id].mErrorStr; }
Effect.prototype.GetErrorGlobal = function()
{
    for (let i = 0; i < this.mPasses.length; i++)
    {
        if (this.mPasses[i].mError)
        {
            return true;
        }
    }
    return false;
}

Effect.prototype.Load = function (jobj )
{
    if (jobj.ver !== "0.1")
    {
        console.log("Wrong Format");
        return false;
    }

    let numPasses = jobj.renderpass.length;

    if( numPasses<1 || numPasses>this.mMaxPasses )
    {
        console.log("Corrupted Shader - " + numPasses);
        return false;
    }

    this.mPasses = [];
    for (let j = 0; j < numPasses; j++)
    {
        let rpass = jobj.renderpass[j];

        // skip sound passes if in thumbnail mode
        if( this.mForceMuted && rpass.type === "sound" ) continue;

        let wpass = new EffectPass(this.mRenderer, this.mIs20, this.mIsLowEnd, this.mShaderTextureLOD,
                                   this.mTextureCallbackFun, this.mTextureCallbackObj, this.mForceMuted, this.mForcePaused, this.mGainNode,
                                   this.mProgramDownscale, j, this);

        wpass.Create(rpass.type, this.mAudioContext);

        let numInputs = rpass.inputs.length;

        for (let i = 0; i < 4; i++)
        {
            wpass.NewTexture(this.mAudioContext, i, null, null, null);
        }
        for (let i = 0; i < numInputs; i++)
        {
            let lid  = rpass.inputs[i].channel;
            let styp = rpass.inputs[i].type;
            let sid  = rpass.inputs[i].id;
            let ssrc = rpass.inputs[i].filepath;
            let psrc = rpass.inputs[i].previewfilepath;
            let samp = rpass.inputs[i].sampler;

            wpass.NewTexture(this.mAudioContext, lid, { mType: styp, mID: sid, mSrc: ssrc, mSampler: samp, mPreviewSrc: psrc }, this.mBuffers, this.mCubeBuffers, this.mKeyboard);
        }

        for (let i = 0; i < 4; i++)
        {
            wpass.SetOutputs(i, null);
        }

        let numOutputs = rpass.outputs.length;
        for (let i = 0; i < numOutputs; i++)
        {
            let outputID = rpass.outputs[i].id;
            let outputCH = rpass.outputs[i].channel;
            wpass.SetOutputs(outputCH, outputID);
        }

        // create some hardcoded names. This should come from the DB
        let rpassName = "";
        if (rpass.type === "common" ) rpassName = "Common";
        if (rpass.type === "sound"  ) rpassName = "Sound";
        if (rpass.type === "image"  ) rpassName = "Image";
        if (rpass.type === "buffer") rpassName = "Buffer " + String.fromCharCode(65 + assetID_to_bufferID(wpass.mOutputs[0]));
        if (rpass.type === "cubemap") rpassName = "Cube A";// " + String.fromCharCode(65 + assetID_to_bufferID(this.mPasses[j].mOutputs[0]));
        wpass.SetName(rpassName);
        wpass.SetCode(rpass.code);

        this.mPasses.push(wpass);
    }
    return true;
}

Effect.prototype.CompileSome = function ( passes, preventCache, onResolve )
{
    let me = this;

    let to = getRealTime();
    let allPromisses = [];
    for (let j = 0; j < passes.length; j++)
    {
        allPromisses.push(new Promise(function (resolve, reject)
        {
            me.NewShader(passes[j], preventCache, function () { resolve(1); });
        }));
    }

    // aggregated callback when all passes have been compiled
    Promise.all(allPromisses).then(function (values)
    {
        let totalError = false;
        for (let j = 0; j < me.mPasses.length; j++)
        {
            if (me.mPasses[j].mError)
            {
                totalError = true;
                break;
            }
        }
        me.mCompilationTime = getRealTime() - to;
        onResolve(!totalError);
    }).catch(console.log);
}

Effect.prototype.Compile = function (preventCache, onResolve )
{
    let me = this;

    let to = getRealTime();
    let allPromisses = [];
    let numPasses = this.mPasses.length;
    for (let j = 0; j < numPasses; j++)
    {
        allPromisses.push(new Promise(function (resolve, reject)
        {
            me.NewShader(j, preventCache, function () { resolve(1); });
        }));
    }

    // aggregated callback when all passes have been compiled
    Promise.all(allPromisses).then(function (values)
    {
        let totalError = false;
        for (let j = 0; j < numPasses; j++)
        {
            if (me.mPasses[j].mError)
            {
                totalError = true;
                break;
            }
        }
        me.mCompilationTime = getRealTime() - to;
        onResolve(!totalError);
    }).catch(console.log);
}

Effect.prototype.GetCompilationTime = function( id )
{
    return this.mPasses[id].GetCompilationTime()/1000.0;
}
Effect.prototype.GetTotalCompilationTime = function()
{
    return this.mCompilationTime/1000.0;
}

Effect.prototype.DestroyPass = function( id )
{
   this.mPasses[id].Destroy( this.mAudioContext );
   this.mPasses.splice(id, 1);
}

Effect.prototype.AddPass = function( passType, passName, onResolve )
{
    let shaderStr = null;

    if( passType==="sound"   ) shaderStr = "vec2 mainSound( int samp, float time )\n{\n    // A 440 Hz wave that attenuates quickly overt time\n    return vec2( sin(6.2831*440.0*time)*exp(-3.0*time) );\n}";
    if( passType==="buffer"  ) shaderStr = "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    fragColor = vec4(0.0,0.0,1.0,1.0);\n}";
    if( passType==="common"  ) shaderStr = "vec4 someFunction( vec4 a, float b )\n{\n    return a+b;\n}";
    if( passType==="cubemap" ) shaderStr = "void mainCubemap( out vec4 fragColor, in vec2 fragCoord, in vec3 rayOri, in vec3 rayDir )\n{\n    // Ray direction as color\n    vec3 col = 0.5 + 0.5*rayDir;\n\n    // Output to cubemap\n    fragColor = vec4(col,1.0);\n}";

    let id = this.GetNumPasses();
    this.mPasses[id] = new EffectPass( this.mRenderer, this.mIs20, this.mIsLowEnd, this.mShaderTextureLOD,
                                       this.mTextureCallbackFun, this.mTextureCallbackObj, this.mForceMuted, this.mForcePaused, this.mGainNode, 
                                       this.mProgramDownscale, id, this );

    this.mPasses[id].Create( passType, this.mAudioContext );
    this.mPasses[id].SetName( passName );
    this.mPasses[id].SetCode( shaderStr );
    this.NewShader(id, false, function ()
    {
        onResolve();
    });

    return { mId : id, mShader : shaderStr };
}

// this should be removed once we have MultiPass 2.0 and passes render to arbitrary buffers
Effect.prototype.IsBufferPassUsed = function( bufferID )
{
    for (let j=0; j<this.mPasses.length; j++ )
    {
        if( this.mPasses[j].mType !== "buffer" ) continue;
        if( this.mPasses[j].mOutputs[0] === bufferID_to_assetID(bufferID) ) return true;
    }
    return false;
}

Effect.prototype.Save = function()
{
    var result = {};

    result.ver = "0.1";

    result.renderpass = [];

    let numPasses = this.mPasses.length;
    for (let j=0; j<numPasses; j++ )
    {
        result.renderpass[j] = {};

        result.renderpass[j].outputs = new Array();
        for (let i = 0; i<4; i++ )
        {
            let outputID = this.mPasses[j].mOutputs[i];
            if( outputID===null ) continue;
            result.renderpass[j].outputs.push( { channel: i, id: outputID } );
        }
        result.renderpass[j].inputs = new Array();
        for (let i = 0; i<4; i++ )
        {
            if( this.mPasses[j].mInputs[i]===null ) continue;
            result.renderpass[j].inputs.push( {channel: i,
                                               type    : this.mPasses[j].mInputs[i].mInfo.mType,
                                               id      : this.mPasses[j].mInputs[i].mInfo.mID,
                                               filepath: this.mPasses[j].mInputs[i].mInfo.mSrc,
                                               sampler : this.mPasses[j].mInputs[i].mInfo.mSampler });
        }

        result.renderpass[j].code = this.mPasses[j].mSource;
        result.renderpass[j].name = this.mPasses[j].mName
        result.renderpass[j].description = "";
        result.renderpass[j].type = this.mPasses[j].mType;
    }

    result.flags = this.calcFlags();

    return result;
}

Effect.prototype.calcFlags = function ()
{
    let flagVR = false;
    let flagWebcam = false;
    let flagSoundInput = false;
    let flagSoundOutput = false;
    let flagKeyboard = false;
    let flagMultipass = false;
    let flagMusicStream = false;

    let numPasses = this.mPasses.length;
    for (let j = 0; j < numPasses; j++)
    {
        let pass = this.mPasses[j];

        if (pass.mType === "sound") flagSoundOutput = true;
        if (pass.mType === "buffer") flagMultipass = true;

        for (let i = 0; i < 4; i++)
        {
            if (pass.mInputs[i] === null) continue;

            if (pass.mInputs[i].mInfo.mType === "webcam") flagWebcam = true;
            else if (pass.mInputs[i].mInfo.mType === "keyboard") flagKeyboard = true;
            else if (pass.mInputs[i].mInfo.mType === "mic") flagSoundInput = true;
            else if (pass.mInputs[i].mInfo.mType === "musicstream") flagMusicStream = true;
        }

        let n1 = pass.mSource.indexOf("mainVR(");
        let n2 = pass.mSource.indexOf("mainVR (");
        if (n1 > 0 || n2 > 0) flagVR = true;
    }

    return {
        mFlagVR: flagVR,
        mFlagWebcam: flagWebcam,
        mFlagSoundInput: flagSoundInput,
        mFlagSoundOutput: flagSoundOutput,
        mFlagKeyboard: flagKeyboard,
        mFlagMultipass: flagMultipass,
        mFlagMusicStream: flagMusicStream
    };
}//</script>
//   <script>"use strict"

function buildInputsUI( me )
{
    me.mMarks = null;
    
    for( let i=0; i<4; i++ )
    {
        var par = document.getElementById( "texture" + i );
        var can = document.getElementById( "myUnitCanvas" + i);
        var bar = document.getElementById( "inputSelectorControls" + i);

          par.onmouseover = function(ev)
          {
              let ele = piGetSourceElement( ev );
              let pattern = "iChannel" + i;

              me.mMarks = new Array();
              var cm = me.mCodeEditor;
              const num = cm.lineCount();
              for( let j=0; j<num; j++ )
              {
                  var str = cm.getLine( j );
                  var res = str.indexOf( pattern );
                  if( res<0 ) continue;
                  cm.addLineClass( j, "background", "cm-highlight");
                  me.mMarks.push( j );
              }
          }

          par.onmouseout = function(ev)
          {
                let cm = me.mCodeEditor;
                if( me.mMarks===null ) return;
                let num = me.mMarks.length;
                for( let j=0; j<num; j++ )
                {
                    var l = me.mMarks.pop();
                    cm.removeLineClass( l, "background", "cm-highlight");
                }
                me.mMarks = null;
          }

          var ww = can.offsetWidth;
          var hh = can.offsetHeight;
          can.width = ww;
          can.height = hh;

          can.onclick = function (ev)
          {
              var passType = me.mEffect.GetPassType( me.mActiveDoc );
              overlay( i, passType );
          }

        var dei = document.getElementById( "myDeleteInput" + i);
        dei.id="myNoInput" + i;
        dei.onclick = function(ev) { me.SetTexture(i, { mType: null, mID: -1, mSrc: null, mSampler: {} }); }

        var z = document.createElement( "img" );
        z.src="/img/themes/" + gThemeName + "/pause.png";
        z.title="pause/resume";
        z.id="myPauseButton" + i;
        z.className = "uiButtonNew";
        z.style.visibility = 'hidden';
        z.onclick = function(ev) { var ele = piGetSourceElement(ev); var r = me.PauseInput( i ); if( r===true ) ele.src="/img/themes/" + gThemeName + "/play.png"; else ele.src="/img/themes/" + gThemeName + "/pause.png"; }
        bar.appendChild( z );

        z = document.createElement( "img" );
        z.src="/img/themes/" + gThemeName + "/rewind.png";
        z.title="rewind";
        z.id="myRewindButton" + i;
        z.className = "uiButtonNew";
        z.style.visibility = 'hidden';
        z.onclick = function(ev) { var ele = piGetSourceElement(ev); var r = me.RewindInput( i ); }
        bar.appendChild( z );

        z = document.createElement( "img" );
        z.src="/img/themes/" + gThemeName + "/speakerOff.png";
        z.title="mute";
        z.id="myMuteButton" + i;
        z.className = "uiButtonNew";
        z.style.visibility = 'hidden';
        z.onclick = function(ev) { var ele = piGetSourceElement(ev); var r = me.ToggleMuteInput( i ); if( r===true ) ele.src="/img/themes/" + gThemeName + "/speakerOff.png"; else ele.src="/img/themes/" + gThemeName + "/speakerOn.png";}
        bar.appendChild( z );

        z = document.createElement("img");
        z.src = "/img/themes/" + gThemeName + "/options.png";
        z.title = "sampler options";
        z.id = "mySamplingButton" + i;
        z.className = "uiButtonNew";

        z.onclick = function (ev) 
        { 
            var ele = piGetSourceElement(ev);
            var sam = document.getElementById("mySampler"+i);
            if( sam.className==="inputSampler visible")
            {
                sam.className = "inputSampler hidden";
            }
            else
            {
                var eleSamplerFilter = document.getElementById( "mySamplerFilter"+i);
                var eleSamplerWrap   = document.getElementById( "mySamplerWrap"+i);
                var eleSamplerVFlip  = document.getElementById( "mySamplerVFlip"+i);
                var eleSamplerVFlipLabel = document.getElementById( "mySamplerVFlipLabel"+i);

                eleSamplerFilter.value = me.GetSamplerFilter(i);
                eleSamplerWrap.value   = me.GetSamplerWrap(i);
                eleSamplerVFlip.checked = (me.GetSamplerVFlip(i)=="true");

                // see if this input accepts mipmapping
                var al = me.GetAcceptsLinear(i);
                eleSamplerFilter.options[1].style.display = (al==true)?'inherit':'none';
                var am = me.GetAcceptsMipmapping(i);
                eleSamplerFilter.options[2].style.display = (am==true)?'inherit':'none';
                var ar = me.GetAcceptsWrapRepeat(i);
                eleSamplerWrap.options[1].style.display = (ar==true)?'inherit':'none';
                var av = me.GetAcceptsVFlip(i);
                eleSamplerVFlip.style.display = (av==true)?'inherit':'none';
                eleSamplerVFlipLabel.style.display = (av==true)?'inherit':'none';

                sam.className = "inputSampler visible";
            }
        }

        bar.appendChild(z);
    }
}

function ShaderToy( parentElement, editorParent, passParent )
{
    if( parentElement===null ) return;
    if( editorParent===null ) return;
    if( passParent===null ) return;

    var me = this;

    this.mPassParent = passParent
    this.mNeedsSave = false;
    this.mAreThereAnyErrors = false;
    this.mAudioContext = null;
    this.mCreated = false;
    this.mHttpReq = null;
    this.mEffect = null;
    this.mTo = null;
    this.mTOffset = 0;
    this.mCanvas = null;
    this.mFPS = piCreateFPSCounter();
    this.mIsPaused = false;
    this.mForceFrame = false;
    this.mInfo = null;
    this.mCharCounter = document.getElementById("numCharacters");
    this.mCharCounterTotal = document.getElementById("numCharactersTotal");
    this.mEleCompilerTime = document.getElementById("compilationTime");
    this.mPass = [];
    this.mActiveDoc = 0;
    this.mIsEditorFullScreen = false;
    this.mFontSize = 0;
    this.mVR = null;
    this.mState = 0;

    buildInputsUI( this );

    this.mCanvas = document.getElementById("demogl");
    this.mCanvas.tabIndex = "0"; // make it react to keyboard
    this.mCanvas.width = this.mCanvas.offsetWidth;
    this.mCanvas.height = this.mCanvas.offsetHeight;
    this.iSetResolution(this.mCanvas.width, this.mCanvas.height);

    // ---------------------------------------

    this.mHttpReq = new XMLHttpRequest();
    this.mTo = getRealTime();
    this.mTf = 0;
    this.mRestarted = true;
    this.mFPS.Reset( this.mTo );
    this.mMouseIsDown = false;
	this.mMouseSignalDown = false;
    this.mMouseOriX = 0;
    this.mMouseOriY = 0;
    this.mMousePosX = 0;
    this.mMousePosY = 0;
    this.mIsRendering = false;

    // --- audio context ---------------------

    this.mAudioContext = piCreateAudioContext();

    if( this.mAudioContext===null )
    {
         //alert( "no audio!" );
    }

    // --- vr susbsystem ---------------------
/*
    this.mVR = new WebVR( function(b) 
                          {
                               var ele = document.getElementById("myVR");
                               if( b )
                                   ele.style.background="url('/img/themes/" + gThemeName + "/vrOn.png')";
                               else
                                   ele.style.background="url('/img/themes/" + gThemeName + "/vrOff.png')";
                          },
                          this.mCanvas );
*/
    // --- soundcloud context ---------------------
    this.mSoundcloudImage = new Image();
    this.mSoundcloudImage.src = "/img/themes/" + gThemeName + "/soundcloud.png";

    window.onfocus = function()
    {
        if( !this.mIsPaused )
        {
            me.mTOffset = me.mTf;
            me.mTo = getRealTime();
            me.mRestarted = true;
        }
    };

    var refreshCharsAndFlags = function()
    {
         me.setChars();
         //me.setFlags();
         setTimeout( refreshCharsAndFlags, 1500 );
    }
    // ---------------

    this.mErrors = new Array();

    var ekeys = null;
    if( navigator.platform.match("Mac") )
    {
        ekeys = { "Ctrl-S":    function(instance) { doSaveShader(); } ,
                  "Cmd-S":     function(instance) { doSaveShader(); } ,
                  "Alt-Enter": function(instance) { me.SetShaderFromEditor(false,true); },
                  "Cmd-Enter": function(instance) { me.SetShaderFromEditor(false,true); },
                  "Alt--":     function(instance) { me.decreaseFontSize(); },
                  "Cmd--":     function(instance) { me.decreaseFontSize(); },
                  "Alt-=":     function(instance) { me.increaseFontSize(); },
                  "Cmd-=":     function(instance) { me.increaseFontSize(); },
                  "Cmd-Right": function(instance) { me.ChangeTabRight(); },
                  "Cmd-Left":  function(instance) { me.ChangeTabLeft(); },
                  "Cmd-Down":  function(instance) { me.resetTime(false); },
                  "Alt-Down":  function(instance) { me.resetTime(false); },
                  "Alt-Up":    function(instance) { me.pauseTime(false); },
                  "Cmd-Up":    function(instance) { me.pauseTime(false); },
                  "Shift-Tab": "indentLess",
                  "Tab":       "indentMore"
                  };
    }
    else
    {
        ekeys = { "Ctrl-S":    function(instance) { doSaveShader(); } ,
                  "Alt-Enter": function(instance) { me.SetShaderFromEditor(false,true); },
                  "Alt--":     function(instance) { me.decreaseFontSize(); },
                  "Alt-=":     function(instance) { me.increaseFontSize(); },
                  "Alt-Right": function(instance) { me.ChangeTabRight(); },
                  "Alt-Left":  function(instance) { me.ChangeTabLeft(); },
                  "Alt-Down":  function(instance) { me.resetTime(false); },
                  "Alt-Up":    function(instance) { me.pauseTime(false); },
                  "Shift-Tab": "indentLess",
                  "Tab":       "indentMore"
                  };
    }

    this.mCodeEditor = CodeMirror( editorParent,
                                   {
                                       lineNumbers: true,
                                       matchBrackets: true,
                                       indentWithTabs: false,
                                       tabSize: 4,
                                       indentUnit: 4,
                                       mode: "text/x-glsl",
                                       smartIndent: false,
                                       electricChars: false,
                                       foldGutter: true,
                                       gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
                                       extraKeys: ekeys
        });
    //this.mCodeEditor.setOption("readOnly", true);

    this.mCodeEditor.on( "change",         function(instance,ev) { me.mNeedsSave = true; me.mPass[me.mActiveDoc].mDirty=true; me.mPass[me.mActiveDoc].mCharCountDirty = true; } );

    //--------------

    refreshCharsAndFlags(this);
    //this.setCompilationTime();
    this.mEleCompilerTime.textContent = "";

    document.getElementById("uiFontSelector").addEventListener('click', function (ev) {
        me.setFontSize(this.selectedIndex, true);
    });


    this.mCanvas.onmousedown = function(ev)
    {
        var rect = me.mCanvas.getBoundingClientRect();
        me.mMouseOriX = Math.floor((ev.clientX-rect.left)/(rect.right-rect.left)*me.mCanvas.width);
        me.mMouseOriY = Math.floor(me.mCanvas.height - (ev.clientY-rect.top)/(rect.bottom-rect.top)*me.mCanvas.height);
        me.mMousePosX = me.mMouseOriX;
        me.mMousePosY = me.mMouseOriY;
        me.mMouseIsDown = true;
		me.mMouseSignalDown = true;
        if( me.mIsPaused ) me.mForceFrame = true;
//        return false; // prevent mouse pointer change
    }
    this.mCanvas.onmousemove = function(ev)
    {
        if( me.mMouseIsDown )
        {
            var rect = me.mCanvas.getBoundingClientRect();
            me.mMousePosX = Math.floor((ev.clientX-rect.left)/(rect.right-rect.left)*me.mCanvas.width);
            me.mMousePosY = Math.floor(me.mCanvas.height - (ev.clientY-rect.top)/(rect.bottom-rect.top)*me.mCanvas.height);
            if( me.mIsPaused ) me.mForceFrame = true;
        }
    }
    this.mCanvas.onmouseup = function(ev)
    {
        me.mMouseIsDown = false;
        if( me.mIsPaused ) me.mForceFrame = true;
    }

    this.mCanvas.addEventListener("keydown",function(ev)
    {
        me.mEffect.SetKeyDown( me.mActiveDoc, ev.keyCode );
        if( me.mIsPaused ) me.mForceFrame = true;
        ev.preventDefault();
    },false);

    this.mCanvas.addEventListener("keyup",function(ev)
    {
        if ((ev.keyCode == 82) && ev.altKey)
        {
            let r = document.getElementById("myRecord");
            r.click();
        }
        
        me.mEffect.SetKeyUp( me.mActiveDoc, ev.keyCode );
        if( me.mIsPaused ) me.mForceFrame = true;
        ev.preventDefault();
    },false);

    document.getElementById("myResetButton").addEventListener('click',  function( ev )
    {
        me.resetTime(true);
    } );
    document.getElementById("myPauseButton").addEventListener('click',  function() 
    {
        me.pauseTime(true);
    } );

    document.getElementById("myVolume").addEventListener('click',  function(ev)
    {
        var res = me.mEffect.ToggleVolume();
        if( res )
            this.style.background="url('/img/themes/" + gThemeName + "/speakerOff.png')";
        else
            this.style.background="url('/img/themes/" + gThemeName + "/speakerOn.png')";
    } );
/*
    document.getElementById("myVR").addEventListener('click',  function(ev)
    {
        var vr = me.mVR.IsSupported();
        if( !vr )
        {
            alert( "WebVR API is not supported in this browser" );
        }
        else
        {
            if (me.mEffect.IsEnabledVR())
                me.mEffect.DisableVR();
            else
                me.mEffect.EnableVR();
        }
    } );
*/
    var mFullScreenExitHandler= function ()
                                { 
                                    if( piIsFullScreen() ) 
                                    { 
                                    } 
                                    else 
                                    { 
                                        //if( me.mVR.IsSupported() )
                                        {
                                            //me.mEffect.DisableVR();
                                        }
                                    } 
                                };
    this.mCanvas.addEventListener('webkitfullscreenchange', mFullScreenExitHandler, false);
    this.mCanvas.addEventListener('mozfullscreenchange', mFullScreenExitHandler, false);
    this.mCanvas.addEventListener('fullscreenchange', mFullScreenExitHandler, false);
    this.mCanvas.addEventListener('MSFullscreenChange', mFullScreenExitHandler, false);

    document.getElementById("myFullScreen").addEventListener('click',  function( ev )
    {
        piRequestFullScreen( me.mCanvas );
        me.mCanvas.focus(); // put mouse/keyboard focus on canvas
    } );

    //-------------------------

    let resizeCB = function (xres, yres)
    {
        me.mForceFrame = true;
        me.iSetResolution(xres, yres);
    };

    let crashCB = function ()
    {
        me.mIsPaused = true;
        alert('Shadertoy: ooops, your WebGL implementation has crashed!');
    };

    this.mEffect = new Effect(this.mVR, this.mAudioContext, this.mCanvas, this.RefreshTexturThumbail, this, false, false, resizeCB, crashCB);
    if( !this.mEffect.mCreated )
    {
        let ele = document.getElementById("noWebGL");
        ele.style.visibility = "visible";
        this.mCanvas.style.visibility = "hidden";

        let img = document.getElementById("noWebGL_ShaderImage");
        let url = "/media/shaders/" + gShaderID + ".jpg";
        img.onerror = function (ev) { };
        img.src = url;

        this.mIsPaused = true;
        this.mForceFrame = false;
        this.mCreated = false;
        return;
    }
    
    // --- mediaRecorder ---------------------

    this.mMediaRecorder = null;

    document.getElementById("myRecord").onclick = function(ev)
    {
        if (me.mMediaRecorder === null)
        {
            me.mMediaRecorder = piCreateMediaRecorder(
                function (b)
                {
                    var ele = document.getElementById("myRecord");
                    if (b)
                        ele.style.background = "url('/img/themes/" + gThemeName + "/recordOn.png')";
                    else
                        ele.style.background = "url('/img/themes/" + gThemeName + "/recordOff.png')";
                }
                , me.mCanvas);
        }
        if( me.mMediaRecorder === null )
        {
            let ele = document.getElementById("myRecord");
            ele.style.background = "url('/img/themes/" + gThemeName + "/recordDisabled.png')";
            alert('MediaRecord API is not supported in this browser');
            return;
        }

        if (me.mMediaRecorder.state === "inactive") 
        {
            me.mMediaRecorder.start();
        } 
        else 
        {
            me.mMediaRecorder.stop();
        }
    }

    this.mCreated = true;
}

ShaderToy.prototype.saveScreenshot = function()
{
    this.mEffect.saveScreenshot( this.mActiveDoc);
}

ShaderToy.prototype.setFontSize = function( id, fromUI )
{
    if (id < 0) id = 0;
    if (id > 5) id = 5;
    if (id === this.mFontSize) return;

    this.mFontSize = id;
    var edi = document.getElementById("editor");
    edi.style.fontSize = '' + (1.0 + 0.25*id ).toFixed(2) + 'em';

    this.mCodeEditor.refresh();

    if (fromUI)
    {
        var httpReq = new XMLHttpRequest();
        //httpReq.onload = function () { var jsn = httpReq.response; };
        httpReq.open("POST", "/shadertoy", true);
        httpReq.responseType = "json";
        httpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        httpReq.send("upo=1&sfs=" + id);
    }
    else
    {
        var ele = document.getElementById("uiFontSelector");
        ele.selectedIndex = id;
    }
}

ShaderToy.prototype.decreaseFontSize = function()
{
    this.setFontSize( this.mFontSize-1, true );
    var ele = document.getElementById("uiFontSelector");
    ele.selectedIndex = this.mFontSize;
}
ShaderToy.prototype.increaseFontSize = function()
{
    this.setFontSize( this.mFontSize+1, true );
    var ele = document.getElementById("uiFontSelector");
    ele.selectedIndex = this.mFontSize;
}
ShaderToy.prototype.GetNeedSave = function()
{
    return this.mNeedsSave;
}
ShaderToy.prototype.SetNeedSave = function(v)
{
    this.mNeedsSave = v;
}

ShaderToy.prototype.startRendering = function()
{
    this.mIsRendering = true;
    var me = this;

    function renderLoop2()
    {
        me.mEffect.RequestAnimationFrame(renderLoop2);

        if( me.mIsPaused && !me.mForceFrame )
        {
            me.mEffect.UpdateInputs( me.mActiveDoc, false );
            return;
        }
        me.mForceFrame = false;

        var time = getRealTime();

        var ltime = 0.0;
        var dtime = 0.0;
        if( me.mIsPaused ) 
        {
            ltime = me.mTf;
            dtime = 1000.0 / 60.0;
        }
        else
        {
            ltime = me.mTOffset + time - me.mTo;
            if( me.mRestarted )
                dtime = 1000.0/60.0;
            else
                dtime = ltime - me.mTf; 
            me.mTf = ltime;
        }
        me.mRestarted = false;

        var newFPS = me.mFPS.Count( time );

		let mouseOriX = Math.abs( me.mMouseOriX );
		let mouseOriY = Math.abs( me.mMouseOriY );
		if( !me.mMouseIsDown )     mouseOriX = -mouseOriX;
		if( !me.mMouseSignalDown ) mouseOriY = -mouseOriY;
        me.mMouseSignalDown = false;

        me.mEffect.Paint(ltime/1000.0, dtime/1000.0, me.mFPS.GetFPS(), mouseOriX, mouseOriY, me.mMousePosX, me.mMousePosY, me.mIsPaused );

        document.getElementById("myTime").textContent = (ltime/1000.0).toFixed(2);
        if( me.mIsPaused )
        {
        }
        else
        {
            if( newFPS )
            {
                document.getElementById("myFramerate").textContent = me.mFPS.GetFPS().toFixed(1) + " fps";
            }
        }
    }

    renderLoop2();
}

ShaderToy.prototype.iSetResolution = function (xres, yres)
{
    document.getElementById("myResolution").textContent =  "" + xres.toString() + " x "  + yres.toString();
}

ShaderToy.prototype.pauseTime = function(doFocusCanvas)
{
    if( !this.mIsPaused )
    {
        document.getElementById("myPauseButton").style.background="url('/img/themes/" + gThemeName + "/play.png')";
        this.mIsPaused = true;
        this.mEffect.StopOutputs();
    }
    else
    {
        document.getElementById("myPauseButton").style.background="url('/img/themes/" + gThemeName + "/pause.png')";
        this.mTOffset = this.mTf;
        this.mTo = getRealTime();
        this.mIsPaused = false;
        this.mRestarted = true;
        this.mEffect.ResumeOutputs();
        if( doFocusCanvas )
            this.mCanvas.focus();
    }
}

ShaderToy.prototype.resetTime = function(doFocusOnCanvas)
{
    this.mTOffset = 0;
    this.mTo = getRealTime();
    this.mTf = 0;
    this.mRestarted = true;
    this.mFpsTo = this.mTo;
    this.mFpsFrame = 0;
    this.mForceFrame = true;
    this.mEffect.ResetTime();
    if( doFocusOnCanvas )
        this.mCanvas.focus();
}

ShaderToy.prototype.SetErrors = function( isError, errorStr, fromScript )
{
    while( this.mErrors.length > 0 )
    {
        var mark = this.mErrors.pop();
        this.mCodeEditor.removeLineWidget( mark );
    }

    if( !isError )
    {
        this.mForceFrame = true;
    }
    else
    {
        var lineOffset = this.mEffect.GetHeaderSize( this.mActiveDoc );

        var lines = errorStr.split(/\r\n|\r|\n/);

        var maxLines = this.mCodeEditor.lineCount();

        for( let i=0; i<lines.length; i++ )
        {
            let parts = lines[i].split(":");
            if( parts.length===5 || parts.length===6 )
            {
                let lineAsInt = parseInt( parts[2] );

                if( isNaN(lineAsInt) ) // for non-webgl errors
                {
                  let msg = document.createElement("div");
                  msg.appendChild( document.createTextNode( "Unknown Error: " + lines[i] ));
                  msg.className = "errorMessage";
                  let mark = this.mCodeEditor.addLineWidget( 0, msg, {coverGutter: false, noHScroll: true} );
                  this.mErrors.push( mark );
                }
                else
                {
                  let lineNumber = lineAsInt - lineOffset;

                  let msg = document.createElement("div");
                  msg.appendChild(document.createTextNode( parts[3] + " : " + parts[4] ));
                  msg.className = "errorMessage";

                  if( lineNumber>maxLines ) lineNumber = maxLines;

                  let mark = this.mCodeEditor.addLineWidget( lineNumber, msg, {coverGutter: false, noHScroll: true} );
                  this.mErrors.push( mark );
                }
            }
            else if( lines[i] !== null && lines[i]!=="" && lines[i].length>1 && parts[0]!=="Warning")
            {
                let txt = "";
                if( parts.length===4 )
                    txt = parts[2] + " : " + parts[3];
                else
                    txt = "Unknown error: " + lines[i];

                let msg = document.createElement("div");
                msg.appendChild(document.createTextNode( txt ));
                msg.className = "errorMessage";
                let mark = this.mCodeEditor.addLineWidget( 0, msg, {coverGutter: false, noHScroll: true, above: true} );
                this.mErrors.push( mark );
            }
         }
    }
}

ShaderToy.prototype.AllowPublishing = function()
{
    if (this.mAreThereAnyErrors) return false;
    return true;
}

ShaderToy.prototype.GetTotalCompilationTime = function ()
{
    return this.mEffect.GetTotalCompilationTime();
}

ShaderToy.prototype.SetErrorsGlobal = function (areThereAnyErrors, fromScript)
{
    this.mAreThereAnyErrors = areThereAnyErrors;
    let eleWrapper = document.getElementById('editor');

    if( areThereAnyErrors===false )
    {
        this.mForceFrame = true;
        if( fromScript===false )
        {
            eleWrapper.classList.add("errorNo");
            setTimeout(function () { eleWrapper.classList.remove( "errorNo" ); }, 500 );
        }
    }
    else
    {
        if (fromScript === false)
        {
            eleWrapper.classList.add("errorYes");
            setTimeout(function () { eleWrapper.classList.remove("errorYes"); }, 500);
        }
    }
}

ShaderToy.prototype.SetTexture = function( slot, url )
{
    this.mNeedsSave = true;
    var res = this.mEffect.NewTexture( this.mActiveDoc, slot, url );
    if( res.mFailed===false )
    {
        this.mPass[this.mActiveDoc].mDirty = res.mNeedsShaderCompile;
    }
}
ShaderToy.prototype.PauseInput = function (id) { return this.mEffect.PauseInput(this.mActiveDoc, id); }
ShaderToy.prototype.ToggleMuteInput = function (id) { return this.mEffect.ToggleMuteInput(this.mActiveDoc, id); }
ShaderToy.prototype.RewindInput = function (id) { this.mEffect.RewindInput(this.mActiveDoc, id); }
ShaderToy.prototype.GetTexture = function( slot ) { return this.mEffect.GetTexture( this.mActiveDoc, slot ); }
ShaderToy.prototype.GetAcceptsLinear = function (slot) { return this.mEffect.GetAcceptsLinear(this.mActiveDoc, slot); }
ShaderToy.prototype.GetAcceptsMipmapping = function (slot) { return this.mEffect.GetAcceptsMipmapping(this.mActiveDoc, slot); }
ShaderToy.prototype.GetAcceptsWrapRepeat = function (slot) { return this.mEffect.GetAcceptsWrapRepeat(this.mActiveDoc, slot); }
ShaderToy.prototype.GetAcceptsVFlip = function (slot) { return this.mEffect.GetAcceptsVFlip(this.mActiveDoc, slot); }
ShaderToy.prototype.SetSamplerFilter = function (slot, str) { this.mEffect.SetSamplerFilter(this.mActiveDoc, slot, str);  this.mForceFrame = true; }
ShaderToy.prototype.GetSamplerFilter = function (slot) { return this.mEffect.GetSamplerFilter(this.mActiveDoc, slot); }
ShaderToy.prototype.SetSamplerWrap = function (slot, str) { this.mEffect.SetSamplerWrap(this.mActiveDoc, slot, str); this.mForceFrame = true; }
ShaderToy.prototype.GetSamplerWrap = function (slot) { return this.mEffect.GetSamplerWrap(this.mActiveDoc, slot); }
ShaderToy.prototype.SetSamplerVFlip = function (slot, str) { this.mEffect.SetSamplerVFlip(this.mActiveDoc, slot, str); this.mForceFrame = true; }
ShaderToy.prototype.GetSamplerVFlip = function (slot) { return this.mEffect.GetSamplerVFlip(this.mActiveDoc, slot); }

ShaderToy.prototype.ShowTranslatedSource = function()
{
    let str = this.mEffect.GetTranslatedShaderSource(this.mActiveDoc);
    let ve = document.getElementById("centerScreen" );
    doAlert( piGetCoords(ve), {mX:640,mY:512}, "Translated Shader Code", "<pre>"+str+"</pre>", false, null );
}

ShaderToy.prototype.UIStartCompiling = function (affectsUI)
{
    this.mState = 2;
    this.mEleCompilerTime.textContent = "Compiling...";
    if (affectsUI)
    {
        this.setActionsState(false);
    }
}

ShaderToy.prototype.UIEndCompiling = function (affectsUI)
{
    if( affectsUI ) this.setActionsState(true);

    let anyErrors = this.mEffect.GetErrorGlobal();
    this.setCompilationTime();
    this.setSaveOptions(anyErrors);

    for (let i = 0; i < this.mEffect.GetNumPasses(); i++)
    {
        let eleLab = document.getElementById("tab" + i);
        if (this.mEffect.GetError(i) )
            eleLab.classList.add("errorYes");
        else
            eleLab.classList.remove("errorYes");
    }

    this.SetErrors(this.mEffect.GetError(this.mActiveDoc),
                   this.mEffect.GetErrorStr(this.mActiveDoc), false);
    this.SetErrorsGlobal(anyErrors, false);
    this.setChars();
    this.setFlags();

    if (!anyErrors)
    {
        if (!this.mIsRendering)
        {
            this.startRendering();
            this.resetTime();
        }
        this.mForceFrame = true;
    }

    this.mState = 1;
}

ShaderToy.prototype.setActionsState = function (areEnabled)
{
    let eleSave = document.getElementById("saveButton");
    let eleComp = document.getElementById("compileButton");

    if (eleSave !== null)
        eleSave.disabled = !areEnabled;
    eleComp.disabled = !areEnabled;
}

ShaderToy.prototype.SetShaderFromEditor = function( forceall, affectsUI )
{
    if (this.mState!==1) return;

    let num = this.mEffect.GetNumPasses();
    for (let i = 0; i < num; i++)
    {
        if (this.mEffect.GetPassType(i) === "common" && this.mPass[i].mDirty)
        {
            forceall = true;
            break;
        }
    }

    this.UIStartCompiling(affectsUI);
    window.setTimeout(function ()
    {
        let passes = [];
        for( let i=0; i<num; i++ )
        {
            if( this.mPass[i].mDirty || forceall )
            {
                let shaderCode = this.mPass[i].mDocs.getValue();
                this.mEffect.SetCode(i, shaderCode);
                this.mPass[i].mDirty = false;
                this.mPass[i].mCharCount = minify(shaderCode).length;
                this.mPass[i].mCharCountDirty = false;
                passes.push( i );
            }
        }

        let me = this;
        this.mEffect.CompileSome(passes, true, function (worked)
        {
            me.UIEndCompiling(affectsUI)
        });
    }.bind(this), 10);
}


// guiID: { null, texture, cubemap, video, music, mic, keyb, webcam, musicstream, buffer }
ShaderToy.prototype.RefreshTexturThumbail = function( myself, slot, img, forceFrame, guiID, renderID, time, passID )
{
  if (passID !== myself.mActiveDoc) return;

  var canvas = document.getElementById('myUnitCanvas'+slot);
  var i0 = document.getElementById('myPauseButton' + slot);
  var i1 = document.getElementById('myRewindButton' + slot);
  var i2 = document.getElementById('myMuteButton' + slot);
  var i3 = document.getElementById('mySamplingButton' + slot);
  var i4 = document.getElementById('myNoInput' + slot);

  if (guiID === 0  )
  {
      i3.style.visibility = "hidden";
      i4.style.visibility = "hidden";
  }
  else
  {
      i3.style.visibility = "visible";
      i4.style.visibility = "visible";
  }

  if (guiID === 0 || guiID === 1 || guiID===2 || guiID===5 || guiID===6 || guiID===9  )
  {
      i0.style.visibility = "hidden";
      i1.style.visibility = "hidden";
      i2.style.visibility = "hidden";
  }
  else
  {
      i0.style.visibility = "visible";
      i1.style.visibility = "visible";
      i2.style.visibility = "visible";
  }

  var w = canvas.width;
  var h = canvas.height;

  var ctx = canvas.getContext('2d');

  if (guiID === 0)
  {
      ctx.fillStyle = "#000000";
      ctx.fillRect(0, 0, w, h);
  }
  else if (guiID === 1)
  {
      ctx.fillStyle = "#000000";
      if (renderID===0 )
          ctx.fillRect(0, 0, w, h+4);
      else 
      {
        ctx.fillRect(0, 0, w, h);  
        ctx.drawImage(img, 0, 0, w, h);
      }
  }
  else if (guiID === 2) 
  {
      ctx.fillStyle = "#000000";
      if (renderID===0 )
          ctx.fillRect(0, 0, w, h+4);
      else
          ctx.drawImage(img, 0, 0, w, h);
  }
  else if (guiID === 3) 
  {
      ctx.fillStyle = "#000000";
      if (renderID===0 )
          ctx.fillRect(0, 0, w, h+4);
      else
          ctx.drawImage(img, 0, 0, w, h);
  }
  else if (guiID === 4 || guiID === 5 || guiID === 8)
  {

      if (renderID === 0)
      {
          ctx.fillStyle = "#000000";
          ctx.fillRect(0, 0, w, h+4);

          ctx.strokeStyle = "#808080";
          ctx.lineWidth = 1;
          ctx.beginPath();
          let num = w / 2;
          for (let i = 0; i < num; i++)
          {
              let y = Math.sin(64.0 * 6.2831 * i / num + time) * Math.sin(2.0 * 6.2831 * i / num + time);
              let ix = w * i / num;
              let iy = h * (0.5 + 0.4 * y);
              if (i === 0) ctx.moveTo(ix, iy);
              else ctx.lineTo(ix, iy);
          }
          ctx.stroke();

          let str = "Audio not loaded";
          ctx.font = "normal bold 20px Arial";
          ctx.lineWidth = 4;
          ctx.strokeStyle = "#000000";
          ctx.strokeText(str, 14, h / 2);
          ctx.fillStyle = "#ff0000";
          ctx.fillText(str, 14, h / 2);

          document.getElementById("myPauseButton" + slot).src = "/img/themes/" + gThemeName + "/pause.png";
      }
      else 
      {
          var voff = 0;

          ctx.fillStyle = "#000000";
          ctx.fillRect(0, 0, w, h);
          ctx.fillStyle = "#ffffff";

          var numfft = img.wave.length;
          numfft /= 2;
          if (numfft > 512) numfft = 512;
          let num = 32;
          var numb = (numfft / num) | 0;
          var s = ((w - 8 * 2) / num);
          var k = 0;
          for (let i = 0; i < num; i++)
          {
              let f = 0.0;
              for (let j = 0; j < numb; j++)
              {
                  f += img.wave[k++];
              }
              f /= numb;
              f /= 255.0;

              let fr = f;
              let fg = 4.0 * f * (1.0 - f);
              let fb = 1.0 - f;

              let rr = (255.0 * fr) | 0;
              let gg = (255.0 * fg) | 0;
              let bb = (255.0 * fb) | 0;

              let decColor = 0x1000000 + bb + 0x100 * gg + 0x10000 * rr;
              ctx.fillStyle = '#' + decColor.toString(16).substr(1);

              let a = Math.max(2, f * (h - 2 * 20));
              ctx.fillRect(8 + i * s, h - voff - a, 3 * s / 4, a);
          }

          // If it is a music stream then we want to show extra information
          if (guiID===8)
          {
              let str = img.info.user.username + " - " + img.info.title;
              let x = w - 10.0 * (time % 45.0);
              ctx.font = "normal normal 12px Arial";
              ctx.strokeStyle = "#000000";
              ctx.lineWidth = 4;
              ctx.strokeText(str,x,32);
              ctx.fillStyle = "#ffffff";
              ctx.fillText(str,x,32);
              ctx.drawImage(myself.mSoundcloudImage, 45, 0);
          }
      }
  }
  else if (guiID === 6) //kyeboard
  {
    var thereskey = false;
    ctx.fillStyle = "#ffffff";
    for( let i=0; i<256; i++ )
    {
        let x = (w*i/256) | 0;
        if( img.mData[i]>0 )
        {
            thereskey = true;
            break;
        }
    }

    ctx.drawImage( img.mImage, 0, 0, w, h );

    if( thereskey )
    {
        ctx.fillStyle = "#ff8040";
        ctx.globalAlpha = 0.4;
        ctx.fillRect(0,0,w,h);
        ctx.globalAlpha = 1.0;
    }
  }
  else if (guiID === 7) 
  {
      ctx.fillStyle = "#000000";
      if (renderID === 0)
          ctx.fillRect(0, 0, w, h);
      else
          ctx.drawImage(img, 0, 0, w, h);
  }
  else if (guiID === 9)
  {
      if (renderID === 0)
      {
          ctx.fillStyle = "#808080";
          ctx.fillRect(0, 0, w, h);
      }
      else
      {
          ctx.drawImage(img.texture, 0, 0, w, h);
          if( img.data !== null )
          {
                ctx.putImageData( img.data, 0, 0,  0, 0, w, h ); 
          }
      }
  }

  if (time > 0.0)
  {
      let str = time.toFixed(2) + "s";
      ctx.font="normal normal 10px Arial";
      ctx.strokeStyle = "#000000";
      ctx.lineWidth = 4;
      ctx.strokeText(str,4,12);
      ctx.fillStyle = "#ffffff";
      ctx.fillText(str,4,12);
  }

  myself.mForceFrame = forceFrame;
}

ShaderToy.prototype.setChars = function()
{
    if( this.mPass.length===1 )
    {
        if( this.mPass[0].mCharCountDirty )
        {
            this.mPass[0].mCharCount = minify( this.mCodeEditor.getValue() ).length;
            this.mPass[0].mCharCountDirty = false;
        }
        this.mCharCounter.textContent = this.mPass[0].mCharCount + " chars";
        this.mCharCounterTotal.textContent = "";
    }
    else
    {
        let currentPassCount = 0;
        let globalCount = 0;
        for( let i=0; i<this.mPass.length; i++ )
        {
            if( this.mPass[i].mCharCountDirty )
            {
                this.mPass[i].mCharCount = minify( this.mPass[i].mDocs.getValue() ).length;
                this.mPass[i].mCharCountDirty = false;
            }
            if( i===this.mActiveDoc ) currentPassCount = this.mPass[i].mCharCount;
            globalCount += this.mPass[i].mCharCount;
        }
        this.mCharCounter.textContent = currentPassCount;
        this.mCharCounterTotal.textContent = " / " + globalCount + " chars";
    }
}

ShaderToy.prototype.setSaveOptions = function(areThereErrors)
{
    let elePubished = document.getElementById('published');
    if (elePubished === null) return;

    //let compileTime = this.mEffect.GetTotalCompilationTime();
    let disableOptions = areThereErrors;// || (compileTime > kMaxCompileTime);
    elePubished.options[0].disabled = disableOptions;
    elePubished.options[1].disabled = disableOptions;
    if (disableOptions)
    {
        let str = "You can only publish shaders that compile";// in less that " + Math.floor(kMaxCompileTime) + " seconds";
        elePubished.options[0].title = str;
        elePubished.options[1].title = str;
    }
    else
    {
        elePubished.options[0].title = "";
        elePubished.options[1].title = "";
    }
}

ShaderToy.prototype.setCompilationTime = function()
{
    /*
    if( this.mPass.length===1 )
    {
        let ti = this.mEffect.GetCompilationTime(0);
        this.mEleCompilerTime.textContent = "Compiled in " + ti.toFixed(1) + " secs";
    }
    else
    {
        let lti = this.mEffect.GetCompilationTime(this.mActiveDoc);
        let gti = this.mEffect.GetTotalCompilationTime();
        this.mEleCompilerTime.textContent = "Compiled in " + lti.toFixed(1) + " / " + gti.toFixed(1) + " secs";
    }
    */
    let ti = this.mEffect.GetTotalCompilationTime();
    this.mEleCompilerTime.textContent = "Compiled in " + ti.toFixed(1) + " secs";
}

ShaderToy.prototype.setFlags = function()
{
    if( this.mEffect===null ) return;

    var flags = this.mEffect.calcFlags();
/*
    var eleVR = document.getElementById( "myVR" );
    eleVR.style.visibility = ( flags.mFlagVR==true ) ? "visible" : "hidden";
*/
}

ShaderToy.prototype.showChars = function()
{
    let str = this.mCodeEditor.getValue();
    str = minify( str );
    let ve = document.getElementById("centerScreen" );
    doAlert( piGetCoords(ve), {mX:480,mY:-1}, "Minimal Shader Code, (" + str.length + " chars)", "<pre>"+htmlEntities(str)+"</pre>", false, null );
}

ShaderToy.prototype.ChangeTabLeft = function()
{
    var num = this.mEffect.GetNumPasses();
    // find tab pointing to current pass
    var i; for( i=0; i<num; i++ ) if( this.mTab2Pass[i] === this.mActiveDoc ) break;
    // move left
    i = (i - 1 + num ) % num;
    // change
    this.ChangePass( this.mTab2Pass[i]);
}

ShaderToy.prototype.ChangeTabRight = function()
{
    var num = this.mEffect.GetNumPasses();
    // find tab pointing to current pass
    var i; for( i=0; i<num; i++ ) if( this.mTab2Pass[i] === this.mActiveDoc ) break;
    // move right
    i = (i + 1) % num;
    // change
    this.ChangePass( this.mTab2Pass[i]);
}

ShaderToy.prototype.ChangePass = function( id )
{
    this.mActiveDoc = id;
    this.mCodeEditor.swapDoc( this.mPass[id].mDocs );
    this.setChars();
    this.setCompilationTime();
    this.SetErrors( this.mEffect.GetError(id), this.mEffect.GetErrorStr(id), false);
    this.mEffect.UpdateInputs( id, true );

    let num = this.mEffect.GetNumPasses();
    for( let i=0; i<num; i++ )
    {
        let eleLab = document.getElementById( "tab" + i );
        if( i===id )
            eleLab.classList.add("selected");
        else
            eleLab.classList.remove("selected");
    }

    {
    let passType = this.mEffect.GetPassType(id);
    let ele = document.getElementById( "textures" );
    ele.style.visibility = (passType === "common")?"hidden":"visible";

    ele = document.getElementById( "screenshotButton" );
    ele.style.visibility = (passType === "sound" || passType === "buffer" || passType === "cubemap") ? "visible" : "hidden";
    }
}

ShaderToy.prototype.KillPass = function( id )
{
    let ret = confirm(gStrWantDeletePass);
    if (ret !== true)
        return;

    let wasCommon = (this.mEffect.GetPassType(id) === "common");

    this.mNeedsSave = true;
    this.mEffect.DestroyPass( id );
    this.mPass.splice(id, 1);
    this.BuildTabs();

    let activePass = this.mActiveDoc;
    if( activePass >= id )
    {
        activePass--;
        if( activePass<0 ) activePass=0;
    }

    this.ChangePass(activePass);

    this.SetShaderFromEditor(wasCommon, true);
}

ShaderToy.prototype.AddPass = function(passType,passName, outputID)
{
    let iCompiled = function ()
    {
    };

    let res = this.mEffect.AddPass( passType, passName, iCompiled );
    let id = res.mId;

    this.mEffect.SetOutputsByBufferID(id, 0, outputID );

    this.mPass[id] = { mDocs: CodeMirror.Doc( res.mShader, "text/x-glsl"),
                       mDirty: false,
                       mCharCount: minify(res.mShader).length,
                       mCharCountDirty: false };
    this.BuildTabs();

    this.ChangePass( id );
    this.mNeedsSave = true;
}

//-------------------------
const kPassTypes = ["common", "buffer", "cubemap", "image", "sound"];

ShaderToy.prototype.AddPlusTabs = function( passes )
{
    var me = this;

    var numS = this.mEffect.GetNumOfType( "sound" );
    var numB = this.mEffect.GetNumOfType( "buffer" );
    var numC = this.mEffect.GetNumOfType( "common" );
    var numR = this.mEffect.GetNumOfType( "cubemap" );

    if( numS<1 || numB<4 || numC<1 || numR<1)
    {
        var eleCon = document.createElement( "div" );
        eleCon.className = "tabAddContainer";

        var eleSel = document.createElement( "select" );
        eleSel.className = "tabAddSelect";

        var eleOpt = document.createElement("option");
        eleOpt.value = "";
        eleOpt.text = "";
        eleOpt.hidden = true;
        eleSel.appendChild(eleOpt);

        if( numC<1 )
        {
            let eleOpt = document.createElement("option");
            eleOpt.value = 0;
            eleOpt.text = "Common";
            eleSel.appendChild(eleOpt);
        }

        if( numS<1 )
        {
            let eleOpt = document.createElement("option");
            eleOpt.value = 1;
            eleOpt.text = "Sound";
            eleSel.appendChild(eleOpt);
        }

        if( numB<4 )
        {
            for (let i=0; i<4; i++ )
            {
                let isused = me.mEffect.IsBufferPassUsed( i );
                if( isused ) continue;
                let eleOpt = document.createElement("option");
                eleOpt.value = 2+i;
                eleOpt.text = "Buffer " +  String.fromCharCode(65+i);
                eleSel.appendChild(eleOpt);
            }
        }

        if( numR<1 )
        {
            let eleOpt = document.createElement("option");
            eleOpt.value = 6;
            eleOpt.text = "Cubemap A";
            eleSel.appendChild(eleOpt);
        }

        eleSel.onchange = function( ev ) 
                            { 
                                let val = parseInt( this.value );
                                var outputID = null;
                                var passType = "common";
                                var passName = "Common";
                                if( val === 0 )
                                {
                                    outputID = null;
                                    passType = "common";
                                    passName = "Common";
                                }
                                else if( val === 1)
                                {
                                    outputID = null;
                                    passType = "sound";
                                    passName = "Sound";
                                }
                                else if( val >= 2 && val<=5 )
                                {
                                    outputID = val - 2;
                                    passType = "buffer";
                                    passName = "Buffer " + String.fromCharCode(65+outputID);
                                }
                                else if( val === 6)
                                {
                                    outputID = 0;
                                    passType = "cubemap";
                                    passName = "Cube " + "A";
                                }

                                me.AddPass(passType, passName, outputID ); 
                                ev.stopPropagation();
                            }

        eleCon.appendChild( eleSel );

        this.mPassParent.appendChild( eleCon );
    }
}

ShaderToy.prototype.AddTab = function( passType, passName, id )
{
    var me = this;

    var eleTab = document.createElement( "div" );
    eleTab.mNum = id;
    eleTab.onclick = function( ev ) { me.ChangePass( this.mNum ); }
    eleTab.id = "tab"+id;
    eleTab.className = "tab";

    if (this.mEffect.GetError(id) )
    {
        eleTab.classList.add("errorYes");
    }
    else 
    {
        eleTab.classList.remove("errorYes");
    }

    {
    let eleImg = document.createElement( "img" );
    eleImg.className = "tabImage";
    if( passType === "sound" )  eleImg.src = "/img/music.png";
    if( passType === "common" ) eleImg.src = "/img/common.png";
    if( passType === "image" )  eleImg.src = "/img/image.png";
    if( passType === "buffer" ) eleImg.src = "/img/buffer.png";
    if( passType === "cubemap" ) eleImg.src = "/img/cubemap.png";
    eleTab.appendChild( eleImg );
    }

    {
    var eleLab = document.createElement( "label" );
    eleLab.textContent = passName;
    eleTab.appendChild( eleLab );
    }

    if( passType !== "image" )
    {
      let eleImg = document.createElement( "img" );
      eleImg.src = "/img/closeSmall.png";
      eleImg.className = "tabClose";
      eleImg.mNum = id;
      eleImg.onclick = function( ev ) { me.KillPass( this.mNum ); ev.stopPropagation();}
      eleTab.appendChild( eleImg );
    }

    this.mPassParent.appendChild( eleTab );
}

ShaderToy.prototype.BuildTabs = function()
{
    this.mPassParent.innerHTML = '';
    this.AddPlusTabs();

    const num = this.mEffect.GetNumPasses();

    this.mTab2Pass = [];
    let n = 0;
    for (let j = 0; j < 5; j++)
    {
        for (let i = 0; i < num; i++)
        {
            var passType = this.mEffect.GetPassType(i);
            if (passType !== kPassTypes[j] ) continue;
            var passName = this.mEffect.GetPassName(i);
            this.AddTab(passType, passName, i);
            this.mTab2Pass[n++] = i;
        }
    }
}

ShaderToy.prototype.Load = function( jsn, preventCache, doResolve )
{
    try
    {
        this.mEleCompilerTime.textContent = "Compiling...";

        let me = this;

        if (!this.mEffect.Load(jsn)) return;

        this.mPass = [];

        var num = this.mEffect.GetNumPasses();
        for( let i=0; i<num; i++ )
        {
            let shaderCode = this.mEffect.GetCode(i);
            this.mPass[i] = { mDocs:   CodeMirror.Doc( shaderCode, "text/x-glsl"),
                              mDirty:  false,
                              mCharCount: minify(shaderCode).length,
                              mCharCountDirty: false
                             };
        }
        this.mCodeEditor.clearHistory()
        this.BuildTabs();
        this.ChangePass(0);
        this.setSaveOptions(true);
        this.resetTime();

        this.UIStartCompiling(true);
        this.mEffect.Compile(true /*true due to crash reporting*/, function(worked)
        {
            me.UIEndCompiling(true);

            let compilationTime = me.mEffect.GetTotalCompilationTime();
            if (!gIsMyShader && (compilationTime > kMaxCompileTime)) {
                iReportCrash(gShaderID);
            }
        });

        this.mInfo = jsn.info;

        return { mDownloaded  : true,
                 mDate        : jsn.info.date,
                 mViewed      : jsn.info.viewed,
                 mName        : jsn.info.name,
                 mUserName    : jsn.info.username,
                 mDescription : jsn.info.description,
                 mLikes       : jsn.info.likes,
                 mPublished   : jsn.info.published,
                 mHasLiked    : jsn.info.hasliked,
                 mTags        : jsn.info.tags,
                 mParentId    : jsn.info.parentid,
                 mParentName  : jsn.info.parentname };
    }
    catch( e )
    {
        console.log( e );
    }
}

ShaderToy.prototype.Save = function()
{
    var res = this.mEffect.Save();

    if( this.mNeedsSave )
    {
        for( let i=0; i<res.renderpass.length; i++ )
        {
            res.renderpass[i].code = this.mPass[i].mDocs.getValue();
        }
    }

    res.info = this.mInfo;

    return res;
}

// TODO: move this to Effect
ShaderToy.prototype.CheckShaderCorrectness = function ()
{
    let numPasses = this.mEffect.GetNumPasses();
    let countImage = 0;
    let countCommon = 0;
    let countSound = 0;
    let countCubemap = 0;
    let countBuffer = 0;
    for (let j = 0; j < numPasses; j++)
    {
        let passType = this.mEffect.GetPassType(j);
        if (passType === "image"  ) countImage++;
        if (passType === "common" ) countCommon++;
        if (passType === "sound"  ) countSound++;
        if (passType === "cubemap") countCubemap++;
        if (passType === "buffer" ) countBuffer++;
    }
    if (countImage !== 1) return false;
    if (countCommon  > 1) return false;
    if (countSound   > 1) return false;
    if (countCubemap > 1) return false;
    if (countBuffer  > 4) return false;
    return true;
}
//----------------------------------------------------------------------------

var gShaderToy = null;
var gCode = null;
var gIsLiked = 0;
var gRes = null;

function loadNew()
{
    const kk = {
	"ver" : "0.1",
	"info" : { "id":"-1", "date":"1358124981", "viewed":0, "name":"", "username": "None", "description":"", "likes":0, "hasliked":0, "tags":[], "published":0 },
    "flags" : { "mFlagVR" : "false", "mFlagWebcam" : "false", "mFlagSoundInput" : "false", "mFlagSoundOutput" : "false", "mFlagKeyboard" : "false" },
	"renderpass": [ { "inputs": [], "outputs": [ {"channel":0, "id":"4dfGRr" } ], "type":"image", "code": "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    // Normalized pixel coordinates (from 0 to 1)\n    vec2 uv = fragCoord/iResolution.xy;\n\n    // Time varying pixel color\n    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));\n\n    // Output to screen\n    fragColor = vec4(col,1.0);\n}", "name":"", "description":"" } ]
	};
    iLoadShader( [kk] );
}

//======= minify ==========================

function minify( str )
{
    function isSpace( str, i ) { return (str[i]===' ') || (str[i]==='\t'); }
    function isLine( str, i ) { return (str[i]==='\n'); }
    function replaceChars(str)
    {
        var dst = "";
        var num = str.length;
        var isPreprocessor = false;
        for( let i=0; i<num; i++ )
        {
                 if( str[i]==='#'  ) { isPreprocessor = true; }
            else if( str[i]==='\n' ) {  if( isPreprocessor ) { isPreprocessor = false; } else { dst = dst + " "; continue; } }
            else if( str[i]==='\r' ) { dst = dst + " "; continue; }
            else if( str[i]==='\t' ) { dst = dst + " "; continue; }
            else if( i<(num-1) && str[i]==='\\' && str[i+1]==='\n') { i+=1; continue; }
            dst = dst + str[i];
        }
        return dst;
    }
    function removeEmptyLines(str)
    {
        var dst = "";
        var num = str.length;
        var isPreprocessor = false;
        for( var i=0; i<num; i++ )
        {
            if( str[i]==='#' ) isPreprocessor = true;
            var isDestroyableChar = isLine(str,i);
            if( isDestroyableChar && !isPreprocessor ) continue;
            if( isDestroyableChar &&  isPreprocessor ) isPreprocessor = false;
            dst = dst + str[i];
        }
        return dst;
    }
    function removeMultiSpaces(str)
    {
        var dst = "";
        var num = str.length;
        for( var i=0; i<num; i++ )
        {
            if( isSpace(str,i) && (i===(num-1)) ) continue;
            if( isSpace(str,i) && isLine(str,i-1) ) continue;
            if( isSpace(str,i) && isLine(str,i+1) ) continue;
            if( isSpace(str,i) && isSpace(str,i+1) ) continue;
            dst = dst + str[i];
        }
        return dst;
    }
    function removeSingleSpaces(str)
    {
        var dst = "";
        var num = str.length;
        for( var i=0; i<num; i++ )
        {
            var iss = isSpace(str,i);
            if( i===0 && iss ) continue;

            if( i>0 )
            {
            if( iss && ( ( str[i-1]===';' ) || ( str[i-1]===',' ) || ( str[i-1]==='}' ) || ( str[i-1]==='{' ) ||
                         ( str[i-1]==='(' ) || ( str[i-1]===')' ) || ( str[i-1]==='+' ) || ( str[i-1]==='-' ) ||
                         ( str[i-1]==='*' ) || ( str[i-1]==='/' ) || ( str[i-1]==='?' ) || ( str[i-1]==='<' ) ||
                         ( str[i-1]==='>' ) || ( str[i-1]==='[' ) || ( str[i-1]===']' ) || ( str[i-1]===':' ) ||
                         ( str[i-1]==='=' ) || ( str[i-1]==='^' ) || ( str[i-1]==='%' ) || ( str[i-1]==='\n' ) ||
                         ( str[i-1]==='\r' ) ) ) continue;
            }
            if( i>1 )
            {
                if( iss && ( ( str[i-1]==='&' ) && ( str[i-2]==='&' ) ) ) continue;
                if( iss && ( ( str[i-1]==='|' ) && ( str[i-2]==='|' ) ) ) continue;
                if( iss && ( ( str[i-1]==='^' ) && ( str[i-2]==='^' ) ) ) continue;
                if( iss && ( ( str[i-1]==='!' ) && ( str[i-2]==='=' ) ) ) continue;
                if( iss && ( ( str[i-1]==='=' ) && ( str[i-2]==='=' ) ) ) continue;
            }

            if( iss && ( ( str[i+1]===';' ) || ( str[i+1]===',' ) || ( str[i+1]==='}' ) || ( str[i+1]==='{' ) ||
                         ( str[i+1]==='(' ) || ( str[i+1]===')' ) || ( str[i+1]==='+' ) || ( str[i+1]==='-' ) ||
                         ( str[i+1]==='*' ) || ( str[i+1]==='/' ) || ( str[i+1]==='?' ) || ( str[i+1]==='<' ) ||
                         ( str[i+1]==='>' ) || ( str[i+1]==='[' ) || ( str[i+1]===']' ) || ( str[i+1]===':' ) ||
                         ( str[i+1]==='=' ) || ( str[i+1]==='^' ) || ( str[i+1]==='%' ) || ( str[i+1]==='\n' ) ||
                         ( str[i+1]==='\r' ) ) ) continue;
            if( i<(num-2) )
            {
                if( iss && ( ( str[i+1]==='&' ) && ( str[i+2]==='&' ) ) ) continue;
                if( iss && ( ( str[i+1]==='|' ) && ( str[i+2]==='|' ) ) ) continue;
                if( iss && ( ( str[i+1]==='^' ) && ( str[i+2]==='^' ) ) ) continue;
                if( iss && ( ( str[i+1]==='!' ) && ( str[i+2]==='=' ) ) ) continue;
                if( iss && ( ( str[i+1]==='=' ) && ( str[i+2]==='=' ) ) ) continue;
            }
            dst = dst + str[i];
        }
        return dst;
    }

    function removeComments( str )
    {
        var dst = "";
        var num = str.length;
        var state = 0;
        for( var i=0; i<num; i++ )
        {
            if( i<=(num-2) )
            {
                if( state===0 && str[i]==='/' && str[i+1]==='*' ) { state = 1; i+=1; continue; }
                if( state===0 && str[i]==='/' && str[i+1]==='/' ) { state = 2; i+=1; continue; }
                if( state===1 && str[i]==="*" && str[i+1]==="/" ) { dst += ' '; state = 0; i+=1; continue; }
                if( state===2 && (str[i]==="\r" || str[i]==="\n") ) { state = 0; continue; }
            }
            if( state===0 )
                dst = dst + str[i];
        }
        return dst;
    }

    str = removeComments( str );
    str = replaceChars( str  );
    str = removeMultiSpaces( str );
    str = removeSingleSpaces( str );
    str = removeEmptyLines( str );
    return str;
}

//======= minify ==========================

function loadComments()
{
    try
    {
        var httpReq = new XMLHttpRequest();
        httpReq.onload = function()
        {
            var jsn = httpReq.response;
            updatepage( jsn );
        };
        httpReq.open( "POST", "/comment", true );
        httpReq.responseType = "json";
        httpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        httpReq.send( "s=" + gShaderID );
    }
    catch(e)
    {
        return;
    }
}

function iLoadShader( jsnShader )
{
    gRes = gShaderToy.Load( jsnShader[0], gIsMyShader );
    if( gRes.mDownloaded === false )
        return;

    document.title = (gRes.mName==="") ? "New" : gRes.mName;

    var st = document.getElementById( "shaderTitle" );      if( st ) { if( st.value === undefined ) { st.textContent = gRes.mName; st.title = gRes.mName; } else { st.value = gRes.mName;       } }
    var sd = document.getElementById( "shaderDescription"); if( sd ) { if( sd.value === undefined ) { sd.innerHTML = bbc2html(htmlEntities(gRes.mDescription),false); } else { sd.value = gRes.mDescription;} }
    var sp = document.getElementById( "published" );
    if( sp && sp!== undefined )
    {
        sp.selectedIndex = 2;
        for (let i = 0; i < sp.length; i++)
        {
            let option = sp.options[i];

            if (option.value == gRes.mPublished)
            {
                sp.selectedIndex = i;
                break;
            }
        }
    }

    updateLikes();
    var timeVar = "-";
    if( gRes.mDate !== 0 )
    {
        timeVar = piGetTime(gRes.mDate);
    }

    var shaderAuthorName = document.getElementById( "shaderAuthorName"); 
    if( shaderAuthorName) 
    {
        if (gRes.mUserName!=="") shaderAuthorName.innerHTML = "<a class='user' href='/user/" + htmlEntities(gRes.mUserName) +"'>" + htmlEntities(gRes.mUserName) + "</a>";
        else shaderAuthorName.innerHTML = "anonymous user";
    }
    var shaderAuthorDate = document.getElementById( "shaderAuthorDate"); 
    if( shaderAuthorDate ) shaderAuthorDate.innerHTML = timeVar;

    var txtHtml = "";
    var txtPlain = "";
    var numTags = gRes.mTags.length;
    for( let i=0; i<numTags; i++ )
    {
        txtHtml += "<a class='user' href='/results?query=tag%3D" + htmlEntities(gRes.mTags[i]) + "'>" + htmlEntities(gRes.mTags[i]) + "</a>";
        txtPlain += gRes.mTags[i];
        if( i !== (numTags-1) ) { txtHtml += ", "; txtPlain += ", "; }
    }
    var sts = document.getElementById( "shaderTags"); if( sts ) { if( sts.value === undefined ) sts.innerHTML = txtHtml; else sts.value = txtPlain; }

    var shareShader = document.getElementById("shaderShare");

    // like
    var shaderLike = document.getElementById("shaderLike");
    if( shaderLike !== null )
    {
        gIsLiked = gRes.mHasLiked;
        updateLikes();
        shaderLike.onclick = function()
        {
            var url = "s=" + gShaderID + "&l=" + ((gIsLiked==1)?0:1);
            var mHttpReq = new XMLHttpRequest();
            mHttpReq.open( "POST", "/shadertoy", true );
            mHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            mHttpReq.onload = function()
            {
                var jsn = mHttpReq.response;
                if( jsn===null ) return;
                if( jsn.result!==0 )
                {
                    if( gIsLiked===1 ) gRes.mLikes--; else gRes.mLikes++;
                    gIsLiked = 1 - gIsLiked;
                    updateLikes();
                }
            }
            mHttpReq.send( url );
        }
    }

    // Forked
    var shaderForkedFrom = document.getElementById("shaderForkedFrom");
    if (shaderForkedFrom !== null ) {
        if (gRes.mParentId !== "" ) {
            if  (gRes.mParentName !== "") {
                shaderForkedFrom.innerHTML = gStrFork + " <a class='user' href='/view/" + gRes.mParentId + "'>" + gRes.mParentName + "</a>";
            } else {
                shaderForkedFrom.innerHTML = gStrForkDeleted;
            }
        }
    }
}

function loadShader()
{
    gShaderToy.mState = 0;

    try
    {
        var httpReq = new XMLHttpRequest();
        httpReq.addEventListener( 'load', function ( event ) 
        { 
            var jsnShader = event.target.response;
            if (jsnShader === null) { alert("Error loading shader"); return; };
            gShaderToy.mState = 1;
            iLoadShader( jsnShader );
        }, false );
        httpReq.addEventListener( 'error', function () { alert( "Error loading shader" ); }, false );

        httpReq.open( "POST", "/shadertoy", true );
        httpReq.responseType = "json";
        httpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        var str = "{ \"shaders\" : [\"" + gShaderID + "\"] }";
        str = "s=" + encodeURIComponent(str) + "&nt=1&nl=1&np=1";
        httpReq.send( str  );
    }
    catch(e)
    {
         return;
    }
}

function watchInit()
{
    var editorParent = document.getElementById("editor");
    var viewerParent = document.getElementById("player");
    var passParent   = document.getElementById("passManager")

    // cancel/intercept browsers special keys
    document.addEventListener("keydown",function(e)
    {
        // intercept BACKSPACE
        if( e.keyCode===8 )
        {
            var ele = piGetSourceElement(e);

            if( ele.nodeName === "BODY")
            {
                e.preventDefault();
                return;
            }

            if( ele.id === "demogl" )
            {
                var me = gShaderToy;
                me.mEffect.SetKeyDown( me.mActiveDoc, e.keyCode );
                if( me.mIsPaused ) me.mForceFrame = true;
            }
        }
        // intercept CTRL+S (save the web page to disk)
        else if( e.keyCode===83 && (navigator.platform.match("Mac")?e.metaKey:e.ctrlKey))
        {
            e.preventDefault();
        }/*
        // intercept browsers default behaviour for CTRL+F (search)
        else if(e.keyCode===70 && (navigator.platform.match("Mac")?e.metaKey:e.ctrlKey))
        {
            e.preventDefault();
        }*/
    }, false);

    // cancel/intercept browsers default behaviour for ALT+LEFT (prev URL)
    if( !navigator.platform.match("Mac") )
        document.addEventListener("keydown",function(e){if(e.keyCode==37 && e.altKey){e.preventDefault();}}, false);

    // prevent unloading page without saving changes to shader
    window.onbeforeunload = function(e) { if( gShaderToy!==null && gShaderToy.GetNeedSave() ) return "You are about to lose your changes in the code editor."; };

    gShaderToy = new ShaderToy( viewerParent, editorParent, passParent );
    if( !gShaderToy.mCreated )
        return;

    gShaderToy.setFontSize(gFontSize, false);

    //-- get info --------------------------------------------------------

    if( gShaderID===null )
    {
         loadNew();
    }
    else
    {
         loadComments( gShaderID );
         loadShader( gShaderID );
    }
}

function updateLikes()
{
    var shaderLike = document.getElementById("shaderLike");
    if( shaderLike!==null )
    {
        if( gIsLiked==1 )
        {
            shaderLike.src = "/img/themes/" + gThemeName + "/likeYes.png";
        }
        else
        {
            shaderLike.src = "/img/themes/" + gThemeName + "/likeNo.png";
        }
    }

    var shaderStatsViewed  = document.getElementById( "shaderStatsViewed" );   if( shaderStatsViewed  )   shaderStatsViewed.innerHTML = "" + gRes.mViewed;
    var shaderStatsLikes   = document.getElementById( "shaderStatsLikes" );    if( shaderStatsLikes  )    shaderStatsLikes.innerHTML = "" + gRes.mLikes;
    var shaderStatsPrivacy = document.getElementById( "shaderStatsPrivacy" );  
    if( shaderStatsPrivacy ) 
    {
        if (gRes.mPublished == 2) 
        {
            shaderStatsPrivacy.innerHTML = ", Unlisted";
        }
    }
} 

function updatepage( jsn )
{
    var txt = "";
    var numComments = jsn.text ? jsn.text.length : 0;
    var shaderStatsComments = document.getElementById( "shaderStatsComments" ); if( shaderStatsComments  ) shaderStatsComments.innerHTML = "" + numComments;

    for( var i=0; i<numComments; i++ )
    {
    	var timeVar = "-";
        if( jsn.date[i] != 0 )
        {
	        timeVar = piGetTime(jsn.date[i]);
        }
        
        if( jsn.username[i]===gUserName )
        {
            txt += "<div class=\"commentSelf\">";
        }
        else 
        {
            txt += "<div class=\"comment\">";
        }
        txt +=   "<div><img class=\"userPictureSmall\" src=\"" + jsn.userpicture[i] + "\"></img></div>";
        txt +=   "<div class=\"commentContent\"><a class='user' href='/user/"+ htmlEntities(jsn.username[i]) +"'>" + htmlEntities(jsn.username[i]) + "</a>, " + timeVar + "<br>";
        if( jsn.hidden[i]==1) txt += "<i>";
        txt +=  bbc2html(htmlEntities(jsn.text[i]),true);
        if( jsn.hidden[i]==1) txt += "</i>";
        txt +=   "</div>";
        if( gIsMyShader )
        txt +=   "<div class='uiDivBUtton' title='" + ((jsn.hidden[i]==1)?"Unhide this comment":"Hide this comment from all users. You can always unhide it again at a later time.") + "' onclick='hideComment(\"" + jsn.id[i] + "\"," + ((jsn.hidden[i]==1)?"0":"1") + ");'>" + ((jsn.hidden[i]==1)?"unhide":"x") + "</div>";

        txt += "</div>";
    }

    var cc = document.getElementById("myComments");      if( cc ) cc.innerHTML = txt;
    var dd = document.getElementById("commentTextArea"); if( dd ) dd.value = "";
}

function hideComment(commentid, hidden)
{
	var xmlHttp = new XMLHttpRequest();
    if( xmlHttp===null ) return;
    var url = "s=" + gShaderID + "&c=" + commentid + "&hide=" + hidden;

    xmlHttp.open('POST', "/comment", true);
    xmlHttp.responseType = "json";
    xmlHttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xmlHttp.timeout = 15000; // 15 seconds
    xmlHttp.ontimeout = function()
    {
        var ve = document.getElementById("centerScreen" );
        doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't (un)hide the comment", false, null );
        // reenable comment input elements
        form.mybutton.disabled = false;
        form.comment.disabled = false;
    }

    xmlHttp.onload = function()
    {
        var jsn = xmlHttp.response;
        if( jsn.hide && jsn.hide!=0 )
        {
            updatepage( jsn );
        }
        else
        {
            var ve = document.getElementById("centerScreen" );
            doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't (un)hide the comment", false, null );
        }
        //form.mybutton.disabled = false;
        //form.comment.disabled = false;
    }

    xmlHttp.onerror = function()
    {
        var ve = document.getElementById("centerScreen" );
        doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't submit your comment", false, null );
        //form.mybutton.disabled = false;
        //form.comment.disabled = false;
    }
    xmlHttp.send(url);
}

function addComment(usr, form)
{
	var xmlHttp = new XMLHttpRequest();
    if( xmlHttp===null ) return;

    // disable comment input elements while we process the comment submision
    form.mybutton.disabled = true;
    form.comment.disabled = true;

    // encode comments
    var commentsformated = form.comment.value;
    commentsformated = encodeURIComponent( commentsformated );

    var url = "s=" + usr +"&comment=" + commentsformated;
    xmlHttp.open('POST', "/comment", true);
    xmlHttp.responseType = "json";
    xmlHttp.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xmlHttp.timeout = 60000; // 60 seconds
    xmlHttp.ontimeout = function()
    {
        var ve = document.getElementById("centerScreen" );
        doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't submit your comment", false, null );
        // reenable comment input elements
        form.mybutton.disabled = false;
        form.comment.disabled = false;
    }

    xmlHttp.onload = function()
    {
        var jsn = xmlHttp.response;
        if( jsn.added && jsn.added>0 )
        {
            updatepage( jsn );
        }
        else
        {
            var ve = document.getElementById("centerScreen" );

            if (jsn.added && jsn.added == -1) {
                doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Account verification required", "The comment could not be shared. Your account is not verified, please go to your Shadertoy profile for instructions.", false, null );
            } else if (jsn.added && jsn.added == -3) {
                doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Reached maximum of daily comments", "The comment could not be shared. You have reached the maximum daily comments allowed with your current status. If you want to keep improving your status, participate more actively in the Shadertoy community. Please go to your Shadertoy profile for more details.", false, null );
            } else {
                doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't submit your comment", false, null );                
            }
        }
        // reenable comment input elements
        form.mybutton.disabled = false;
        form.comment.disabled = false;
    }

    xmlHttp.onerror = function()
    {
        var ve = document.getElementById("centerScreen" );
        doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "We are sorry, we couldn't submit your comment", false, null );
        // reenable comment input elements
        form.mybutton.disabled = false;
        form.comment.disabled = false;
    }
    xmlHttp.send(url);
}

function checkFormComment(str)
{
    if( str == "" )
    {
        var ve = document.getElementById("centerScreen" );
        doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "You need to write at least 1 character", false, null );
        return false;
    }
    return true;
}

function validateComment( form )
{
   if(checkFormComment( form.comment.value ))
   {
        addComment(gShaderID, form);
        return true;
   }
   form.comment.focus();
   return false;
}

function shaderSaved( res, savedID, isUpdate, dataJSON )
{
    const ve = document.getElementById("centerScreen" );
    if( res===0 )
    {
        gShaderToy.SetNeedSave( false );
        if( isUpdate )
        {
            var eleWrapper = document.getElementById('editor');
            eleWrapper.classList.add("saved");
            setTimeout( function () { eleWrapper.classList.remove("saved"); }, 500 );
        }
        else
        {
            window.location="/view/" + savedID;
        }
	}
    else if( res===-2 ) { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Error", "Shader name \"" +  dataJSON.info.name + "\" is already used by another shader. Please change the name of your shader.", false, null ); }
    else if( res===-3 ) { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Error", "The shader could not be " + (isUpdate?"updated":"added") + ", you might not be logged in anymore, please Sign In again.", false, null); }
    else if( res===-13) { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Error", "The shader coubld not be saved, it is using private assets.", false, null); }
    else if( res===-15) { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Account verification required", "The shader could not be published. Your account is not verified, please go to your Shadertoy profile for instructions.", false, null ); }
    else if( res===-16) { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Reached maximum of daily shaders", "The shader could not be published. You have reached the maximum daily shaders allowed with your current status. If you want to keep improving your status, participate more actively in the Shadertoy community. Please go to your Shadertoy profile for more details.", false, null ); }
    else                { doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Error", "The shader could not be " + (isUpdate?"updated":"added") + ", please try again. Error code : " + res, false, null); }
}

function openSubmitShaderForm( isUpdate )
{
    if (gShaderToy.mState !== 1) return;

    var ve = document.getElementById("centerScreen" );
    var s1 = document.getElementById('shaderTitle');
    var s2 = document.getElementById('shaderTags');
    var s3 = document.getElementById('shaderDescription');

    if( !s1.validity.valid ) { doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "You must give a name to your shader", false, null ); return; }
    if( !s2.validity.valid ) { doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "You must assign at least one tag to your shader", false, null ); return; }
    if( !s3.validity.valid ) { doAlert( piGetCoords(ve), {mX:320,mY:-1}, "Error", "You must give a description to your shader", false, null ); return; }
    if( !checkFormComment(s1.value) ) return false;
    if( !checkFormComment(s3.value) ) return false;
    if (!gShaderToy.CheckShaderCorrectness()) { doAlert(piGetCoords(ve), { mX: 400, mY: -1 }, "Error", "The shader can not be saved. Too many Image passes.", false, null); return; }

    gShaderToy.setActionsState(false);

    var sp = document.getElementById('published');
    const publishedStatus = sp.options[ sp.selectedIndex ].value;
    if( publishedStatus===1 || publishedStatus===3 )
    {
        gShaderToy.SetShaderFromEditor(true,false)
        if (!gShaderToy.AllowPublishing() )
        {
            let ve = document.getElementById("centerScreen" );
            doAlert( piGetCoords(ve), {mX:400,mY:-1}, "Error", "The shader can not be published. It does not compile.", false, null );
            gShaderToy.setActionsState(true);
            gShaderToy.mState = 1;
            return false;
        }

        let compileTime = gShaderToy.GetTotalCompilationTime();
        if (compileTime > kMaxCompileTime)
        {
            let ve = document.getElementById("centerScreen");
            doAlert(piGetCoords(ve), { mX: 400, mY: -1 }, "Warning", "This shader takes too long to compile. Shadertoy might disable real-time rendering when browsing.", false, null);
        }
    }

    var dataJSON = gShaderToy.Save();

    dataJSON.info.name        = s1.value;
    dataJSON.info.tags        = s2.value.split( "," );
    dataJSON.info.description = s3.value;
    dataJSON.info.published   = publishedStatus;

    // short wait to make sure the rendering is done
    // before reading the buffer from the gpu
    setTimeout(() => {
        // screenshot
        var canvas = document.getElementById("demogl");
        var dataURL = canvas.toDataURL("image/jpeg");

        var dataTXT = JSON.stringify( dataJSON, null );
        dataTXT = encodeURIComponent( dataTXT );

        // send
        var mHttpReq = new XMLHttpRequest();
        mHttpReq.open( "POST", "/shadertoy", true );
        mHttpReq.responseType = "json";
        mHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

        var url = "a="; if( isUpdate ) url = "u=";
        url += dataTXT;
        
        if (dataJSON.flags.mFlagWebcam === false) { url += "&ss=" + dataURL; }

        mHttpReq.ontimeout = function (e)
        {
            console.log("TIMEOUT");
        };

        mHttpReq.onerror = function()
        {
            gShaderToy.setActionsState(true);
            gShaderToy.mState = 1;
            shaderSaved(-3, "NOID", isUpdate, dataJSON);
        }
        mHttpReq.onload = function()
        {
            gShaderToy.setActionsState(true);
            gShaderToy.mState = 1;
            try 
            {
                let jsn = mHttpReq.response;
                shaderSaved( jsn.result, jsn.id, isUpdate, dataJSON );
            } 
            catch(e) 
            {
                shaderSaved( -3, "NOID", isUpdate, dataJSON );
            }
        }
        mHttpReq.send( url );
    }, 100);
}

function doFork()
{
    var dataJSON = gShaderToy.Save();

    let eleButton = document.getElementById("myForkButton");
    if (eleButton.disabled) return; // prevent double save

    eleButton.disabled = true;

    let ve = document.getElementById("centerScreen");
    doAlert(piGetCoords(ve), { mX: 256, mY: 180 }, "Please Wait", "Forking Shader...", null);

    // short wait to make sure the rendering is done
    // before reading the buffer from the gpu
    setTimeout(() => {
        var dataTXT = JSON.stringify( dataJSON, null );
        dataTXT = encodeURIComponent( dataTXT );

        // Submit the values to the cloud
        var mHttpReq = new XMLHttpRequest();
        mHttpReq.open( "POST", "/shadertoy", true );
        mHttpReq.responseType = "json";
        mHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');

        var url = "f=" + dataTXT;

        mHttpReq.onerror = function()
        {
            shaderSaved(-3, "NOID", false, dataJSON);
            eleButton.disabled = false;
        }
        mHttpReq.onload = function()
        {
            eleButton.disabled = false;
            try 
            {
                var jsn = mHttpReq.response;
                shaderSaved( jsn.result, jsn.id, false, dataJSON );
            } 
            catch(e) 
            {
                shaderSaved( -3, "NOID", false, dataJSON );
            }
        }
        mHttpReq.send( url );
    }, 200);
}
