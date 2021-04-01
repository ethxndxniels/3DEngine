#include "DirectionalLight.h"
#include "Cube.h"
#include "BindableCommon.h"
#include "TransformCbufDoubleboi.h"
#include "imgui/imgui.h"
#include "Sphere.h"

DirectionalLight::DirectionalLight( Graphics& gfx )
{
	using namespace Bind;
	namespace dx = DirectX;

	auto pvs = VertexShader::Resolve( gfx, "LightVS.cso" );

	auto pvsbc = pvs->GetBytecode();
	AddBind( std::move( pvs ) );

	AddBind( PixelShader::Resolve( gfx, "LightPS.cso" ) );\
	AddBind( Sampler::Resolve( gfx ) );


	DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant( gfx.GetCamera() );
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixInverse( &determinant, gfx.GetCamera() );
	lbuf.mvpMatrix = gfx.GetCamera() * gfx.GetProjection();
	DirectX::XMVECTOR determinant2 = DirectX::XMMatrixDeterminant( lbuf.mvpMatrix );
	DirectX::XMMATRIX viewMatrix2 = DirectX::XMMatrixInverse( &determinant2, lbuf.mvpMatrix );
	DirectX::XMMatrixTranspose( lbuf.mvpMatrix );

	pcs2 = PixelConstantBuffer<LightBufferType>::Resolve( gfx, lbuf, 0u );
	AddBind( pcs2 );
	pcs = PixelConstantBuffer<CamPosBuffer>::Resolve( gfx, cambuf, 1u );
	AddBind( pcs );


	AddBind( Blender::Resolve( gfx, false ) );

	AddBind( Topology::Resolve( gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST ) );

	AddBind( Rasterizer::Resolve( gfx, true ) );
}

void DirectionalLight::SetDirection( DirectX::XMFLOAT3 direction ) noexcept
{
	//this->lightDirection = direction;
}

void DirectionalLight::SpawnControlWindow( Graphics& gfx ) noexcept
{
	if ( ImGui::Begin( "Directional Light" ) )
	{
		ImGui::Text( "Direction" );

	}
	ImGui::End();
}

DirectX::XMMATRIX DirectionalLight::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixTranslation( 1.0f, 1.0f, 1.0f );
}

void DirectionalLight::DrawDirLight( Graphics& gfx )
{
	gfx.GetContext()->IASetVertexBuffers( 0, 0, NULL, NULL, NULL );
	gfx.GetContext()->IASetIndexBuffer( NULL, (DXGI_FORMAT)0, 0 );
	gfx.GetContext()->IASetInputLayout( NULL );
	const float color[] = { 0.07f,0.0f,0.12f };
	gfx.GetContext()->OMSetRenderTargets( 1, gfx.GetLightBuffer(), NULL );
	gfx.GetContext()->PSSetShaderResources( 0, 3, gfx.GetShaderResources() );
	gfx.GetContext()->PSSetShaderResources( 3, 1, gfx.GetDepthResource() );

	cambuf.camPos.x = gfx.GetCamera().r[3].m128_f32[0];
	cambuf.camPos.y = gfx.GetCamera().r[3].m128_f32[1];
	cambuf.camPos.z = gfx.GetCamera().r[3].m128_f32[2];
	pcs->Update( gfx, cambuf );

	DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant( gfx.GetCamera() );
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixInverse( &determinant, gfx.GetCamera() );
	lbuf.mvpMatrix = gfx.GetCamera() * gfx.GetProjection();
	DirectX::XMVECTOR determinant2 = DirectX::XMMatrixDeterminant( lbuf.mvpMatrix );
	DirectX::XMMATRIX viewMatrix2 = DirectX::XMMatrixInverse( &determinant2, lbuf.mvpMatrix );
	DirectX::XMMatrixTranspose( lbuf.mvpMatrix );
	pcs2->Update( gfx, lbuf );

	for ( auto& b : binds )
	{
		b->Bind( gfx );
	}

	gfx.GetContext()->Draw( 3, 0 );

	ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr, nullptr };
	gfx.GetContext()->PSSetShaderResources( 0, 4, null );
}