
// Physics
extern float	cpm_pm_jump_z;
extern float	cpm_pm_airstopaccelerate;
extern float	cpm_pm_aircontrol;
extern float	cpm_pm_strafeaccelerate;
extern float	cpm_pm_wishspeed;
extern float	pm_accelerate; // located in bg_pmove.c
extern float	pm_friction; // located in bg_pmove.c

void CPM_UpdateSettings(int num);
void CPM_PM_Aircontrol ( pmove_t *pm, vec3_t wishdir, float wishspeed );

// Weapon switching
extern int	cpm_weapondrop;
extern int	cpm_weaponraise;
extern int	cpm_outofammodelay;

// Z Knockback
extern float	cpm_knockback_z;

// Respawn delay
extern int	cpm_clientrespawndelay;

// Hit tones
extern int		cpm_hittones;
