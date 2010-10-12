#pragma once

#include "Platform.h"
#include <list>
#include <set>
#include <queue>
#include "BasicMath.h"
#include "D3DX10.h"

namespace TRE
{

class ProgressiveMesh
{
public:
	ProgressiveMesh(void);
	~ProgressiveMesh(void);

	void SetInitData(UINT32* index, FLOAT32* vertex, UINT32 f_count, UINT32 v_count);

	void Generate(UINT32& f_count, UINT32& v_count);

	void RetrieveData(UINT32*& index, FLOAT32*& vertex, UINT32& f_count, UINT32& v_count);

private:

	struct FaceAttribute
	{
		/************************************************************************
		* Value	Description
		* 0		Face still in mesh
		* 1		Face removed
		************************************************************************/
		UINT8 Flag;
	};

	struct QuadricErrorMetric
	{
		Matrix3x3 A;

		Vector3 B;

		FLOAT32 C;

		void GetFromFace( const Vector3& v0, const Vector3& v1, const Vector3& v2 )
		{
			// area of the face
			Vector3 vec10 = v1 - v0;
			Vector3 vec20 = v2 - v0;
			Vector3 cross21 = vec10.Cross( vec20 );
			FLOAT32 area = 0.5f * cross21.Det();

			// A
			Vector3 normal = cross21.Normalize();
			A = normal.Multiple( normal ) * area;

			// b
			FLOAT32 d = normal.Dot( v0 ) * -1.0f;
			B = normal * d * area;

			// c
			C = d * d * area;
		}

		QuadricErrorMetric operator + ( const QuadricErrorMetric& qem )
		{
			QuadricErrorMetric result;
			result.A = A + qem.A;
			result.B = B + qem.B;
			result.C = C + qem.C;
			return result;
		}

		void operator += ( const QuadricErrorMetric& qem )
		{
			A += qem.A;
			B += qem.B;
			C += qem.C;
		}

		/***************************************************************
		* @return	false	matrix inverse doesn't exist
		***************************************************************/
		bool Solve( Vector3& v, FLOAT32& qem )
		{
			Matrix3x3 InvA;
			bool InvAExist = A.Inverse( InvA );
			if( InvAExist )
			{
				v = InvA * B * -1.0f;
				qem = CalcQEM( v );
				return true;
			}
			else
			{
				return false;
			}
		};

		FLOAT32 CalcQEM( Vector3& vec )
		{
			FLOAT32 r = ( vec * A ).Dot( vec ) + B.Dot( vec ) * 2.0f + C;
			return r < 0.0f ? 0.0f : r;
		};
	};

	struct VertexAttribute
	{
		QuadricErrorMetric QEM;

		UINT32 CollapsedIndice;

		UINT32 CompressedIndice;

		std::list<UINT32> Faces;

		UINT8 Flag;
	};

	struct Edge
	{
		UINT32 V0;

		UINT32 V1;

		Vector3 CollapsedVertice;

		FLOAT32 QEM;

		BOOL operator == ( const Edge& edge )
		{
			if( V0 == edge.V0 && V1 == edge.V1 && std::abs( QEM - edge.QEM ) <= 1e-10 )
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		};
	};

	struct EdgeCompare
	{
		bool operator () ( const Edge& e0, const Edge& e1 )
		{
			if( e0.V0 < e1.V0 )
			{
				return true;
			}
			else if ( e0.V0 == e1.V0 )
			{
				if( e0.V1 < e1.V1 )
				{
					return true;
				}
				else
				{
					return false;
				};
			}
			else
			{
				return false;
			};
		};
	};

	struct EdgeQEMCompare
	{
		bool operator () ( const Edge& e0, const Edge& e1 )
		{
			return e0.QEM > e1.QEM;
		};
	};

	std::set<Edge, EdgeCompare> m_SolvedEdge;

	std::priority_queue< Edge, std::vector<Edge>, EdgeQEMCompare > m_EdgePriorityQueue;

	FaceAttribute* m_FaceAttributes;

	VertexAttribute* m_VertexAttributes;

	UINT32 m_InitFaceCount;

	UINT32 m_FaceCount;

	UINT32 m_VertexCount;

	UINT32 m_InitVertexCount;

	UINT32* m_Faces;

	FLOAT32* m_Vertex;

	void CalcQEMCoefs4Vertex( UINT32 v );

	void SolveQEM4EdgeCollapse( UINT32 v0, UINT32 v1, Edge& edge );

	void CollapseEdge( const Edge& edge );
};

};
