
#include "cg_local.h"

/*
==========================
CG_RailTrail2 - WICKED!

Alternate Fire trail

FIXME: Kinda buggy on space maps(when you shoot into space & players) the rail trail disappears instantly.
==========================
*/
void CG_RailTrail2( clientInfo_t *ci, vec3_t start, vec3_t end ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;
	//le->generic1 = ci->team;
	le->leType = LE_RAIL_EFFECT;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value * 2;

	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	le->color[0] = ci->color[0] * 0.75;
	le->color[1] = ci->color[1] * 0.75;
	le->color[2] = ci->color[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );

	// core
	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value * 2;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy( start, re->origin );
	VectorCopy( end, re->oldorigin );

	// nudge down a bit so it isn't exactly in center
	re->origin[2] -= 8;
	re->oldorigin[2] -= 8;

	le->color[0] = ci->color[0] * 0.75;
	le->color[1] = ci->color[1] * 0.75;
	le->color[2] = ci->color[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
}

/*
====================
CG_AddRailEffect - CG_RailTrail2 effect(AF trail)
====================
*/
void CG_AddRailEffect( localEntity_t *le ) {
	localEntity_t	*nle;
	refEntity_t		*re, *nre;
	vec3_t			forward, right, up, posx, posy, pos, oldpos, move, vec;
	float			c, len, radius, theta, sn, cs, spacing;
	int				i = 0, r, g, b;

	re = &le->refEntity;
	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	spacing = 16.0f;

	VectorCopy (re->origin, move);
	VectorSubtract (re->oldorigin, re->origin, vec );

	// Find the forward, right, and up vectors
	VectorCopy( vec, forward );
	VectorNormalize( forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	len = VectorNormalize (vec);
	VectorScale (vec, spacing, vec);

	r = 0.5 * c;
	g = 0.0 * c;
	b = 0.5 * c;

	for (i = 0; i < len; i += spacing) {
		radius = ( 4.0 + ( 0xff - c ) / 16 )* sin( i / len * M_PI );

		theta = (float)(i - cg.time % 360) * M_PI / 180 * 2;

		cs = cos(theta);
		sn = sin(theta);

		VectorScale( right, cs, posx );
		VectorScale( up, sn, posy );
		VectorAdd( posx, posy, pos );

		VectorScale( pos, radius, pos );

		VectorAdd( pos, move, pos );

		if ( i > 0 ) {
			nle = CG_AllocLocalEntity();
			nle->startTime = cg.time;
			nle->endTime = cg.time + 1;

			nre = &nle->refEntity;

			nre->reType = RT_RAIL_CORE;
			nre->customShader = cgs.media.railCoreShader;

			nre->shaderRGBA[0] = r;
			nre->shaderRGBA[1] = g;
			nre->shaderRGBA[2] = b;
			nre->shaderRGBA[3] = 0xff * c * le->color[3];

			AxisClear( nre->axis );

			VectorCopy( pos, nre->origin );
			VectorCopy( oldpos, nre->oldorigin );

			trap_R_AddRefEntityToScene( nre );
		}

		VectorAdd (move, vec, move);
		VectorCopy( pos, oldpos );
	}
}

/*
==========================
CG_RailTrail3 - Quake FU Trail
==========================
*/
void CG_RailTrail3( clientInfo_t *ci, vec3_t start, vec3_t end ) {
	localEntity_t	*le;
	refEntity_t		*re;

	vec3_t n0, n1, n2;
	float dis, rlength, rtime;
	int i, r;
	
	VectorSubtract(end, start, n0);
	dis = VectorNormalize(n0);
	rlength = 32;
	rtime = 1600;
	
	PerpendicularVector(n1, n0);
	CrossProduct(n0, n1, n2);
	
	for(r = 0; r < dis / rlength; r++)
	{
		le = CG_AllocLocalEntity();
		re = &le->refEntity;
	
		le->leType = LE_EXPAND_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + rtime;
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	
		re->shaderTime = cg.time / 1000.0f;
		VectorMA( start, r * rlength, n0, re->origin );
	
		re->reType = RT_MODEL;
		re->renderfx |= RF_NOSHADOW;
		re->hModel = cgs.media.teleportEffectModel;
		if(r == (int) dis / rlength)
			re->customShader = cgs.media.railEndShader;
		else if(r == 0)
			re->customShader = cgs.media.railStartShader;
		else
			re->customShader = cgs.media.railRingsShader2;
		
		for(i = 0; i < 3; i++)
		{
			re->axis[0][i] = n2[i];
			re->axis[1][i] = n1[i];
			re->axis[2][i] = n0[i] * rlength / 62.125;
			if(r == (int) dis / rlength)
				re->axis[2][i] *= (((int) dis) % ((int) rlength)) / rlength;
			
		}

		le->color[0] = ci->color[0];
		le->color[1] = ci->color[1];
		le->color[2] = ci->color[2];
		le->color[3] = 255;
		le->refEntity.shaderRGBA[3] = 1;
	}
}

/*
==========================
CG_AddExpandingFade - CG_RailTrail3 effect
==========================
*/
void CG_AddExpandingFade( localEntity_t *le ) {
	refEntity_t *re;
	float c, reScale;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	
			
	reScale = 16 * 0.0075 / (c + 0.2);
	VectorNormalize(re->axis[0]);
	VectorNormalize(re->axis[1]);
	re->axis[0][0] *= reScale;
	re->axis[0][1] *= reScale;
	re->axis[0][2] *= reScale;

	re->axis[1][0] *= reScale;
	re->axis[1][1] *= reScale;
	re->axis[1][2] *= reScale;
	
	c *= 0xff;
	
	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;
	
	trap_R_AddRefEntityToScene( re );
}
