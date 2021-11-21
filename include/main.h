#pragma once

// Screen dimension constants
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

enum UpdateState {
    WarningScreen = 0,
    WelcomeScreen,
    CheckXeniumDetected,

    ReadXeniumOS,

    ReadBootloader,

    CalculateCRCs,
    MatchVersion,

    EraseBootloader,
    ProgramBootloader,

    //
    Done,
    DoneError,
    Exit
};
