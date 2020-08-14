/*
 * File    : VDispData.h
 * Remark  : Contain definition of VDisplay data, used by NanoOS application and VDisplay component of NanoOS Terminal
 *           to exchange data.
 *
 */

#ifndef VDISPDATA_H
#define VDISPDATA_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push)
#pragma pack(1)

#define VDISP_WIDTH				240
#define VDISP_HEIGHT			320

#define VDISP_COLORREF			unsigned int
#define VDISP_RGB(r, g, b)		\
	(VDISP_COLORREF) (((unsigned int)(r & 0xFF)) | (((unsigned int)(g & 0xFF)) << 8) | (((unsigned int)(b & 0xFF)) << 16))

#define VDISP_GET_RVALUE(Color)	(Color & 0xFF)
#define VDISP_GET_GVALUE(Color)	((Color >> 8) & 0xFF)
#define VDISP_GET_BVALUE(Color)	((Color >> 16) & 0xFF)

typedef struct _VDISP_PIXEL_DATA {
	int		X;
	int		Y;
	VDISP_COLORREF	Color;
} VDISP_PIXEL_DATA, *PVDISP_PIXEL_DATA;

typedef struct _VDISP_RECTANGLE {
	int		Left;
	int		Top;
	int		Right;
	int		Bottom;
} VDISP_RECTANGLE, *PVDISP_RECTANGLE;

typedef struct _VDISP_FILL_RECT_DATA {
	VDISP_RECTANGLE		Rectangle;
	VDISP_COLORREF		Color;
} VDISP_FILL_RECT_DATA, *PVDISP_FILL_RECT_DATA;

/*
 * IN and OUT tag are relative to NanoOS point of view
 */

#define VDISP_IN_TYPE__NONE		1
#define VDISP_IN_TYPE__TOUCH	2
#define VDISP_IN_TYPE__RELEASED	3
#define VDISP_IN_TYPE__TAB		4
#define VDISP_IN_TYPE__DRAG		5
#define VDISP_IN_TYPE__PIXEL	6

typedef struct _VDISP_IN_DATA {
	unsigned int	Type;
	int				X;
	int				Y;
	VDISP_COLORREF	Color;
} VDISP_IN_DATA, *PVDISP_IN_DATA;

#define VDISP_OUT_TYPE__NONE		1
#define VDISP_OUT_TYPE__SET_PIXEL	2
#define VDISP_OUT_TYPE__GET_PIXEL	3
#define VDISP_OUT_TYPE__FILL_RECT	4
#define VDISP_OUT_TYPE__UPDATE		5

/* Must be less than USB_DATA_SIZE (64 bytes) */
typedef struct _VDISP_OUT_DATA {
	unsigned int		Type;
	union {
		unsigned char			Raw[58];
		VDISP_PIXEL_DATA		Pixel;
		VDISP_FILL_RECT_DATA	FillRect;
	} u;
} VDISP_OUT_DATA, *PVDISP_OUT_DATA;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  // End of VDISPDATA_H
