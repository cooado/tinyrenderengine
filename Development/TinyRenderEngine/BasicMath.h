#ifndef _TRE_BASICMATH_H__
#define _TRE_BASICMATH_H__

#include "Platform.h"
#include <memory>

namespace TRE
{
	struct Vector3;

	/*********************************************************************
	* Row major 3x3 matrix
	*********************************************************************/
	struct Matrix3x3
	{
		union
		{
			struct 
			{
				FLOAT32 m00;
				FLOAT32 m01;
				FLOAT32 m02;
				FLOAT32 m10;
				FLOAT32 m11;
				FLOAT32 m12;
				FLOAT32 m20;
				FLOAT32 m21;
				FLOAT32 m22;
			};
			FLOAT32 m[9];
		};

		void Zero()
		{
			memset( (void*)m, 0, sizeof( FLOAT32 ) * 9 );
		};

		Matrix3x3 operator + ( const Matrix3x3& mat )
		{
			Matrix3x3 result;
			result.m00 = m00 + mat.m00;
			result.m01 = m01 + mat.m01;
			result.m02 = m02 + mat.m02;
			result.m10 = m10 + mat.m10;
			result.m11 = m11 + mat.m11;
			result.m12 = m12 + mat.m12;
			result.m20 = m20 + mat.m20;
			result.m21 = m21 + mat.m21;
			result.m22 = m22 + mat.m22;
			return result;
		};

		Matrix3x3& operator += ( const Matrix3x3& mat )
		{
			m00 += mat.m00;
			m01 += mat.m01;
			m02 += mat.m02;
			m10 += mat.m10;
			m11 += mat.m11;
			m12 += mat.m12;
			m20 += mat.m20;
			m21 += mat.m21;
			m22 += mat.m22;
			return *this;
		};

		Matrix3x3 operator * ( const FLOAT32 f )
		{
			Matrix3x3 result;
			result.m00 = m00 * f;
			result.m01 = m01 * f;
			result.m02 = m02 * f;
			result.m10 = m10 * f;
			result.m11 = m11 * f;
			result.m12 = m12 * f;
			result.m20 = m20 * f;
			result.m21 = m21 * f;
			result.m22 = m22 * f;
			return result;
		};

		FLOAT32 Det()
		{
			return m00*(m22*m11-m21*m12)-m10*(m22*m01-m21*m02)+m20*(m12*m01-m11*m02);
		};

		bool Inverse( Matrix3x3& result )
		{
			FLOAT32 det = Det();
			if( std::abs( det ) > 1e-10 )
			{
				result.m00 = (m22*m11-m21*m12)/det;
				result.m01 = (m21*m02-m22*m01)/det;
				result.m02 = (m12*m01-m11*m02)/det;
				result.m10 = (m20*m12-m22*m10)/det;
				result.m11 = (m22*m00-m20*m02)/det;
				result.m12 = (m10*m02-m12*m00)/det;
				result.m20 = (m21*m10-m20*m11)/det;
				result.m21 = (m20*m01-m21*m00)/det;
				result.m22 = (m11*m00-m10*m01)/det;
				return true;
			}
			else 
			{
				return false;
			};
		};

		Vector3 operator * ( const Vector3& vec );
	};

	struct Vector3
	{
		union
		{
			struct
			{
				FLOAT32 v0;
				FLOAT32 v1;
				FLOAT32 v2;
			};
			FLOAT32 v[3];
		};

		Vector3 operator - ( const Vector3& vec ) const
		{
			Vector3 result;
			result.v0 = v0 - vec.v0;
			result.v1 = v1 - vec.v1;
			result.v2 = v2 - vec.v2;
			return result;
		};

		Vector3 operator + ( const Vector3& vec )
		{
			Vector3 result;
			result.v0 = v0 + vec.v0;
			result.v1 = v1 + vec.v1;
			result.v2 = v2 + vec.v2;
			return result;
		};

		Vector3 operator * ( const FLOAT32 f )
		{
			Vector3 result;
			result.v0 = v0 * f;
			result.v1 = v1 * f;
			result.v2 = v2 * f;
			return result;
		};

		Vector3& operator += ( const Vector3& vec )
		{
			v0 += vec.v0;
			v1 += vec.v1;
			v2 += vec.v2;
			return *this;
		};

		Vector3 Cross( const Vector3& vec)
		{
			Vector3 result;
			result.v0 = v1 * vec.v2 - v2 * vec.v1;
			result.v1 = ( v0 * vec.v2 - v2 * vec.v0 ) * -1.0f;
			result.v2 = v0 * vec.v1 - v1 * vec.v0;
			return result;
		};

		FLOAT32 Det()
		{
			return std::sqrt( v0 * v0 + v1 * v1 + v2 * v2 );
		};

		Vector3& Normalize()
		{
			FLOAT32 d = Det();
			v0 = v0 / d;
			v1 = v1 / d;
			v2 = v2 / d;
			return *this;
		};

		Matrix3x3 Multiple( const Vector3& vec )
		{
			Matrix3x3 result;
			result.m00 = v0 * vec.v0;
			result.m01 = v0 * vec.v1;
			result.m02 = v0 * vec.v2;
			result.m10 = v1 * vec.v0;
			result.m11 = v1 * vec.v1;
			result.m12 = v1 * vec.v2;
			result.m20 = v2 * vec.v0;
			result.m21 = v2 * vec.v1;
			result.m22 = v2 * vec.v2;
			return result;
		};

		Vector3 operator * ( const Matrix3x3& mat )
		{
			Vector3 result;
			result.v0 = v0 * mat.m00 + v1 * mat.m10 + v2 * mat.m20;
			result.v1 = v0 * mat.m01 + v1 * mat.m11 + v2 * mat.m21;
			result.v2 = v0 * mat.m02 + v1 * mat.m12 + v2 * mat.m22;
			return result;
		};

		FLOAT32 Dot( const Vector3& vec )
		{
			return v0 * vec.v0 + v1 * vec.v1 + v2 * vec.v2;
		};

		void Zero()
		{
			v0 = 0;
			v1 = 0;
			v2 = 0;
		};
	};
};

#endif