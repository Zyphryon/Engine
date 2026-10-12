// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Primitive.hpp"

#if defined(ZY_PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <webgl/webgl1_ext.h>
#include <webgl/webgl2_ext.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#if defined(ZY_PLATFORM_WEB)

// The browser links every entry point statically, so the web only fills in what WebGL spells differently or lacks.

using GLdouble = double;

#ifndef GL_LINE
#define GL_LINE                          0x1B01
#endif
#ifndef GL_FILL
#define GL_FILL                          0x1B02
#endif
#ifndef GL_CLAMP_TO_BORDER
#define GL_CLAMP_TO_BORDER               0x812D
#endif
#ifndef GL_TEXTURE_BORDER_COLOR
#define GL_TEXTURE_BORDER_COLOR          0x1004
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY
#define GL_TEXTURE_MAX_ANISOTROPY        0x84FE
#endif
#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY
#define GL_MAX_TEXTURE_MAX_ANISOTROPY    0x84FF
#endif
#ifndef GL_DEPTH_CLAMP
#define GL_DEPTH_CLAMP                   0x864F
#endif
#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB              0x8DB9
#endif
#ifndef GL_LOWER_LEFT
#define GL_LOWER_LEFT                    0x8CA1
#endif
#ifndef GL_UPPER_LEFT
#define GL_UPPER_LEFT                    0x8CA2
#endif
#ifndef GL_NEGATIVE_ONE_TO_ONE
#define GL_NEGATIVE_ONE_TO_ONE           0x935E
#endif
#ifndef GL_ZERO_TO_ONE
#define GL_ZERO_TO_ONE                   0x935F
#endif
#ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
#define GL_TEXTURE_CUBE_MAP_SEAMLESS     0x884F
#endif
#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE            0x8642
#endif
#ifndef GL_MAP_PERSISTENT_BIT
#define GL_MAP_PERSISTENT_BIT            0x0040
#endif
#ifndef GL_MAP_COHERENT_BIT
#define GL_MAP_COHERENT_BIT              0x0080
#endif
#ifndef GL_DYNAMIC_STORAGE_BIT
#define GL_DYNAMIC_STORAGE_BIT           0x0100
#endif
#ifndef GL_CLIENT_STORAGE_BIT
#define GL_CLIENT_STORAGE_BIT            0x0200
#endif
#ifndef GL_COMPLETION_STATUS_KHR
#define GL_COMPLETION_STATUS_KHR         0x91B1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif

inline void glDepthRange(GLdouble Near, GLdouble Far)
{
    glDepthRangef(static_cast<GLfloat>(Near), static_cast<GLfloat>(Far));
}

inline void glPrimitiveRestartIndex(GLuint)
{
    // WebGL always restarts on the fixed index, and has no other.
}

inline void glClipControl(GLenum Origin, GLenum Depth)
{
    glClipControlEXT(Origin, Depth);
}

inline void glPolygonMode(GLenum Face, GLenum Mode)
{
    glPolygonModeWEBGL(Face, Mode);
}

inline void glDrawElementsBaseVertex(GLenum Mode, GLsizei Count, GLenum Type, const void * Indices, GLint Base)
{
    glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(Mode, Count, Type, Indices, 1, Base, 0);
}

inline void glDrawElementsInstancedBaseVertex(GLenum Mode, GLsizei Count, GLenum Type, const void * Indices, GLsizei Instances, GLint Base)
{
    glDrawElementsInstancedBaseVertexBaseInstanceWEBGL(Mode, Count, Type, Indices, Instances, Base, 0);
}

inline void glMaxShaderCompilerThreads(GLuint)
{
    // The browser picks its own compiler threads; KHR_parallel_shader_compile is a query alone on the web.
}

inline void glBindVertexBuffer(GLuint, GLuint, GLintptr, GLsizei)
{
}

inline void glVertexAttribBinding(GLuint, GLuint)
{
}

inline void glVertexAttribFormat(GLuint, GLint, GLenum, GLboolean, GLuint)
{
}

inline void glVertexAttribIFormat(GLuint, GLint, GLenum, GLuint)
{
}

inline void glVertexBindingDivisor(GLuint, GLuint)
{
}

inline void glBufferStorage(GLenum, GLsizeiptr, const void *, GLbitfield)
{
}

inline void glBindBuffersRange(GLenum, GLuint, GLsizei, const GLuint *, const GLintptr *, const GLsizeiptr *)
{
}

inline void glBindSamplers(GLuint, GLsizei, const GLuint *)
{
}

inline void glBindTextures(GLuint, GLsizei, const GLuint *)
{
}

inline void glProgramUniform1i(GLuint, GLint, GLint)
{
}

#else

// No system header may follow and declare the same names over the loader's.
#define __gl_h_
#define __GL_H__
#define __gl3_h_
#define __glext_h_
#define __gl_glext_h_
#define __glcorearb_h_

#if defined(ZY_PLATFORM_WINDOWS)
#define ZY_GL_API __stdcall
#else
#define ZY_GL_API
#endif

using GLenum     = UInt32;
using GLboolean  = Byte;
using GLbitfield = UInt32;
using GLbyte     = SInt8;
using GLubyte    = Byte;
using GLshort    = SInt16;
using GLushort   = UInt16;
using GLint      = SInt32;
using GLuint     = UInt32;
using GLsizei    = SInt32;
using GLfloat    = Real32;
using GLdouble   = Real64;
using GLchar     = Char;
using GLintptr   = SInt;
using GLsizeiptr = SInt;
using GLint64    = SInt64;
using GLuint64   = UInt64;
using GLsync     = Ptr<struct GLsyncObject>;

#define GL_ACTIVE_UNIFORMS                            0x8B86
#define GL_ALREADY_SIGNALED                           0x911A
#define GL_ALWAYS                                     0x0207
#define GL_ARRAY_BUFFER                               0x8892
#define GL_BACK                                       0x0405
#define GL_BLEND                                      0x0BE2
#define GL_BYTE                                       0x1400
#define GL_CLAMP_TO_BORDER                            0x812D
#define GL_CLAMP_TO_EDGE                              0x812F
#define GL_CLIENT_STORAGE_BIT                         0x0200
#define GL_COLOR                                      0x1800
#define GL_COLOR_ATTACHMENT0                          0x8CE0
#define GL_COMPARE_REF_TO_TEXTURE                     0x884E
#define GL_COMPILE_STATUS                             0x8B81
#define GL_COMPLETION_STATUS_KHR                      0x91B1
#define GL_COMPRESSED_R11_EAC                         0x9270
#define GL_COMPRESSED_RED_RGTC1                       0x8DBB
#define GL_COMPRESSED_RG11_EAC                        0x9272
#define GL_COMPRESSED_RGB8_ETC2                       0x9274
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2   0x9276
#define GL_COMPRESSED_RGBA8_ETC2_EAC                  0x9278
#define GL_COMPRESSED_RGBA_BPTC_UNORM                 0x8E8C
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT              0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT              0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT              0x83F3
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT           0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT         0x8E8F
#define GL_COMPRESSED_RG_RGTC2                        0x8DBD
#define GL_COMPRESSED_SIGNED_R11_EAC                  0x9271
#define GL_COMPRESSED_SIGNED_RED_RGTC1                0x8DBC
#define GL_COMPRESSED_SIGNED_RG11_EAC                 0x9273
#define GL_COMPRESSED_SIGNED_RG_RGTC2                 0x8DBE
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC           0x9279
#define GL_COMPRESSED_SRGB8_ETC2                      0x9275
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9277
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM           0x8E8D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT        0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT        0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT        0x8C4F
#define GL_CONDITION_SATISFIED                        0x911C
#define GL_CONTEXT_FLAGS                              0x821E
#define GL_CONTEXT_PROFILE_MASK                       0x9126
#define GL_COPY_READ_BUFFER                           0x8F36
#define GL_COPY_WRITE_BUFFER                          0x8F37
#define GL_CULL_FACE                                  0x0B44
#define GL_CW                                         0x0900
#define GL_DECR                                       0x1E03
#define GL_DECR_WRAP                                  0x8508
#define GL_DELETE_STATUS                              0x8B80
#define GL_DEPTH                                      0x1801
#define GL_DEPTH24_STENCIL8                           0x88F0
#define GL_DEPTH32F_STENCIL8                          0x8CAD
#define GL_DEPTH_ATTACHMENT                           0x8D00
#define GL_DEPTH_CLAMP                                0x864F
#define GL_DEPTH_COMPONENT                            0x1902
#define GL_DEPTH_COMPONENT16                          0x81A5
#define GL_DEPTH_COMPONENT32F                         0x8CAC
#define GL_DEPTH_STENCIL                              0x84F9
#define GL_DEPTH_STENCIL_ATTACHMENT                   0x821A
#define GL_DEPTH_TEST                                 0x0B71
#define GL_DRAW_FRAMEBUFFER                           0x8CA9
#define GL_DST_ALPHA                                  0x0304
#define GL_DST_COLOR                                  0x0306
#define GL_DYNAMIC_DRAW                               0x88E8
#define GL_DYNAMIC_STORAGE_BIT                        0x0100
#define GL_ELEMENT_ARRAY_BUFFER                       0x8893
#define GL_EQUAL                                      0x0202
#define GL_EXTENSIONS                                 0x1F03
#define GL_FALSE                                      0
#define GL_FILL                                       0x1B02
#define GL_FLOAT                                      0x1406
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV             0x8DAD
#define GL_FRAGMENT_SHADER                            0x8B30
#define GL_FRAMEBUFFER                                0x8D40
#define GL_FRAMEBUFFER_COMPLETE                       0x8CD5
#define GL_FRAMEBUFFER_SRGB                           0x8DB9
#define GL_FRONT                                      0x0404
#define GL_FRONT_AND_BACK                             0x0408
#define GL_FUNC_ADD                                   0x8006
#define GL_FUNC_REVERSE_SUBTRACT                      0x800B
#define GL_FUNC_SUBTRACT                              0x800A
#define GL_GEQUAL                                     0x0206
#define GL_GREATER                                    0x0204
#define GL_HALF_FLOAT                                 0x140B
#define GL_INCR                                       0x1E02
#define GL_INCR_WRAP                                  0x8507
#define GL_INFO_LOG_LENGTH                            0x8B84
#define GL_INT                                        0x1404
#define GL_INT_2_10_10_10_REV                         0x8D9F
#define GL_INVALID_ENUM                               0x0500
#define GL_INVALID_FRAMEBUFFER_OPERATION              0x0506
#define GL_INVALID_INDEX                              0xFFFFFFFFu
#define GL_INVALID_OPERATION                          0x0502
#define GL_INVALID_VALUE                              0x0501
#define GL_INVERT                                     0x150A
#define GL_KEEP                                       0x1E00
#define GL_LEQUAL                                     0x0203
#define GL_LESS                                       0x0201
#define GL_LINE                                       0x1B01
#define GL_LINEAR                                     0x2601
#define GL_LINEAR_MIPMAP_LINEAR                       0x2703
#define GL_LINEAR_MIPMAP_NEAREST                      0x2701
#define GL_LINES                                      0x0001
#define GL_LINE_STRIP                                 0x0003
#define GL_LINK_STATUS                                0x8B82
#define GL_LOWER_LEFT                                 0x8CA1
#define GL_MAJOR_VERSION                              0x821B
#define GL_MAP_COHERENT_BIT                           0x0080
#define GL_MAP_FLUSH_EXPLICIT_BIT                     0x0010
#define GL_MAP_INVALIDATE_BUFFER_BIT                  0x0008
#define GL_MAP_INVALIDATE_RANGE_BIT                   0x0004
#define GL_MAP_PERSISTENT_BIT                         0x0040
#define GL_MAP_READ_BIT                               0x0001
#define GL_MAP_UNSYNCHRONIZED_BIT                     0x0020
#define GL_MAP_WRITE_BIT                              0x0002
#define GL_MAX                                        0x8008
#define GL_MAX_ARRAY_TEXTURE_LAYERS                   0x88FF
#define GL_MAX_COLOR_ATTACHMENTS                      0x8CDF
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS           0x8B4D
#define GL_MAX_DRAW_BUFFERS                           0x8824
#define GL_MAX_SHADER_COMPILER_THREADS_KHR            0x91B0
#define GL_MAX_TEXTURE_IMAGE_UNITS                    0x8872
#define GL_MAX_TEXTURE_MAX_ANISOTROPY                 0x84FF
#define GL_MAX_TEXTURE_SIZE                           0x0D33
#define GL_MAX_UNIFORM_BLOCK_SIZE                     0x8A30
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                0x8A2F
#define GL_MAX_VERTEX_ATTRIBS                         0x8869
#define GL_MAX_VERTEX_ATTRIB_BINDINGS                 0x82DA
#define GL_MIN                                        0x8007
#define GL_MINOR_VERSION                              0x821C
#define GL_MIRRORED_REPEAT                            0x8370
#define GL_NEAREST                                    0x2600
#define GL_NEAREST_MIPMAP_LINEAR                      0x2702
#define GL_NEAREST_MIPMAP_NEAREST                     0x2700
#define GL_NEGATIVE_ONE_TO_ONE                        0x935E
#define GL_NEVER                                      0x0200
#define GL_NONE                                       0
#define GL_NOTEQUAL                                   0x0205
#define GL_NO_ERROR                                   0
#define GL_NUM_EXTENSIONS                             0x821D
#define GL_ONE                                        1
#define GL_ONE_MINUS_DST_ALPHA                        0x0305
#define GL_ONE_MINUS_DST_COLOR                        0x0307
#define GL_ONE_MINUS_SRC_ALPHA                        0x0303
#define GL_ONE_MINUS_SRC_COLOR                        0x0301
#define GL_OUT_OF_MEMORY                              0x0505
#define GL_PACK_ALIGNMENT                             0x0D05
#define GL_POINTS                                     0x0000
#define GL_POLYGON_OFFSET_FILL                        0x8037
#define GL_PRIMITIVE_RESTART                          0x8F9D
#define GL_PRIMITIVE_RESTART_FIXED_INDEX              0x8D69
#define GL_PROGRAM_POINT_SIZE                         0x8642
#define GL_QUERY_COUNTER_BITS                         0x8864
#define GL_QUERY_RESULT                               0x8866
#define GL_QUERY_RESULT_AVAILABLE                     0x8867
#define GL_R11F_G11F_B10F                             0x8C3A
#define GL_R16                                        0x822A
#define GL_R16F                                       0x822D
#define GL_R16I                                       0x8233
#define GL_R16UI                                      0x8234
#define GL_R16_SNORM                                  0x8F98
#define GL_R32F                                       0x822E
#define GL_R32I                                       0x8235
#define GL_R32UI                                      0x8236
#define GL_R8                                         0x8229
#define GL_R8I                                        0x8231
#define GL_R8UI                                       0x8232
#define GL_R8_SNORM                                   0x8F94
#define GL_READ_FRAMEBUFFER                           0x8CA8
#define GL_RED                                        0x1903
#define GL_RED_INTEGER                                0x8D94
#define GL_RENDERER                                   0x1F01
#define GL_REPEAT                                     0x2901
#define GL_REPLACE                                    0x1E01
#define GL_RG                                         0x8227
#define GL_RG16                                       0x822C
#define GL_RG16F                                      0x822F
#define GL_RG16I                                      0x8239
#define GL_RG16UI                                     0x823A
#define GL_RG16_SNORM                                 0x8F99
#define GL_RG32F                                      0x8230
#define GL_RG32I                                      0x823B
#define GL_RG32UI                                     0x823C
#define GL_RG8                                        0x822B
#define GL_RG8I                                       0x8237
#define GL_RG8UI                                      0x8238
#define GL_RG8_SNORM                                  0x8F95
#define GL_RGB                                        0x1907
#define GL_RGB10_A2                                   0x8059
#define GL_RGB10_A2UI                                 0x906F
#define GL_RGB32F                                     0x8815
#define GL_RGB32I                                     0x8D83
#define GL_RGB32UI                                    0x8D71
#define GL_RGB9_E5                                    0x8C3D
#define GL_RGBA                                       0x1908
#define GL_RGBA16                                     0x805B
#define GL_RGBA16F                                    0x881A
#define GL_RGBA16I                                    0x8D88
#define GL_RGBA16UI                                   0x8D76
#define GL_RGBA16_SNORM                               0x8F9B
#define GL_RGBA32F                                    0x8814
#define GL_RGBA32I                                    0x8D82
#define GL_RGBA32UI                                   0x8D70
#define GL_RGBA8                                      0x8058
#define GL_RGBA8I                                     0x8D8E
#define GL_RGBA8UI                                    0x8D7C
#define GL_RGBA8_SNORM                                0x8F97
#define GL_RGBA_INTEGER                               0x8D99
#define GL_RGB_INTEGER                                0x8D98
#define GL_RG_INTEGER                                 0x8228
#define GL_SCISSOR_TEST                               0x0C11
#define GL_SHADER_TYPE                                0x8B4F
#define GL_SHADING_LANGUAGE_VERSION                   0x8B8C
#define GL_SHORT                                      0x1402
#define GL_SRC_ALPHA                                  0x0302
#define GL_SRC_COLOR                                  0x0300
#define GL_SRGB8_ALPHA8                               0x8C43
#define GL_STATIC_DRAW                                0x88E4
#define GL_STENCIL                                    0x1802
#define GL_STENCIL_ATTACHMENT                         0x8D20
#define GL_STENCIL_TEST                               0x0B90
#define GL_STREAM_DRAW                                0x88E0
#define GL_SYNC_FLUSH_COMMANDS_BIT                    0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE                 0x9117
#define GL_TEXTURE0                                   0x84C0
#define GL_TEXTURE_2D                                 0x0DE1
#define GL_TEXTURE_2D_ARRAY                           0x8C1A
#define GL_TEXTURE_BORDER_COLOR                       0x1004
#define GL_TEXTURE_COMPARE_FUNC                       0x884D
#define GL_TEXTURE_COMPARE_MODE                       0x884C
#define GL_TEXTURE_CUBE_MAP                           0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X                0x8515
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                  0x884F
#define GL_TEXTURE_MAG_FILTER                         0x2800
#define GL_TEXTURE_MAX_ANISOTROPY                     0x84FE
#define GL_TEXTURE_MIN_FILTER                         0x2801
#define GL_TEXTURE_WRAP_R                             0x8072
#define GL_TEXTURE_WRAP_S                             0x2802
#define GL_TEXTURE_WRAP_T                             0x2803
#define GL_TIMEOUT_EXPIRED                            0x911B
#define GL_TIMEOUT_IGNORED                            0xFFFFFFFFFFFFFFFFull
#define GL_TIMESTAMP                                  0x8E28
#define GL_TIME_ELAPSED                               0x88BF
#define GL_TRIANGLES                                  0x0004
#define GL_TRIANGLE_STRIP                             0x0005
#define GL_TRUE                                       1
#define GL_UNIFORM_BUFFER                             0x8A11
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT            0x8A34
#define GL_UNPACK_ALIGNMENT                           0x0CF5
#define GL_UNPACK_ROW_LENGTH                          0x0CF2
#define GL_UNSIGNED_BYTE                              0x1401
#define GL_UNSIGNED_INT                               0x1405
#define GL_UNSIGNED_INT_10F_11F_11F_REV               0x8C3B
#define GL_UNSIGNED_INT_24_8                          0x84FA
#define GL_UNSIGNED_INT_2_10_10_10_REV                0x8368
#define GL_UNSIGNED_INT_5_9_9_9_REV                   0x8C3E
#define GL_UNSIGNED_SHORT                             0x1403
#define GL_UPPER_LEFT                                 0x8CA2
#define GL_VENDOR                                     0x1F00
#define GL_VERSION                                    0x1F02
#define GL_VERTEX_SHADER                              0x8B31
#define GL_WAIT_FAILED                                0x911D
#define GL_ZERO                                       0
#define GL_ZERO_TO_ONE                                0x935F

#define ZY_GL_FEATURE(Feature, Return, Name, Parameters)   \
    using PFN_zygl##Name = Return (ZY_GL_API *) Parameters; \
    inline PFN_zygl##Name zygl##Name = nullptr;
#define ZY_GL_CORE(Return, Name, Parameters) ZY_GL_FEATURE(Core, Return, Name, Parameters)
#include "OGLApi.inl"
#undef ZY_GL_CORE
#undef ZY_GL_FEATURE

// The global names are what the driver and Tracy call; each is a macro over the loader's own pointer, so no
// symbol of the system's OpenGL library is ever shadowed by one of ours.
#define glActiveTexture                    zyglActiveTexture
#define glAttachShader                     zyglAttachShader
#define glBeginQuery                       zyglBeginQuery
#define glBindBuffer                       zyglBindBuffer
#define glBindBufferBase                   zyglBindBufferBase
#define glBindBufferRange                  zyglBindBufferRange
#define glBindFramebuffer                  zyglBindFramebuffer
#define glBindSampler                      zyglBindSampler
#define glBindTexture                      zyglBindTexture
#define glBindVertexArray                  zyglBindVertexArray
#define glBlendEquationSeparate            zyglBlendEquationSeparate
#define glBlendFuncSeparate                zyglBlendFuncSeparate
#define glBufferData                       zyglBufferData
#define glBufferSubData                    zyglBufferSubData
#define glCheckFramebufferStatus           zyglCheckFramebufferStatus
#define glClearBufferfi                    zyglClearBufferfi
#define glClearBufferfv                    zyglClearBufferfv
#define glClearBufferiv                    zyglClearBufferiv
#define glClientWaitSync                   zyglClientWaitSync
#define glColorMask                        zyglColorMask
#define glCompileShader                    zyglCompileShader
#define glCompressedTexSubImage2D          zyglCompressedTexSubImage2D
#define glCompressedTexSubImage3D          zyglCompressedTexSubImage3D
#define glCopyBufferSubData                zyglCopyBufferSubData
#define glCreateProgram                    zyglCreateProgram
#define glCreateShader                     zyglCreateShader
#define glCullFace                         zyglCullFace
#define glDeleteBuffers                    zyglDeleteBuffers
#define glDeleteFramebuffers               zyglDeleteFramebuffers
#define glDeleteProgram                    zyglDeleteProgram
#define glDeleteQueries                    zyglDeleteQueries
#define glDeleteSamplers                   zyglDeleteSamplers
#define glDeleteShader                     zyglDeleteShader
#define glDeleteSync                       zyglDeleteSync
#define glDeleteTextures                   zyglDeleteTextures
#define glDeleteVertexArrays               zyglDeleteVertexArrays
#define glDepthFunc                        zyglDepthFunc
#define glDepthMask                        zyglDepthMask
#define glDepthRange                       zyglDepthRange
#define glDetachShader                     zyglDetachShader
#define glDisable                          zyglDisable
#define glDisableVertexAttribArray         zyglDisableVertexAttribArray
#define glDrawArrays                       zyglDrawArrays
#define glDrawArraysInstanced              zyglDrawArraysInstanced
#define glDrawBuffers                      zyglDrawBuffers
#define glDrawElements                     zyglDrawElements
#define glDrawElementsInstanced            zyglDrawElementsInstanced
#define glEnable                           zyglEnable
#define glEnableVertexAttribArray          zyglEnableVertexAttribArray
#define glEndQuery                         zyglEndQuery
#define glFenceSync                        zyglFenceSync
#define glFinish                           zyglFinish
#define glFlush                            zyglFlush
#define glFramebufferTexture2D             zyglFramebufferTexture2D
#define glFramebufferTextureLayer          zyglFramebufferTextureLayer
#define glFrontFace                        zyglFrontFace
#define glGenBuffers                       zyglGenBuffers
#define glGenFramebuffers                  zyglGenFramebuffers
#define glGenQueries                       zyglGenQueries
#define glGenSamplers                      zyglGenSamplers
#define glGenTextures                      zyglGenTextures
#define glGenVertexArrays                  zyglGenVertexArrays
#define glGetError                         zyglGetError
#define glGetFloatv                        zyglGetFloatv
#define glGetInteger64v                    zyglGetInteger64v
#define glGetIntegerv                      zyglGetIntegerv
#define glGetProgramInfoLog                zyglGetProgramInfoLog
#define glGetProgramiv                     zyglGetProgramiv
#define glGetQueryiv                       zyglGetQueryiv
#define glGetShaderInfoLog                 zyglGetShaderInfoLog
#define glGetShaderiv                      zyglGetShaderiv
#define glGetString                        zyglGetString
#define glGetStringi                       zyglGetStringi
#define glGetUniformBlockIndex             zyglGetUniformBlockIndex
#define glGetUniformLocation               zyglGetUniformLocation
#define glLinkProgram                      zyglLinkProgram
#define glMapBufferRange                   zyglMapBufferRange
#define glPixelStorei                      zyglPixelStorei
#define glPolygonOffset                    zyglPolygonOffset
#define glPrimitiveRestartIndex            zyglPrimitiveRestartIndex
#define glReadBuffer                       zyglReadBuffer
#define glSamplerParameterf                zyglSamplerParameterf
#define glSamplerParameterfv               zyglSamplerParameterfv
#define glSamplerParameteri                zyglSamplerParameteri
#define glScissor                          zyglScissor
#define glShaderSource                     zyglShaderSource
#define glStencilFuncSeparate              zyglStencilFuncSeparate
#define glStencilMask                      zyglStencilMask
#define glStencilOpSeparate                zyglStencilOpSeparate
#define glTexSubImage2D                    zyglTexSubImage2D
#define glTexSubImage3D                    zyglTexSubImage3D
#define glUniform1i                        zyglUniform1i
#define glUniformBlockBinding              zyglUniformBlockBinding
#define glUnmapBuffer                      zyglUnmapBuffer
#define glUseProgram                       zyglUseProgram
#define glVertexAttribDivisor              zyglVertexAttribDivisor
#define glVertexAttribIPointer             zyglVertexAttribIPointer
#define glVertexAttribPointer              zyglVertexAttribPointer
#define glViewport                         zyglViewport
#define glClipControl                      zyglClipControl
#define glTexStorage2D                     zyglTexStorage2D
#define glTexStorage3D                     zyglTexStorage3D
#define glDrawElementsBaseVertex           zyglDrawElementsBaseVertex
#define glDrawElementsInstancedBaseVertex  zyglDrawElementsInstancedBaseVertex
#define glBindVertexBuffer                 zyglBindVertexBuffer
#define glVertexAttribBinding              zyglVertexAttribBinding
#define glVertexAttribFormat               zyglVertexAttribFormat
#define glVertexAttribIFormat              zyglVertexAttribIFormat
#define glVertexBindingDivisor             zyglVertexBindingDivisor
#define glBufferStorage                    zyglBufferStorage
#define glBindBuffersRange                 zyglBindBuffersRange
#define glBindSamplers                     zyglBindSamplers
#define glBindTextures                     zyglBindTextures
#define glProgramUniform1i                 zyglProgramUniform1i
#define glPolygonMode                      zyglPolygonMode
#define glMaxShaderCompilerThreads         zyglMaxShaderCompilerThreads
#define glGetQueryObjectiv                 zyglGetQueryObjectiv
#define glGetQueryObjectui64v              zyglGetQueryObjectui64v
#define glQueryCounter                     zyglQueryCounter
#define glInvalidateFramebuffer            zyglInvalidateFramebuffer

// The window systems are declared as GL is, with no system header, so every API the driver calls sits here.
#if defined(ZY_PLATFORM_WINDOWS)

struct HDC__;
struct HGLRC__;

#define WGL_DRAW_TO_WINDOW_ARB               0x2001
#define WGL_ACCELERATION_ARB                 0x2003
#define WGL_SUPPORT_OPENGL_ARB               0x2010
#define WGL_DOUBLE_BUFFER_ARB                0x2011
#define WGL_PIXEL_TYPE_ARB                   0x2013
#define WGL_COLOR_BITS_ARB                   0x2014
#define WGL_ALPHA_BITS_ARB                   0x201B
#define WGL_DEPTH_BITS_ARB                   0x2022
#define WGL_STENCIL_BITS_ARB                 0x2023
#define WGL_FULL_ACCELERATION_ARB            0x2027
#define WGL_TYPE_RGBA_ARB                    0x202B
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB     0x20A9
#define WGL_TYPE_RGBA_FLOAT_ARB              0x21A0
#define WGL_CONTEXT_MAJOR_VERSION_ARB        0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB        0x2092
#define WGL_CONTEXT_FLAGS_ARB                0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB         0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB            0x0001
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB     0x0001

namespace ZyGraphic::OGL
{
    using WGLCreateContext           = Ptr<HGLRC__> (ZY_GL_API *)(Ptr<HDC__>);
    using WGLDeleteContext           = SInt32 (ZY_GL_API *)(Ptr<HGLRC__>);
    using WGLMakeCurrent             = SInt32 (ZY_GL_API *)(Ptr<HDC__>, Ptr<HGLRC__>);
    using WGLGetExtensionsStringARB  = ConstPtr<Char> (ZY_GL_API *)(Ptr<HDC__>);
    using WGLChoosePixelFormatARB    = SInt32 (ZY_GL_API *)(Ptr<HDC__>, ConstPtr<SInt32>, ConstPtr<Real32>, UInt32, Ptr<SInt32>, Ptr<UInt32>);
    using WGLCreateContextAttribsARB = Ptr<HGLRC__> (ZY_GL_API *)(Ptr<HDC__>, Ptr<HGLRC__>, ConstPtr<SInt32>);
    using WGLSwapIntervalEXT         = SInt32 (ZY_GL_API *)(SInt32);

    /// \brief Holds what a throwaway context reveals of WGL, which answers nothing before one is current.
    struct WGLBootstrap final
    {
        /// The WGL extensions, separated by spaces.
        Str                        Extensions;

        /// The pixel format chooser of `WGL_ARB_pixel_format`.
        WGLChoosePixelFormatARB    ChoosePixelFormat    = nullptr;

        /// The context constructor of `WGL_ARB_create_context`.
        WGLCreateContextAttribsARB CreateContextAttribs = nullptr;

        /// The swap interval setter of `WGL_EXT_swap_control`.
        WGLSwapIntervalEXT         SwapInterval         = nullptr;
    };
}

#elif defined(ZY_PLATFORM_LINUX)

struct _XDisplay;

#define GLX_DOUBLEBUFFER                    5
#define GLX_DEPTH_SIZE                      12
#define GLX_STENCIL_SIZE                    13
#define GLX_VISUAL_ID                       0x800B
#define GLX_DRAWABLE_TYPE                   0x8010
#define GLX_RENDER_TYPE                     0x8011
#define GLX_WINDOW_BIT                      0x0001
#define GLX_RGBA_BIT                        0x0001
#define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB    0x20B2
#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
#define GLX_CONTEXT_FLAGS_ARB               0x2094
#define GLX_CONTEXT_PROFILE_MASK_ARB        0x9126
#define GLX_CONTEXT_DEBUG_BIT_ARB           0x0001
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB    0x0001

namespace ZyGraphic::OGL
{
    // GLX hands its configurations and contexts out as opaque pointers, and its drawables as X ids, a machine word.
    using GLXFBConfig = Ptr<void>;
    using GLXContext  = Ptr<void>;
    using GLXDrawable = UInt;

    // GLX's own `Bool` is an `int`, which is what these take and return wherever the headers say `Bool`.
    using GLXGetFBConfigs            = Ptr<GLXFBConfig> (*)(Ptr<_XDisplay>, SInt32, Ptr<SInt32>);
    using GLXGetFBConfigAttrib       = SInt32 (*)(Ptr<_XDisplay>, GLXFBConfig, SInt32, Ptr<SInt32>);
    using GLXMakeCurrent             = SInt32 (*)(Ptr<_XDisplay>, GLXDrawable, GLXContext);
    using GLXDestroyContext          = void (*)(Ptr<_XDisplay>, GLXContext);
    using GLXSwapBuffers             = void (*)(Ptr<_XDisplay>, GLXDrawable);
    using GLXQueryExtensionsString   = ConstPtr<Char> (*)(Ptr<_XDisplay>, SInt32);
    using GLXCreateContextAttribsARB = GLXContext (*)(Ptr<_XDisplay>, GLXFBConfig, GLXContext, SInt32, ConstPtr<SInt32>);
    using GLXSwapIntervalEXT         = void (*)(Ptr<_XDisplay>, GLXDrawable, SInt32);
    using GLXSwapIntervalMESA        = SInt32 (*)(UInt32);

    /// \brief Holds the GLX functions a context is chosen and created with.
    struct GLXInterface final
    {
        /// The lister of the screen's framebuffer configurations.
        GLXGetFBConfigs            GetFBConfigs          = nullptr;

        /// The reader of one attribute of a framebuffer configuration.
        GLXGetFBConfigAttrib       GetFBConfigAttrib     = nullptr;

        /// The binder of a context to a drawable on the calling thread.
        GLXMakeCurrent             MakeCurrent           = nullptr;

        /// The destructor of a context.
        GLXDestroyContext          DestroyContext        = nullptr;

        /// The reader of the screen's GLX extensions.
        GLXQueryExtensionsString   QueryExtensionsString = nullptr;

        /// The context constructor of `GLX_ARB_create_context`.
        GLXCreateContextAttribsARB CreateContextAttribs  = nullptr;
    };
}

#endif

#endif