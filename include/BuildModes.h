#ifndef BUILD_MODES_H
#define BUILD_MODES_H

// Build Mode Definitions
// These are set via platformio.ini build_flags

// BUILD_MODE_ULTRA_LIGHT: API only, no HTML interface
// BUILD_MODE_LIGHT: Simple HTML interface with minimal assets
// BUILD_MODE_NORMAL: Full featured interface with branding (default)

// If no build mode is defined, default to NORMAL
#if !defined(BUILD_MODE_ULTRA_LIGHT) && !defined(BUILD_MODE_LIGHT) && !defined(BUILD_MODE_NORMAL)
    #define BUILD_MODE_NORMAL
#endif

// Define feature flags based on build mode
#ifdef BUILD_MODE_ULTRA_LIGHT
    // Ultra Light Mode: API only, no HTML interface
    #define ENABLE_API_ONLY
    #undef ENABLE_HTML_INTERFACE
    #undef ENABLE_BRANDING
    #undef ENABLE_EXTERNAL_RESOURCES
#endif

#ifdef BUILD_MODE_LIGHT
    // Light Mode: Simple HTML interface with minimal assets
    #define ENABLE_API_ONLY
    #define ENABLE_HTML_INTERFACE
    #undef ENABLE_BRANDING
    #undef ENABLE_EXTERNAL_RESOURCES
#endif

#ifdef BUILD_MODE_NORMAL
    // Normal Mode: Full featured interface with branding
    #define ENABLE_API_ONLY
    #define ENABLE_HTML_INTERFACE
    #define ENABLE_BRANDING
    #define ENABLE_EXTERNAL_RESOURCES
#endif

#endif // BUILD_MODES_H