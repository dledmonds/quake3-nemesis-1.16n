
/*
-----------------------------------------
File:	bg_promode.c
Auth:	arQon
Date:	6/06/05
Site:	www.promode.org
-----------------------------------------
*/

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_promode.h"

float cpm_pm_jump_z;

// Physics
float	cpm_pm_airstopaccelerate = 1.0f;
float	cpm_pm_aircontrol = 0.0f;
float	cpm_pm_strafeaccelerate = 1.0f;
float	cpm_pm_wishspeed = 400.0f;

// Weapon switching
int		cpm_weapondrop = 200;
int		cpm_weaponraise = 250;
int		cpm_outofammodelay = 500;

// Respawn delay
int		cpm_clientrespawndelay = 1700;

// Hit tones
int		cpm_hittones = 0;

void CPM_UpdateSettings(int num) {

	cpm_pm_jump_z = 0.0f; // turn off double-jump in vq3

	// Physics
	cpm_pm_airstopaccelerate = 1.0f;
	cpm_pm_aircontrol = 0.0f;
	cpm_pm_strafeaccelerate = 1.0f;
	cpm_pm_wishspeed = 400.0f;
	pm_accelerate = 10.0f;
	pm_friction = 6.0f;

	// vq3 Weapon switching
	cpm_weapondrop = 200;
	cpm_weaponraise = 250;
	cpm_outofammodelay = 500;

	// Respawn delay
	cpm_clientrespawndelay = 1700;

	// Hit tones
	cpm_hittones = 0;

	if (num)
	{
		cpm_pm_jump_z = 100.0f; // enable double-jump

		// Physics
		cpm_pm_airstopaccelerate = 2.5f;
		cpm_pm_aircontrol = 150.0f;
		cpm_pm_strafeaccelerate = 70.0f;
		cpm_pm_wishspeed = 30.0f;
		pm_accelerate = 15.0f;
		pm_friction = 8.0f;

		// Weapon switching
		cpm_weapondrop = 0;
		cpm_weaponraise = 0;
		cpm_outofammodelay = 100;

		// Respawn delay
		cpm_clientrespawndelay = 500;

		// Hit tones
		cpm_hittones = 1;
	}
}

void CPM_PM_Aircontrol (pmove_t *pm, vec3_t wishdir, float wishspeed )
{
	float	zspeed, speed, dot, k;
	int		i;

	if ( (pm->ps->movementDir && pm->ps->movementDir !=4) || wishspeed == 0.0f) 
		return; // can't control movement if not moveing forward or backward

	zspeed = pm->ps->velocity[2];
	pm->ps->velocity[2] = 0;
	speed = VectorNormalize(pm->ps->velocity);

	dot = DotProduct(pm->ps->velocity,wishdir);
	k = 32.0f; 
	k *= cpm_pm_aircontrol*dot*dot*pml.frametime;
	
	
	if (dot > 0.0f) {	// we can't change direction while slowing down
		for (i=0; i < 2; i++)
			pm->ps->velocity[i] = pm->ps->velocity[i]*speed + wishdir[i]*k;
		VectorNormalize(pm->ps->velocity);
	}
	
	for (i=0; i < 2; i++) 
		pm->ps->velocity[i] *=speed;

	pm->ps->velocity[2] = zspeed;
}
