/*
	DS5_Output.h is part of

    DualSenseWindows
	https://github.com/Ohjurot/DualSense-Windows

    DualSensitive
    https://github.com/tpetsas/dualsensitive

	Contributors of this file:
	11.2020 Ludwig Füchsl
    05.2025 Thanasis Petsas

	Licensed under the MIT License (To be found in repository root directory)
*/

#include "DS5_Output.h"
#include "logger.h"

void __DS5W::Output::createHidOutputBuffer(unsigned char* hidOutBuffer, DS5W::DS5OutputState* ptrOutputState) {
	// Feature mask
	hidOutBuffer[0x00] = 0xFF;
	hidOutBuffer[0x01] = 0xF7;

	// Rumbel motors
	hidOutBuffer[0x02] = ptrOutputState->rightRumble;
	hidOutBuffer[0x03] = ptrOutputState->leftRumble;

	// Mic led
	hidOutBuffer[0x08] = (unsigned char)ptrOutputState->microphoneLed;

	// Player led
	hidOutBuffer[0x2B] = ptrOutputState->playerLeds.bitmask;
	if (ptrOutputState->playerLeds.playerLedFade) {
		hidOutBuffer[0x2B] &= ~(0x20);
	}
	else {
		hidOutBuffer[0x2B] |= 0x20;
	}

	// Player led brightness
	hidOutBuffer[0x26] = 0x03;
	hidOutBuffer[0x29] = ptrOutputState->disableLeds ? 0x01 : 0x2;
	hidOutBuffer[0x2A] = ptrOutputState->playerLeds.brightness;

	// Lightbar
	hidOutBuffer[0x2C] = ptrOutputState->lightbar.r;
	hidOutBuffer[0x2D] = ptrOutputState->lightbar.g;
	hidOutBuffer[0x2E] = ptrOutputState->lightbar.b;

	// Adaptive Triggers
    // TODO:
    // prioritize TriggerSetting parsing over TriggerEffect here!
    if (ptrOutputState->triggerSettingEnabled) {
        DEBUG_PRINT("triggerSettingsEnabled found!");
        processTriggerSetting(&ptrOutputState->leftTriggerSetting, &hidOutBuffer[0x15]);
        processTriggerSetting(&ptrOutputState->rightTriggerSetting, &hidOutBuffer[0x0A]);
    } else {
        processTrigger(&ptrOutputState->leftTriggerEffect, &hidOutBuffer[0x15]);
        processTrigger(&ptrOutputState->rightTriggerEffect, &hidOutBuffer[0x0A]);
    }
}

void __DS5W::Output::processTriggerSetting(DS5W::TriggerSetting *setting, unsigned char *buffer) {
    setTriggerProfile(buffer, setting->profile, setting->extras);
}

void __DS5W::Output::processTrigger(DS5W::TriggerEffect* ptrEffect, unsigned char* buffer) {
	// Switch on effect
	switch (ptrEffect->effectType) {
		// Continious
		case DS5W::TriggerEffectType::ContinuousResitance:
			// Mode
			buffer[0x00] = 0x01;
			// Parameters
			buffer[0x01] = ptrEffect->Continuous.startPosition;
			buffer[0x02] = ptrEffect->Continuous.force;

			break;

		// Section
        // Pulse
		case DS5W::TriggerEffectType::SectionResitance:
			// Mode
			buffer[0x00] = 0x02;
			// Parameters
			buffer[0x01] = ptrEffect->Continuous.startPosition;
			buffer[0x02] = ptrEffect->Continuous.force;

			break;

		// EffectEx
		case DS5W::TriggerEffectType::EffectEx:
			// Mode
			buffer[0x00] = 0x02 | 0x20 | 0x04;
			// Parameters
			buffer[0x01] = 0xFF - ptrEffect->EffectEx.startPosition;
			// Keep flag
			if (ptrEffect->EffectEx.keepEffect) {
				buffer[0x02] = 0x02;
			}
			// Forces
			buffer[0x04] = ptrEffect->EffectEx.beginForce;
			buffer[0x05] = ptrEffect->EffectEx.middleForce;
			buffer[0x06] = ptrEffect->EffectEx.endForce;
			// Frequency
			buffer[0x09] = max(1, ptrEffect->EffectEx.frequency / 2);

			break;

		// Calibrate
		case DS5W::TriggerEffectType::Calibrate:
			// Mode 
			buffer[0x00] = 0xFC;

			break;
        // Custom Profiles
        // GameCube
		case DS5W::TriggerEffectType::GameCube:
			// Mode Pulse
			buffer[0] = 0x02;
			// Parameters
			buffer[1] = 144;
			buffer[2] = 160;
			buffer[3] = 255;
            // the rest 3 - 10 should be 0
            for (int i=4; i<=10; i++) {
			    buffer[i] = 0;
            }
			break;
		case DS5W::TriggerEffectType::Hardest:
			// Mode Pulse
			buffer[0] = 0x02;
			// Parameters
			buffer[1] = 0;
			buffer[2] = 255;
			buffer[3] = 255;
			buffer[4] = 255;
			buffer[5] = 255;
			buffer[6] = 255;
			buffer[7] = 255;
            // the rest 3 - 10 should be 0
            for (int i=8; i<=10; i++) {
			    buffer[i] = 0;
            }
			break;
		// No resistance / default
		case DS5W::TriggerEffectType::NoResitance:
			__fallthrough;
		default:
			// All zero
			buffer[0x00] = 0x00;
			buffer[0x01] = 0x00;
			buffer[0x02] = 0x00;

			break;
	}		
}
