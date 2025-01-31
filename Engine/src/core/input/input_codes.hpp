#pragma once

#include <stdint.h> // TODO: Precompile headers.
#include <array>
#include <utility>

namespace Alvar
{
	using KeyCode = uint16_t;

	namespace Key
	{
		enum : KeyCode
		{
			// From glfw3.h
			Space               = 32,
			Apostrophe          = 39, /* ' */
			Comma               = 44, /* , */
			Minus               = 45, /* - */
			Period              = 46, /* . */
			Slash               = 47, /* / */

			D0                  = 48, /* 0 */
			D1                  = 49, /* 1 */
			D2                  = 50, /* 2 */
			D3                  = 51, /* 3 */
			D4                  = 52, /* 4 */
			D5                  = 53, /* 5 */
			D6                  = 54, /* 6 */
			D7                  = 55, /* 7 */
			D8                  = 56, /* 8 */
			D9                  = 57, /* 9 */

			Semicolon           = 59, /* ; */
			Equal               = 61, /* = */

			A                   = 65,
			B                   = 66,
			C                   = 67,
			D                   = 68,
			E                   = 69,
			F                   = 70,
			G                   = 71,
			H                   = 72,
			I                   = 73,
			J                   = 74,
			K                   = 75,
			L                   = 76,
			M                   = 77,
			N                   = 78,
			O                   = 79,
			P                   = 80,
			Q                   = 81,
			R                   = 82,
			S                   = 83,
			T                   = 84,
			U                   = 85,
			V                   = 86,
			W                   = 87,
			X                   = 88,
			Y                   = 89,
			Z                   = 90,

			LeftBracket         = 91,  /* [ */
			Backslash           = 92,  /* \ */
			RightBracket        = 93,  /* ] */
			GraveAccent         = 96,  /* ` */

			World1              = 161, /* non-US #1 */
			World2              = 162, /* non-US #2 */

			/* Function keys */
			Escape              = 256,
			Enter               = 257,
			Tab                 = 258,
			Backspace           = 259,
			Insert              = 260,
			Delete              = 261,
			Right               = 262,
			Left                = 263,
			Down                = 264,
			Up                  = 265,
			PageUp              = 266,
			PageDown            = 267,
			Home                = 268,
			End                 = 269,
			CapsLock            = 280,
			ScrollLock          = 281,
			NumLock             = 282,
			PrintScreen         = 283,
			Pause               = 284,
			F1                  = 290,
			F2                  = 291,
			F3                  = 292,
			F4                  = 293,
			F5                  = 294,
			F6                  = 295,
			F7                  = 296,
			F8                  = 297,
			F9                  = 298,
			F10                 = 299,
			F11                 = 300,
			F12                 = 301,
			F13                 = 302,
			F14                 = 303,
			F15                 = 304,
			F16                 = 305,
			F17                 = 306,
			F18                 = 307,
			F19                 = 308,
			F20                 = 309,
			F21                 = 310,
			F22                 = 311,
			F23                 = 312,
			F24                 = 313,
			F25                 = 314,

			/* Keypad */
			KP0                 = 320,
			KP1                 = 321,
			KP2                 = 322,
			KP3                 = 323,
			KP4                 = 324,
			KP5                 = 325,
			KP6                 = 326,
			KP7                 = 327,
			KP8                 = 328,
			KP9                 = 329,
			KPDecimal           = 330,
			KPDivide            = 331,
			KPMultiply          = 332,
			KPSubtract          = 333,
			KPAdd               = 334,
			KPEnter             = 335,
			KPEqual             = 336,

			LeftShift           = 340,
			LeftControl         = 341,
			LeftAlt             = 342,
			LeftSuper           = 343,
			RightShift          = 344,
			RightControl        = 345,
			RightAlt            = 346,
			RightSuper          = 347,
			Menu                = 348
		};

	constexpr std::array<std::pair<uint16_t, int>, 97> KeyCodeMap = {{
		{Space, 0}, {Apostrophe, 1}, {Comma, 2}, {Minus, 3}, {Period, 4}, {Slash, 5},
		{D0, 6}, {D1, 7}, {D2, 8}, {D3, 9}, {D4, 10}, {D5, 11}, {D6, 12}, {D7, 13}, {D8, 14}, {D9, 15},
		{Semicolon, 16}, {Equal, 17}, {A, 18}, {B, 19}, {C, 20}, {D, 21}, {E, 22}, {F, 23}, {G, 24}, {H, 25}, 
		{I, 26}, {J, 27}, {K, 28}, {L, 29}, {M, 30}, {N, 31}, {O, 32}, {P, 33}, {Q, 34}, {R, 35}, {S, 36}, 
		{T, 37}, {U, 38}, {V, 39}, {W, 40}, {X, 41}, {Y, 42}, {Z, 43}, {LeftBracket, 44}, {Backslash, 45}, 
		{RightBracket, 46}, {GraveAccent, 47}, {World1, 48}, {World2, 49}, {Escape, 50}, {Enter, 51}, 
		{Tab, 52}, {Backspace, 53}, {Insert, 54}, {Delete, 55}, {Right, 56}, {Left, 57}, {Down, 58}, 
		{Up, 59}, {PageUp, 60}, {PageDown, 61}, {Home, 62}, {End, 63}, {CapsLock, 64}, {ScrollLock, 65}, 
		{NumLock, 66}, {PrintScreen, 67}, {Pause, 68}, {F1, 69}, {F2, 70}, {F3, 71}, {F4, 72}, {F5, 73}, 
		{F6, 74}, {F7, 75}, {F8, 76}, {F9, 77}, {F10, 78}, {F11, 79}, {F12, 80}, {F13, 81}, {F14, 82}, 
		{F15, 83}, {F16, 84}, {F17, 85}, {F18, 86}, {F19, 87}, {F20, 88}, {F21, 89}, {F22, 90}, 
		{F23, 91}, {F24, 92}, {F25, 93}, {LeftShift, 94}, {LeftControl, 95}, {LeftAlt, 96}
	}};

	}

	using MouseCode = uint16_t;

	namespace Mouse
	{
		enum : MouseCode
		{
			// From glfw3.h
			Button0                = 0,
			Button1                = 1,
			Button2                = 2,
			Button3                = 3,
			Button4                = 4,
			Button5                = 5,
			Button6                = 6,
			Button7                = 7,

			ButtonLast             = Button7,
			ButtonLeft             = Button0,
			ButtonRight            = Button1,
			ButtonMiddle           = Button2
		};
	}
}