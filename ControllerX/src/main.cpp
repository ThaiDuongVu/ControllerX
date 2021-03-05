#include <iostream>
#include <string>
#include <windows.h>
#include <xinput.h>
#include <winuser.h>

#define SHORT_RANGE 32768
#define TRIGGER_RANGE 255

/*----- Values can be changed based on preferences -----*/
double analog_stick_deadzone = 0.15f;
double mouse_move_sensitivity = 15.0f;
double mouse_scroll_sensitivity = 80.0f;
double trigger_sensitivity = 0.25f;
/*------------------------------------------------------*/

/*----- More keys can be added/removed based on preferences -----*/
/// <summary>
/// Keyboard buttons
/// </summary>
enum class KEYBOARD_BUTTON
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,
	WINDOWS = VK_LWIN,
	TAB = VK_TAB,
	ENTER = VK_RETURN,
	CTRL = VK_CONTROL,
	ESC = VK_ESCAPE,
	ALT = VK_MENU,
	SPACE = VK_SPACE,
	VOL_UP = VK_VOLUME_UP,
	VOL_DOWN = VK_VOLUME_DOWN,
	MEDIA_PLAY_PAUSE = VK_MEDIA_PLAY_PAUSE,
	MEDIA_NEXT = VK_MEDIA_NEXT_TRACK,
	MEDIA_PREVIOUS = VK_MEDIA_PREV_TRACK,
};
/*---------------------------------------------------------------*/

/// <summary>
/// Gamepad buttons.
/// </summary>
enum class GAMEPAD_BUTTON
{
	UP = 1,
	DOWN = 2,
	LEFT = 4,
	RIGHT = 8,
	ESCAPE = 16,
	START = 32,
	LEFT_STICK = 64,
	RIGHT_STICK = 128,
	LEFT_SHOULDER = 256,
	RIGHT_SHOULDER = 512,
	A = 4096,
	B = 8192,
	X = 16384,
	Y = 32768,
};

/// <summary>
/// Simulate keyboard buttons with gamepad buttons.
/// </summary>
/// <param name="button">Button to simulate</param>
/// <param name="button_buffer">Current button buffer</param>
void simulate_keyboard(GAMEPAD_BUTTON button, DWORD& button_buffer);

/// <summary>
/// Simulate mouse vertical & horizontal movement with right analog stick.
/// </summary>
/// <param name="x">Delta x to move</param>
/// <param name="y">Delta y to move</param>
void simulate_mouse_movement(double x, double y);
/// <summary>
/// Simulate mouse vertical & horizontal scroll with left analog stick.
/// </summary>
/// <param name="x">Delta x to scroll</param>
/// <param name="y">Delta y to scroll</param>
void simulate_mouse_scroll(double x, double y);

/// <summary>
/// Simulate left mouse click with gamepad trigger.
/// </summary>
/// <param name="trigger">Trigger value to check</param>
/// <param name="right_trigger_buffer">Current right trigger buffer</param>
void simulate_left_mouse(DWORD trigger, DWORD& right_trigger_buffer);
/// <summary>
/// Simulate right mouse click with gamepad trigger.
/// </summary>
/// <param name="trigger"></param>
/// <param name="left_trigger_buffer"></param>
void simulate_right_mouse(DWORD trigger, DWORD& left_trigger_buffer);

/// <summary>
/// Print current keymap to console.
/// </summary>
void print_keymap();

/// <summary>
/// Print current specification.
/// </summary>
void print_spec();

/// <summary>
/// Print all commands available in command mode.
/// </summary>
void command_help();

/// <summary>
/// Process a user input command.
/// </summary>
/// <param name="command">User input command</param>
void process_command(std::string command);

int main()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
	std::cout << "\n  ControllerX up and running...  " << std::endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << "  Press F1 to enter command mode.  \n" << std::endl;

	// Current input state
	XINPUT_STATE input_state;
	ZeroMemory(&input_state, sizeof(XINPUT_STATE));

	// Current keyboard state
	XINPUT_KEYSTROKE keystroke;
	ZeroMemory(&keystroke, sizeof(keystroke));

	// Buffer for gamepad buttons
	DWORD button_buffer = 0;
	// Buffer for gamepad right trigger
	DWORD right_trigger_buffer = 2;
	// Buffer for gamepad left trigger
	DWORD left_trigger_buffer = 2;

	bool is_command_mode = false;
	std::string command;

	// Main program loop
	while (true)
	{
		// Press F1 to enter command mode
		if (GetKeyState(VK_F1) & 0x8000)
		{
			// Clear the console and give feedback
			system("cls");

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			std::cout << "\n  Waiting for command..." << std::endl;
			std::cout << "  Type \"help\" for the list of available commands.  \n" << std::endl;

			std::cout << "> ";
			// Enter command mode
			is_command_mode = true;
		}

		// If user is in command mode
		if (is_command_mode)
		{
			// Get input command
			std::cin >> command;
			system("cls");
			// Process given command
			process_command(command);

			// Exit command mode
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
			std::cout << "  Command mode exitted.  " << std::endl;
			is_command_mode = false;

			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			std::cout << "  Press F1 to enter command mode again.  \n" << std::endl;
		}

		// If controller is connected then simulate input
		if (XInputGetState(0, &input_state) == ERROR_SUCCESS)
		{
			simulate_keyboard((GAMEPAD_BUTTON)input_state.Gamepad.wButtons, button_buffer);

			simulate_mouse_movement(input_state.Gamepad.sThumbRX, input_state.Gamepad.sThumbRY);
			simulate_mouse_scroll(input_state.Gamepad.sThumbLX, input_state.Gamepad.sThumbLY);

			simulate_left_mouse(input_state.Gamepad.bRightTrigger, right_trigger_buffer);
			simulate_right_mouse(input_state.Gamepad.bLeftTrigger, left_trigger_buffer);

			// Delay by 1 milisecond
			Sleep(1);
		}
		// If controller is not connected then log an error then quit.
		else
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
			std::cout << "\n  Error: Controller not connected  \n" << std::endl;
			std::cin.get();
			break;
		}
	}

	return 0;
}

void simulate_keyboard(GAMEPAD_BUTTON button, DWORD& button_buffer)
{
	// If input button is the same as button buffer then do nothing
	if ((DWORD)button == button_buffer) return;

	// Key up & down input to simulate
	INPUT inputs[2];
	ZeroMemory(inputs, sizeof(inputs));

	// Set input type to keyboard
	inputs[0].type = INPUT_KEYBOARD;
	inputs[1].type = INPUT_KEYBOARD;

	// Set input keyup flag
	inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

	// Map input button to keyboard action
	/*----- Gamepad buttons can be remapped based on preferences -----*/
	switch (button)
	{
	case GAMEPAD_BUTTON::UP:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::UP;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::UP;
		break;

	case GAMEPAD_BUTTON::DOWN:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::DOWN;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::DOWN;
		break;

	case GAMEPAD_BUTTON::LEFT:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::LEFT;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::LEFT;
		break;

	case GAMEPAD_BUTTON::RIGHT:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::RIGHT;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::RIGHT;
		break;

	case GAMEPAD_BUTTON::ESCAPE:
		quick_exit(0);
		break;

	case GAMEPAD_BUTTON::START:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::WINDOWS;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::WINDOWS;
		break;

	case GAMEPAD_BUTTON::LEFT_STICK:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_PLAY_PAUSE;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_PLAY_PAUSE;
		break;

	case GAMEPAD_BUTTON::RIGHT_STICK:
		inputs[0].type = INPUT_MOUSE;
		inputs[1].type = INPUT_MOUSE;

		inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
		inputs[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
		break;

	case GAMEPAD_BUTTON::LEFT_SHOULDER:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_PREVIOUS;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_PREVIOUS;
		break;

	case GAMEPAD_BUTTON::RIGHT_SHOULDER:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_NEXT;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::MEDIA_NEXT;
		break;

	case GAMEPAD_BUTTON::A:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::VOL_DOWN;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::VOL_DOWN;
		break;

	case GAMEPAD_BUTTON::B:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::ESC;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::ESC;
		break;

	case GAMEPAD_BUTTON::X:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::ALT;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::ALT;
		break;

	case GAMEPAD_BUTTON::Y:
		inputs[0].ki.wVk = (WORD)KEYBOARD_BUTTON::VOL_UP;
		inputs[1].ki.wVk = (WORD)KEYBOARD_BUTTON::VOL_UP;
		break;

	default:
		break;
	}
	/*----------------------------------------------------------------*/

	// Assign button to button buffer
	button_buffer = (DWORD)button;

	// Send input to Windows
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void simulate_mouse_movement(double x, double y)
{
	// Mouse point
	POINT point;
	// Get current mouse position and store in point
	GetCursorPos(&point);

	// Store current mouse position in a pair of doubles
	// Current mouse x position
	double current_x = point.x;
	// Current mouse y position
	double current_y = point.y;

	// Check whether analog stick movement is out of deadzone
	// X axis deadzone check
	bool out_deadzone_x = abs(x / SHORT_RANGE) > analog_stick_deadzone;
	// Y axis deadzone check
	bool out_deadzone_y = abs(y / SHORT_RANGE) > analog_stick_deadzone;

	// If deadzone check complete then simulate movement
	// Simulate mouse x movement
	if (out_deadzone_x) current_x += x / SHORT_RANGE * (double)mouse_move_sensitivity;
	// Simulate mouse y movement
	if (out_deadzone_y) current_y -= y / SHORT_RANGE * (double)mouse_move_sensitivity;

	// Update cursor position on screen
	if (out_deadzone_x || out_deadzone_y) SetCursorPos((int)current_x, (int)current_y);
}

void simulate_mouse_scroll(double x, double y)
{
	// Vertical & horizontal input scroll to simulate
	INPUT inputs[2];
	ZeroMemory(inputs, sizeof(inputs));

	// Check whether analog stick movement is out of deadzone
	// X axis deadzone check
	bool out_deadzone_x = abs(x / SHORT_RANGE) > analog_stick_deadzone;
	// Y axis deadzone check
	bool out_deadzone_y = abs(y / SHORT_RANGE) > analog_stick_deadzone;

	// If x axis deadzone check complete then simulate vertical scroll
	if (out_deadzone_x)
	{
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.mouseData = (DWORD)(x / SHORT_RANGE * (double)mouse_scroll_sensitivity);
		inputs[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
	}
	// If y axis deadzone check complete then simulate horizontal scroll
	if (out_deadzone_y)
	{
		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.mouseData = (DWORD)(y / SHORT_RANGE * (double)mouse_scroll_sensitivity);
		inputs[1].mi.dwFlags = MOUSEEVENTF_HWHEEL;
	}

	// Send input to Windows
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void simulate_left_mouse(DWORD trigger, DWORD& right_trigger_buffer)
{
	// Left mouse click input to simulate
	INPUT inputs[2];
	ZeroMemory(inputs, sizeof(inputs));

	// If current trigger value exceeds trigger threshold then simulate a mouse click
	if ((double)(trigger / TRIGGER_RANGE) >= trigger_sensitivity)
	{
		// If mouse is already down (trigger buffer is set) then do nothing
		if (right_trigger_buffer == 1) return;

		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

		right_trigger_buffer = 1;
	}
	// If current trigger value is less than trigger threshold then simulate a mouse release
	else
	{
		// If mouse is already down (trigger buffer is set) then do nothing
		if (right_trigger_buffer == 2) return;

		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

		right_trigger_buffer = 2;
	}

	// Send input to Windows
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void simulate_right_mouse(DWORD trigger, DWORD& left_trigger_buffer)
{
	// Right mouse click input to simulate
	INPUT inputs[2];
	ZeroMemory(inputs, sizeof(inputs));

	// If current trigger value exceeds trigger threshold then simulate a mouse click
	if ((double)(trigger / TRIGGER_RANGE) >= trigger_sensitivity)
	{
		// If mouse is already down (trigger buffer is set) then do nothing
		if (left_trigger_buffer == 1) return;

		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

		left_trigger_buffer = 1;
	}
	// If current trigger value is less than trigger threshold then simulate a mouse release
	else
	{
		// If mouse is already down (trigger buffer is set) then do nothing
		if (left_trigger_buffer == 2) return;

		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;

		left_trigger_buffer = 2;
	}

	// Send input to Windows
	SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

void print_keymap()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
	std::cout << "   ---------------- Current Keymaps -----------------   " << std::endl;
	std::cout << "  |          Left Stick   ---   Mouse Movement      |  " << std::endl;
	std::cout << "  |         Right Stick   ---   Mouse Scroll        |  " << std::endl;
	std::cout << "  |        Left Trigger   ---   Right Mouse Click   |  " << std::endl;
	std::cout << "  |       Right Trigger   ---   Left Mouse Click    |  " << std::endl;
	std::cout << "  |       Left Shoulder   ---   Media Previous      |  " << std::endl;
	std::cout << "  |      Right Shoulder   ---   Media Next          |  " << std::endl;
	std::cout << "  |   Left Stick Button   ---   Media Play/Pause    |  " << std::endl;
	std::cout << "  |  Right Stick Button   ---   Middle Mouse Click  |  " << std::endl;
	std::cout << "  |            D-Pad Up   ---   Up Arrow Key        |  " << std::endl;
	std::cout << "  |          D-Pad Down   ---   Down Arrow Key      |  " << std::endl;
	std::cout << "  |          D-Pad Left   ---   Left Arrow Key      |  " << std::endl;
	std::cout << "  |         D-Pad Right   ---   Right Arrow Key     |  " << std::endl;
	std::cout << "  |                   A   ---   Volume Down         |  " << std::endl;
	std::cout << "  |                   B   ---   Esc                 |  " << std::endl;
	std::cout << "  |                   X   ---   Alt                 |  " << std::endl;
	std::cout << "  |                   Y   ---   Volume Up           |  " << std::endl;
	std::cout << "  |                Back   ---   Windows Start Menu  |  " << std::endl;
	std::cout << "  |               Start   ---   Exit Controllerx    |  " << std::endl;
	std::cout << "   -------------------------------------------------   " << std::endl;
}

void print_spec()
{
	std::cout << "  Analog Stick Deadzone: " << analog_stick_deadzone << std::endl;
	std::cout << "  Mouse Move Sensitivity: " << mouse_move_sensitivity << std::endl;
	std::cout << "  Mouse Scroll Sensitivity: " << mouse_scroll_sensitivity << std::endl;
}

void command_help()
{
	std::cout << "  Available commands:" << std::endl;
	std::cout << "> exit_command: Exit command mode.  " << std::endl;
	std::cout << "> print_keymap: Print current controller to mouse/keyboard map.  " << std::endl;
	std::cout << "> print_spec: Print current controller specification." << std::endl;
	std::cout << "> exit: Exit ControllerX.  " << std::endl;
}

void process_command(std::string command)
{
	std::cout << "" << std::endl;

	if (command._Equal("help"))
	{
		command_help();
	}
	else if (command._Equal("print_keymap"))
	{
		print_keymap();
	}
	else if (command._Equal("print_spec"))
	{
		print_spec();
	}
	else if (command._Equal("exit_command"))
	{
		return;
	}
	else if (command._Equal("exit"))
	{
		quick_exit(0);
	}
	else
	{
		std::cout << "  Command not found.  " << std::endl;
	}

	std::cout << "" << std::endl;
}