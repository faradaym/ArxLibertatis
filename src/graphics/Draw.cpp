/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#include "graphics/Draw.h"

#include "core/Application.h"
#include "core/GameTime.h"

#include "graphics/VertexBuffer.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/data/Mesh.h"

using std::min;
using std::max;

long ZMAPMODE=1;
TextureContainer * Zmap;
Vec3f SPRmins;
Vec3f SPRmaxs;

extern TextureContainer * enviro;

CircularVertexBuffer<TexturedVertex> * pDynamicVertexBuffer_TLVERTEX;

void EERIEDRAWPRIM(Renderer::Primitive primitive, const TexturedVertex * vertices, size_t count, bool nocount) {
	
	if(!nocount) {
		EERIEDrawnPolys++;
	}
	
	pDynamicVertexBuffer_TLVERTEX->draw(primitive, vertices, count);
}

void EERIEDraw2DLine(float x0, float y0, float x1, float y1, float z, Color col) {
	
	TexturedVertex v[2];
	v[0].p.x = x0;
	v[0].p.y = y0;
	v[0].p.z = v[1].p.z = z;
	v[1].p.x = x1;
	v[1].p.y = y1;
	v[1].color = v[0].color = col.toBGRA();
	v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}

void EERIEDraw2DRect(float x0, float y0, float x1, float y1, float z, Color col) {
	
	TexturedVertex v[5];
	v[4].p.x = v[3].p.x = v[0].p.x = x0;
	v[4].p.y = v[1].p.y = v[0].p.y = y0;
	v[2].p.x = v[1].p.x = x1;
	v[3].p.y = v[2].p.y = y1;
	v[4].p.z = v[3].p.z = v[2].p.z = v[1].p.z = v[0].p.z = z;
	v[4].color = v[3].color = v[2].color = v[1].color = v[0].color = col.toBGRA();
	v[4].rhw = v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::LineStrip, v, 5);
}

void EERIEDrawFill2DRectDegrad(float x0, float y0, float x1, float y1, float z, Color cold, Color cole) {
	
	TexturedVertex v[4];
	v[0].p.x = v[2].p.x = x0;
	v[0].p.y = v[1].p.y = y0;
	v[1].p.x = v[3].p.x = x1;
	v[2].p.y = v[3].p.y = y1;
	v[0].color = v[1].color = cold.toBGRA();
	v[2].color = v[3].color = cole.toBGRA();
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = z;
	v[3].rhw = v[2].rhw = v[1].rhw = v[0].rhw = 1.f;
	
	GRenderer->ResetTexture(0);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDraw3DCylinder(const EERIE_CYLINDER & cyl, Color col) {
	
	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {
		
		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		
		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
		// Draw vertical 
		Vec3f from = cyl.origin + Vec3f(es, 0.f, ec);
		EERIEDraw3DLine(from, from + Vec3f(0.f, cyl.height, 0.f),  col);
		// Draw high pos
		Vec3f from2 = cyl.origin + Vec3f(es, cyl.height, ec);
		Vec3f to = cyl.origin + Vec3f(es2, cyl.height, ec2);
		EERIEDraw3DLine(from2, to,  col);
	}
}

void EERIEDraw3DCylinderBase(const EERIE_CYLINDER & cyl, Color col) {
	
	#define STEPCYL 16
	for(long i = 0; i < 360 - STEPCYL; i += STEPCYL) {
		
		float es = sin(radians(MAKEANGLE((float)i))) * cyl.radius;
		float ec = cos(radians(MAKEANGLE((float)i))) * cyl.radius;
		float es2 = sin(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		float ec2 = cos(radians(MAKEANGLE((float)(i + STEPCYL)))) * cyl.radius;
		
		// Draw low pos
		EERIEDraw3DLine(cyl.origin + Vec3f(es, 0.f, ec), cyl.origin + Vec3f(es2, 0.f, ec2),  col);
	}
}

void EERIEDrawCircle(float x0, float y0, float r, Color col, float z) {
	
	float lx = x0;
	float ly = y0 + r;
	GRenderer->ResetTexture(0);
	
	for(long i = 0; i < 361; i += 10) {
		float t = radians((float)i);
		float x = x0 - sin(t) * r;
		float y = y0 + cos(t) * r;
		EERIEDraw2DLine(lx, ly, x, y, z, col);
		lx = x;
		ly = y;
	}
}

void EERIEDrawTrue3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {
	
	Vec3f vect = dest - orgn;
	float m = ffsqrt(vect.lengthSqr());

	if(m <= 0)
		return;

	vect *= 1 / m;
	
	Vec3f cpos = orgn;

	while(m > 0) {
		float dep=std::min(m,30.f);
		Vec3f tpos = cpos + (vect * dep);
		EERIEDraw3DLine(cpos, tpos, col);
		cpos = tpos;
		m -= dep;
	}
}

void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color col) {
	
	TexturedVertex v[2];
	TexturedVertex in;
	
	in.p = orgn;
	EE_RTP(&in,&v[0]);
	if(v[0].p.z < 0.f) {
		return;
	}
	
	in.p = dest;
	EE_RTP(&in,&v[1]);
	if(v[1].p.z<0.f) {
		return;
	}
	
	GRenderer->ResetTexture(0);
	v[1].color = v[0].color = col.toBGRA();
	
	EERIEDRAWPRIM(Renderer::LineList, v, 2);
}
#define BASICFOCAL 350.f
//*************************************************************************************
//*************************************************************************************

void EERIEDrawSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color color, float Zpos) {
	
	TexturedVertex out;
	
	EE_RTP(in, &out);
	out.rhw *= 3000.f;

	if ((out.p.z>0.f) && (out.p.z<1000.f)
		&& (out.p.x>-1000) && (out.p.x<2500.f)
		&& (out.p.y>-500) && (out.p.y<1800.f))
	{
		float use_focal=BASICFOCAL*Xratio;
		float t;

		if(siz < 0) {
			t = -siz;
		} else {
			t = siz * ((out.rhw-1.f)*use_focal*0.001f);

			if(t <= 0.f)
				t = 0.00000001f;
		}
		
		if(Zpos <= 1.f) {
			out.p.z = Zpos;
			out.rhw = 1.f - out.p.z;
		} else {
			out.rhw *= (1.f/3000.f);
		}

		SPRmaxs.x=out.p.x+t;
		SPRmins.x=out.p.x-t;
		SPRmaxs.y=out.p.y+t;
		SPRmins.y=out.p.y-t;

		ColorBGRA col = color.toBGRA();
		TexturedVertex v[4];
		v[0] = TexturedVertex(Vec3f(SPRmins.x, SPRmins.y, out.p.z), out.rhw, col, out.specular, Vec2f::ZERO);
		v[1] = TexturedVertex(Vec3f(SPRmaxs.x, SPRmins.y, out.p.z), out.rhw, col, out.specular, Vec2f::X_AXIS);
		v[2] = TexturedVertex(Vec3f(SPRmins.x, SPRmaxs.y, out.p.z), out.rhw, col, out.specular, Vec2f::Y_AXIS);
		v[3] = TexturedVertex(Vec3f(SPRmaxs.x, SPRmaxs.y, out.p.z), out.rhw, col, out.specular, Vec2f(1.f, 1.f));

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEDrawRotatedSprite(TexturedVertex * in, float siz, TextureContainer * tex, Color color,
                            float Zpos, float rot) {
	
	TexturedVertex out;

	EE_RTP(in, &out);
	out.rhw *= 3000.f;
	
	if(out.p.z > 0.f && out.p.z < 1000.f) {
		float use_focal = BASICFOCAL * Xratio;
	
		float t = siz * ((out.rhw - 1.f) * use_focal * 0.001f); 

		if(t <= 0.f)
			t = 0.00000001f;

		if(Zpos<=1.f) {
			out.p.z = Zpos; 
			out.rhw = 1.f - out.p.z;
		} else {
			out.rhw *= (1.f/3000.f);
		}

		ColorBGRA col = color.toBGRA();
		TexturedVertex v[4];
		v[0] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::ZERO);
		v[1] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::X_AXIS);
		v[2] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f(1.f, 1.f));
		v[3] = TexturedVertex(Vec3f(0, 0, out.p.z), out.rhw, col, out.specular, Vec2f::Y_AXIS);
		
		
		SPRmaxs.x=out.p.x+t;
		SPRmins.x=out.p.x-t;
		
		SPRmaxs.y=out.p.y+t;			
		SPRmins.y=out.p.y-t;

		SPRmaxs.z = SPRmins.z = out.p.z; 

		for(long i=0;i<4;i++) {
			float tt = radians(MAKEANGLE(rot+90.f*i+45+90));
			v[i].p.x = EEsin(tt) * t + out.p.x;
			v[i].p.y = EEcos(tt) * t + out.p.y;
		}

		GRenderer->SetTexture(0, tex);
		EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
	}
	else SPRmaxs.x=-1;
}

//*************************************************************************************
//*************************************************************************************

void EERIEPOLY_DrawWired(EERIEPOLY * ep, Color color) {
	
	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	
	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	memcpy(ltv,ep->tv,sizeof(TexturedVertex)*to);							
	ltv[0].p.z-=0.0002f;
	ltv[1].p.z-=0.0002f;
	ltv[2].p.z-=0.0002f;
	ltv[3].p.z-=0.0002f;

	if(to == 4) {
		memcpy(&ltv[2],&ep->tv[3],sizeof(TexturedVertex));
		memcpy(&ltv[3],&ep->tv[2],sizeof(TexturedVertex));
		memcpy(&ltv[4],&ep->tv[0],sizeof(TexturedVertex));
		ltv[4].p.z-=0.0002f;
	} else {
		memcpy(&ltv[to],&ltv[0],sizeof(TexturedVertex));
	}

	GRenderer->ResetTexture(0);

	ColorBGRA col = color.toBGRA();
	if(col)
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=col;
	else if(to == 4)
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=ltv[4].color=0xFF00FF00;
	else
		ltv[0].color=ltv[1].color=ltv[2].color=ltv[3].color=0xFFFFFF00;
	
	EERIEDRAWPRIM(Renderer::LineStrip, ltv, to + 1);
}

void EERIEPOLY_DrawNormals(EERIEPOLY * ep) {
	TexturedVertex ltv[5];
	ltv[0] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::ZERO);
	ltv[1] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::X_AXIS);
	ltv[2] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f(1.f, 1.f));
	ltv[3] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	ltv[4] = TexturedVertex(Vec3f(0, 0, 0.5), 1.f, 1, 1, Vec2f::Y_AXIS);
	
	TexturedVertex lv;
	long to = (ep->type & POLY_QUAD) ? 4 : 3;

	lv.p = ep->center;
	EE_RTP(&lv,&ltv[0]);
	lv.p += ep->norm * 10.f;
	EE_RTP(&lv,&ltv[1]);
	GRenderer->ResetTexture(0);
	ltv[1].color=ltv[0].color=0xFFFF0000;

	if ((ltv[1].p.z>0.f) && (ltv[0].p.z>0.f))
		EERIEDRAWPRIM(Renderer::LineList, ltv, 3);

	for(long h = 0; h < to; h++) {
		lv.p = ep->v[h].p;
		EE_RTP(&lv,&ltv[0]);
		lv.p += ep->nrml[h] * 10.f;
		EE_RTP(&lv,&ltv[1]);
		GRenderer->ResetTexture(0);
		ltv[1].color=ltv[0].color=Color::yellow.toBGR();

		if ((ltv[1].p.z>0.f) &&  (ltv[0].p.z>0.f))
			EERIEDRAWPRIM(Renderer::LineList, ltv, 3);
	}
}


//-----------------------------------------------------------------------------

void EERIEDrawBitmap(Rect rect, float z, TextureContainer * tex, Color color) {
	EERIEDrawBitmap(rect.left, rect.top, rect.width(), rect.height(), z, tex, color);
}

void EERIEDrawBitmap(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(0.f,  0.f));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(uv.x, 0.f));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(0.f,  uv.y));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(uv.x, uv.y));
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
}

void EERIEDrawBitmap_uv(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ONE;
	u0 *= uv.x, u1 *= uv.x, v0 *= uv.y, v1 *= uv.y;

	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(u1, v0));
	v[2] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(u1, v1));
	v[3] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(u0, v1));

	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);
}

void EERIEDrawBitmapUVs(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                        Color color, float u0, float v0, float u1, float v1, float u2, float v2,
	                      float u3, float v3) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	v[0] = TexturedVertex(Vec3f(x,      y,      z), 1.f, col, 0xff000000, Vec2f(u0, v0));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), 1.f, col, 0xff000000, Vec2f(u1, v1));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), 1.f, col, 0xff000000, Vec2f(u2, v2));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), 1.f, col, 0xff000000, Vec2f(u3, v3));
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);	
}

void EERIEDrawBitmap2(float x, float y, float sx, float sy, float z, TextureContainer * tex, Color color) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	float rhw = 1.f - z;
	v[0] = TexturedVertex(Vec3f(x,      y,      z), rhw, col, 0xFF000000, Vec2f(0.f,  0.f));
	v[1] = TexturedVertex(Vec3f(x + sx, y,      z), rhw, col, 0xFF000000, Vec2f(uv.x, 0.f));
	v[2] = TexturedVertex(Vec3f(x,      y + sy, z), rhw, col, 0xFF000000, Vec2f(0.f,  uv.y));
	v[3] = TexturedVertex(Vec3f(x + sx, y + sy, z), rhw, col, 0xFF000000, Vec2f(uv.x, uv.y));
	
	GRenderer->SetTexture(0, tex);
	
	if(tex && tex->hasColorKey()) {
		GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
	}
	
	EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
	
	if(tex && tex->hasColorKey()) {
		GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
	}
}

void EERIEDrawBitmap2DecalY(float x, float y, float sx, float sy, float z, TextureContainer * tex,
                            Color color, float _fDeltaY) {
	
	// Match pixel and texel origins.
	x -= .5f, y -= .5f;
	
	Vec2f uv = (tex) ? tex->uv : Vec2f::ZERO;
	float sv = uv.y * _fDeltaY;
	
	ColorBGRA col = color.toBGRA();
	TexturedVertex v[4];
	float fDy = _fDeltaY * sy;
	if(sx < 0) {
		v[0] = TexturedVertex(Vec3f(x,      y + fDy, z), 1.f, col, 0xFF000000, Vec2f(uv.x, sv));
		v[1] = TexturedVertex(Vec3f(x - sx, y + fDy, z), 1.f, col, 0xFF000000, Vec2f(0.f,  sv));
		v[2] = TexturedVertex(Vec3f(x - sx, y + sy,  z), 1.f, col, 0xFF000000, Vec2f(0.f,  uv.y));
		v[3] = TexturedVertex(Vec3f(x,      y + sy,  z), 1.f, col, 0xFF000000, Vec2f(uv.x, uv.y));
	} else {
		v[0] = TexturedVertex(Vec3f(x,      y + fDy, z), 1.f, col, 0xFF000000, Vec2f(0.f,  sv));
		v[1] = TexturedVertex(Vec3f(x + sx, y + fDy, z), 1.f, col, 0xFF000000, Vec2f(uv.x, sv));
		v[2] = TexturedVertex(Vec3f(x + sx, y + sy,  z), 1.f, col, 0xFF000000, Vec2f(uv.x, uv.y));
		v[3] = TexturedVertex(Vec3f(x,      y + sy,  z), 1.f, col, 0xFF000000, Vec2f(0.f,  uv.y));
	}
	
	GRenderer->SetTexture(0, tex);
	EERIEDRAWPRIM(Renderer::TriangleFan, v, 4);	
}
