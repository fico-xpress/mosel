/******************************************
  Mosel NI Examples
  =================
    
  File hadoop.c
  `````````````
  A basic IO driver to access HDFS filesystems

  (c) 2013 Fair Isaac Corporation
      author: Y. Colombani
*******************************************/
/*
setenv JAVA_HOME "path/to/java"
setenv HADOOP_HOME "path/to/hadoop"
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${JAVA_HOME}/lib/server:${HADOOP_HOME}/lib/native

gcc -fPIC -Wall -I${XPRESSDIR}/include -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux -I${HADOOP_HOME}/include -shared -D_REENTRANT hadoop.c -o hadoop.dso  -L${JAVA_HOME}/lib/server -L${HADOOP_HOME}/lib/native -lhdfs -ljvm
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <alloca.h>
#include <sys/stat.h>

#define XPRM_NICOMPAT 3004002	/* Compatibility level: Mosel 3.4.2 */
#include "xprm_ni.h"

#include "mmsystem.h"
#include "jni.h"
#include "hdfs.h"

#define INCSIZE 8192

typedef struct
	{
	 mmsystem_imci system;
	 struct Sysctx **sysctx;
	} s_hctx;

typedef struct
	{
	 hdfsFS fs;
	 hdfsFile f;
	} s_hdfs;

static int chkres(int);
static void *had_reset(XPRMcontext ctx,void *libctx);
static int hplchad(XPRMcontext ctx,void *libctx);
static int hplcmkdir(XPRMcontext ctx,void *libctx);
static void *fsdrv_open(XPRMcontext ctx,int *mode,const char *filename);
static int fsdrv_close(XPRMcontext ctx,s_hdfs *file,int mode);
static long fsdrv_read(XPRMcontext ctx,s_hdfs *file,char *buf,unsigned long sizeb);
static long fsdrv_write(XPRMcontext ctx,s_hdfs *file,void *buf,unsigned long sizeb);
static int fsdrv_rm(XPRMcontext ctx,const char *fname);
static char *read_openoptions(char *fname,char **host,int *port,char **user);

static XPRMdsofct tabfct[]=
	{
	 {"hadoop",1000,XPRM_TYP_NOT,1,"S",hplchad},
	 {"hdfsmkdir",1001,XPRM_TYP_INT,1,"S",hplcmkdir}
	};

static XPRMiofcttab iodrv_fs[]=
	{
	 {XPRM_IOCTRL_OPEN,(void *)fsdrv_open},
	 {XPRM_IOCTRL_CLOSE,(void *)fsdrv_close},
	 {XPRM_IOCTRL_READ,(void *)fsdrv_read},
	 {XPRM_IOCTRL_WRITE,(void *)fsdrv_write},
	 {XPRM_IOCTRL_INFO,"[//[user@]host[:port]/]path"},
	 {XPRM_IOCTRL_RM,(void *)fsdrv_rm},
	 {0,NULL}
	};

static XPRMiodrvtab iodrv_hdfs[]=
	{
	 {"hdfs",iodrv_fs},
	 {NULL,NULL}
	};

static const char *had_deplist[]={"mmsystem",NULL};
static XPRMdsoserv tabserv[]=
	{
	 {XPRM_SRV_RESET,(void *)had_reset},
	 {XPRM_SRV_DEPLST,(void *)had_deplist},
	 {XPRM_SRV_IODRVS,iodrv_hdfs},
	 {XPRM_SRV_CHKRES,(void*)chkres},
	};
static XPRMdsointer dsointer= 
	{ 
	 0,NULL,
	 sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         0,NULL,
	 sizeof(tabserv)/sizeof(XPRMdsoserv),tabserv
	};

static hdfsFS ConnectAsUser(const char *host,int port,const char *user);
static void setclspath(XPRMcontext ctx);
static int jvmrunning(void);
static int addtopath(const char *toadd,char **path,size_t *maxlen);

static XPRMnifct mm;             /* To store the mosel function table */
static int exec_disabled=0;
static int clspathset;

/************************************************/
/* Initialise the library just after loading it */
/************************************************/
DSO_INIT hadoop_init(XPRMnifct nifct,int *interver,int *libver,XPRMdsointer **interf)
{
 mm=nifct;
 *interver=XPRM_NIVERS;
 *libver=XPRM_MKVER(0,0,3);
 *interf=&dsointer;
 clspathset=jvmrunning()?1:0;
 return 0;
}

/**********************/
/* Check restrictions */
/**********************/
static int chkres(int r)
{
 exec_disabled=r&XPRM_RESTR_NOEXEC;
 return 0;
}

/*******************************/
/* Reset the library for a run */
/*******************************/
static void *had_reset(XPRMcontext ctx,void *libctx)
{
 s_hctx *hctx;

 if(libctx!=NULL)
 {
  free(libctx);
  return NULL;
 }
 else
 {
  if((hctx=malloc(sizeof(s_hctx)))==NULL)
  {
   mm->dispmsg(ctx,"HADOOP: Out of memory error.\n");
   return NULL;
  }
  else
  {
   memset(hctx,0,sizeof(s_hctx));
   hctx->sysctx=(struct Sysctx **)(mm->getdsoctx(ctx,mm->finddso("mmsystem"),
   						(void **)(&(hctx->system))));
   return hctx;
  }
 }
}

/****************************/
/* Execute a Hadoop command */
/****************************/
static int hplchad(XPRMcontext ctx,void *libctx)
{
 s_hctx *hctx;
 const char *cmd;
 const char *hadhome;
 char *hadcmd;

 hctx=libctx;
 cmd=XPRM_POP_STRING(ctx);

 if(exec_disabled)
 {
  mm->dispmsg(ctx,"HADOOP: Operation disabled by restrictions.\n");
  return RT_ERROR;
 }
 else
 {
  if(cmd==NULL) cmd="";
  hadhome=hctx->system->getenv(ctx,*hctx->sysctx,"HADOOP_HOME");
  if(hadhome==NULL)
  {
   hadcmd=alloca(strlen(cmd)+16);
   sprintf(hadcmd,"hadoop %s",cmd);
  }
  else
  {
   hadcmd=alloca(strlen(cmd)+strlen(hadhome)+16);
   sprintf(hadcmd,"%s/bin/hadoop %s",hadhome,cmd);
  }
  hctx->system->system(ctx,*hctx->sysctx,hadcmd);
  return RT_OK;
 }
}

/**********************/
/* Create a directory */
/**********************/
static int hplcmkdir(XPRMcontext ctx,void *libctx)
{
 const char *fname;
 char *name;
 char *host;
 int port;
 char *user;
 hdfsFS fs;

 setclspath(ctx);
 fname=XPRM_TOP_ST(ctx)->string;
 if(fname==NULL)
  XPRM_TOP_ST(ctx)->integer=-1;
 else
 {
  port=0;
  user=NULL;
  host="default";
  name=read_openoptions(strcpy(alloca(strlen(fname)+1),fname),&host,&port,&user);
  if(name==NULL)
  {
   mm->dispmsg(ctx,"HDFS URI must be of the form: [//[user@]host[:port]/]path\n");
   return RT_ERROR;
  }
  else
  {
   if((fs=ConnectAsUser(host,port,user))==NULL)
    XPRM_TOP_ST(ctx)->integer=-1;
   else
   {
    XPRM_TOP_ST(ctx)->integer=hdfsCreateDirectory(fs,name);
    hdfsDisconnect(fs);
   }
  }
 }
 return RT_OK;
}

		/**********************/
		/* IO driver for HDFS */
		/**********************/
/* Open file */
static void *fsdrv_open(XPRMcontext ctx,int *mode,const char *fname)
{
 char *name;
 char *host;
 int port;
 char *user;
 s_hdfs *file;
 int flags;

 setclspath(ctx);
 port=0;
 user=NULL;
 host="default";
 name=read_openoptions(strcpy(alloca(strlen(fname)+1),fname),&host,&port,&user);
 if(name==NULL)
 {
  mm->setioerrmsg(ctx,"HDFS URI must be of the form: [//[user@]host[:port]/]path\n",5);
  return NULL;
 }
 if((file=malloc(sizeof(s_hdfs)))==NULL)
 {
  mm->setioerrmsg(ctx,strerror(errno),5);
  return NULL;
 }
 if((file->fs=ConnectAsUser(host,port,user))==NULL)
 {
  mm->setioerrmsg(ctx,strerror(errno),5);
  free(file);
  return NULL;
 }
 else
 {
  if(((*mode)&(XPRM_F_READ|XPRM_F_WRITE))==XPRM_F_READ)
   flags=O_RDONLY;
  else
  {
   flags=O_CREAT|O_WRONLY;
   if((*mode)&XPRM_F_APPEND)
    flags|=O_APPEND;
   else
    flags|=O_TRUNC;
  }

  if((file->f=hdfsOpenFile(file->fs,name,flags,0,0,0))==NULL)
  {
   mm->setioerrmsg(ctx,strerror(errno),5);
   hdfsDisconnect(file->fs);
   free(file);
   return NULL;
  }
  else
   return file;
 }
}

/* Close file */
static int fsdrv_close(XPRMcontext ctx,s_hdfs *file,int mode)
{
 int rts;

 rts=hdfsCloseFile(file->fs,file->f)!=0;
 hdfsDisconnect(file->fs);
 free(file);
 if(rts)
  mm->setioerrmsg(ctx,strerror(errno),5);
 return rts;
}

/* Read a block */
static long fsdrv_read(XPRMcontext ctx,s_hdfs *file,char *buf,unsigned long sizeb)
{
 tSize ts;

 ts=hdfsRead(file->fs,file->f,buf,sizeb);
 if(ts<0)
  mm->setioerrmsg(ctx,strerror(errno),5);
 return (long)ts;
}

/* Write a block */
static long fsdrv_write(XPRMcontext ctx,s_hdfs *file,void *buf,unsigned long sizeb)
{
 if((hdfsWrite(file->fs,file->f,buf,sizeb)!=sizeb)||
    hdfsFlush(file->fs,file->f))
 {
  mm->setioerrmsg(ctx,strerror(errno),5);
  return 0;
 }
 else
  return sizeb;
}

/* Delete a file */
static int fsdrv_rm(XPRMcontext ctx,const char *fname)
{
 char *name;
 char *host;
 int port;
 char *user;
 hdfsFS fs;
 int rts;

 port=0;
 user=host=NULL;
 name=read_openoptions(strcpy(alloca(strlen(fname)+1),fname),&host,&port,&user);
 if((fs=ConnectAsUser(host,port,user))==NULL)
  return 4;
 else
 {
  rts=hdfsDelete(fs,name,0)?4:0;
  hdfsDisconnect(fs);
  return rts;
 }
}

/**************************************/
/* Parse options of connection string */
/**************************************/
static char *read_openoptions(char *fname,char **host,int *port,char **user)
{
 char *ps,*name;

 if((fname[0]!='/')||(fname[1]!='/'))
 {
  *host="localhost";
  *port=9000;
  return fname;
 }
 else
 {
  while(*fname=='/') fname++;
  if((name=strchr(fname,'/'))==NULL)
   return NULL;
  else
  {
   *name='\0';
   if((ps=strchr(fname,'@'))!=NULL)
   {
    *ps='\0';
    *user=fname;
    fname=ps+1;
   }
   if((ps=strchr(fname,':'))!=NULL)
   {
    *ps='\0';
    *port=strtol(ps+1,NULL,10);
   }
   else
    *port=9000;
   *host=fname;
   return (name+1);
  }
 }
}

/**************************/
/* Connect to a HDFS node */
/**************************/
static hdfsFS ConnectAsUser(const char *host,int port,const char *user)
{
 struct hdfsBuilder *bld;

 if((bld=hdfsNewBuilder())==NULL)
  return NULL;
 else
 {
  hdfsBuilderSetNameNode(bld,host);
  hdfsBuilderSetNameNodePort(bld,port);
  if(user!=NULL)
   hdfsBuilderSetUserName(bld,user);
  return hdfsBuilderConnect(bld);
 }
}

/********************************************/
/* Set CLASSPATH according to HADOOP_HOME */
/********************************************/
static void setclspath(XPRMcontext ctx)
{
 struct stat bstat;
 const char *hadpfx;
 const char *clspath;
 char *apath=NULL;
 size_t maxlen=0;
 char *libpath;
 int l;

 if(!clspathset)
 {
  hadpfx=getenv("HADOOP_HOME");
  clspath=getenv("CLASSPATH");
  if((hadpfx==NULL)&&(clspath==NULL))
  {
   mm->dispmsg(ctx,"HADOOP: HADOOP_HOME and CLASSPATH are not set!\n");
  }
  else
  if((hadpfx!=NULL)&&
     (!stat(hadpfx,&bstat)&&((bstat.st_mode&S_IFMT)==S_IFDIR)))
  {
   if(clspath!=NULL)
   {
    l=strlen(clspath);
    addtopath(clspath,&apath,&maxlen);
   }
   else
    l=0;
   libpath=alloca(strlen(hadpfx)+32);
   sprintf(libpath,"%s/share/hadoop/common/*",hadpfx);
   addtopath(libpath,&apath,&maxlen);
   sprintf(libpath,"%s/share/hadoop/common/lib/*",hadpfx);
   addtopath(libpath,&apath,&maxlen);
   sprintf(libpath,"%s/share/hadoop/hdfs/*",hadpfx);
   addtopath(libpath,&apath,&maxlen);
   sprintf(libpath,"%s/share/hadoop/hdfs/lib/*",hadpfx);
   addtopath(libpath,&apath,&maxlen);
   if(apath!=NULL)
   {
    if(strlen(apath)>l)
     setenv("CLASSPATH",apath,1);
    free(apath);
   }
   clspathset=1;
  }
  else
  if(hadpfx!=NULL)
   mm->dispmsg(ctx,"HADOOP: HADOOP_HOME is not a directory (%s)!\n",hadpfx);
 }
}

/******************************************/
/* Check whether a JVM is already running */
/******************************************/
static int jvmrunning(void)
{
 const jsize vmBufLength = 1;
 JavaVM* vmBuf[1]; 
 jint noVMs = 0;
 
 return (JNI_GetCreatedJavaVMs(&(vmBuf[0]), vmBufLength, &noVMs)==0)?noVMs:0;
}

/*************************************/
/* Append a directory to a path list */
/*************************************/
static int addtopath(const char *toadd,char **path,size_t *maxlen)
{
 size_t ol,l,newmaxlen;
 char *newpath;

 if((toadd!=NULL)&&((l=strlen(toadd))>0))
 {
  ol=(*path==NULL)?0:strlen(*path);
  if(ol+l+2>=*maxlen)
  {
   newmaxlen=*maxlen+INCSIZE;
   while(newmaxlen<=ol+l+2) newmaxlen+=INCSIZE;
   newpath=realloc(*path,newmaxlen);
   if(newpath==NULL)
   {
    mm->dispmsg(NULL,"HADOOP: Out of memory error.\n");
    return 1;
   }
   else
   {
    *maxlen=newmaxlen;
    *path=newpath;
   }
  }
  if(ol==0)
   memcpy(*path,toadd,l+1);
  else
  {
   (*path)[ol]=':';
   memcpy((*path)+ol+1,toadd,l+1);
  }
  return 0;
 }
 else
  return 0;
}
