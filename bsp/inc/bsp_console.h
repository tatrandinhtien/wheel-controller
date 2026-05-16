/**
 * @file bsp_console.h
 * @brief BSP Console API (rename from console.h)
 */

#ifndef BSP_CONSOLE_H_
#define BSP_CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

void BSP_Console_Init(void);
void BSP_Console_Print(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CONSOLE_H_ */
