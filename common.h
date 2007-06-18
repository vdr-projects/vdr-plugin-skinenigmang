/*
 * common.h: 'EnigmaNG' skin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#ifndef __SKINENIGMA_COMMON_H
#define __SKINENIGMA_COMMON_H

#include <string>
#include <stdio.h>

#include <vdr/config.h>

// unset HAVE_FREETYPE if VDR version >=1.5.3
#ifdef HAVE_FREETYPE
#  if VDRVERSNUM > 10502
#    undef HAVE_FREETYPE
#  endif
#endif

#ifdef DEBUG
#define debug(x...) printf("EnigmaNG: " x);
#define error(x...) printf("EnigmaNG: " x);
#else
#define debug(x...) ;
#define error(x...) esyslog("EnigmaNG: " x);
#endif

#endif // __SKINENIGMA_COMMON_H
// vim:et:sw=2:ts=2:
