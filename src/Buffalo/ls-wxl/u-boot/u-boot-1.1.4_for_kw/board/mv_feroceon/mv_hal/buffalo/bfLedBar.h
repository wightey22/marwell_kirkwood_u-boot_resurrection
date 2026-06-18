#if defined(__BFLEDBAR_H__)
#else
#define __BFLEDBAR_H__
extern MV_BOOL bfSetLedBar(MV_U8 value);
extern MV_BOOL bfTestLedBar(MV_16 loop, MV_U16 delay);
extern MV_BOOL bfNcTestLedBar(MV_16 loop, MV_U16 delay);
extern MV_BOOL bfGetLedBarControl(MV_BOOL ope);

#endif
