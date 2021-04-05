#include "stdafx.h"
#include "InputSystem.h"
float InputSystem::s_mouseDelta = 0.0f;
bool InputSystem::s_isMouseMoveing = false;

Vector2 InputSystem::s_mouseLocation(0.0f, 0.0f);

std::unordered_map<int32_t, bool> InputSystem::s_keyActions;
std::unordered_map<int32_t, bool> InputSystem::s_mouseActions;
std::unordered_map<int32_t, KeyboardType> InputSystem::s_keyboardTypesMap;
//-----------------------------------------------------------------------------
InputSystem::InputSystem() noexcept
{
#if PLATFORM_WINDOWS
    s_keyboardTypesMap[0x00B] = KeyboardType::KEY_0;
    s_keyboardTypesMap[0x002] = KeyboardType::KEY_1;
    s_keyboardTypesMap[0x003] = KeyboardType::KEY_2;
    s_keyboardTypesMap[0x004] = KeyboardType::KEY_3;
    s_keyboardTypesMap[0x005] = KeyboardType::KEY_4;
    s_keyboardTypesMap[0x006] = KeyboardType::KEY_5;
    s_keyboardTypesMap[0x007] = KeyboardType::KEY_6;
    s_keyboardTypesMap[0x008] = KeyboardType::KEY_7;
    s_keyboardTypesMap[0x009] = KeyboardType::KEY_8;
    s_keyboardTypesMap[0x00A] = KeyboardType::KEY_9;
    s_keyboardTypesMap[0x01E] = KeyboardType::KEY_A;
    s_keyboardTypesMap[0x030] = KeyboardType::KEY_B;
    s_keyboardTypesMap[0x02E] = KeyboardType::KEY_C;
    s_keyboardTypesMap[0x020] = KeyboardType::KEY_D;
    s_keyboardTypesMap[0x012] = KeyboardType::KEY_E;
    s_keyboardTypesMap[0x021] = KeyboardType::KEY_F;
    s_keyboardTypesMap[0x022] = KeyboardType::KEY_G;
    s_keyboardTypesMap[0x023] = KeyboardType::KEY_H;
    s_keyboardTypesMap[0x017] = KeyboardType::KEY_I;
    s_keyboardTypesMap[0x024] = KeyboardType::KEY_J;
    s_keyboardTypesMap[0x025] = KeyboardType::KEY_K;
    s_keyboardTypesMap[0x026] = KeyboardType::KEY_L;
    s_keyboardTypesMap[0x032] = KeyboardType::KEY_M;
    s_keyboardTypesMap[0x031] = KeyboardType::KEY_N;
    s_keyboardTypesMap[0x018] = KeyboardType::KEY_O;
    s_keyboardTypesMap[0x019] = KeyboardType::KEY_P;
    s_keyboardTypesMap[0x010] = KeyboardType::KEY_Q;
    s_keyboardTypesMap[0x013] = KeyboardType::KEY_R;
    s_keyboardTypesMap[0x01F] = KeyboardType::KEY_S;
    s_keyboardTypesMap[0x014] = KeyboardType::KEY_T;
    s_keyboardTypesMap[0x016] = KeyboardType::KEY_U;
    s_keyboardTypesMap[0x02F] = KeyboardType::KEY_V;
    s_keyboardTypesMap[0x011] = KeyboardType::KEY_W;
    s_keyboardTypesMap[0x02D] = KeyboardType::KEY_X;
    s_keyboardTypesMap[0x015] = KeyboardType::KEY_Y;
    s_keyboardTypesMap[0x02C] = KeyboardType::KEY_Z;

    s_keyboardTypesMap[0x028] = KeyboardType::KEY_APOSTROPHE;
    s_keyboardTypesMap[0x02B] = KeyboardType::KEY_BACKSLASH;
    s_keyboardTypesMap[0x033] = KeyboardType::KEY_COMMA;
    s_keyboardTypesMap[0x00D] = KeyboardType::KEY_EQUAL;
    s_keyboardTypesMap[0x029] = KeyboardType::KEY_GRAVE_ACCENT;
    s_keyboardTypesMap[0x01A] = KeyboardType::KEY_LEFT_BRACKET;
    s_keyboardTypesMap[0x00C] = KeyboardType::KEY_MINUS;
    s_keyboardTypesMap[0x034] = KeyboardType::KEY_PERIOD;
    s_keyboardTypesMap[0x01B] = KeyboardType::KEY_RIGHT_BRACKET;
    s_keyboardTypesMap[0x027] = KeyboardType::KEY_SEMICOLON;
    s_keyboardTypesMap[0x035] = KeyboardType::KEY_SLASH;
    s_keyboardTypesMap[0x056] = KeyboardType::KEY_WORLD_2;

    s_keyboardTypesMap[0x00E] = KeyboardType::KEY_BACKSPACE;
    s_keyboardTypesMap[0x153] = KeyboardType::KEY_DELETE;
    s_keyboardTypesMap[0x14F] = KeyboardType::KEY_END;
    s_keyboardTypesMap[0x01C] = KeyboardType::KEY_ENTER;
    s_keyboardTypesMap[0x001] = KeyboardType::KEY_ESCAPE;
    s_keyboardTypesMap[0x147] = KeyboardType::KEY_HOME;
    s_keyboardTypesMap[0x152] = KeyboardType::KEY_INSERT;
    s_keyboardTypesMap[0x15D] = KeyboardType::KEY_MENU;
    s_keyboardTypesMap[0x151] = KeyboardType::KEY_PAGE_DOWN;
    s_keyboardTypesMap[0x149] = KeyboardType::KEY_PAGE_UP;
    s_keyboardTypesMap[0x045] = KeyboardType::KEY_PAUSE;
    s_keyboardTypesMap[0x146] = KeyboardType::KEY_PAUSE;
    s_keyboardTypesMap[0x039] = KeyboardType::KEY_SPACE;
    s_keyboardTypesMap[0x00F] = KeyboardType::KEY_TAB;
    s_keyboardTypesMap[0x03A] = KeyboardType::KEY_CAPS_LOCK;
    s_keyboardTypesMap[0x145] = KeyboardType::KEY_NUM_LOCK;
    s_keyboardTypesMap[0x046] = KeyboardType::KEY_SCROLL_LOCK;
    s_keyboardTypesMap[0x03B] = KeyboardType::KEY_F1;
    s_keyboardTypesMap[0x03C] = KeyboardType::KEY_F2;
    s_keyboardTypesMap[0x03D] = KeyboardType::KEY_F3;
    s_keyboardTypesMap[0x03E] = KeyboardType::KEY_F4;
    s_keyboardTypesMap[0x03F] = KeyboardType::KEY_F5;
    s_keyboardTypesMap[0x040] = KeyboardType::KEY_F6;
    s_keyboardTypesMap[0x041] = KeyboardType::KEY_F7;
    s_keyboardTypesMap[0x042] = KeyboardType::KEY_F8;
    s_keyboardTypesMap[0x043] = KeyboardType::KEY_F9;
    s_keyboardTypesMap[0x044] = KeyboardType::KEY_F10;
    s_keyboardTypesMap[0x057] = KeyboardType::KEY_F11;
    s_keyboardTypesMap[0x058] = KeyboardType::KEY_F12;
    s_keyboardTypesMap[0x064] = KeyboardType::KEY_F13;
    s_keyboardTypesMap[0x065] = KeyboardType::KEY_F14;
    s_keyboardTypesMap[0x066] = KeyboardType::KEY_F15;
    s_keyboardTypesMap[0x067] = KeyboardType::KEY_F16;
    s_keyboardTypesMap[0x068] = KeyboardType::KEY_F17;
    s_keyboardTypesMap[0x069] = KeyboardType::KEY_F18;
    s_keyboardTypesMap[0x06A] = KeyboardType::KEY_F19;
    s_keyboardTypesMap[0x06B] = KeyboardType::KEY_F20;
    s_keyboardTypesMap[0x06C] = KeyboardType::KEY_F21;
    s_keyboardTypesMap[0x06D] = KeyboardType::KEY_F22;
    s_keyboardTypesMap[0x06E] = KeyboardType::KEY_F23;
    s_keyboardTypesMap[0x076] = KeyboardType::KEY_F24;
    s_keyboardTypesMap[0x038] = KeyboardType::KEY_LEFT_ALT;
    s_keyboardTypesMap[0x01D] = KeyboardType::KEY_LEFT_CONTROL;
    s_keyboardTypesMap[0x02A] = KeyboardType::KEY_LEFT_SHIFT;
    s_keyboardTypesMap[0x15B] = KeyboardType::KEY_LEFT_SUPER;
    s_keyboardTypesMap[0x137] = KeyboardType::KEY_PRINT_SCREEN;
    s_keyboardTypesMap[0x138] = KeyboardType::KEY_RIGHT_ALT;
    s_keyboardTypesMap[0x11D] = KeyboardType::KEY_RIGHT_CONTROL;
    s_keyboardTypesMap[0x036] = KeyboardType::KEY_RIGHT_SHIFT;
    s_keyboardTypesMap[0x15C] = KeyboardType::KEY_RIGHT_SUPER;
    s_keyboardTypesMap[0x150] = KeyboardType::KEY_DOWN;
    s_keyboardTypesMap[0x14B] = KeyboardType::KEY_LEFT;
    s_keyboardTypesMap[0x14D] = KeyboardType::KEY_RIGHT;
    s_keyboardTypesMap[0x148] = KeyboardType::KEY_UP;

    s_keyboardTypesMap[0x052] = KeyboardType::KEY_KP_0;
    s_keyboardTypesMap[0x04F] = KeyboardType::KEY_KP_1;
    s_keyboardTypesMap[0x050] = KeyboardType::KEY_KP_2;
    s_keyboardTypesMap[0x051] = KeyboardType::KEY_KP_3;
    s_keyboardTypesMap[0x04B] = KeyboardType::KEY_KP_4;
    s_keyboardTypesMap[0x04C] = KeyboardType::KEY_KP_5;
    s_keyboardTypesMap[0x04D] = KeyboardType::KEY_KP_6;
    s_keyboardTypesMap[0x047] = KeyboardType::KEY_KP_7;
    s_keyboardTypesMap[0x048] = KeyboardType::KEY_KP_8;
    s_keyboardTypesMap[0x049] = KeyboardType::KEY_KP_9;
    s_keyboardTypesMap[0x04E] = KeyboardType::KEY_KP_ADD;
    s_keyboardTypesMap[0x053] = KeyboardType::KEY_KP_DECIMAL;
    s_keyboardTypesMap[0x135] = KeyboardType::KEY_KP_DIVIDE;
    s_keyboardTypesMap[0x11C] = KeyboardType::KEY_KP_ENTER;
    s_keyboardTypesMap[0x037] = KeyboardType::KEY_KP_MULTIPLY;
    s_keyboardTypesMap[0x04A] = KeyboardType::KEY_KP_SUBTRACT;
#endif // PLATFORM_WINDOWS
}


//-----------------------------------------------------------------------------
//void InputSystem::KeyDown(unsigned int input) noexcept
//{
//	m_keys[input] = true;
//}
////-----------------------------------------------------------------------------
//void InputSystem::KeyUp(unsigned int input) noexcept
//{
//	m_keys[input] = false;
//}
////-----------------------------------------------------------------------------
//bool InputSystem::IsKeyDown(unsigned int key) noexcept
//{
//	return m_keys[key];
//}
////-----------------------------------------------------------------------------
void InputSystem::onKeyDown(KeyboardType key)
{
    s_keyActions[(int32_t)key] = true;
}

void InputSystem::onKeyUp(KeyboardType key)
{
    s_keyActions[(int32_t)key] = false;
}

void InputSystem::onMouseDown(MouseType type, const Vector2& pos)
{
    s_mouseActions[(int32_t)type] = true;
    s_mouseLocation.Set(pos.x, pos.y);
}

void InputSystem::onMouseUp(MouseType type, const Vector2& pos)
{
    s_mouseActions[(int32_t)type] = false;
    s_mouseLocation.Set(pos.x, pos.y);
}

void InputSystem::onMouseMove(const Vector2& pos)
{
    s_isMouseMoveing = true;
    s_mouseLocation.Set(pos.x, pos.y);
}

void InputSystem::onMouseWheel(const float delta, const Vector2& pos)
{
    s_mouseDelta = delta;
    s_mouseLocation.Set(pos.x, pos.y);
}