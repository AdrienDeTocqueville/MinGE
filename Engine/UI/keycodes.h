#pragma once

#include <Ultralight/KeyCodes.h>
#include <SFML/Window/Keyboard.hpp>

int sf_ul(sf::Keyboard::Key key)
{
	using namespace ultralight::KeyCodes;

	switch (key)
	{
	case sf::Keyboard::LSystem:	return GK_LWIN;
	case sf::Keyboard::RSystem:	return GK_RWIN;
	case sf::Keyboard::Menu:	return GK_APPS;
	case sf::Keyboard::Semicolon:	return GK_OEM_1;
	case sf::Keyboard::Slash:	return GK_OEM_2;
	case sf::Keyboard::Equal:	return GK_OEM_PLUS;
	case sf::Keyboard::Hyphen:	return GK_OEM_MINUS;
	case sf::Keyboard::LBracket:	return GK_OEM_4;
	case sf::Keyboard::RBracket:	return GK_OEM_6;
	case sf::Keyboard::Comma:	return GK_OEM_COMMA;
	case sf::Keyboard::Period:	return GK_OEM_PERIOD;
	case sf::Keyboard::Quote:	return GK_OEM_7;
	case sf::Keyboard::Backslash:	return GK_OEM_5;
	case sf::Keyboard::Tilde:	return GK_OEM_3;
	case sf::Keyboard::Escape:	return GK_ESCAPE;
	case sf::Keyboard::Space:	return GK_SPACE;
	case sf::Keyboard::Enter:	return GK_RETURN;
	case sf::Keyboard::Backspace:	return GK_BACK;
	case sf::Keyboard::Tab:		return GK_TAB;
	case sf::Keyboard::PageUp:	return GK_PRIOR;
	case sf::Keyboard::PageDown:	return GK_NEXT;
	case sf::Keyboard::End:		return GK_END;
	case sf::Keyboard::Home:	return GK_HOME;
	case sf::Keyboard::Insert:	return GK_INSERT;
	case sf::Keyboard::Delete:	return GK_DELETE;
	case sf::Keyboard::Add:		return GK_ADD;
	case sf::Keyboard::Subtract:	return GK_SUBTRACT;
	case sf::Keyboard::Multiply:	return GK_MULTIPLY;
	case sf::Keyboard::Divide:	return GK_DIVIDE;
	case sf::Keyboard::Pause:	return GK_PAUSE;
	case sf::Keyboard::F1:		return GK_F1;
	case sf::Keyboard::F2:		return GK_F2;
	case sf::Keyboard::F3:		return GK_F3;
	case sf::Keyboard::F4:		return GK_F4;
	case sf::Keyboard::F5:		return GK_F5;
	case sf::Keyboard::F6:		return GK_F6;
	case sf::Keyboard::F7:		return GK_F7;
	case sf::Keyboard::F8:		return GK_F8;
	case sf::Keyboard::F9:		return GK_F9;
	case sf::Keyboard::F10:		return GK_F10;
	case sf::Keyboard::F11:		return GK_F11;
	case sf::Keyboard::F12:		return GK_F12;
	case sf::Keyboard::F13:		return GK_F13;
	case sf::Keyboard::F14:		return GK_F14;
	case sf::Keyboard::F15:		return GK_F15;
	case sf::Keyboard::Left:	return GK_LEFT;
	case sf::Keyboard::Right:	return GK_RIGHT;
	case sf::Keyboard::Up:		return GK_UP;
	case sf::Keyboard::Down:	return GK_DOWN;
	case sf::Keyboard::Numpad0:	return GK_NUMPAD0;
	case sf::Keyboard::Numpad1:	return GK_NUMPAD1;
	case sf::Keyboard::Numpad2:	return GK_NUMPAD2;
	case sf::Keyboard::Numpad3:	return GK_NUMPAD3;
	case sf::Keyboard::Numpad4:	return GK_NUMPAD4;
	case sf::Keyboard::Numpad5:	return GK_NUMPAD5;
	case sf::Keyboard::Numpad6:	return GK_NUMPAD6;
	case sf::Keyboard::Numpad7:	return GK_NUMPAD7;
	case sf::Keyboard::Numpad8:	return GK_NUMPAD8;
	case sf::Keyboard::Numpad9:	return GK_NUMPAD9;
	case sf::Keyboard::A:		return GK_A;
	case sf::Keyboard::Z:		return GK_Z;
	case sf::Keyboard::E:		return GK_E;
	case sf::Keyboard::R:		return GK_R;
	case sf::Keyboard::T:		return GK_T;
	case sf::Keyboard::Y:		return GK_Y;
	case sf::Keyboard::U:		return GK_U;
	case sf::Keyboard::I:		return GK_I;
	case sf::Keyboard::O:		return GK_O;
	case sf::Keyboard::P:		return GK_P;
	case sf::Keyboard::Q:		return GK_Q;
	case sf::Keyboard::S:		return GK_S;
	case sf::Keyboard::D:		return GK_D;
	case sf::Keyboard::F:		return GK_F;
	case sf::Keyboard::G:		return GK_G;
	case sf::Keyboard::H:		return GK_H;
	case sf::Keyboard::J:		return GK_J;
	case sf::Keyboard::K:		return GK_K;
	case sf::Keyboard::L:		return GK_L;
	case sf::Keyboard::M:		return GK_M;
	case sf::Keyboard::W:		return GK_W;
	case sf::Keyboard::X:		return GK_X;
	case sf::Keyboard::C:		return GK_C;
	case sf::Keyboard::V:		return GK_V;
	case sf::Keyboard::B:		return GK_B;
	case sf::Keyboard::N:		return GK_N;
	case sf::Keyboard::Num0:	return GK_0;
	case sf::Keyboard::Num1:	return GK_1;
	case sf::Keyboard::Num2:	return GK_2;
	case sf::Keyboard::Num3:	return GK_3;
	case sf::Keyboard::Num4:	return GK_4;
	case sf::Keyboard::Num5:	return GK_5;
	case sf::Keyboard::Num6:	return GK_6;
	case sf::Keyboard::Num7:	return GK_7;
	case sf::Keyboard::Num8:	return GK_8;
	case sf::Keyboard::Num9:	return GK_9;
	}

	return GK_UNKNOWN;
}
