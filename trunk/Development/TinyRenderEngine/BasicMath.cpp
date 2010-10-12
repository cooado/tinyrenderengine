#include "BasicMath.h"

namespace TRE
{
	Vector3 Matrix3x3::operator * ( const Vector3& vec )
	{
		Vector3 result;
		result.v0 = m00 * vec.v0 + m01 * vec.v1 + m02 * vec.v2;
		result.v1 = m10 * vec.v0 + m11 * vec.v1 + m12 * vec.v2;
		result.v2 = m20 * vec.v0 + m21 * vec.v1 + m22 * vec.v2;
		return result;
	}
};