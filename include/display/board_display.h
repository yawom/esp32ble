#pragma once

// Include the appropriate display driver based on the board type
#if defined(LILYGO_T_DISPLAY_S3)
    #include "display/LGFX_Parallel_ST7789.h"
#else
    #error "Unsupported board type. Please define LILYGO_T_DISPLAY_S3."
#endif
