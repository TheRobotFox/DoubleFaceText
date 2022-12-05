#pragma once

#define H_KERN 1.4 	// Horizontal Kerning multiplier
#define V_KERN 1.4 	// Vertical Kerning multiplier
#define SPACE_WIDTH 3 	// Space width in units

#include "NoFont/Font_Driver.h"
const unsigned char raw[214] = {0x69, 0x9f, 0x99, 0x99, 0xe9, 0x9f, 0x99, 0x9e, 0x72, 0x49, 0x23, 0xe9, 0x99, 0x99, 0x9e, 0xf8, 0x8e, 0x88, 0x8f, 0xf8, 0x8e, 0x88, 0x88, 0xf8, 0x88, 0x8b, 0x9f, 0x99, 0x9f, 0x99, 0x99, 0xe9, 0x24, 0x97, 0xf1, 0x11, 0x11, 0x9e, 0x99, 0xac, 0xca, 0x99, 0x88, 0x88, 0x88, 0x8f, 0x8e, 0xff, 0x58, 0xc6, 0x31, 0x8e, 0x73, 0x5a, 0xce, 0x71, 0x69, 0x99, 0x99, 0x96, 0xe9, 0x9e, 0x88, 0x88, 0x69, 0x99, 0x99, 0xa5, 0xe9, 0x9e, 0xca, 0xa9, 0x78, 0x86, 0x11, 0x1e, 0xf9, 0x8, 0x42, 0x10, 0x84, 0x99, 0x99, 0x99, 0x96, 0x8c, 0x63, 0x18, 0xa9, 0x44, 0x8c, 0x63, 0x1a, 0xff, 0x71, 0x8c, 0x54, 0x42, 0x2a, 0x31, 0x8c, 0x63, 0x15, 0x10, 0x84, 0xf1, 0x12, 0x48, 0x8f, 0x2f, 0x92, 0x49, 0x74, 0x62, 0x22, 0x22, 0x1f, 0xe1, 0x16, 0x11, 0x1e, 0x88, 0x89, 0xf1, 0x11, 0xf8, 0x8f, 0x11, 0x1f, 0xf8, 0x8f, 0x99, 0x9f, 0xf1, 0x11, 0x11, 0x11, 0xf9, 0x9f, 0x99, 0x9f, 0xf9, 0x9f, 0x11, 0x1f, 0xf9, 0x99, 0x99, 0x9f, 0x6a, 0xa9, 0x95, 0x56, 0xea, 0xab, 0xd5, 0x57, 0x34, 0x48, 0x44, 0x43, 0xc2, 0x21, 0x22, 0x2c, 0xfd, 0xf1, 0x17, 0x44, 0x4, 0xb4, 0x0, 0x0, 0x0, 0x3e, 0x0, 0x7c, 0x0, 0x24, 0xa5, 0x24, 0x92, 0x24, 0x49, 0xc0, 0x1, 0x9, 0xf2, 0x10, 0x0, 0x0, 0x1, 0xf0, 0x0, 0x0, 0x5, 0x5d, 0xf7, 0x54, 0x0, 0x52, 0xbe, 0xa5, 0x7d, 0x4a, 0x0, 0xf, 0x0, 0x3e, 0x0, 0x0, 0x0, 0x0, 0x1f};

const struct Font_Char charset[56] = {
	{4, 8, raw+0},
	{4, 8, raw+4},
	{3, 8, raw+8},
	{4, 8, raw+11},
	{4, 8, raw+15},
	{4, 8, raw+19},
	{4, 8, raw+23},
	{4, 8, raw+27},
	{3, 8, raw+31},
	{4, 8, raw+34},
	{4, 8, raw+38},
	{4, 8, raw+42},
	{5, 8, raw+46},
	{5, 8, raw+51},
	{4, 8, raw+56},
	{4, 8, raw+60},
	{4, 8, raw+64},
	{4, 8, raw+68},
	{4, 8, raw+72},
	{5, 8, raw+76},
	{4, 8, raw+81},
	{5, 8, raw+85},
	{5, 8, raw+90},
	{5, 8, raw+95},
	{5, 8, raw+100},
	{4, 8, raw+105},
	{3, 8, raw+109},
	{5, 8, raw+112},
	{4, 8, raw+117},
	{4, 8, raw+121},
	{4, 8, raw+125},
	{4, 8, raw+129},
	{4, 8, raw+133},
	{4, 8, raw+137},
	{4, 8, raw+141},
	{4, 8, raw+145},
	{2, 8, raw+149},
	{2, 8, raw+151},
	{2, 8, raw+153},
	{2, 8, raw+155},
	{4, 8, raw+157},
	{4, 8, raw+161},
	{1, 8, raw+165},
	{4, 8, raw+166},
	{3, 8, raw+170},
	{5, 8, raw+173},
	{3, 8, raw+178},
	{3, 8, raw+181},
	{1, 8, raw+184},
	{5, 8, raw+185},
	{5, 8, raw+190},
	{5, 8, raw+195},
	{5, 8, raw+200},
	{2, 8, raw+205},
	{2, 8, raw+207},
	{5, 8, raw+209}};

struct Font font = {charset, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, charset+42 /* !|33 */, charset+44 /* "|34 */, charset+52 /* #|35 */, 0, 0, 0, charset+48 /* '|39 */, charset+36 /* (|40 */, charset+37 /* )|41 */, charset+51 /* *|42 */, charset+49 /* +|43 */, charset+54 /* ,|44 */, charset+50 /* -|45 */, charset+53 /* .|46 */, charset+46 /* /|47 */, charset+35 /* 0|48 */, charset+26 /* 1|49 */, charset+27 /* 2|50 */, charset+28 /* 3|51 */, charset+29 /* 4|52 */, charset+30 /* 5|53 */, charset+31 /* 6|54 */, charset+32 /* 7|55 */, charset+33 /* 8|56 */, charset+34 /* 9|57 */, 0, 0, 0, charset+45 /* =|61 */, 0, charset+43 /* ?|63 */, 0, charset+0 /* A|65 */, charset+1 /* B|66 */, charset+2 /* C|67 */, charset+3 /* D|68 */, charset+4 /* E|69 */, charset+5 /* F|70 */, charset+6 /* G|71 */, charset+7 /* H|72 */, charset+8 /* I|73 */, charset+9 /* J|74 */, charset+10 /* K|75 */, charset+11 /* L|76 */, charset+12 /* M|77 */, charset+13 /* N|78 */, charset+14 /* O|79 */, charset+15 /* P|80 */, charset+16 /* Q|81 */, charset+17 /* R|82 */, charset+18 /* S|83 */, charset+19 /* T|84 */, charset+20 /* U|85 */, charset+21 /* V|86 */, charset+22 /* W|87 */, charset+23 /* X|88 */, charset+24 /* Y|89 */, charset+25 /* Z|90 */, charset+38 /* [|91 */, 0, charset+39 /* ]|93 */, 0, charset+55 /* _|95 */, 0, charset+0 /* a|97 */, charset+1 /* b|98 */, charset+2 /* c|99 */, charset+3 /* d|100 */, charset+4 /* e|101 */, charset+5 /* f|102 */, charset+6 /* g|103 */, charset+7 /* h|104 */, charset+8 /* i|105 */, charset+9 /* j|106 */, charset+10 /* k|107 */, charset+11 /* l|108 */, charset+12 /* m|109 */, charset+13 /* n|110 */, charset+14 /* o|111 */, charset+15 /* p|112 */, charset+16 /* q|113 */, charset+17 /* r|114 */, charset+18 /* s|115 */, charset+19 /* t|116 */, charset+20 /* u|117 */, charset+21 /* v|118 */, charset+22 /* w|119 */, charset+23 /* x|120 */, charset+24 /* y|121 */, charset+25 /* z|122 */, charset+40 /* {|123 */, 0, charset+41 /* }|125 */}, 8, H_KERN, V_KERN, SPACE_WIDTH};

#undef H_KERN
#undef V_KERN
#undef SPACE_WIDTH
#undef raw
#undef charset
#undef map
