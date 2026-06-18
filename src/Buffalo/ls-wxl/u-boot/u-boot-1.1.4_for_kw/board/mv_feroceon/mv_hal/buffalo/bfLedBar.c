#include "gpp/mvGpp.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

#define BAR_COLOR_NONE	0
#define BAR_COLOR_RED	1
#define BAR_COLOR_BLUE	2

static MV_BOOL
bfSetLedBarColor(MV_U8 color)
{
	MV_32 pin_red = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED_RED, 0);
	MV_32 pin_blue = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED_BLUE, 0);

	if(pin_red == MV_ERROR || pin_blue == MV_ERROR)
	{
		printf("%s : This board doesn't support bar led function?\n", __FUNCTION__);
		return MV_FALSE;
	}

	bfGppOutRegBitNagate(pin_red);
	bfGppOutRegBitNagate(pin_blue);

	switch(color)
	{
	case BAR_COLOR_RED:
		bfGppOutRegBitAssert(pin_red);
		break;
	case BAR_COLOR_BLUE:
		bfGppOutRegBitAssert(pin_blue);
		break;
	default:
		printf("%s : Unkown operation requested(%u)\n", __FUNCTION__, color);
		return MV_FALSE;
		break;
	}
	return MV_TRUE;
}

static MV_BOOL
bfSetLedBarStatus(MV_U8 value, MV_U8 present_value)
{
	MV_32 pin = 0;
	int i = 0;
	MV_BOOL NotSupportFunction = MV_FALSE;

	if(value == present_value)
		return MV_TRUE;

	if(present_value == 0)
	{
		for(i = 0; i < 10; i++)
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			if(pin == MV_ERROR)
			{
				NotSupportFunction = MV_TRUE;
				continue;
			}
			bfGppOutRegBitNagate(pin);
		}
	}

	if(NotSupportFunction == MV_TRUE)
	{
		printf("%s : This board doesn't support bar led function?\n", __FUNCTION__);
		return MV_FALSE;
	}

	if(present_value > value)
	{
		for(i = present_value - 1; i >= value; i--)
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			//printf("Negating GPIO %d\n", pin);
			bfGppOutRegBitNagate(pin);
		}
	}
	else if(present_value < value)
	{
		for(i = present_value; i < value; i++)
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			//printf("Asserting GPIO %d\n", pin);
			bfGppOutRegBitAssert(pin);
		}
	}
	else
	{
		for(i = 0; i < value; i++)
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			//printf("Asserting GPIO %d\n", pin);
			bfGppOutRegBitAssert(pin);
		}
	}
	return MV_TRUE;
}

static MV_BOOL
bfSetLedBarStatusBin(MV_U16 value)
{
	MV_32 pin = 0;
	MV_8 i = 0;

	for(i = 0; i < 10; i++)
	{
		if(!(value & (1 << i)))
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			//printf("Negating GPIO %d\n", pin);
			bfGppOutRegBitNagate(pin);
		}
	}
	for(i = 0; i < 10; i++)
	{
		if((value & (1 << i)))
		{
			pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
			//printf("Asserting GPIO %d\n", pin);
			bfGppOutRegBitAssert(pin);
		}
	}
	return MV_TRUE;
}

static MV_U8
bfGetLedBarValue(void)
{
	MV_32 pin_red = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED_RED, 0);
	MV_32 pin_blue = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED_BLUE, 0);
	MV_BOOL NotSupportFunction = MV_FALSE;
	MV_U8 present_value = 0;
	MV_32 pin = 0;
	MV_8 i = 0;

	if(pin_red == MV_ERROR || pin_blue == MV_ERROR)
	{
		printf("%s : This board doesn't support bar led function?\n", __FUNCTION__);
		return -1;
	}

	for(i = 0; i < 10; i++)
	{
		pin = mvBoardGpioPinNumGet(BOARD_GPP_BAR_LED, i);
		if(pin == MV_ERROR)
		{
			NotSupportFunction = MV_TRUE;
			continue;
		}
		if(bfGppOutRegBitTest(pin) == MV_TRUE)
			present_value++;
		else
			break;
	}

	if(NotSupportFunction == MV_TRUE)
	{
		printf("%s : This board doesn't support bar led function?\n", __FUNCTION__);
		return -1;
	}

	// inverted polarity pin.
	if((bfGppOutRegBitTest(pin_blue) == MV_FALSE))
		present_value *= 10;

	if(present_value >= 0 && present_value <= 100)
		return present_value;
	return -1;
}

MV_BOOL
bfSetLedBar(MV_U8 value)
{
	MV_U8 set_value = 0;
	MV_U8 present_value = 0;
	//printf("%s : requested value is %d\n", __FUNCTION__, value);

	if (value < 0 || value > 100)
		return MV_FALSE;

	present_value = bfGetLedBarValue();
	//printf("%s : present value is %d\n", __FUNCTION__, present_value);

	if(value >= 10)
	{
		if(present_value < 10 || present_value == 0)
		{
			if(bfSetLedBarColor(BAR_COLOR_BLUE) == MV_FALSE)
				return MV_FALSE;
		}
		else
		{
			present_value = present_value / 10;
		}
		set_value = value / 10;
	}
	else
	{
		if(present_value >= 10 || present_value == 0)
		{
			if(bfSetLedBarColor(BAR_COLOR_RED) == MV_FALSE)
				return MV_FALSE;
			present_value = present_value / 10;
		}
		set_value = value;
	}

	return bfSetLedBarStatus(set_value, present_value);
}

MV_BOOL
bfTestLedBar(MV_16 loop, MV_U16 delay)
{
	MV_U8 on_led_num = 4;
	MV_U16 patern = 0;
	MV_U16 i = 0;

	for(i = 0; i < on_led_num; i++)
	{
		patern |= (1 << i);
	}

	for(i = 0; i < loop; i++)
	{
		bfSetLedBarStatusBin(patern);
		if(patern & (1 << 9))
			patern = ((patern & 0x1ff) * 2) + 1;
		else
			patern = patern * 2;
		mvOsSleep(delay);
	}
	bfSetLedBarStatusBin(0);
	return MV_TRUE;
}

MV_BOOL
bfNcTestLedBar(MV_16 loop, MV_U16 delay)
{
	MV_U8 on_led_num = 4;
	MV_U16 patern = 0;
	MV_U16 i = 0;
	MV_16 j = 0, k = 0;

	for(i = 0; i < 10 || loop == -1; i++)
	{
		for(j = 0; j < 9 + on_led_num; j++)
		{
			patern = 0;
			for(k = j - on_led_num; k <= j; k++)
			{
				if(k >= 0 && k <= 10)
					patern |= (1 << k);
			}
			bfSetLedBarStatusBin(patern);
			mvOsSleep(delay);
		}
		for(j = 9 + on_led_num; j >= 0; j--)
		{
			patern = 0;
			for(k = j - on_led_num; k <= j; k++)
			{
				if(k >= 0 && k <= 10)
					patern |= (1 << k);
			}
			bfSetLedBarStatusBin(patern);
			mvOsSleep(delay);
		}
	}
	bfSetLedBarStatusBin(0);
	return MV_TRUE;
}

MV_BOOL
bfGetLedBarControl(MV_BOOL ope)
{
	MV_32 pin = mvBoardGpioPinNumGet(BOARD_GPP_LED_FULL_BRIGHT, 0);

	if(pin == MV_ERROR)
	{
		printf("%s : This board doesn't support bar led function?\n", __FUNCTION__);
		return MV_FALSE;
	}

	if(ope == MV_TRUE)
	{
		if(bfGppOutRegBitTest(pin))
		{
			bfGppOutRegBitNagate(pin);
			//printf("pin (%d) negated.\n", pin);
		}
	}
	else
	{
		if(!bfGppOutRegBitTest(pin))
		{
			bfGppOutRegBitAssert(pin);
			//printf("pin (%d) asserted.\n", pin);
		}
	}

	return MV_TRUE;
}
