#include "System.h"
#include "stdio.h"
#include "PMONDef.h"
#include "fw.h"
#include "Debug.h"
#include "hcc/src/api/api_tiny.h"
#include "gsm.h"
#include "wdt.h"
#include "DESP.h"

//#define MAX_GENERIC_CB 10 // Max number of functions that can be registered for generic read/write
#define MINIMUM(x,y)	((x)<(y)?(x):(y))
extern int FWVersion;

static U8 Blocked = 1;

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
int getPlatform(void) {
// Lommy platform 5 (LGCV1), 6 (LGCV2), 7 (BLACKBIRD), 8 (ROBIN)
#if defined _DRAGONFLY_
  return 9;
#endif 
}

	extern void restartTimeoutBootloader(void);

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int handlePMON(pmonFrames *pmon) {
  int rc,reqnum,actnum;
  static F_FIND find;
  static int init=FALSE;
  static F_FILE *file;
  static int size;
  static int bytestogo;
  
  static char name[256];
  static int Readfileclosed=TRUE;
  static int Writefileclosed=TRUE;

  if(Blocked)
    return 0;
    
  if (!init) {
    init=TRUE;
 //   rc=f_enterFS();
  }
  //   hexDumpDebug(DBG_INFO, __MODULE__, __LINE__, "PMON data", (void*)&pmon->reqCmd,64);
  
	restartTimeoutBootloader();
  
  
  switch (pmon->reqCmd.cmd) {
    case PMON_VERSION:
      pmon->respVersion.version=FWVersion;
      pmon->respVersion.platform=getPlatform();
      pmon->respVersion.bootmode=1;
      return sizeof(resp_pmonVersion);

    case PMON_DEVICEID:
        getDeviceIDDESP(pmon->respDeviceID.deviceid);
        return sizeof(resp_pmonDeviceID);

    case PMON_IMEI:
        initGSM();
        powerGSM(1, "");
        OS_WAIT(1000);
        pmon->respImei.rc = getIMEIGSM(pmon->respImei.str);
        powerGSM(0, "");
        return sizeof(resp_pmonIMEI);
    
    //-----------------------------------------------------------------------------
    // Filesystem functions
    //-----------------------------------------------------------------------------
    case PMON_FS_FINDDIR:
      {
        if (pmon->reqFSFindDir.first) { 
          strcpy(name, pmon->reqFSFindDir.data);
          rc=f_findfirst(name, &find);
       

          if (rc!=F_ERR_NOTFOUND && rc!=F_NO_ERROR) {
            pmon->respFSFindDir.more=-1; // Indicate error
          }
          
          if (rc==F_NO_ERROR)
            pmon->respFSFindDir.more=1;
          
          if (rc==F_ERR_NOTFOUND) {
            pmon->respFSFindDir.more=0;
            strcpy((char*)pmon->respFSFindDir.data, "");
          } else {
            // Truncate really long names...
         //   if (strlen(find.filename)>PMON_FRAMESIZE-5-1) 
         //     find.filename[PMON_FRAMESIZE-5-1]=0;

            strcpy(pmon->respFSFindDir.data, find.filename);
          }
          pmon->respFSFindDir.filesize=find.filesize;
          if (find.attr==F_ATTR_DIR) 
            pmon->respFSFindDir.filesize=-1; // indicate subdir
          return sizeof(resp_pmonFSFindDir);

        } else {
        	rc=f_findnext(&find); // Ask for next entry

          if (rc==F_NO_ERROR) {
            pmon->respFSFindDir.filesize=find.filesize;
            if (find.attr==F_ATTR_DIR) 
              pmon->respFSFindDir.filesize=-1; // indicate subdir
            // Truncate really long names...
           // if (strlen(find.filename)>PMON_FRAMESIZE-5-1) 
           //   find.filename[PMON_FRAMESIZE-5-1]=0;

            strcpy((char*)pmon->respFSFindDir.data, find.filename);
            pmon->respFSFindDir.more=1;
          }
  
        	if (rc==F_ERR_NOTFOUND) {
            pmon->respFSFindDir.more=0; // Indicate no more
            //f_releaseFS(fn_gettaskID());
          }
          return sizeof(resp_pmonFSFindDir);
        }
      }

    case PMON_FS_CHDIR:
      {
        rc=f_chdir((char *)pmon->reqFSCHDir.data);
        if (rc!=F_NO_ERROR && rc!=F_ERR_NOTFOUND) {
          pmon->respFSCHDir.rc=2;
        }
        if (rc==F_ERR_NOTFOUND) {
          pmon->respFSCHDir.rc=3;
        }
        return sizeof(resp_pmonFSCHDir);
      }

    case PMON_FS_MKDIR:
      {
        rc=f_mkdir((char *)pmon->reqFSMKDir.data);
        if (rc!=F_NO_ERROR) {
          pmon->respFSMKDir.rc=1;
        }
        return sizeof(resp_pmonFSMKDir);
      }

    case PMON_FS_RMDIR:
      {
        rc=f_rmdir((char *)pmon->reqFSRMDir.data);
        if (rc!=F_NO_ERROR) {
          pmon->respFSRMDir.rc=1;
        }
        return sizeof(resp_pmonFSRMDir);
      }

    case PMON_FS_DELETE:
      {
        rc=f_delete((char *)pmon->reqFSDelete.data);
        if (rc!=F_NO_ERROR && rc!=F_ERR_NOTFOUND) {
          pmon->respFSDelete.rc=1;
        }
        if (rc==F_ERR_NOTFOUND) {
          pmon->respFSDelete.rc=2;
        }
        return sizeof(resp_pmonFSDelete);
      }


    case PMON_FS_READ:
  		switch (pmon->reqFSRead.state) {
        // Open file
        case 0:
          {
          int filesize;
          if (Readfileclosed==FALSE)
            f_close(file);      // DO NOT CALL f_close() on a already closed file !!!!!!!!!!!
          size=0;
          // Get the length of the file..
          rc=f_filelength((char*)pmon->reqFSRead.data);
          if (rc==F_ERR_INVALID) {
            pmon->respFSRead.more=-1;
            return sizeof(resp_pmonFSRead);
          } 
          filesize=rc;      
          file=f_open((char*)pmon->reqFSRead.data,"r");
        	if (!file) {
            pmon->respFSRead.more=-1;
            Readfileclosed=TRUE;
          } else {
            pmon->respFSRead.more=0;
            Readfileclosed=FALSE;
          }
          bytestogo=filesize;

         // messageDebug(DBG_INFO, __MODULE__, __LINE__, "bytestogo=%i", bytestogo); 

          pmon->respFSRead.length=bytestogo;
          return sizeof(resp_pmonFSRead);
          }

        // Read the data...
        case 1:
          reqnum=MINIMUM(sizeof(pmon->respFSRead.data), bytestogo);
          // If we could not read the corretct number of bytes, close file and report error
          actnum = f_read(pmon->respFSRead.data,1,reqnum,file);
       	  //if (reqnum!=f_read(pmon->respFSRead.data,1,reqnum,file)) {
          if(actnum != reqnum)
          {
            pmon->respFSRead.more=-1;
            Readfileclosed=TRUE;
   		    f_close(file);
            return sizeof(resp_pmonFSRead);
          }
          bytestogo -= reqnum;

          if (bytestogo==0) {
   		      f_close(file);
            Readfileclosed=TRUE;
            pmon->respFSRead.more=0;
          } else
            pmon->respFSRead.more=1;

          pmon->respFSRead.length=reqnum;
          return sizeof(resp_pmonFSRead);
      }
      pmon->respFSRead.more=-1;
      return sizeof(resp_pmonFSRead);

    case PMON_FS_WRITE:
  		switch (pmon->reqFSWrite.state) {
        // Create file
        case 0:
          if (Writefileclosed==FALSE)
            f_close(file);
          size=0;
          strcpy(name, (char*)pmon->reqFSWrite.data);
          file=f_open((char*)pmon->reqFSWrite.data,"w+");
          if (file) {
            Writefileclosed=FALSE;
            pmon->respFSWrite.rc=0;
          } else {
            Writefileclosed=TRUE;
            pmon->respFSWrite.rc=1;
          }
          return sizeof(resp_pmonFSWrite);

        // Fill it with data..
        case 1:
       		size=f_write(pmon->reqFSWrite.data,1,pmon->reqFSWrite.length,file);
          if (size==pmon->reqFSWrite.length)
            pmon->respFSWrite.rc=0;
          else {
   		      f_close(file);
            Writefileclosed=TRUE;
            pmon->respFSWrite.rc=1;
          }
            
          pmon->respFSWrite.length=size; // Indicate how many bytes we have written until now
          return sizeof(resp_pmonFSWrite);

        // Close the file
        case 2:
          if (Writefileclosed==FALSE)
		        rc=f_close(file);
          {
         //   unsigned char attr;
         //   f_getattr(name, &attr);
         //   f_setattr(name, attr | 0x20);
          }
          
          Writefileclosed=TRUE;
          if (rc==F_NO_ERROR)
            pmon->respFSWrite.rc=0;
          else
            pmon->respFSWrite.rc=1;
          pmon->respFSWrite.length=size; // Indicate how many bytes we wrote in total
          return sizeof(resp_pmonFSWrite);
      }
      pmon->respFSWrite.rc=1;
      return sizeof(resp_pmonFSWrite);

      
      case PMON_FS_SPACE:
      {
        F_SPACE space;
        pmon->respFSSpace.rc=0;

        pmon->reqFSSpace.drv &= ~0x20; // to uppercase
        pmon->reqFSSpace.drv -= 0x41; // 'A' is 0

        rc=f_getfreespace(&space);
        if (rc!=F_NO_ERROR) {
          pmon->respFSSpace.rc=2;
        }
        pmon->respFSSpace.total=space.total;
        pmon->respFSSpace.free=space.free;
        pmon->respFSSpace.used=space.total-space.free;
        pmon->respFSSpace.bad=0;

        return sizeof(resp_pmonFSSpace);
      }

    case PMON_RESET:
      pmon->respReset.rc=0;
//      setResetCause(REBOOT);
      rebootWDT(); // this will force a reset
     
      return sizeof(resp_pmonReset);

    default:
      return 0;
  }
}

//-----------------------------------------------------------------------------
// initPMON()
//-----------------------------------------------------------------------------
void initPMON(int devnum) 
{
    Blocked = 0;
    //  memset(cbGeneric,0,sizeof(cbGeneric));
    //  SysInfo=getSysInfo();
}

