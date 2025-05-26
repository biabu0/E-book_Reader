
#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdio.h>
#include<debug_manager.h>

#define FB_DEVICE_NAME "/dev/fb0"

#define COLOR_BACKGROUND   0xE7DBB5  /* 泛黄纸张 */
#define COLOR_FOREGROUND   0x514438  /* 褐色字体 */


#define APP_EMERG "<0>"
#define APP_ALERT "<1>"
#define APP_CRIT "<2>"
#define APP_ERR "<3>"
#define APP_WARNING "<4>"
#define APP_NOTICE "<5>"
#define APP_INFO "<6>"
#define APP_DEBUG "<7>"


//#define DBG_PRINTF(...)  
#define DBG_PRINTF DebugPrint
//#define DBG_PRINTF printf

#endif /* _CONFIG_H */
