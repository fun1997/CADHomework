#pragma once

#include <QtCore/qglobal.h>
#include <QOpenGLFunctions_3_3_Core>
#include <QVector3D>

#ifndef BUILD_STATIC
# if defined(GLWIDGET_LIB)
#  define GLWIDGET_EXPORT Q_DECL_EXPORT
# else
#  define GLWIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define GLWIDGET_EXPORT
#endif

//Global Types
#define OPENGLCLASS QOpenGLFunctions_3_3_Core
typedef float Float;
////Global Constants
//#define MaxFloat std::numeric_limits<Float>::max()
//#define Infinity std::numeric_limits<Float>::infinity()

namespace vrt {
	const int MAX_LIGHT_COUNT = 10;
	const Float PI = 3.14159269;
}