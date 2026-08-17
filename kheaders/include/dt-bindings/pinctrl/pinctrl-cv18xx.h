/* SPDX-License-Identifier: GPL-2.0-only OR BSD-2-Clause */


#ifndef _DT_BINDINGS_PINCTRL_CV18XX_H
#define _DT_BINDINGS_PINCTRL_CV18XX_H

#define PIN_MUX_INVALD				0xff

#define PINMUX2(pin, mux, mux2)	\
	(((pin) & 0xffff) | (((mux) & 0xff) << 16) | (((mux2) & 0xff) << 24))

#define PINMUX(pin, mux) \
	PINMUX2(pin, mux, PIN_MUX_INVALD)

#endif 
