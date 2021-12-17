#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "system.h"
#include "FileSystem.h"
#include "stringutil.h"



//**********************
int initFileSystem(void) {
    int res;
    int i;
    static char str[128];
    static F_SPACE space;

    
    // Single volume on physical disk
    res = cfs_mount(0, &Fatfs);
    
    if(res == FR_OK)
    {
        if(cfs_getfree("0:", &fre_clust, &fs) == FR_NO_FILESYSTEM) //create FS if not already present on disk
        {
          res = cfs_mkfs (0, 0, 2048);
        }
    }
/*    
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "Listing files on A: (DataFlash)");
    if (!f_findfirst("A:\\*.*", &find)) {
      do {
        sprintf(str,"%i", find.filesize);
        messageDebug(DBG_INFO, __MODULE__, __LINE__, "%s %s", find.attr&F_ATTR_DIR? "<DIR>":str, find.filename);
        OS_WAIT(100);
      } while (!f_findnext(&find));
    }
    f_getfreespace(0, &space);
    messageDebug(DBG_INFO, __MODULE__, __LINE__, "total space=%i KB, free space=%i KB", space.total/1024, space.free/1024);
*/   
    return ConvErr(res);
}

