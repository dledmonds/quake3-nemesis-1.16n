// Cyrus - Nemesis media shader - www.q3nemesis.com - 2006

powerups/quadColor
{
	nopicmip
	
	deformVertexes wave 100 sin 3 0 0 0
	{
		map textures/effects/quadcolor.tga
		blendfunc GL_ONE GL_ONE
		rgbGen entity
		tcGen environment
                tcmod rotate 30
                tcmod scroll 1 .1
	}
}

powerups/quadred
{
	deformVertexes wave 100 sin 3 0 0 0
	{
		map textures/effects/quadmap3.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
                tcmod rotate 30
                tcmod scroll 1 .1
	}
}

team_red_icon
{
	nopicmip       	
	{
		map team_media/team_red.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA           
	}

}

team_blue_icon
{
	nopicmip       	
	{
		map team_media/team_blue.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA           
	}

}

scoreboard_bar_red
{
	nopicmip       	
	{
		map team_media/bar_red.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

}

scoreboard_bar_blue
{
	nopicmip       	
	{
		map team_media/bar_blue.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}

}

freezeShader
{
	deformVertexes wave 100 sin 2 0 0 0
	{
		map textures/effects/popsicle.jpg
		blendfunc add
		rgbGen const ( 0.95 1 1 )
		tcMod scroll 0.1 0.1
		tcGen environment
	}
}

freezeMarkShader
{
	nopicmip
	polygonoffset
	{
		clampmap gfx/damage/freeze_stain.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbgen identitylighting
		alphagen vertex
	}
}

plasmaExplosion2
{
	cull disable
	{
		clampmap sprites/plasmaboom_color.tga
		blendfunc add
                tcMod stretch triangle .6 0.1 0 8
                tcmod rotate 999
                rgbGen wave inversesawtooth 0 1 0 1.5
                rgbGen entity
	}
}

sprites/plasma_color
{
	cull disable
	{
		clampmap sprites/plasma_color.tga
		blendfunc GL_ONE GL_ONE
                tcMod rotate 931
			rgbgen entity
	}
}

lightningBoltNew
{
	cull none
	{
		map gfx/misc/lightning3new.tga
		blendFunc GL_ONE GL_ONE
		rgbgen wave sin 1 0.5 0 7.1
		tcmod scale  2 1
		tcMod scroll -5 0
	}
	{
		map gfx/misc/lightning3new.tga
		blendFunc GL_ONE GL_ONE
		rgbgen wave sin 1 0.8 0 8.1
		tcmod scale  -1.3 -1
		tcMod scroll -7.2 0
	}
}

railRing
{
	cull none
	{
		map models/weaphits/railring.tga
                blendFunc GL_ONE GL_ONE
		rgbGen entity
		tcMod scale 1 1
		tcMod scroll 2 0
	}
}

railRingStart
{
	cull none
	{
		map models/weaphits/railringfade.tga
                blendFunc GL_ONE GL_ONE
		rgbGen entity
		tcMod scale -1 -1
		tcMod scroll 2 0
	}
}

railRingEnd
{
	cull none
	{
		map models/weaphits/railringfade.tga
                blendFunc GL_ONE GL_ONE
		rgbGen entity
		tcMod scale 1 1
		tcMod scroll 2 0
	}
}

gfx/misc/bluespark
{
	cull none
	{
		map	gfx/misc/bluespark.tga
		blendFunc GL_ONE GL_ONE
	}
}

gfx/misc/spark_team
{
	{
		map	gfx/misc/spark_team.tga
		blendFunc GL_ONE GL_ONE
		rgbgen entity
	}
}

gfx/2d/bigchars-threewave
{
	nopicmip
	nomipmaps
	{
		map gfx/2d/bigchars-threewave.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}

gfx/2d/bigchars-cpma
{
	nopicmip
	nomipmaps
	{
		map gfx/2d/bigchars-cpma.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
	}
}
