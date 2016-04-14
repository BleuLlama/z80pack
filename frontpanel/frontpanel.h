/* frontpanel.h		front panel api include file */


/* Copyright (c) 2007-2008, John Kichury

   This software is freely distributable free of charge and without license fees with the 
   following conditions:

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JOHN KICHURY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The above copyright notice must be included in any copies of this software.

*/

#ifndef _FP_API_DEFS
#define _FP_API_DEFS

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef _WINDLL
#define FP_DLLPORT __declspec (dllexport)
#else
#define FP_DLLPORT
#endif

#ifndef _SIM_DEFS_H_
#define _SIM_DEFS_H_
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
#endif
typedef unsigned long long uint64;

#define FP_SW_DOWN	0
#define FP_SW_UP	1
#define FP_SW_CENTER	2

FP_DLLPORT int	fp_test(int n);
FP_DLLPORT int     fp_init(char *cfg_fname );
FP_DLLPORT int	fp_openWindow(char *title);
FP_DLLPORT void	fp_framerate(float f);
FP_DLLPORT void	fp_sampleData(void);
FP_DLLPORT void	fp_sampleDataWarp(int clockwarp);
FP_DLLPORT void	fp_sampleLightGroup(int groupnum, int clockwarp);
FP_DLLPORT void	fp_sampleSwitches(void);
FP_DLLPORT void	fp_quit(void);

/* data binding functions */

FP_DLLPORT void 	fp_bindSimclock( uint64 *addr);
FP_DLLPORT void	fp_bindRunFlag(uint8 *addr);
FP_DLLPORT int	fp_bindLight32(char *name, uint32 *bits, int bitnum);
FP_DLLPORT int	fp_bindLight16(char *name, uint16 *bits, int bitnum);
FP_DLLPORT int	fp_bindLight8(char *name, uint8 *bits, int bitnum);
FP_DLLPORT int	fp_bindLight8invert(char *name, uint8 *bits, int bitnum, uint8 mask);
FP_DLLPORT int	fp_bindLight16invert(char *name, uint16 *bits, int bitnum, uint16 mask);
FP_DLLPORT int	fp_bindLight32invert(char *name, uint32 *bits, int bitnum, uint32 mask);

FP_DLLPORT int	fp_bindSwitch32(char *name, uint32 *loc_down, uint32 *loc_up, int bitnum);
FP_DLLPORT int	fp_bindSwitch16(char *name, uint16 *loc_down, uint16 *loc_up, int bitnum);
FP_DLLPORT int	fp_bindSwitch8(char *name, uint8 *loc_down, uint8 *loc_up, int bitnum);

/* callbacks */

FP_DLLPORT int	fp_addSwitchCallback(char *name, void (*cbfunc)(int state, int val), int userval);
FP_DLLPORT void	fp_addQuitCallback(void (*cbfunc)(void));

/* error reporting */

FP_DLLPORT void    fp_ignoreBindErrors(int n);		/* n=1 - ignore    n=0 report errors */
#ifdef __cplusplus
 }
#endif



#endif

