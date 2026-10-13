#pragma once
// Documented layout: USB at top. Screen TL=P3, buzzer TR=P2,
// matrix BL=P4, encoder BR=P1. Ports rotate with board orientation.
static constexpr int UserButton = 45;
static constexpr int PixelPin = 21;
static constexpr int DisplayCS = 9, DisplayDC = 15, DisplayReset = 16;
static constexpr int EncoderButton = 4, EncoderA = 3, EncoderB = 2;
static constexpr int BuzzerPin = 6; // P2 IO1.
static constexpr int MatrixData = 17; // P4 IO1; P4 IO2 (18) is DOUT, leave input.
