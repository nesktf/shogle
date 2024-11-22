#pragma once

#define NTF_ENABLE 1
#define NTF_DISABLE 0


#ifndef SHOGLE_USE_CUSTOM_ASSERT
#define SHOGLE_USE_CUSTOM_ASSERT NTF_ENABLE
#endif

#ifndef SHOGLE_ENABLE_INTERNAL_LOGS
#define SHOGLE_ENABLE_INTERNAL_LOGS NTF_ENABLE
#endif

#ifndef SHOGLE_GL_RAII_UNLOAD
#define SHOGLE_GL_RAII_UNLOAD NTF_ENABLE
#endif

// Might replace glm at some point (?)
#ifndef SHOGLE_USE_GLM
#define SHOGLE_USE_GLM NTF_ENABLE
#endif