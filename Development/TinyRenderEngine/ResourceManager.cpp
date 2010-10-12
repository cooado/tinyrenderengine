#include "Prerequisites.h"

namespace TRE
{
	template< typename Input, typename Output >
	bool AnimationSampler< Input, Output >::GetOutput( const Input& i, Output& out )
	{
		if( m_Outputs.size() == 0 )
		{
			return false;
		};

		UINT32 index = m_InputCurrentIndex;

		if( i >= m_Inputs[ index ] )
		{
			index++;
			while( index < m_Inputs.size() )
			{
				if( i <= m_Inputs[ index ] )
				{
					Input& key0 = m_Inputs[ index - 1 ];
					Input& key1 = m_Inputs[ index ];

					FLOAT32 s = ( i - key0 ) / ( key1 - key0 );
					m_InputCurrentIndex = index - 1;
					out = Interpolate( m_Outputs[ index - 1 ], m_Outputs[ index ], s );
					return true;
				};
				index++;
			};
			// Input key larger than right bound
			out = *m_Outputs.rbegin();
			return true;
		}
		else
		{
			index--;
			while( index >= 0 )
			{
				if( i >= m_Inputs[ index ] )
				{
					Input& key0 = m_Inputs[ index ];
					Input& key1 = m_Inputs[ index + 1 ];

					FLOAT32 s = ( i - key0 ) / ( key1 - key0 );
					m_InputCurrentIndex = index;
					out = Interpolate( m_Outputs[ index ], m_Outputs[ index + 1 ], s );
					return true;
				};
				index--;
			};
			// Input key smaller than left bound
			out = *m_Outputs.begin();
			return true;
		};
	};

	template< typename Input, typename Output >
	Output AnimationSampler< Input, Output >::Interpolate( Output& v0, Output& v1, FLOAT32 s )
	{
		return v0 + s * ( v1 - v0 );
	};


	void Animation::Advance( const FLOAT32 delta )
	{
		m_fCurrentTime += delta;

		if( eAPT == APT_Loop )
		{
			if( m_fCurrentTime > m_fEndTime )
			{
				FLOAT32 f = m_fCurrentTime / ( m_fEndTime - m_fBeginTime );
				f = floor( f );
				m_fCurrentTime -= f * ( m_fEndTime - m_fBeginTime );
			};
		};

		for( UINT32 i = 0; i < m_vSamplers.size(); i++ )
		{
			D3DXMATRIX m;
			m_vSamplers[ i ].GetOutput( m_fCurrentTime, m );
			// Update targets
			for( UINT32 iT = 0; iT < m_vSamplerTargets[ i ].size(); iT++ )
			{
				*( m_vSamplerTargets[ i ].at( iT ) ) = m;
			};
		};

		// Child animations
		for( UINT32 i = 0; i < m_vChildAnims.size(); i++ )
		{
			m_vChildAnims[ i ]->Advance( delta );
		};
	};

	AnimationSampler< FLOAT32, D3DXMATRIX >& Animation::AddSampler()
	{
		AnimationSampler< FLOAT32, D3DXMATRIX > sampler;
		m_vSamplers.push_back( sampler );
		std::vector< D3DXMATRIX* > st;
		m_vSamplerTargets.push_back( st );
		return *m_vSamplers.rbegin();
	};

	void Node::PropagateTransform()
	{
		for( std::set< Node* >::iterator iNode = setChildren.begin(); iNode != setChildren.end(); iNode++ )
		{
			Node* child = *iNode;
			D3DXMatrixMultiply( &child->mAccumulatedTransform, &child->mLocalTransform, &mAccumulatedTransform );
			child->PropagateTransform();
		};
	};

	void Node::UpdateTransform()
	{
		if( pParent != NULL )
		{
			D3DXMatrixMultiply( &mAccumulatedTransform, &mLocalTransform, &pParent->mAccumulatedTransform );
		}
		else
		{
			mAccumulatedTransform = mLocalTransform;
		};
	};

	void Node::UpdateAndPropagateTransform()
	{
		UpdateTransform();
		PropagateTransform();
	};

	D3DXMATRIX& Node::GetWorldMatrix()
	{
		return mAccumulatedTransform;
	};

	void Node::GetLookAtMatrix(TRE::Node *n, D3DXMATRIX &m)
	{
		D3DXVECTOR3 eye( mAccumulatedTransform._41, mAccumulatedTransform._42, mAccumulatedTransform._43 );
		D3DXVECTOR3 at( n->mAccumulatedTransform._41, n->mAccumulatedTransform._42, n->mAccumulatedTransform._43 );
		D3DXVECTOR3 up( 0, 1, 0 );
		D3DXMatrixLookAtLH( &m, &eye, &at, &up );
	};

	/*********************************************************************************************************
	* Mesh
	*********************************************************************************************************/
	void Mesh::SetBuffersToDevice()
	{
		g_Renderer.pDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		ID3D10Buffer* pVB[ 3 ] = 
		{ 
			Buffers[ Mesh::BS_POSITION ], 
			Buffers[ Mesh::BS_NORMAL ], 
			Buffers[ Mesh::BS_TEXCOORD0 ] 
		};
		UINT pStrides[ 3 ] = 
		{
			BufferStrides[ Mesh::BS_POSITION ],
			BufferStrides[ Mesh::BS_NORMAL ],
			BufferStrides[ Mesh::BS_TEXCOORD0 ]
		};
		UINT pOffsets[ 3 ] = { 0, 0, 0 };
		g_Renderer.pDevice->IASetVertexBuffers( 0, 3, pVB, pStrides, pOffsets );
		g_Renderer.pDevice->IASetIndexBuffer( Buffers[ Mesh::BS_INDEX ], DXGI_FORMAT_R32_UINT, 0 );
	};
};