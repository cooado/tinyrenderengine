#include "ProgressiveMesh.h"

namespace TRE
{

ProgressiveMesh::ProgressiveMesh(void)
{
}


ProgressiveMesh::~ProgressiveMesh(void)
{
}

void ProgressiveMesh::SetInitData(UINT32* index, FLOAT32* vertex, UINT32 f_count, UINT32 v_count)
{
	m_Faces = index;
	m_InitFaceCount = f_count;
	m_FaceCount = m_InitFaceCount;
	m_Vertex = vertex;
	m_InitVertexCount = v_count;
	m_VertexCount = m_InitVertexCount;

	// build vertex attributes
	m_VertexAttributes = new VertexAttribute[ m_InitVertexCount ];
	for( UINT32 i = 0; i < m_InitVertexCount; i++ )
	{
		m_VertexAttributes[ i ].Flag = 0;
		m_VertexAttributes[ i ].CollapsedIndice = 0;
		m_VertexAttributes[ i ].CompressedIndice = 0;
		m_VertexAttributes[ i ].QEM.A.Zero();
		m_VertexAttributes[ i ].QEM.B.Zero();
		m_VertexAttributes[ i ].QEM.C = 0.0f;
	};

	// build face attributes
	m_FaceAttributes = new FaceAttribute[ m_InitFaceCount ];
	for( UINT32 i = 0; i < m_InitFaceCount; i++ )
	{
		m_FaceAttributes[ i ].Flag = 0;
		// build face list for every vertice
		m_VertexAttributes[ m_Faces[ 3*i ] ].Faces.push_back( i );
		m_VertexAttributes[ m_Faces[ 3*i+1 ] ].Faces.push_back( i );
		m_VertexAttributes[ m_Faces[ 3*i+2 ] ].Faces.push_back( i );
	};

	// calculate qem for every vertex
	for( UINT32 i = 0; i < m_VertexCount; i++ )
	{
		CalcQEMCoefs4Vertex( i );
	};

	// evaluate edge collapse qem
	for( UINT32 i = 0; i < m_InitFaceCount; i++ )
	{
		if( ( *( m_FaceAttributes + i ) ).Flag == 0 )
		{
			// index 
			UINT32 i0 = *( m_Faces + 3 * i );
			UINT32 i1 = *( m_Faces + 3 * i + 1 );
			UINT32 i2 = *( m_Faces + 3 * i + 2 );

			// edge 0
			Edge e;
			e.V0 = ( i0 < i1 ? i0 : i1 );
			e.V1 = ( i0 >= i1 ? i0 : i1 );
			if( m_SolvedEdge.find( e ) == m_SolvedEdge.end() )
			{
				Edge edge;
				SolveQEM4EdgeCollapse( e.V0, e.V1, edge );
				// store result
				m_EdgePriorityQueue.push( edge );
				m_SolvedEdge.insert( edge );

			}

			// edge 1
			e.V0 = ( i1 < i2 ? i1 : i2 );
			e.V1 = ( i1 >= i2 ? i1 : i2 );
			if( m_SolvedEdge.find( e ) == m_SolvedEdge.end() )
			{
				Edge edge;
				SolveQEM4EdgeCollapse( e.V0, e.V1, edge );
				// store result
				m_EdgePriorityQueue.push( edge );
				m_SolvedEdge.insert( edge );
			}

			// edge 2
			e.V0 = ( i0 < i2 ? i0 : i2 );
			e.V1 = ( i0 >= i2 ? i0 : i2 );
			if( m_SolvedEdge.find( e ) == m_SolvedEdge.end() )
			{
				Edge edge;
				SolveQEM4EdgeCollapse( e.V0, e.V1, edge );
				// store result
				m_EdgePriorityQueue.push( edge );
				m_SolvedEdge.insert( edge );
			}
		};
	};
}

void ProgressiveMesh::CalcQEMCoefs4Vertex( UINT32 v )
{
	VertexAttribute* va = m_VertexAttributes + v;
	va->QEM.A.Zero();
	va->QEM.B.Zero();
	va->QEM.C = 0.0f;
	for( std::list< UINT32 >::iterator i = va->Faces.begin(); i != va->Faces.end(); i++ )
	{
		if( m_FaceAttributes[ *i ].Flag == 0 )
		{
			// index 
			UINT32 i0 = *( m_Faces + 3 *( *i ) );
			UINT32 i1 = *( m_Faces + 3 *( *i ) + 1 );
			UINT32 i2 = *( m_Faces + 3 *( *i ) + 2 );

			// vertex
			Vector3 v0;
			v0.v0 = *( m_Vertex + 3 * i0 );
			v0.v1 = *( m_Vertex + 3 * i0 + 1 );
			v0.v2 = *( m_Vertex + 3 * i0 + 2 );
			Vector3 v1;
			v1.v0 = *( m_Vertex + 3 * i1 );
			v1.v1 = *( m_Vertex + 3 * i1 + 1 );
			v1.v2 = *( m_Vertex + 3 * i1 + 2 );
			Vector3 v2;
			v2.v0 = *( m_Vertex + 3 * i2 );
			v2.v1 = *( m_Vertex + 3 * i2 + 1 );
			v2.v2 = *( m_Vertex + 3 * i2 + 2 );

			QuadricErrorMetric qem;
			qem.GetFromFace( v0, v1, v2 );

			//// area of the face
			//Vector3 vec10 = v1 - v0;
			//Vector3 vec20 = v2 - v0;
			//Vector3 cross21 = vec10.Cross( vec20 );
			//FLOAT32 area = 0.5f * cross21.Det();

			//// A
			//Vector3 normal = cross21.Normalize();
			//Matrix3x3 a = normal.Multiple( normal );

			//// b
			//FLOAT32 d = normal.Dot( v0 ) * -1.0f;
			//Vector3 b = normal * d;

			//// c
			//FLOAT32 c = d * d;

			// add to vertex attribute
			va->QEM += qem;
			//va->QEM.A += a * area;
			//va->QEM.B += b * area;
			//va->QEM.C += c * area;
		}
	};
}

void ProgressiveMesh::SolveQEM4EdgeCollapse( UINT32 v0, UINT32 v1, Edge& edge )
{
	// solve qem
	QuadricErrorMetric qem;
	qem = m_VertexAttributes[v0].QEM + m_VertexAttributes[v1].QEM;
	Vector3 newvertice;
	FLOAT32 value;
	bool Solved = qem.Solve( newvertice, value );
	if( !Solved )
	{
		newvertice.v0 = ( m_Vertex[ 3*v0 ] + m_Vertex[ 3*v1 ] ) * 0.5f;
		newvertice.v1 = ( m_Vertex[ 3*v0+1 ] + m_Vertex[ 3*v1+1 ] ) * 0.5f;
		newvertice.v2 = ( m_Vertex[ 3*v0+2 ] + m_Vertex[ 3*v1+2 ] ) * 0.5f;
		value = qem.CalcQEM( newvertice );
	}
	edge.CollapsedVertice = newvertice;
	edge.QEM = value;
	edge.V0 = v0;
	edge.V1 = v1;
};

void ProgressiveMesh::Generate(UINT32& f_count, UINT32& v_count)
{
	while( f_count < m_FaceCount || v_count < m_VertexCount )
	{
		Edge edge = m_EdgePriorityQueue.top();
		m_EdgePriorityQueue.pop();
		
		// validate the vertex existance
		if( m_VertexAttributes[ edge.V0 ].Flag == 0 && m_VertexAttributes[ edge.V1 ].Flag == 0 )
		{
			// validate qem data
			Edge e2;
			SolveQEM4EdgeCollapse( edge.V0, edge.V1, e2 );
			if( edge == e2 )
			{
				CollapseEdge( edge );
			}
			else
			{
				m_EdgePriorityQueue.push( e2 );
				m_SolvedEdge.insert( e2 );
			}
		}
	};
}

void ProgressiveMesh::CollapseEdge( const Edge& edge )
{
	// check edge validation
	TRE_ASSERT( m_VertexAttributes[ edge.V0 ].Flag != 1 && m_VertexAttributes[ edge.V1 ].Flag != 1 );

	VertexAttribute* va0 = m_VertexAttributes + edge.V0;
	for( std::list< UINT32 >::iterator i = va0->Faces.begin(); i != va0->Faces.end(); i++ )
	{
		if( m_FaceAttributes[ *i ].Flag == 0 )
		{
			if( m_Faces[ 3*(*i) ] == edge.V1 || m_Faces[ 3*(*i)+1 ] == edge.V1 || m_Faces[ 3*(*i)+2 ] == edge.V1 )
			{
				// remove the face
				m_FaceAttributes[ (*i) ].Flag = 1;
				m_FaceCount--;
			}
		}
	}
	VertexAttribute* va1 = m_VertexAttributes + edge.V1;
	for( std::list< UINT32 >::iterator i = va1->Faces.begin(); i != va1->Faces.end(); i++ )
	{
		if( m_FaceAttributes[ *i ].Flag == 0 )
		{
			// modify v1 index to v0
			if( m_Faces[ 3*(*i) ] == edge.V1 )
			{
				m_Faces[ 3*(*i) ] = edge.V0;
			}
			if( m_Faces[ 3*(*i)+1 ] == edge.V1 )
			{
				m_Faces[ 3*(*i)+1 ] = edge.V0;
			}
			if( m_Faces[ 3*(*i)+2 ] == edge.V1 )
			{
				m_Faces[ 3*(*i)+2 ] = edge.V0;
			}
			
			// update v0 associating faces
			va0->Faces.push_back( *i );
		}
	}

	// update v0 to new value
	m_Vertex[ 3*edge.V0 ] = edge.CollapsedVertice.v0;
	m_Vertex[ 3*edge.V0 +1 ] = edge.CollapsedVertice.v1;
	m_Vertex[ 3*edge.V0 +2 ] = edge.CollapsedVertice.v2;

	// remove v1
	m_VertexAttributes[ edge.V1 ].Flag = 1;
	m_VertexAttributes[ edge.V1 ].CollapsedIndice = edge.V0;
	m_VertexCount--;

	// TODO::remove obsolete edges in priority queue
	
	// re-evaluate qem
	for( std::list< UINT32 >::iterator i = va0->Faces.begin(); i != va0->Faces.end(); i++ )
	{
		if( m_FaceAttributes[ *i ].Flag == 0 )
		{
			// index 
			UINT32 i0 = m_Faces[ 3*(*i) ];
			UINT32 i1 = m_Faces[ 3*(*i)+1 ];
			UINT32 i2 = m_Faces[ 3*(*i)+2 ];

			CalcQEMCoefs4Vertex( i0 );
			CalcQEMCoefs4Vertex( i1 );
			CalcQEMCoefs4Vertex( i2 );
		}
	}
}

void ProgressiveMesh::RetrieveData(UINT32*& index, FLOAT32*& vertex, UINT32& f_count, UINT32& v_count)
{
	index = new UINT32[ m_FaceCount * 3 ];
	vertex = new FLOAT32[ m_VertexCount * 3 ];
	f_count = m_FaceCount;
	v_count = m_VertexCount;

	// build new vertex index and populate vertex buffer
	UINT32 vcount = 0;
	for( UINT32 i = 0; i < m_InitVertexCount; i++ )
	{
		if( m_VertexAttributes[ i ].Flag == 0 )
		{
			m_VertexAttributes[ i ].CompressedIndice = vcount;
			vertex[ vcount*3 ] = m_Vertex[ i*3 ];
			vertex[ vcount*3+1 ] = m_Vertex[ i*3+1 ];
			vertex[ vcount*3+2 ] = m_Vertex[ i*3+2 ];

			vcount++;
		};
	};

	// populate index buffer
	UINT32 fcount = 0;
	for( UINT i = 0; i < m_InitFaceCount; i++ )
	{
		if( m_FaceAttributes[ i ].Flag == 0 )
		{
			index[ fcount*3 ] = m_VertexAttributes[ m_Faces[ i*3 ] ].CompressedIndice;
			index[ fcount*3+1 ] = m_VertexAttributes[ m_Faces[ i*3+1 ] ].CompressedIndice;
			index[ fcount*3+2 ] = m_VertexAttributes[ m_Faces[ i*3+2 ] ].CompressedIndice;

			fcount++;
		};
	};
}

};