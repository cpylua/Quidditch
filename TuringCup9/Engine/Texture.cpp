/*--------------------------------------------------------------------
 *	Author:		Altfee
 *	Created:	2008/8/18
 *	File:		Texture.cpp
*--------------------------------------------------------------------*/

#include "Texture.h"
#include "Graphics.h"

cTexture::cTexture()
{
	m_pGraphics = NULL;
	m_pTexture  = NULL;
	m_width		= 0;
	m_height	= 0;
}

cTexture::~cTexture()
{
	free();
}

bool cTexture::free()
{
	SAFE_RELEASE(m_pTexture)
	m_pGraphics = NULL;
	m_width		= 0;
	m_height	= 0;
	return true;
}

/*bool cTexture::create( cGraphics* p_pGraphics, 
					  unsigned p_width, unsigned p_height, 
					  D3DFORMAT p_format )
{
	return true;
}

bool cTexture::create( cGraphics* p_pGraphics, IDirect3DTexture9* p_pTexture )
{
	return true;
}*/

bool cTexture::load
( 
	cGraphics *p_pGraphics, 
	TCHAR* p_fileName, 
	unsigned p_transparent /*= 0*/, 
	unsigned p_width /*= D3DX_DEFAULT*/, 
	unsigned p_height /*= D3DX_DEFAULT*/,
	D3DFORMAT p_format /*= D3DFMT_UNKNOWN*/ 
)
{
	free();
	
	if( (m_pGraphics = p_pGraphics) == NULL )
		return false;
	if( m_pGraphics->getD3DDevice() == NULL )
		return false;
	if( p_fileName == NULL )
		return false;

	if( FAILED(D3DXCreateTextureFromFileEx(m_pGraphics->getD3DDevice(),
		p_fileName, p_width, p_height, D3DX_DEFAULT,
		0, p_format, D3DPOOL_MANAGED, 
		D3DX_FILTER_TRIANGLE, D3DX_FILTER_TRIANGLE,
		p_transparent, NULL, NULL, &m_pTexture)) )
	{
		return false;
	}

	D3DSURFACE_DESC d3dsd;
	if( FAILED(m_pTexture->GetLevelDesc(0, &d3dsd)) )
		return false;

	m_width		= d3dsd.Width;
	m_height	= d3dsd.Height;
	m_D3DFormat = d3dsd.Format;

	return true;
}

bool cTexture::isLoaded()
{
	if( m_pTexture )
		return true;
	
	return false;
}

bool cTexture::draw
( 
	long p_destX, long p_destY, 
	long p_srcX /*= 0*/, long p_srcY /*= 0*/, 
	long p_width /*= 0*/, long p_height /*= 0*/, 
	D3DCOLOR p_color /*= 0xffffffff*/,
	D3DXVECTOR2 *p_pRotationCenter /*= NULL*/,
	float p_angle /*= 0.0f*/, 
	float p_XScale /*= 1.0f*/, float p_YScale /*= 1.0f*/
)
{
	RECT rect;
	ID3DXSprite *pSprite;
	
	if(m_pTexture == NULL)
		return FALSE;
	if(m_pGraphics == NULL)
		return FALSE;
	if((pSprite = m_pGraphics->getSprite()) == NULL)
		return FALSE;
	
	if(!p_width)
		p_width = m_width;
	if(!p_height)
		p_height = m_height;
	
	rect.left = p_srcX;
	rect.top  = p_srcY;
	rect.right = rect.left + p_width;
	rect.bottom = rect.top + p_height;
	
	//D3DXVECTOR3 Center = D3DXVECTOR3(10, 10, 1);          // 材质的矩形区域的中心点
	D3DXVECTOR3 Position = D3DXVECTOR3((float)p_destX, (float)p_destY, 0);   // 画的位置
	
	pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXVECTOR2 Scaling(p_XScale, p_YScale);                         // 縮放比例
	/*D3DXVECTOR2 RotationCenter( p_destX + p_width / 2,
								p_destY + p_height / 2 );*/
	D3DXMATRIX  Matrix;  // 坐标转换矩阵 
	D3DXVECTOR2 d3dx_vector2((float)p_destX, (float)p_destY);
	D3DXMatrixTransformation2D(&Matrix, &d3dx_vector2, 
		0.0, &Scaling, p_pRotationCenter, p_angle, 0 );
	pSprite->SetTransform(&Matrix);
	pSprite->Draw( m_pTexture, &rect, NULL, &Position, p_color);
	pSprite->End();
	return true;
}

BOOL cTexture::loadFromResource
( 
	cGraphics *p_pGraphics, 
	TCHAR* pSrcResource, 
	unsigned p_transparent /*= 0*/,
	unsigned p_width /*= D3DX_DEFAULT*/,
	unsigned p_height /*= D3DX_DEFAULT*/,
	D3DFORMAT p_format /*= D3DFMT_UNKNOWN*/ 
)
{
	UNREFERENCED_PARAMETER(p_transparent);

	if( (m_pGraphics = p_pGraphics) == NULL )
		return false;

	D3DXCreateTextureFromResourceEx( m_pGraphics->getD3DDevice(),
		NULL,pSrcResource, p_width, p_height, D3DX_DEFAULT,
		0, p_format, D3DPOOL_MANAGED, 
		D3DX_FILTER_TRIANGLE, D3DX_FILTER_TRIANGLE,
		0, NULL, NULL, &m_pTexture);

	D3DSURFACE_DESC d3dsd;
	if( FAILED(m_pTexture->GetLevelDesc(0, &d3dsd)) )
		return false;
	m_width		= d3dsd.Width;
	m_height	= d3dsd.Height;
	m_D3DFormat = d3dsd.Format;

	return TRUE;

}