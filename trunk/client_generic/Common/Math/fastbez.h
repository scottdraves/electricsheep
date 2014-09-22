#ifndef	_FASTBEZ_H
#define	_FASTBEZ_H

#include "base.h"
#include "MathBase.h"
#include "Vector3.h"

namespace	Base
{

namespace	Math
{

/*
	Pretty fast bezier curves using de Casteljau's algo.
	Info: http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/surface/bezier-de-casteljau.html
	Very suitable for simd and other types of opts.
*/

/*
	CFastBezier.

*/
class	CFastBezier
{
	fp4		m_Curve[4];

	public:
			CFastBezier( fp4 _a, fp4 _b, fp4 _c, fp4 _d )
			{
				m_Curve[0] = _a;
				m_Curve[1] = _b;
				m_Curve[2] = _c;
				m_Curve[3] = _d;
			}

			inline fp4 Sample( const fp4 _time ) const
			{
				fp4	a[3];
				fp4	b[2];
				fp4	res;

				a[0] = lerpMacro( m_Curve[0], m_Curve[1], _time );
				a[1] = lerpMacro( m_Curve[1], m_Curve[2], _time );
				a[2] = lerpMacro( m_Curve[2], m_Curve[3], _time );
				b[0] = lerpMacro( a[0], a[1], _time );
				b[1] = lerpMacro( a[1], a[2], _time );
				res = lerpMacro( b[0], b[1], _time );

				return( res );
			}
};

};

};

#endif
