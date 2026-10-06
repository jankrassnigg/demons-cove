InputAction
{
	string %Set{"Default"}
	string %Action{"move_forwards"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_w"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_posy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"move_backwards"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_s"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_negy"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"move_left"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_a"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_negx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"move_right"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_d"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_leftstick_posx"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"jump"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"keyboard_space"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_button_a"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_left_shoulder"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"shoot"}
	bool %TimeScale{false}
	Slot
	{
		string %Key{"mouse_button_0"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"keyboard_right_ctrl"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_right_trigger"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_right_shoulder"}
		float %Scale{1}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"turn_left"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"mouse_move_negx"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"keyboard_left"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_rightstick_negx"}
		float %Scale{3.5}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"turn_right"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"mouse_move_posx"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"keyboard_right"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_rightstick_posx"}
		float %Scale{3.5}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"turn_up"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"mouse_move_negy"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"keyboard_up"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_rightstick_posy"}
		float %Scale{2.5}
	}
}
InputAction
{
	string %Set{"Default"}
	string %Action{"turn_down"}
	bool %TimeScale{true}
	Slot
	{
		string %Key{"mouse_move_posy"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"keyboard_down"}
		float %Scale{1}
	}
	Slot
	{
		string %Key{"controller0_rightstick_negy"}
		float %Scale{2.5}
	}
}
