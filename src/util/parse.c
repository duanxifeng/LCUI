﻿/* ***************************************************************************
 * parse.c -- parse data from string
 *
 * Copyright (C) 2015-2017 by Liu Chao <lc-soft@live.cn>
 *
 * This file is part of the LCUI project, and may only be used, modified, and
 * distributed under the terms of the GPLv2.
 *
 * (GPLv2 is abbreviation of GNU General Public License Version 2)
 *
 * By continuing to use, modify, or distribute this file you indicate that you
 * have read the license and understand and accept it fully.
 *
 * The LCUI project is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GPL v2 for more details.
 *
 * You should have received a copy of the GPLv2 along with this file. It is
 * usually in the LICENSE.TXT file, If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************/

/* ****************************************************************************
 * parse.c -- 字符串中的书局解析
 *
 * 版权所有 (C) 2015-2017 归属于 刘超 <lc-soft@live.cn>
 *
 * 这个文件是LCUI项目的一部分，并且只可以根据GPLv2许可协议来使用、更改和发布。
 *
 * (GPLv2 是 GNU通用公共许可证第二版 的英文缩写)
 *
 * 继续使用、修改或发布本文件，表明您已经阅读并完全理解和接受这个许可协议。
 *
 * LCUI 项目是基于使用目的而加以散布的，但不负任何担保责任，甚至没有适销性或特
 * 定用途的隐含担保，详情请参照GPLv2许可协议。
 *
 * 您应已收到附随于本文件的GPLv2许可协议的副本，它通常在LICENSE.TXT文件中，如果
 * 没有，请查看：<http://www.gnu.org/licenses/>.
 * ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <LCUI_Build.h>
#include <LCUI/LCUI.h>
#include <LCUI/util/parse.h>

/** 从字符串中解析出数值，包括px、%、dp等单位 */
LCUI_BOOL ParseNumber( LCUI_Style s, const char *str )
{
	int n = 0;
	char num_str[32];
	const char *p = str;
	LCUI_BOOL has_sign = FALSE, has_point = FALSE;

	if( str == NULL ) {
		return FALSE;
	}
	/* 先取出数值 */
	while( 1 ) {
		if( *p >= '0' && *p <= '9' );
		else if( !has_sign && (*p == '-' || *p == '+') ) {
			has_sign = TRUE;
		} else if( !has_point && *p == '.' ) {
			has_point = TRUE;
		} else {
			break;
		}
		if( n < 30 ) {
			num_str[n++] = *p;
		}
		++p;
	}
	if( n == 0 ) {
		return FALSE;
	}
	num_str[n] = 0;
	switch( *p ) {
	case 'd':
	case 'D':
		s->type = SVT_NONE;
		if( !p[1] ) {
			break;
		}
		if( p[1] == 'p' || p[1] == 'P' ) {
			s->type = SVT_DIP;
			sscanf( num_str, "%f", &s->dip );
			break;
		}
		if( p[1] == 'i' || p[1] == 'I' ) {
			if( p[2] == 'p' || p[2] == 'P' ) {
				s->type = SVT_DIP;
				sscanf( num_str, "%f", &s->dip );
				break;
			}
		}
		break;
	case 's':
	case 'S':
		if( p[1] == 'p' || p[1] == 'P' ) {
			s->type = SVT_SP;
			sscanf( num_str, "%f", &s->sp );
		} else {
			s->type = SVT_NONE;
		}
		break;
	case 'P':
	case 'p':
		if( p[1] == 'x' || p[1] == 'X' ) {
			s->type = SVT_PX;
			sscanf( num_str, "%f", &s->px );
		} else if( p[1] == 't' || p[1] == 'T' ) {
			s->type = SVT_PT;
			sscanf( num_str, "%f", &s->pt );
		} else {
			s->type = SVT_NONE;
		}
		break;
	case '%':
		if( 1 != sscanf( num_str, "%f", &s->scale ) ) {
			return FALSE;
		}
		s->scale /= 100.0;
		s->type = SVT_SCALE;
		break;
	case 0:
		if( has_point && 1 == sscanf( num_str, "%f", &s->scale ) ) {
			s->type = SVT_SCALE;
			break;
		}
		if( 1 == sscanf( num_str, "%d", &s->value ) ) {
			s->type = SVT_VALUE;
			break;
		}
	default:
		s->type = SVT_NONE;
		s->is_valid = FALSE;
		return FALSE;
	}
	s->is_valid = TRUE;
	return TRUE;
}

LCUI_BOOL ParseRGBA( LCUI_Style var, const char *str )
{
	float data[4];
	char buf[16];
	const char *p;
	int i, buf_i;

	if( !strstr( str, "rgba(" ) ) {
		return FALSE;
	}
	for( p = str + 5, i = 0, buf_i = 0; *p && i < 4; ++p ) {
		if( *p == '.' || (*p >= '0' && *p <= '9') ) {
			if( buf_i < 15 ) {
				buf[buf_i++] = *p;
			}
			continue;
		}
		if( *p == ' ' ) {
			buf_i = 0;
			continue;
		}
		if( *p == ',' || *p == ')' ) {
			buf[buf_i] = 0;
			sscanf( buf, "%f", &data[i] );
			buf_i = 0;
			i += 1;
		}
	}
	if( *p ) {
		return FALSE;
	}
	var->type = SVT_COLOR;
	var->color.a = (uchar_t)(255.0 * data[3]);
	var->color.r = (uchar_t)data[0];
	var->color.g = (uchar_t)data[1];
	var->color.b = (uchar_t)data[2];
	var->is_valid = TRUE;
	return TRUE;
}

LCUI_BOOL ParseRGB( LCUI_Style var, const char *str )
{
	float data[3];
	char buf[16];
	const char *p;
	int i, buf_i;

	if( !strstr( str, "rgb(" ) ) {
		return FALSE;
	}
	for( p = str + 4, i = 0, buf_i = 0; *p && i < 3; ++p ) {
		if( *p == '.' || (*p >= '0' && *p <= '9') ) {
			if( buf_i < 15 ) {
				buf[buf_i++] = *p;
			}
			continue;
		}
		if( *p == ' ' ) {
			buf_i = 0;
			continue;
		}
		if( *p == ',' || *p == ')' ) {
			buf[buf_i] = 0;
			sscanf( buf, "%f", &data[i] );
			buf_i = 0;
			i += 1;
		}
	}
	if( *p ) {
		return FALSE;
	}
	var->type = SVT_COLOR;
	var->color.a = 255;
	var->color.r = (uchar_t)data[0];
	var->color.g = (uchar_t)data[1];
	var->color.b = (uchar_t)data[2];
	var->is_valid = TRUE;
	return TRUE;
}

/** 从字符串中解析出色彩值，支持格式：#fff、#ffffff, rgba(R,G,B,A)、rgb(R,G,B) */
LCUI_BOOL ParseColor( LCUI_Style var, const char *str )
{
	const char *p;
	int len = 0, status = 0, r, g, b;
	for( p = str; *p; ++p, ++len ) {
		switch( *p ) {
		case '#':
			len == 0 ? status = 3 : 0;
			break;
		case 'r':
			status == 0 ? status = 1 : 0;
			break;
		case 'g':
			status == 1 ? status <<= 1 : 0;
			break;
		case 'b':
			status == 2 ? status <<= 1 : 0;
			break;
		case 'a':
			status == 4 ? status <<= 1 : 0;
			break;
		default:
			if( status < 3 ) {
				break;
			}
		}
	}
	switch( status ) {
	case 3:
		status = 0;
		if( len == 4 ) {
			status = sscanf( str, "#%1X%1X%1X", &r, &g, &b );
			r *= 255 / 0xf; g *= 255 / 0xf; b *= 255 / 0xf;
		} else if( len == 7 ) {
			status = sscanf( str, "#%2X%2X%2X", &r, &g, &b );
		}
		break;
	case 4: return ParseRGB( var, str );
	case 8: return ParseRGBA( var, str );
	default:break;
	}
	if( status == 3 ) {
		var->type = SVT_COLOR;
		var->color.a = 255;
		var->color.r = r;
		var->color.g = g;
		var->color.b = b;
		var->is_valid = TRUE;
		return TRUE;
	}
	if( strcmp( "transparent", str ) == 0 ) {
		var->is_valid = TRUE;
		var->color.alpha = 0;
		var->color.red = 255;
		var->color.green = 255;
		var->color.blue = 255;
		var->type = SVT_COLOR;
		return TRUE;
	}
	return FALSE;
}
