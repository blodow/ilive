#pragma once

namespace amp {
    
int getVol();
bool getPower();
bool getMute();
int getInput();

// updateState();

bool setVol(int vol);
bool toggleMute();
bool turnOff();
bool turnOn();

}

